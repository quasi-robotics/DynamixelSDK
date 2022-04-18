#include <dynamixel_sdk/port_handler_linux.h>
#include <iostream>

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

struct A {
private:
  void f() {
    std::cout << "proof!" << std::endl;
  }
  int i = 10;
};

struct Ai { typedef int A::*type; };
template class rob<Ai, &A::i>;

struct Af { typedef void(A::*type)(); };
template class rob<Af, &A::f>;

struct PHi { typedef int dynamixel::PortHandlerLinux::*type; };
template class rob<PHi, &dynamixel::PortHandlerLinux::socket_fd_>;
}

namespace quasi {
class PortHandlerLinux : public dynamixel::PortHandlerLinux {
public: 
  PortHandlerLinux(const char *port_name): dynamixel::PortHandlerLinux(port_name) {}

  int get_socket() const {
    return this->*result<PHi>::ptr;
  }
};
}

int main() {
  A a;
  (a.*result<Af>::ptr)();
  std::cout << "a.i = " << a.*result<Ai>::ptr << std::endl;

  quasi::PortHandlerLinux* port = new quasi::PortHandlerLinux("/dev/ttyACM1");

  std::cout << "port->socket_fd_ = " << port->get_socket() << std::endl;

  return 0;
}