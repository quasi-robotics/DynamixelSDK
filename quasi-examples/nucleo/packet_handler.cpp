#include "packet_handler.h"

using namespace quasi;
const uint8_t kProtocolVer = 2;

PacketHandler::PacketHandler(DXLPortHandler& port) : port_(port), buf_(DEFAULT_DXL_BUF_LENGTH), id_(0) {

  tx_packet_.is_init = false;  
  rx_packet_.is_init = false;
}

PacketHandler::~PacketHandler() {

}

DXLLibErrorCode_t PacketHandler::txStatusPacket(uint8_t *data, uint16_t data_len, uint8_t err_code) {

  if(!port_.getOpenState()) return DXL_LIB_ERROR_PORT_NOT_OPEN;

  DXLLibErrorCode_t err = DXL_LIB_OK;
  // Send Status Packet
  err = begin_make_dxl_packet(&tx_packet_, id_, kProtocolVer, DXL_INST_STATUS, err_code, &buf_[0], buf_.size());
  if (err != DXL_LIB_OK) return err;
  err = add_param_to_dxl_packet(&tx_packet_, data, data_len);
  if (err != DXL_LIB_OK) return err;
  err = end_make_dxl_packet(&tx_packet_);
  if (err != DXL_LIB_OK) return err;
  size_t sent = port_.write(tx_packet_.p_packet_buf, tx_packet_.generated_packet_length);
  //SerialUSB.printf("txStatusPacket sent: %d\n", sent);
  return err;
}


DXLLibErrorCode_t PacketHandler::rxPacket() {

  DXLLibErrorCode_t err = DXL_LIB_OK;

  if (!port_.getOpenState()) return DXL_LIB_ERROR_PORT_NOT_OPEN;

  // Receive Instruction Packet
  err = begin_parse_dxl_packet(&rx_packet_, kProtocolVer, &buf_[0], buf_.size());
  if (err != DXL_LIB_OK) return err;
  err = DXL_LIB_ERROR_TIMEOUT;
  while(port_.available() > 0) {
    err = parse_dxl_packet(&rx_packet_, port_.read());
    if(err == DXL_LIB_OK){
      if(rx_packet_.inst_idx != DXL_INST_STATUS) {
        if(rx_packet_.id == id_ || rx_packet_.id == DXL_BROADCAST_ID){
          err =  DXL_LIB_OK;
        } else {
          err = DXL_LIB_ERROR_WRONG_PACKET;
        }
      } else {
        err = DXL_LIB_ERROR_WRONG_PACKET;
      }
      break;
    } else if(err != DXL_LIB_PROCEEDING){
      break;
    }
  }
  return err;
}

DXLLibErrorCode_t PacketHandler::rxWritePacket(uint8_t *data, uint16_t data_len) {

  DXLLibErrorCode_t err = rxPacket();
  if (err != DXL_LIB_OK) return err;

  if (rx_packet_.inst_idx != DXL_INST_WRITE) return DXL_LIB_ERROR_WRONG_PACKET;
  uint8_t *p_rx_param = rx_packet_.p_param_buf;
  uint16_t addr = ((uint16_t)p_rx_param[1]<<8) | (uint16_t)p_rx_param[0];
  uint8_t* r_data = &p_rx_param[2];
  uint16_t r_data_length = rx_packet_.recv_param_len-2;
  //SerialUSB.printf("Got addr: %d, data_len: %d\n", addr, r_data_length);
  if(r_data_length > data_len){
    return DXL_LIB_ERROR_NOT_ENOUGH_BUFFER_SIZE;
  } else {
    for(uint16_t i = 0; i < r_data_length; i++ ) {
      data[i] = r_data[i];
    }
  }
  return DXL_LIB_OK;
}