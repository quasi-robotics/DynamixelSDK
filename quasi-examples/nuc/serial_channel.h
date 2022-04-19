#ifndef _SERIAL_CHANNEL_H_
#define _SERIAL_CHANNEL_H_
#include <vector>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <iomanip>


#include <dynamixel_sdk/dynamixel_sdk.h>
#include "port_handler_linux.h"
#include "msg_queue.h"

namespace quasi {

using PortHandlerPtr = std::unique_ptr<quasi::PortHandlerLinux>;
using PacketHandlerPtr = dynamixel::PacketHandler*;

class SerialChannel;
class SubscriptionBase;
class DataHolder {
public:
  template<typename Data>
  DataHolder(uint8_t dataID, const Data& data) : data_(nullptr), len_(sizeof(Data)+1) {
    data_ = new uint8_t[len_];
    data_[0] = dataID;
    std::memcpy(&data_[1], &data, sizeof(Data));
  }
  DataHolder(const uint8_t* data, uint16_t len) : len_(len) {
    data_ = new uint8_t[len_];
    std::memcpy(data_, data, len_);
  }
  inline void destroy() { if(data_) { delete[] data_; } data_ = nullptr; len_ = 0;}
private:
  friend class SerialChannel;
  friend class SubscriptionBase;
  DataHolder() : data_(nullptr), len_(0) {}
  uint8_t* data_;
  uint16_t len_;
};

class SubscriptionBase {
public:
  explicit SubscriptionBase(uint8_t dataID);
  virtual ~SubscriptionBase();
  inline uint8_t getID() const { return data_id_; }
  inline bool push(const DataHolder&  dh) { return stop_ ? false : queue_.push(dh); }
private:
  void run();
  virtual void execute_callback(uint8_t* data, uint16_t len) = 0;
  uint8_t data_id_;
  std::unique_ptr<std::thread> callback_thread_;
  MsgQueue<DataHolder> queue_;
  std::atomic<bool> stop_;
};

template<typename Data, typename Callback = std::function<void(const Data&)>>
class Subscription : public SubscriptionBase {
public:
  Subscription(uint8_t dataID, Callback&& callback) : SubscriptionBase(dataID), callback_(std::move(callback)) {}
  virtual ~Subscription() {}
private:
  void execute_callback(uint8_t* data, uint16_t len) override {
    Data msg;
    std::memcpy(&msg, data, sizeof(Data));
    callback_(msg);
  }
  Callback callback_;  
};

class SerialChannel {
public:
  SerialChannel();
  ~SerialChannel();
  
  bool begin(const std::string& usb_port, int baud_rate);

  template<typename Data>
  void publish(uint8_t dataID, const Data& data) {
    if (stop_) return;
    DataHolder dh(dataID, data);
    queue_.push(dh);
  }

  template<typename Data, typename Callback = std::function<void(const Data&)>>
  bool subscribe(uint8_t dataID, Callback&& callback) {
    if (stop_) return false;
    subscriptions_.push_back(new Subscription<Data,Callback>(dataID, std::move(callback)));
    uint16_t psize = sizeof(Data) + 1;
    if (psize > max_packet_size_) max_packet_size_ = psize;
    return true;
  }

  void stop();

private:
  PortHandlerPtr create_port(const std::string& usb_port, int baud_rate);

  void run_read();
  void run_write();

  void execute_subscriptions(uint8_t* data, uint16_t len);

  std::vector<SubscriptionBase*> subscriptions_;
  uint16_t max_packet_size_;
  MsgQueue<DataHolder> queue_;
  PortHandlerPtr port_;
  PacketHandlerPtr packet_;
  std::atomic<bool> stop_;
  std::mutex serial_mutex_;
  std::unique_ptr<std::thread> read_thread_;
  std::unique_ptr<std::thread> write_thread_;

};

}

#endif //_SERIAL_CHANNEL_H_