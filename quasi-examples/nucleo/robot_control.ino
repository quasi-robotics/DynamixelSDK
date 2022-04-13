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
  DataProvider(SerialChannel& channel, uint16_t interval_ms = 20): 
    channel_(channel), thread_(nullptr), interval_(interval_ms) {}
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
      channel_.publish(Sensor::DATAID, sensor.getData());
      vTaskDelay(interval_ / portTICK_PERIOD_MS); 
    }
  }

  SerialChannel& channel_;
  thread* thread_;
  uint16_t interval_;
  Sensor sensor;
};


const uint8_t MELODY_DATAID=5;

const int BUZZER_PIN = 11;
static MelodyPlayer player(BUZZER_PIN);

static SerialChannel channel;
static DataProvider<ImuSensor> imu(channel, 1000);
static DataProvider<DistnaceSensor> dist(channel, 1000);

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
  player.begin();

  delay(100);
  player.play(MelodyPlayer::ON);
  vTaskStartScheduler();
}


void loop() {}