#ifndef _SENSORS_H
#define _SENSORS_H

#include <Arduino.h>

namespace quasi {

struct Orientation {
  double w;
  double x;
  double y;
  double z;
};

struct Vector3D {
  float x;
  float y;
  float z;
};

struct ImuData {
  Orientation orientation;
  Vector3D linear_acceleration;
  Vector3D angular_velocity;
};

class ImuSensor {
public:
  static const uint8_t DATAID=3;
  ImuSensor();
  bool init();

  ImuData getData();
};

class DistnaceSensor {
public:
  static const uint8_t DATAID=2;
  DistnaceSensor();
  bool init();

  float getData();
private:
  float dist_;
};


}
#endif //_SENSORS_H