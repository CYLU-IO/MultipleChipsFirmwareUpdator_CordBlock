#include "Arduino.h"
#include "JTAG2.h"
#include "NVM.h"
#include "UPDI_lo_lvl.h"

void NVM_fuse_write (uint16_t address, uint8_t data) {
  // Setup UPDI pointer
  UPDI::stptr_w(NVM::NVM_base + NVM::DATA_lo);
  // Send data to the NVM controller
  UPDI::stinc_b(data);
  UPDI::stinc_b(0x00);
  // Send address to the NVM controller
  UPDI::stinc_b(address & 0xFF);
  UPDI::stinc_b(address >> 8);
  // Execute fuse write
  NVM::command<false>(NVM::WFU);
}

void NVM_buffered_write(const uint16_t address, const uint16_t length, const uint8_t buff_size, const uint8_t write_cmnd, char *payload) {
  uint16_t current_byte_index = 6;          /* Index of the first byte to send inside the JTAG2 command body */
  uint16_t bytes_remaining = length;          /* number of bytes to write */

  // Sends a block of bytes from the command body to memory, using the UPDI interface
  // On entry, the UPDI pointer must already point to the desired address
  // On exit, the UPDI pointer points to the next byte after the last one written
  // Returns updated index into the command body, pointing to the first unsent byte.
  auto updi_send_block = [] (uint8_t count, uint16_t index, char *payload) {
    count--;
    
    NVM::wait<true>();
    UPDI::rep(count);
    UPDI::stinc_b(payload[index]);
    for (uint8_t i = count; i; i--) {
      UPDI_io::put(payload[++index]);
      UPDI_io::get();
    }

    return ++index;
  };

  // Setup UPDI pointer for block transfer
  UPDI::stptr_w(address);
  /* Check address alignment, calculate number of unaligned bytes to send */
  uint8_t unaligned_bytes = (-address & (buff_size - 1));
  if (unaligned_bytes > bytes_remaining) unaligned_bytes = bytes_remaining;
  /* If there are unaligned bytes, they must be sent first */
  if (unaligned_bytes) {
    // Send unaligned block
    current_byte_index = updi_send_block(unaligned_bytes, current_byte_index, payload);
    bytes_remaining -= unaligned_bytes;
    NVM::command<true>(write_cmnd);
  }
  while (bytes_remaining) {
    /* Send a buff_size amount of bytes */
    if (bytes_remaining >= buff_size) {
      current_byte_index = updi_send_block(buff_size, current_byte_index, payload);
      bytes_remaining -= buff_size;
    }
    /* Send a NumBytes amount of bytes */
    else {
      current_byte_index = updi_send_block(bytes_remaining, current_byte_index, payload);
      bytes_remaining = 0;
    }
    NVM::command<true>(write_cmnd);
  }
}
