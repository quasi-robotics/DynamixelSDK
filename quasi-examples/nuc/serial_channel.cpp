#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include "serial_channel.h"


using namespace quasi;

const size_t DEFAULT_DXL_BUF_LENGTH = 2048;
const uint8_t DEVICE_ID = 1;

SerialChannel::SerialChannel() : max_packet_size_(0), packet_(nullptr), stop_(false) {

}

bool SerialChannel::begin(const std::string& usb_port, int baud_rate) {
  if (port_) return true;
    port_ = std::make_shared<quasi::PortHandlerLinuxExt>(usb_port.c_str());

  if (!port_->openPort()) {
    std::cerr << "DynamixelSDKWrapper: Failed to open the port: " << usb_port << std::endl;
    return false;
  }

  if (!port_->setBaudRate(baud_rate)) {
    std::cerr << "DynamixelSDKWrapper: Failed to change the baudrate: " << baud_rate << std::endl;
    return false;
  }

  packet_ = dynamixel::PacketHandler::getPacketHandler();
  return true;
}

void SerialChannel::execute_subscriptions(uint8_t* data, uint16_t len) {
  for(auto sub : subscriptions_) {
    if (data[0] == sub->getID()) {
      sub->execute_callback(&data[1], len-1);
    }
  }
}

void SerialChannel::run_read() {
  Buffer buf(DEFAULT_DXL_BUF_LENGTH);
  uint8_t error;
  while (!stop_) {
    if (port_->waitForData(3.0)) {
      bool ret = false;
      {
        std::lock_guard<std::mutex> lock(serial_mutex_);
        ret = packet_->readRx(port_.get(), DEVICE_ID, max_packet_size_, &buf[0], &error) == COMM_SUCCESS;
      }
      if ( ret ) {
        //std::cout << "Got: " << (int)buf[0] << std::endl;
        // std::cout << "Got: " << std::fixed << std::setw(11) << std::setprecision(6) << *(float*)&buf[1] << std::endl;
        execute_subscriptions(&buf[0], max_packet_size_);
      }
    } else {
      //std::cerr << "timeout" << std::endl;
    }
  }
  //std::cout << "rt exit" << std::endl;
}

void SerialChannel::run_write() {
  DataHolder dh;
  while (!stop_) {
    if ( queue_.pop(dh, std::chrono::milliseconds(200)) && dh.data_) {
      std::lock_guard<std::mutex> lock(serial_mutex_);
      if (packet_->writeTxOnly(port_.get(), DEVICE_ID, 0, dh.len_, dh.data_) == COMM_SUCCESS) {
          //std::cout << "Sent: " << (int)dh.len_ << "type: " << (int)dh.data_[0] << std::endl;
      }
      dh.destroy();
    }
  }
  //std::cout << "wt exit" << std::endl;
}