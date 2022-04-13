
#ifndef _PORTHANDLERLINUX_EXT_H_
#define _PORTHANDLERLINUX_EXT_H_


#include  <dynamixel_sdk/port_handler.h>
#include <mutex>

namespace quasi
{

////////////////////////////////////////////////////////////////////////////////
/// @brief The class for control port in Linux
////////////////////////////////////////////////////////////////////////////////
class PortHandlerLinuxExt : public dynamixel::PortHandler
{
 private:
  int     socket_fd_;
  int     baudrate_;
  char    port_name_[100];

  double  packet_start_time_;
  double  packet_timeout_;
  double  tx_time_per_byte;
  std::mutex mutex_;

  bool    setupPort(const int cflag_baud);
  bool    setCustomBaudrate(int speed);
  int     getCFlagBaud(const int baudrate);

  double  getCurrentTime();
  double  getTimeSinceStart();

 public:
  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that initializes instance of PortHandler and gets port_name
  /// @description The function initializes instance of PortHandler and gets port_name.
  ////////////////////////////////////////////////////////////////////////////////
  PortHandlerLinuxExt(const char *port_name);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that closes the port
  /// @description The function calls PortHandlerLinux::closePort() to close the port.
  ////////////////////////////////////////////////////////////////////////////////
  virtual ~PortHandlerLinuxExt() { closePort(); }

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that opens the port
  /// @description The function calls PortHandlerLinux::setBaudRate() to open the port.
  /// @return communication results which come from PortHandlerLinux::setBaudRate()
  ////////////////////////////////////////////////////////////////////////////////
  bool    openPort();

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that closes the port
  /// @description The function closes the port.
  ////////////////////////////////////////////////////////////////////////////////
  void    closePort();

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that clears the port
  /// @description The function clears the port.
  ////////////////////////////////////////////////////////////////////////////////
  void    clearPort();

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that sets port name into the port handler
  /// @description The function sets port name into the port handler.
  /// @param port_name Port name
  ////////////////////////////////////////////////////////////////////////////////
  void    setPortName(const char *port_name);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that returns port name set into the port handler
  /// @description The function returns current port name set into the port handler.
  /// @return Port name
  ////////////////////////////////////////////////////////////////////////////////
  char   *getPortName();

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that sets baudrate into the port handler
  /// @description The function sets baudrate into the port handler.
  /// @param baudrate Baudrate
  /// @return false
  /// @return   when error was occurred during port opening
  /// @return or true
  ////////////////////////////////////////////////////////////////////////////////
  bool    setBaudRate(const int baudrate);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that returns current baudrate set into the port handler
  /// @description The function returns current baudrate set into the port handler.
  /// @return Baudrate
  ////////////////////////////////////////////////////////////////////////////////
  int     getBaudRate();

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that checks how much bytes are able to be read from the port buffer
  /// @description The function checks how much bytes are able to be read from the port buffer
  /// @description and returns the number.
  /// @return Length of read-able bytes in the port buffer
  ////////////////////////////////////////////////////////////////////////////////
  int     getBytesAvailable();

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that reads bytes from the port buffer
  /// @description The function gets bytes from the port buffer,
  /// @description and returns a number of bytes read.
  /// @param packet Buffer for the packet received
  /// @param length Length of the buffer for read
  /// @return -1
  /// @return   when error was occurred
  /// @return or Length of bytes read
  ////////////////////////////////////////////////////////////////////////////////
  int     readPort(uint8_t *packet, int length);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that writes bytes on the port buffer
  /// @description The function writes bytes on the port buffer,
  /// @description and returns a number of bytes which are successfully written.
  /// @param packet Buffer which would be written on the port buffer
  /// @param length Length of the buffer for write
  /// @return -1
  /// @return   when error was occurred
  /// @return or Length of bytes written
  ////////////////////////////////////////////////////////////////////////////////
  int     writePort(uint8_t *packet, int length);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that sets and starts stopwatch for watching packet timeout
  /// @description The function sets the stopwatch by getting current time and the time of packet timeout with packet_length.
  /// @param packet_length Length of the packet expected to be received
  ////////////////////////////////////////////////////////////////////////////////
  void    setPacketTimeout(uint16_t packet_length);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that sets and starts stopwatch for watching packet timeout
  /// @description The function sets the stopwatch by getting current time and the time of packet timeout with msec.
  /// @param packet_length Length of the packet expected to be received
  ////////////////////////////////////////////////////////////////////////////////
  void    setPacketTimeout(double msec);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that checks whether packet timeout is occurred
  /// @description The function checks whether current time is passed by the time of packet timeout from the time set by PortHandlerLinux::setPacketTimeout().
  ////////////////////////////////////////////////////////////////////////////////
  bool    isPacketTimeout();

  bool    waitForData(double timeout = 1.0); 
};

}


#endif /* DYNAMIXEL_SDK_INCLUDE_DYNAMIXEL_SDK_LINUX_PORTHANDLERLINUX_H_ */
