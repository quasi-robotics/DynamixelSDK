
#if defined(__linux__)

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

#include "port_handler_ext.h"

namespace {

template<typename Tag>
struct result {
  /* export it ... */
  typedef typename Tag::type type;
  static type ptr;
};

template<typename Tag>
typename result<Tag>::type result<Tag>::ptr;

template<typename Tag, typename Tag::type p>
struct rob : result<Tag> {
  /* fill it ... */
  struct filler {
    filler() { result<Tag>::ptr = p; }
  };
  static filler filler_obj;
};

template<typename Tag, typename Tag::type p>
typename rob<Tag, p>::filler rob<Tag, p>::filler_obj;


struct PHi { typedef int dynamixel::PortHandlerLinux::*type; };
template class rob<PHi, &dynamixel::PortHandlerLinux::socket_fd_>;
}

using namespace quasi;

bool PortHandlerLinux::waitForData(double timeout) {
  int socket_fd = this->*result<PHi>::ptr;
  if ( socket_fd == -1) return false;
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(socket_fd, &fds);
  struct timeval tp;// = { 10, 0 }; /* 10 seconds */
  timeout += 0.5e-9;
  tp.tv_sec = (long) timeout;
  tp.tv_usec = (timeout - tp.tv_sec) * 1000000;//1000000000L;
  //printf("%ld %ld\n", tp.tv_sec, tp.tv_usec);
  int ret = select(socket_fd+1, &fds, NULL, NULL, &tp);
  if (ret == -1) {
      printf("[PortHandlerLinux::waitForData] select failed!\n");
  }

/* ret == 0 means timeout, ret == 1 means descriptor is ready for reading,
   ret == -1 means error (check errno) */
   return ret == 1;
}
#endif
