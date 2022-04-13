#ifndef _PACKET_HANDLER_H
#define _PACKET_HANDLER_H

#include <Dynamixel2Arduino.h>
#include <vector>
#include <memory>

namespace quasi {
  using Buffer = std::vector<uint8_t>;

  class PacketHandler {
  public:
    PacketHandler(DXLPortHandler& port);
    ~PacketHandler(); 

    void setID(uint8_t id) { id_ = id; }
    uint8_t getID() const { return id_; }


    DXLLibErrorCode_t txStatusPacket(uint8_t *data, uint16_t data_len, uint8_t err_code = 0);

    DXLLibErrorCode_t rxWritePacket(uint8_t *data, uint16_t data_len);

  private:
    DXLLibErrorCode_t rxPacket();

    Buffer buf_;
    DXLPortHandler& port_;
    InfoToMakeDXLPacket_t tx_packet_;
    InfoToParseDXLPacket_t rx_packet_;
    uint8_t id_;

  };
}

#endif // _PACKET_HANDLER_H
