#include <USBSerial.h>
#include <usbd_cdc_if.h>

#include "rtos_cpp.h"
#include "port_handler.h"
#include "logger.h"

static quasi::thread* _notify_thread;

static int8_t (* cdc_receive_callback)(uint8_t *Buf, uint32_t *Len);
static int8_t _on_receive_cdc(uint8_t *Buf, uint32_t *Len) {
  int8_t rc = cdc_receive_callback(Buf, Len);
  if (_notify_thread) {
    _notify_thread->give_ISR();
  }  
  return rc;
}

void quasi::init_read_notification(USBSerial&, thread& th) {
// Substitude Arduino's USB CDC Receive callback with ours
  _notify_thread = &th;
  HAL_NVIC_SetPriority(OTG_FS_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, UART_IRQ_SUBPRIO);
  cdc_receive_callback = USBD_CDC_fops.Receive;
  USBD_CDC_fops.Receive = _on_receive_cdc;
}

void quasi::wait_serial_event(USBSerial&, TickType_t waitTicks) {
  quasi::thread::take(waitTicks);
}

void quasi::init_read_notification(HardwareSerial&, thread&) {
}

void quasi::wait_serial_event(HardwareSerial&, TickType_t waitTicks) {
  vTaskDelay(3 / portTICK_PERIOD_MS); 
}