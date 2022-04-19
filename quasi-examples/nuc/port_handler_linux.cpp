
#if defined(__linux__)

#include <cstdio>
//#include <sys/select.h>
#include <sys/poll.h>

#include "port_handler_linux.h"

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

// bool PortHandlerLinux::waitForData(double timeout) {
//   int socket_fd = this->*result<PHi>::ptr;
//   if ( socket_fd == -1) return false;
//   fd_set fds;
//   FD_ZERO(&fds);
//   FD_SET(socket_fd, &fds);
//   struct timeval tp;// = { 10, 0 }; /* 10 seconds */
//   timeout += 0.5e-9;
//   tp.tv_sec = (long) timeout;
//   tp.tv_usec = (timeout - tp.tv_sec) * 1000000;//1000000000L;
//   tp.tv_sec = (long) 2;
//   tp.tv_usec = 0;
//   //printf("%ld %ld\n", tp.tv_sec, tp.tv_usec);
//   int ret = select(socket_fd+1, &fds, NULL, NULL, &tp);
//   if (ret == -1) {
//       printf("[PortHandlerLinux::waitForData] select failed!\n");
//   }
//   // if (ret == 0) {
//   //     printf("[PortHandlerLinux::waitForData] select timeout!\n");
//   // } else {
//   //     printf("[PortHandlerLinux::waitForData] select got some!\n");
//   // }
// /* ret == 0 means timeout, ret == 1 means descriptor is ready for reading,
//    ret == -1 means error (check errno) */
//    return ret == 1;
// }

PortHandlerLinux::WaitReturn PortHandlerLinux::waitForData(int timeout) {
  int socket_fd = this->*result<PHi>::ptr;
    //printf("[PortHandlerLinux::waitForData] %d!\n", socket_fd);
  if ( socket_fd == -1) return WaitError;
  struct pollfd fds[1];
  fds[0].fd = socket_fd;
  fds[0].events = POLLIN;
  int pollrc = poll( fds, 1, timeout);
  if (pollrc < 0) {
    printf("[PortHandlerLinux::waitForData] poll failed!\n");
    return WaitError;
  } else if (pollrc > 0) {
    if (fds[0].revents & POLLERR ||  fds[0].revents & POLLHUP) {
      printf("[PortHandlerLinux::waitForData] POLLERR = %d, POLLHUP = %d\n", fds[0].revents & POLLERR, fds[0].revents & POLLHUP);
      return WaitError;
    }
    if( fds[0].revents & POLLIN ) {
      //printf("[PortHandlerLinux::waitForData] select got some!\n");
      return WaitDataReady;
    }
  }
  //printf("[PortHandlerLinux::waitForData] select timeout!\n");
  return WaitTimeout;
}

#endif
