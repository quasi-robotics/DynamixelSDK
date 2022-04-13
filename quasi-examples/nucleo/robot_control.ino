#include <Dynamixel2Arduino.h>
#include "rtos_cpp.h"
#include "packet_handler.h"

#include "melody.h"
#include "sensors.h"
#include "serial_channel.h"

using namespace quasi;


template<typename Sensor>
class DataProvider {
public:
  DataProvider(SerialChannel& channel, uint8_t dataID, uint16_t interval_ms = 20): 
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
  uint8_t dataID_;
  thread* thread_;
  uint16_t interval_;
  Sensor sensor;
};


const uint8_t HB_DATAID=1;
const uint8_t IMU_DATAID = 3;
const uint8_t DIST_DATAID = 2;
const uint8_t MELODY_DATAID = 5;

const int BUZZER_PIN = 11;
static MelodyPlayer player(BUZZER_PIN);

static SerialChannel channel;
static DataProvider<ImuSensor> imu(channel, IMU_DATAID, 20);
static DataProvider<DistnaceSensor> dist(channel, DIST_DATAID, 20);

void setup() {
  DEBUG_begin();
  delay(1000);
  pinMode(LED_BUILTIN, OUTPUT);

  channel.begin();
  imu.begin();
  dist.begin();
  channel.subscribe<uint8_t>(MELODY_DATAID, [](const uint8_t& midx) {
    DEBUG_printf("Got %d\n", midx);
    player.play(midx);
  });

  channel.subscribe<uint32_t>(HB_DATAID, [](const uint32_t& beat) {
    DEBUG_printf("HeartBeat %d\n", beat);
  });

  player.begin();

  delay(100);
  player.play(MelodyPlayer::ON);
  vTaskStartScheduler();
}


void loop() {}