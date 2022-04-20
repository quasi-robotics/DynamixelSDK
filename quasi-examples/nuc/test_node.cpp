#include <iostream>
#include <string>
#include <thread>

#include <dynamixel_sdk/dynamixel_sdk.h>
#include "serial_channel.h"

using namespace quasi;

struct Orientation {
  float w;
  float x;
  float y;
  float z;
};
std:: ostream& operator<< (std::ostream& os, const Orientation& o) {
  os << "(w:" << o.w << "x:" << o.x << ", y:" << o.y << ", z:" << o.z << ")";
  return os;
}

struct Vector3D {
  float x;
  float y;
  float z;
};
std:: ostream& operator<< (std::ostream& os, const Vector3D& v) {
  os << "(x:" << v.x << ", y:" << v.y << ", z:" << v.z << ")";
  return os;
}

struct ImuData {
  Orientation orientation;
  Vector3D linear_acceleration;
  Vector3D angular_velocity;
};
std:: ostream& operator<< (std::ostream& os, const ImuData& i) {
  os << "(IMU: orientation: " << i.orientation << std::endl << 
        "   linear_acceleration: " << i.linear_acceleration << std::endl << 
        "   angular_velocity:" << i.angular_velocity << ")";
  return os;
}

class HeartBeat {
public:
  HeartBeat(SerialChannel& channel, uint8_t dataID, uint16_t interval_ms = 200): 
    channel_(channel), dataID_(dataID), stop_(false), interval_(interval_ms), beat_(0) {}
  ~HeartBeat() {
    if (thread_) {
      stop_ = true;
      thread_->join();
    }
  }

  void begin() {
    if (thread_) return;
    beat_ = 0;
    thread_ = std::make_unique<std::thread>( [this]{ this->run(); });
  }

private:
  void run() {
    while(!stop_) {
      channel_.publish(dataID_, beat_++);
      std::this_thread::sleep_for(std::chrono::milliseconds(interval_));
    }
  }

  SerialChannel& channel_;
  uint8_t dataID_;
  std::unique_ptr<std::thread> thread_;
  std::atomic<bool> stop_;
  uint16_t interval_;
  uint32_t beat_;
};

const uint8_t HB_DATAID=1;
const uint8_t IMU_DATAID = 3;
const uint8_t DIST_DATAID = 2;
const uint8_t MELODY_DATAID=5;

int main() {

  SerialChannel channel;
  if (!channel.begin("/dev/ttyACM1", 1000000) ) {
    return 1;
  }

  channel.subscribe<float>(DIST_DATAID, [](const float& value) {
    std::cout << "Got dist: " << value << std::endl;
  });
  channel.subscribe<ImuData>(IMU_DATAID, [](const ImuData& value) {
    std::cout << value << std::endl;
  });
  HeartBeat hb(channel, HB_DATAID);
  hb.begin();

  int c;
  std::string input;
  while(1) {
    c = getchar();
    if (c == 'q')
      break;
    if (c == '1' || c == '2' || c =='3' || c == '0') {
      uint8_t midx = c - '0';
      std::cout << "aboout to send : " << (int)midx << std::endl;
      channel.publish(MELODY_DATAID, midx);
    }
    if (c == '9') {
      std::cout << "reconnect testing" << std::endl;
      channel.reconnect("/dev/ttyACM1");
    }
  }
  std::cout << "exit" << std::endl;

  return 0;
}
