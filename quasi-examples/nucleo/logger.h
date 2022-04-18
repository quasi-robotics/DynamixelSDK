#ifndef __QUASI_LOGGER_H__
#define __QUASI_LOGGER_H__

#include <Print.h>
#include "rtos_cpp.h"
#include <map>

namespace quasi {

struct LogMsg {
  LogMsg(uint16_t cap) : data_(nullptr), cap_(cap), len_(0) {}
  void destroy() { if (data_) delete[] data_; data_ = nullptr; cap_ = 0; len_ = 0;}
  void release() { data_ = nullptr; len_ = 0;}

  size_t write(uint8_t b) {
    if (!data_) { 
      data_ = new uint8_t[cap_];
      len_ = 0;
    }
    if (len_ == cap_) return 0;
    data_[len_++] = b;
    return 1;
  }
  size_t write(const uint8_t *buffer, size_t size) {
    if (!data_) { 
      data_ = new uint8_t[cap_];
      len_ = 0;
    }
    size_t n = 0;
    while (cap_ > len_ && size--) {
      data_[len_++] = buffer[n++];
    }
    return n;
  }

  uint8_t* data_;
  uint16_t cap_;
  uint16_t len_;
};

template<class Sink> 
class Logger : public Print {
public:
  Logger(Sink& sink) : sink_(sink), sink_thread_(nullptr), queue_(20) {}

  void begin() {
    if(sink_thread_) return;
    sink_thread_ = new thread("Logger", [this]{ this->run(); });
  }
  
  size_t write(uint8_t b) override {
    LogMsg* msg = getMsg();
    return msg->write(b);
  }
  size_t write(const uint8_t *buffer, size_t size) override {
    LogMsg* msg = getMsg();
    return msg->write(buffer, size);
  }
  void flash() {
    LogMsg* msg = getMsg();
    //DEBUG_SERIAL.printf("flash %d\n", msg->len_);
    queue_.push(*msg);
    msg->release();
  }
private:
  void run() {
    while(true) {
      LogMsg msg(0);
      if (queue_.pop(msg, 500/portTICK_PERIOD_MS)) {
        //DEBUG_SERIAL.printf("queue size: %d , %d\n", queue_.waiting(), queue_.available());
        sink_.write(msg.data_, msg.len_);
        msg.destroy();
      } else {
        //DEBUG_SERIAL.println("no msgs");
      }
    }
  }
  LogMsg* getMsg() {
    lock_guard<mutex> lock(logs_mutex_);
    uint32_t tid = this_thread::get_id();
    auto i = log_msgs_.find(tid);
    if (i != log_msgs_.end())
      return i->second;
      //DEBUG_SERIAL.println("new msg");
    LogMsg* msg = new LogMsg(1024);
    log_msgs_[tid] = msg;
    return msg;
  }
  Sink& sink_;
  thread* sink_thread_;
  MsgQueue<LogMsg> queue_;
  mutex logs_mutex_;
  std::map<uint32_t, LogMsg*> log_msgs_;
};

}

#define DEBUG_ENABLE 1

#if DEBUG_ENABLE
  #define DEBUG_SERIAL Serial
  // static quasi::Logger<USBSerial> __logger__(SerialUSB);
  // #define DEBUG_begin() { __logger__.begin(); }
  // #define DEBUG_printf(...) { __logger__.printf(__VA_ARGS__); __logger__.flash(); }
  // #define DEBUG_println(x) { __logger__.println(x); __logger__.flash(); }
  // #define DEBUG_print(x) { __logger__.print(x); __logger__.flash(); }
  // #define DEBUG_printf(...) { long s = micros(); __logger__.printf(__VA_ARGS__); __logger__.printf("Timer: %ld\n",micros()-s); __logger__.flash(); }
  // #define DEBUG_println(x) { long s = micros();__logger__.println(x); __logger__.printf("Timer: %ld\n",micros()-s); __logger__.flash(); }
  // #define DEBUG_print(x) { long s = micros(); __logger__.print(x); __logger__.printf("Timer: %ld\n",micros()-s); __logger__.flash(); }
  static quasi::mutex usb_mutex;
  #define DEBUG_begin() { DEBUG_SERIAL.begin(1000000); }
  #define DEBUG_printf(...) { quasi::lock_guard<quasi::mutex> lock(usb_mutex); DEBUG_SERIAL.printf(__VA_ARGS__); }
  #define DEBUG_println(x) { quasi::lock_guard<quasi::mutex> lock(usb_mutex); DEBUG_SERIAL.println(x); }
  #define DEBUG_print(x) { quasi::lock_guard<quasi::mutex> lock(usb_mutex); DEBUG_SERIAL.print(x);  }
  // #define DEBUG_printf(...) { long s = micros(); quasi::lock_guard<quasi::mutex> lock(usb_mutex); DEBUG_SERIAL.printf(__VA_ARGS__); DEBUG_SERIAL.printf("Timer: %ld\n",micros()-s); }
  // #define DEBUG_println(x) { long s = micros(); quasi::lock_guard<quasi::mutex> lock(usb_mutex); DEBUG_SERIAL.println(x); DEBUG_SERIAL.printf("Timer: %ld\n",micros()-s);}
  // #define DEBUG_print(x) { long s = micros(); quasi::lock_guard<quasi::mutex> lock(usb_mutex); DEBUG_SERIAL.print(x); DEBUG_SERIAL.printf("Timer: %ld\n",micros()-s); }

#else
  #define DEBUG_begin()
  #define DEBUG_printf(...) 
  #define DEBUG_println(x) 
  #define DEBUG_print(x) 
#endif

#endif //__QUASI_LOGGER_H__