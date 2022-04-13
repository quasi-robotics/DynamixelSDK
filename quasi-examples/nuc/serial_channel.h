#ifndef _SERIAL_CHANNEL_H_
#define _SERIAL_CHANNEL_H_
#include <vector>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>

#include <dynamixel_sdk/dynamixel_sdk.h>
#include "port_handler_ext.h"
#include "msg_queue.h"

namespace quasi {

using PortHandlerPtr = std::shared_ptr<quasi::PortHandlerLinuxExt>;
using PacketHandlerPtr = dynamixel::PacketHandler*;
using Buffer = std::vector<uint8_t>;

class SerialChannel;
class SubscriptionBase {
public:
  explicit SubscriptionBase(uint8_t dataID): data_id_(dataID) {}
  virtual ~SubscriptionBase() {}
  uint8_t getID() const { return data_id_; }
private:
  friend class SerialChannel;
  virtual void execute_callback(uint8_t* data, uint16_t len) = 0;
  uint8_t data_id_;
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

class DataHolder {
public:
  template<typename Data>
  explicit DataHolder(uint8_t dataID, const Data& data) : data_(nullptr), len_(sizeof(Data)+1) {
    data_ = new uint8_t[len_];
    data_[0] = dataID;
    std::memcpy(&data_[1], &data, sizeof(Data));
  }
  inline void destroy() { if(data_) { delete[] data_; } data_ = nullptr; len_ = 0;}
private:
  friend class SerialChannel;
  DataHolder() : data_(nullptr), len_(0) {}
  uint8_t* data_;
  uint16_t len_;
};

class SerialChannel {
public:
  SerialChannel();

  bool begin(const std::string& usb_port, int baud_rate);

  template<typename Data>
  void publish(uint8_t dataID, const Data& data) {
    DataHolder dh(dataID, data);
    queue_.push(dh);
  }

  template<typename Data, typename Callback = std::function<void(const Data&)>>
  void subscribe(uint8_t dataID, Callback&& callback) {
    subscriptions_.push_back(new Subscription<Data,Callback>(dataID, std::move(callback)));
    uint16_t psize = sizeof(Data) + 1;
    if (psize > max_packet_size_) max_packet_size_ = psize;
  }

  void run_read();
  void run_write();
  void stop() { stop_ = true; }

private:

  void execute_subscriptions(uint8_t* data, uint16_t len);

  std::vector<SubscriptionBase*> subscriptions_;
  uint16_t max_packet_size_;
  MsgQueue<DataHolder> queue_;
  PortHandlerPtr port_;
  PacketHandlerPtr packet_;
  std::atomic<bool> stop_;
  std::mutex serial_mutex_;

};

}

#endif //_SERIAL_CHANNEL_H_