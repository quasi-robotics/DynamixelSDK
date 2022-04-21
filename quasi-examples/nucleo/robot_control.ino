#include <Dynamixel2Arduino.h>
#include "rtos_cpp.h"
#include "packet_handler.h"

#include "melody.h"
#include "sensors.h"
#include "serial_channel.h"
#include "logger.h"
#include "pod.h"

using namespace quasi;


template<typename Sensor>
class DataProvider {
public:
  DataProvider(SerialChannel& channel, uint16_t dataID, uint16_t interval_ms = 20): 
    channel_(channel), dataID_(dataID), thread_(nullptr), interval_(interval_ms) {}
  ~DataProvider() {}

  void begin() {
    if (thread_) return;
    if (sensor.init()) {
      thread_ = new thread("DataProvider", [this]{ this->run(); });
    }
  }

private:
  void run() {
    while(true) {
      channel_.publish(dataID_, sensor.getData());
      vTaskDelay(interval_ / portTICK_PERIOD_MS); 
    }
  }

  SerialChannel& channel_;
  uint16_t dataID_;
  thread* thread_;
  uint16_t interval_;
  Sensor sensor;
};


const int BUZZER_PIN = PF14;
static MelodyPlayer player(BUZZER_PIN);

static SerialChannel channel;
static DataProvider<ImuSensor> imu(channel, IMU_DATAID, 20);
static DataProvider<DistnaceSensor> dist(channel, DIST_DATAID, 20);

void setup() {
  SerialUSB.begin(1000000);
  DEBUG_begin();
  delay(1000);
  pinMode(LED_BUILTIN, OUTPUT);
  DEBUG_println("Starting");

  channel.begin(SerialUSB);
  imu.begin();
  dist.begin();
  channel.subscribe<uint8_t>(SOUND_DATAID, [](const uint8_t& midx) {
    DEBUG_printf("Got %d\n", midx);
    player.play(midx);
    DEBUG_printf("melody  thread %ld\n", this_thread::get_id());

  });

  channel.subscribe<uint32_t>(HB_DATAID, [](const uint32_t& beat) {
    // DEBUG_printf("HeartBeat %d\n", beat);
    // DEBUG_printf("HB thread %ld\n", this_thread::get_id());

  });

  channel.subscribe<data::Velocity>(CMDVEL_DATAID, [](const data::Velocity& vel) {
    DEBUG_printf("HB thread %ld, vel.linear: (", this_thread::get_id());
    DEBUG_print(vel.linear.x); DEBUG_print(", ");
    DEBUG_print(vel.linear.y); DEBUG_print(", ");
    DEBUG_print(vel.linear.z); DEBUG_print(") vel.angular: (");
    DEBUG_print(vel.angular.x); DEBUG_print(", ");
    DEBUG_print(vel.angular.y); DEBUG_print(", ");
    DEBUG_print(vel.angular.z); DEBUG_println(")");

  });
  channel.subscribe<data::Melody>(MELODY_DATAID, [](const data::Melody& mel) {
    player.play(mel);
  });

  player.begin();

  delay(2000);
  player.play(MelodyPlayer::ON);
  DEBUG_printf("setup thread %ld\n", this_thread::get_id());
  vTaskStartScheduler();
}


void loop() {}