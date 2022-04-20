#include "serial_channel.h"
#include <vector>
#include "logger.h"

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


void SerialChannel::init_protocol_threads() {
  packet_ = new PacketHandler(*port_);
  packet_->setID(1);
  read_thread_ = new thread("SerialRead", [this]{ this->run_read(); });
  write_thread_ = new thread("SerialWrite", [this]{ this->run_write(); });
  port_->initReadNotification(*read_thread_);
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
    port_->waitSerialEvent();
    DXLLibErrorCode_t err = packet_->rxWritePacket(&buf[0], max_packet_size_);
    if (err == DXL_LIB_OK) {
      //DEBUG_printf("Got type: %d\n", buf[0]);
      execute_subscriptions(&buf[0], max_packet_size_);
    } else {
      DEBUG_printf("rxWritePacket err: %d\n", err);
    }
  }
}

void SerialChannel::run_write() {
  DataHolder dh;
  while(true) {
    //DEBUG_printf("queue size: %d\n", queue_.waiting());
    if ( queue_.pop(dh, 200/portTICK_PERIOD_MS) && dh.data_) {
      //DEBUG_printf("dh.len_: %d\n", dh.len_);
      DXLLibErrorCode_t err = packet_->txStatusPacket(dh.data_, dh.len_);
      if (err != DXL_LIB_OK) {
        DEBUG_printf("txStatus err: %d\n", err);
      }
      dh.destroy();
    }
  }
}