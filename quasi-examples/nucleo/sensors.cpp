#include "sensors.h"

using namespace quasi;

ImuSensor::ImuSensor() {

}
bool ImuSensor::init() { 
  return true; 
}

data::Imu ImuSensor::getData() { 
  return data::Imu{{100.1, 200.2, 300.3, 400.4}, {1.1, 2.2, 3.3}, {10.1, 20.2, 30.3} }; 
}


DistnaceSensor::DistnaceSensor() {

}
bool DistnaceSensor::init() { 
  range.range = -100.0; return true; 
}

data::Range DistnaceSensor::getData() { 
  range.range  += 1.1; 
  return range; 
}
