#ifndef _SERIAL_CHANNEL_H_
#define _SERIAL_CHANNEL_H_
#include <vector>
#include <functional>
#include <cstring>
#include "rtos_cpp.h"
#include "packet_handler.h"
#include "port_handler.h"

namespace quasi {

class SerialChannel;
class SubscriptionBase;

class DataHolder {
public:
  template<typename Data>
  explicit DataHolder(uint16_t dataID, const Data& data) : data_(nullptr), len_(sizeof(Data)+sizeof(uint16_t)) {
    data_ = new uint8_t[len_];
    *(uint16_t*)data_ = dataID;
    std::memcpy(&data_[2], &data, sizeof(Data));
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
  explicit SubscriptionBase(uint16_t dataID);
  virtual ~SubscriptionBase() {}
  inline uint16_t getID() const { return data_id_; }
  inline void push(const DataHolder&  dh) { queue_.push(dh); };
private:
  void run();
  virtual void execute_callback(uint8_t* data, uint16_t len) = 0;
  uint16_t data_id_;
  thread* callback_thread_;
  MsgQueue<DataHolder> queue_;
};

template<typename Data, typename Callback = std::function<void(const Data&)>>
class Subscription : public SubscriptionBase {
public:
  Subscription(uint16_t dataID, Callback&& callback) : SubscriptionBase(dataID), callback_(std::move(callback)) {}
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

  template<typename Device>
  void begin(Device& port) {
    if (port_) return;
    port_ = new PortHandler<Device>(port);
    port_->begin();
    init_protocol_threads();
  }

  template<typename Data>
  void publish(uint16_t dataID, const Data& data) {
    DataHolder dh(dataID, data);
    queue_.push(dh);
  }

  template<typename Data, typename Callback = std::function<void(const Data&)>>
  void subscribe(uint16_t dataID, Callback&& callback) {
    subscriptions_.push_back(new Subscription<Data,Callback>(dataID, std::move(callback)));
  }

private:
  void init_protocol_threads();

  void run_read();
  void run_write();
  void execute_subscriptions(uint8_t* data, uint16_t len);

  std::vector<SubscriptionBase*> subscriptions_;
  MsgQueue<DataHolder> queue_;
  PortHandlerBase* port_;
  PacketHandler* packet_;
  thread* read_thread_;
  thread* write_thread_;


};

}

#endif //_SERIAL_CHANNEL_H_