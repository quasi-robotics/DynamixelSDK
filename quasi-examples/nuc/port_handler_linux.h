
#ifndef _PORTHANDLERLINUX_EXT_H_
#define _PORTHANDLERLINUX_EXT_H_


#include <dynamixel_sdk/port_handler_linux.h>


namespace quasi
{

class PortHandlerLinux : public dynamixel::PortHandlerLinux {
public: 
  PortHandlerLinux(const char *port_name): dynamixel::PortHandlerLinux(port_name) {}

  enum WaitReturn {
    WaitDataReady = 0,
    WaitTimeout,
    WaitError,
  };
  WaitReturn waitForData(int timeout = 500 /* ms */); 

};

}


#endif /* _PORTHANDLERLINUX_EXT_H_ */
