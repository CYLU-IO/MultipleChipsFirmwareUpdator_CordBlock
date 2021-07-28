/*
   updi_io.cpp

   Created: 18-11-2017 10:36:54
    Author: JMR_2
*/

// Includes
#include <Arduino.h>
#include "updi_io.h"

#include "wiring_private.h"

// Functions
/* Sends regular characters through the UPDI link */

Uart Serial3 (&sercom0, 5, 6, SERCOM_RX_PAD_1, UART_TX_PAD_0);

uint8_t UPDI_io::put(char c) {
  Serial3.write(c);
  Serial3.flush();
  //delayMicroseconds(10);
  long start = millis();
  while (!Serial3.available() && millis() - start < 20) {}
  char d = Serial3.read();
  if (c != d) {
    Serial.println("echo failed! " + String(d, HEX));
  }
  return c;
}

/* Sends special sequences through the UPDI link */
uint8_t UPDI_io::put(ctrl c)
{
  Serial3.begin(300, SERIAL_8N1);

  switch (c) {
    case double_break:
      Serial3.write((uint8_t)0x00);
      Serial3.flush();
      Serial3.write((uint8_t)0x00);
      Serial3.flush();
      break;
    case single_break:
      Serial3.write((uint8_t)0x00);
      Serial3.flush();
      break;
    default:
      break;
  }
  delay(15);
  while (Serial3.available()) {
    Serial3.read();
  }
  Serial3.begin(230400, SERIAL_8E2);
  return 0;
}

uint8_t UPDI_io::get() {
  uint8_t c;
  while (!Serial3.available()) {}
  c = Serial3.read();
  //delayMicroseconds(5);
  //Serial.println("get! " + String(c, HEX));
  return c;
}

void UPDI_io::init(void)
{
 
  Serial3.begin(230400, SERIAL_8E2);
  //Serial.begin(115200);
}
