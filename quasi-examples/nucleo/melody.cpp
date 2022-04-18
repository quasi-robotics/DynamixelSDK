#include <Arduino.h>
#include "melody.h"

using namespace quasi;


MelodyPlayer::MelodyPlayer(int buzzer_pin) : pin_(buzzer_pin), queue_(20), thread_(nullptr) {
  melodies_ = { 
    { {C4,4}, {D4,4}, {E4,4}, {F4,4}, {G4,4}, {A4,4}, {B4,4}, {C5,4} },
    { {C5,4}, {B4,4}, {A4,4}, {G4,4}, {F4,4}, {E4,4}, {D4,4}, {C4,4} },
    { {1000, 1}, {1000, 1}, {1000, 1}, {1000, 1}, {0,8}, {0,8}, {0,8}, {0,8} },
    { {1000, 3}, {500, 3}, {1000, 3}, {500, 3}, {1000, 3}, {500, 3}, {1000, 3}, {500, 3} }
  };
}

void MelodyPlayer::begin() {
  if (thread_) return;
  pinMode(pin_, OUTPUT);
  thread_ = new thread("Melody", [this]{ this->run(); });
}

void MelodyPlayer::play(int melody_idx) {
  if (!thread_) return;
  if (melody_idx <0 || melody_idx >= melodies_.size() ) return;
  queue_.reset();
  for( int i = 0; i < melodies_[melody_idx].size(); i++) {
    queue_.push(melodies_[melody_idx][i]);
  }
}

void MelodyPlayer::run() {
  Note note;
  while(1) {
    if (queue_.pop(note, 500/portTICK_PERIOD_MS)) {
      uint32_t duration = 1000/note.duration;
      // to distinguish the notes, set a minimum time between them.
      // the note's duration + 30% seems to work well:
      //duration = duration * 1.30;    
      tone(pin_, note.frequency, duration);
      vTaskDelay( duration / portTICK_PERIOD_MS); 
      noTone(pin_);
    // } else {
    //   DEBUG_SERIAL.println("melody - no notes");
    }
  }
}
