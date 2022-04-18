#ifndef _PORT_HANDLER_H
#define _PORT_HANDLER_H

#include <Arduino.h>
#include "rtos_cpp.h"

namespace quasi {
class PortHandlerBase
{
  public:
    PortHandlerBase() : open_state_(false) {}
    virtual ~PortHandlerBase() {}

    virtual void begin() = 0;
    virtual int available() = 0;
    virtual int read() = 0;
    virtual size_t write(uint8_t *buf, size_t len) = 0;
    
    virtual void initReadNotification(thread&);
    virtual void waitSerialEvent(TickType_t waitTicks = portMAX_DELAY) = 0;

    inline bool getOpenState() const {return open_state_;}
    inline void setOpenState(bool flag) {open_state_ = flag; }

  private:
    bool open_state_;
};

void init_read_notification(USBSerial&, thread& th);
void init_read_notification(HardwareSerial&, thread& th);

void wait_serial_event(USBSerial&, TickType_t waitTicks = portMAX_DELAY);
void wait_serial_event(HardwareSerial&, TickType_t waitTicks = portMAX_DELAY);

template<typename Device>
class PortHandler : public PortHandlerBase {
public:
  PortHandler(Device& port): port_(port) {}

  void begin() override {
    setOpenState(true);
  }
  int available() override {
    return port_.available();
  }
  int read() override {
    return port_.read();
  }
  size_t write(uint8_t *buf, size_t len) override {
    return port_.write(buf, len);
  }

  void initReadNotification(thread& th) override {
    init_read_notification(port_, th);
  }
  void waitSerialEvent(TickType_t waitTicks = portMAX_DELAY) override {
    wait_serial_event(port_, waitTicks);
  }

private:
  Device& port_;
};

}
#endif //_PORT_HANDLER_H