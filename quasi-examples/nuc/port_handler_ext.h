
#ifndef _PORTHANDLERLINUX_EXT_H_
#define _PORTHANDLERLINUX_EXT_H_


#include <dynamixel_sdk/port_handler_linux.h>


namespace quasi
{

class PortHandlerLinux : public dynamixel::PortHandlerLinux {
public: 
  PortHandlerLinux(const char *port_name): dynamixel::PortHandlerLinux(port_name) {}

  bool    waitForData(double timeout = 1.0); 

};

}


#endif /* DYNAMIXEL_SDK_INCLUDE_DYNAMIXEL_SDK_LINUX_PORTHANDLERLINUX_H_ */
