#include <iostream>
#include <string>
#include <thread>

#include <dynamixel_sdk/dynamixel_sdk.h>
#include "port_handler_ext.h"
#include "serial_channel.h"


struct Orientation {
  double w;
  double x;
  double y;
  double z;
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

const uint8_t IMU_DATAID = 3;
const uint8_t DIST_DATAID = 2;
const uint8_t MELODY_DATAID=5;

using namespace quasi;

int main() {

  SerialChannel channel;
  if (!channel.begin("/dev/ttyACM0", 1000000) ) {
    return 1;
  }

  channel.subscribe<float>(DIST_DATAID, [](const float& value) {
    std::cout << "Got dist: " << value << std::endl;
  });
  channel.subscribe<ImuData>(IMU_DATAID, [](const ImuData& value) {
    std::cout << value << std::endl;
  });

  std::thread rt([&channel](){channel.run_read();});
  std::thread wt([&channel](){channel.run_write();});
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
  }
  std::cout << "exit" << std::endl;

  channel.stop();
  rt.join();
  wt.join();
  return 0;
}
