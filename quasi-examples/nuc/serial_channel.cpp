#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include "serial_channel.h"

using namespace quasi;
using Buffer = std::vector<uint8_t>;

const size_t DEFAULT_DXL_BUF_LENGTH = 2048;
const uint8_t DEVICE_ID = 1;

SubscriptionBase::SubscriptionBase(uint8_t dataID): data_id_(dataID), stop_(false) {
  callback_thread_ = std::make_unique<std::thread>([this]{ this->run(); });
}

SubscriptionBase::~SubscriptionBase() {
  if (callback_thread_) {
    stop_ = true;
    callback_thread_->join();
  }
}

void SubscriptionBase::run() {
  DataHolder dh;
  while(!stop_) {
    if (queue_.pop(dh, std::chrono::milliseconds(200)) && dh.data_) {
      execute_callback(dh.data_, dh.len_);
      dh.destroy();
    }
  }
}


SerialChannel::SerialChannel() : max_packet_size_(0), packet_(nullptr), stop_(true), baudrate_(1000000) {
}

SerialChannel::~SerialChannel() {
  stop();
}

bool SerialChannel::begin(const std::string& usb_port, int baudrate) {
  if (port_) return true;
  packet_ = dynamixel::PacketHandler::getPacketHandler();
  if (!reconnect(usb_port, baudrate)) return false;

  stop_ = false;
  read_thread_ = std::make_unique<std::thread>([this] {this->run_read(); });
  write_thread_ = std::make_unique<std::thread>([this] {this->run_write(); });
  return true;
}

bool SerialChannel::reconnect(const std::string& usb_port, int baudrate) {
  if (!packet_) return false; // do nothing if begin has not been called yet
  auto port = std::make_shared<quasi::PortHandlerLinux>(usb_port.c_str());

  if (!port->openPort()) {
    std::cerr << "Failed to open the port: " << usb_port << std::endl;
    return false;
  }

  if (!port->setBaudRate(baudrate)) {
    std::cerr << "Failed to change the baudrate: " << baudrate << std::endl;
    return false;
  }
  usb_port_ = usb_port;
  baudrate_ = baudrate;
  // port_.reset();
  // std::this_thread::sleep_for(std::chrono::seconds(3));
  port_.swap(port);
  return true;
}

void SerialChannel::stop() {
  for(auto sub : subscriptions_) {
    delete sub;
  }
  stop_ = true;
  if (read_thread_) {
    read_thread_->join();
    read_thread_.release();
  }
  if (write_thread_) {
    write_thread_->join();
    write_thread_.release();
  }
  port_.reset();
}
void SerialChannel::execute_subscriptions(uint8_t* data, uint16_t len) {
  for(auto sub : subscriptions_) {
    if (data[0] == sub->getID()) {
      DataHolder dh(&data[1], len-1);
//      std::cout << "dh: " << int(data[0]) << ", len: " << len-1 << std::endl;
      sub->push(dh);
    }
  }
}

void SerialChannel::run_read() {
  Buffer buf(DEFAULT_DXL_BUF_LENGTH);
  uint8_t error;
  while (!stop_) {
    if (port_) {
      PortHandlerPtr port(port_); // make a copy for thread-safety
      PortHandlerLinux::WaitReturn ret = port->waitForData(1000 /* ms */);
      switch (ret) {
        case PortHandlerLinux::WaitDataReady:
          if (packet_->readRx(port.get(), DEVICE_ID, max_packet_size_, &buf[0], &error) == COMM_SUCCESS) {
            execute_subscriptions(&buf[0], max_packet_size_);
          }
          break;
        case PortHandlerLinux::WaitError:
          //exit(100);
          port_.reset();
          break;
        case PortHandlerLinux::WaitTimeout:
          std::cerr << "timeout" << std::endl;
          break;
      }
    } else {
      // try to reconnect
      if (!reconnect(usb_port_, baudrate_)) { 
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
    }
  }
  //std::cout << "rt exit" << std::endl;
}

void SerialChannel::run_write() {
  DataHolder dh;
  while (!stop_) {
    if ( queue_.pop(dh, std::chrono::milliseconds(200)) && dh.data_) {
      PortHandlerPtr port(port_); // make a copy for thread-safety
      if (port && packet_->writeTxOnly(port_.get(), DEVICE_ID, 0, dh.len_, dh.data_) == COMM_SUCCESS) {
          //std::cout << "Sent: " << (int)dh.len_ << "type: " << (int)dh.data_[0] << std::endl;
      }
      dh.destroy();
    }
  }
  //std::cout << "wt exit" << std::endl;
}