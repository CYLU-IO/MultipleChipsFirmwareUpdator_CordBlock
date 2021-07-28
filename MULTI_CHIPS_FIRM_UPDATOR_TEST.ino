#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>

#include "JTAG2.h"
#include "updi_lo_lvl.h"
#include "updi_io.h"
#include "NVM.h"
#include "wiring_private.h"

#define DEBUG                   1

/*** SERIAL ***/
#define CMD_FAIL                0x11
#define CMD_EOF                 0x20
#define CMD_REQ_ADR             0x41 //'A'
#define CMD_LOAD_MODULE         0x42 //'B'
#define CMD_CONFIRM_RECEIVE     0x43 //'C'
#define CMD_DO_MODULE           0x44 //'D'
#define CMD_REQ_DATA            0x45 //'E'
#define CMD_UPDATE_FIRM         0x46 //'F'
#define CMD_HI                  0x48 //'H'
#define CMD_INIT_MODULE         0x49 //'I'
#define CMD_LINK_MODULE         0x4C //'L'
#define CMD_RESET_MODULE        0x52 //'R'
#define CMD_UPDATE_DATA         0x55 //'U'
#define CMD_START               0xFF

typedef enum CMD_STATE {
  RC_NONE,
  RC_HEADER,
  RC_PAYLOAD,
  RC_CHECK
};

const char MY_SSID[] = "Edwin's Room(2.4G)"; // Loaded from arduino_secrets.h
const char MY_PASS[] = "Edw23190"; // Loaded from arduino_secrets.h

WiFiClient    wifiClient;  // HTTP
int status = WL_IDLE_STATUS;

#define CMD_SIGN_ON 0x49 //custom
#define CMD_ENTER_PROGMODE 0x50 //return 2 chars
#define CMD_LEAVE_PROGMODE 0x51 //return 2 chars
#define CMD_LOAD_ADDRESS 0x55 //return 2 chars
#define CMD_WRITE_MEM 0x64 //custom
#define CMD_READ_MEM 0x74 //custom
#define CMD_READ_SINGNATURE 0x75 //return 5 chars
#define CMD_ERASE_CHIP 0x76 //custom
#define CMD_RST 0x77 //custom

// *** UPDI PARAMETERS
#define UPDI_FLASH_APP_START 0x4000

void setup() {
  Serial.begin(9600);
  serialInit();

  while (!Serial) ;
  Serial.println("===== Multi-Chips Firmware Updator V1 by CYLU.IO =====");

  /*while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(MY_SSID);
    status = WiFi.begin(MY_SSID, MY_PASS);
    }

    Serial.println("WiFi connected");*/
}

void loop() {
  if (Serial.available()) { //for test only
    int c = Serial.read();

    if (c == 87) { //W
      Serial.println("Serial3 Commanding");
      Serial3.begin(9600);
      char p[1] = {0x01};
      sendCmd(Serial3, CMD_UPDATE_FIRM, p, 1);
    }

    if (c == 'R') {
      Serial.println("Upload");
      uplaodSketch();
    }
  }
}

void SERCOM0_Handler() {
  Serial3.IrqHandler();
}
