#ifndef _SENSORS_H
#define _SENSORS_H

#include <Arduino.h>
#include "pod.h"
namespace quasi {


class ImuSensor {
public:
  ImuSensor();
  bool init();

  data::Imu getData();
};

class DistnaceSensor {
public:
  DistnaceSensor();
  bool init();

  data::Range getData();
private:
  data::Range range;
};


}
#endif //_SENSORS_H