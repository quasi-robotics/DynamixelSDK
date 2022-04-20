#ifndef _MELODY_H
#define _MELODY_H

#include <vector>
#include <rtos_cpp.h>
#include "pod.h"

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


using Melody = std::vector<data::Note>;
using Melodies = std::vector<Melody>;

class MelodyPlayer {
public:
  enum {
    OFF = 0, ON, LOW_BATTERY, ERROR
  };

  MelodyPlayer(int buzzer_pin);
  void begin();

  void play(int melody_idx);
  void play(const data::Note& note);
  void play(const Melody& melody);
  void play(const data::Melody& melody);

private:
  void run();

  int pin_;
  MsgQueue<data::Note> queue_;
  Melodies melodies_;
  thread* thread_;
};
} // namespace quasi

#endif