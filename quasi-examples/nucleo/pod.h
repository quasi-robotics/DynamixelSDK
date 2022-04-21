#ifndef QUASI_R2_POD_H
#define QUASI_R2_POD_H

#include <memory>

namespace quasi {
  // to nuc
  const uint16_t DIST_DATAID = 2;
  const uint16_t IMU_DATAID = 3;

  // to nucleo
  const uint16_t HB_DATAID=1;
  const uint16_t MELODY_DATAID = 2;
  const uint16_t SOUND_DATAID = 5;
  const uint16_t CMDVEL_DATAID = 6;

namespace data {
  struct Vector3 {
    float x, y, z;
  };
  struct Orientation {
    float w, x, y, z;
  };
  struct Imu {
    Orientation orientation;
    Vector3 linear_acceleration;
    Vector3 angular_velocity;
  };
  struct Velocity {
    Vector3 linear;
    Vector3 angular;
  };
  struct Note {
    uint16_t frequency;
    uint16_t duration;
  };
  struct Melody {
    Note notes[20];
    uint16_t size;
  };
  struct Range {
    float range;
  };

}  
} // namespace quasi


#endif // QUASI_R2_POD_H