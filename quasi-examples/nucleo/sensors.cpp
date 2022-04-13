#include "sensors.h"

using namespace quasi;

ImuSensor::ImuSensor() {

}
bool ImuSensor::init() { 
  return true; 
}

ImuData ImuSensor::getData() { 
  return ImuData{{100.1, 200.2, 300.3, 400.4}, {1.1, 2.2, 3.3}, {10.1, 20.2, 30.3} }; 
}


DistnaceSensor::DistnaceSensor() {

}
bool DistnaceSensor::init() { 
  dist_ = -100.0; return true; 
}

float DistnaceSensor::getData() { 
  dist_ += 1.1; return dist_; 
}
