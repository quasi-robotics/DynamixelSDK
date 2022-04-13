#ifndef _MELODY_H
#define _MELODY_H

#include <vector>
#include <rtos_cpp.h>

namespace quasi {

enum Frequency {
  C4 = 262,
  D4 = 294,
  E4 = 330,
  F4 = 349,
  G4 = 392,
  A4 = 440,
  B4 = 494,
  C5 = 523
};

struct Note {
  uint16_t frequency;
  uint8_t duration;
};

using Melody = std::vector<Note>;
using Melodies = std::vector<Melody>;

class MelodyPlayer {
public:
  enum {
    OFF = 0, ON, LOW_BATTERY, ERROR
  };

  MelodyPlayer(int buzzer_pin);
  void begin();

  void play(int melody_idx);

private:
  void run();

  int pin_;
  MsgQueue<Note> queue_;
  Melodies melodies_;
  thread* thread_;
};
} // namespace quasi

#endif