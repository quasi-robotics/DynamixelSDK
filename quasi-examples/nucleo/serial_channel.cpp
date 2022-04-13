#include "serial_channel.h"
#include <vector>

using namespace quasi;

using Buffer = std::vector<uint8_t>;

SubscriptionBase::SubscriptionBase(uint8_t dataID): data_id_(dataID), queue_(10) {
    callback_thread_ = new thread("subscriper", [this]{ this->run(); });
}

void SubscriptionBase::run() {
  DataHolder dh;
  while(1) {
    if (queue_.pop(dh, 200/portTICK_PERIOD_MS) && dh.data_) {
      execute_callback(dh.data_, dh.len_);
      dh.destroy();
    }
  }
}


SerialChannel::SerialChannel() : max_packet_size_(0), queue_(10), port_(nullptr), packet_(nullptr), read_thread_(nullptr), write_thread_(nullptr) {

}

void SerialChannel::begin() {
  if (port_) return;
  port_ = new DYNAMIXEL::USBSerialPortHandler(Serial);
  packet_ = new PacketHandler(*port_);
  port_->begin();
  packet_->setID(1);
  read_thread_ = new thread("SerialRead", [this]{ this->run_read(); });
  write_thread_ = new thread("SerialWrite", [this]{ this->run_write(); });
}

void SerialChannel::execute_subscriptions(uint8_t* data, uint16_t len) {
  for(auto sub : subscriptions_) {
    if (data[0] == sub->getID()) {
      DataHolder dh(&data[1], len-1);
      sub->push(dh);
    }
  }
}

void SerialChannel::run_read() {
  Buffer buf(DEFAULT_DXL_BUF_LENGTH);
  while(true) {
    bool ret = true;
    {
      //lock_guard<mutex> lock(serial_mutex_);
      ret = packet_->rxWritePacket(&buf[0], max_packet_size_) == DXL_LIB_OK;
    }
    if (ret) {
      DEBUG_printf("Got type: %d\n", buf[0]);
      execute_subscriptions(&buf[0], max_packet_size_);
    }
    vTaskDelay(5 / portTICK_PERIOD_MS); 
  }
}

void SerialChannel::run_write() {
  DataHolder dh;
  while(true) {
    //DEBUG_printf("queue size: %d\n", queue_.waiting());
    if ( queue_.pop(dh, 200/portTICK_PERIOD_MS) && dh.data_) {
      //DEBUG_printf("dh.len_: %d\n", dh.len_);
      //lock_guard<mutex> lock(serial_mutex_);
      DXLLibErrorCode_t err = packet_->txStatusPacket(dh.data_, dh.len_);
      if (err != DXL_LIB_OK) {
        DEBUG_printf("txStatus err: %d\n", err);
      }
      dh.destroy();
    }
      //vTaskDelay(200 / portTICK_PERIOD_MS); 
  }
}