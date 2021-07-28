void updi_send_cmd(byte cmd) {
  int l = 0;
  updi_send_cmd(cmd, NULL, l);
}

static void updi_send_cmd(byte cmd, char* payload, int &length) {
  switch (cmd) {
    case CMD_SIGN_ON: {
        UPDI_io::put(UPDI_io::double_break);
        UPDI::stcs(UPDI::reg::Control_B, 8);
        UPDI::stcs(UPDI::reg::Control_A, 0x80);
        break;
      }

    case CMD_RST: {
        UPDI::stcs(UPDI::reg::ASI_Reset_Request, UPDI::RESET_ON);
        UPDI::stcs(UPDI::reg::ASI_Reset_Request, UPDI::RESET_OFF);

        while ( UPDI::CPU_mode<0x0E>() == 0 );
        break;
      }

    case CMD_ENTER_PROGMODE: {
        updi_send_cmd(CMD_RST);
        const uint8_t system_status = UPDI::CPU_mode<0xEF>();
        switch (system_status) {
          case 0x82:
            UPDI::write_key(UPDI::NVM_Prog);
            updi_send_cmd(CMD_RST);

          case 0x08: {
              UPDI::sts_b(NVM::NVM_base | NVM::CTRLA, NVM::PBC);
              break;
            }
        }
        break;
      }

    case CMD_LEAVE_PROGMODE: {
        if (UPDI::CPU_mode<0xEF>() == 0x08) updi_send_cmd(CMD_RST);
        break;
      }

    case CMD_READ_SINGNATURE: {
        UPDI_io::put(UPDI::SYNCH);
        UPDI_io::put(0xE5);
        for (uint8_t i = 0; i < 16; i++) {
          payload[i] = UPDI_io::get();
        }
        break;
      }

    case CMD_READ_MEM: {
        if (UPDI::CPU_mode() != 0x08) {
          Serial.println("[ERROR] Not in the programming mode!");
          return;
        }
        
        const uint16_t NumBytes = payload[1] | (payload[2] << 8);
        // Get physical address for reading
        const uint16_t address = payload[3] | (payload[4] << 8);
        // Set UPDI pointer to address
        UPDI::stptr_w(address);
        // Read block
        UPDI::rep(NumBytes - 1);
        payload[1] = UPDI::ldinc_b();
        for (uint16_t i = 2; i <= NumBytes; i++) {
          payload[i] = UPDI_io::get();
        }
        length = NumBytes;
        payload[0] = 0x01; //OK

        break;
      }

    case CMD_WRITE_MEM: {
        if (UPDI::CPU_mode() != 0x08) {
          Serial.println(UPDI::CPU_mode(), HEX);
          return;
        }

        const uint8_t mem_type = payload[1];
        const uint16_t buffer_length = payload[2] | (payload[3] << 8);              /* number of bytes to write */
        const uint16_t address = payload[4] | (payload[5] << 8);

        const bool is_flash = ((mem_type == MTYPE_FLASH) || (mem_type == MTYPE_BOOT_FLASH));
        const uint8_t buff_size = is_flash ? 128 : 64;
        const uint8_t write_cmnd = is_flash ? NVM::WP : NVM::ERWP;
        switch (mem_type) {
          case MTYPE_FUSE_BITS:
          case MTYPE_LOCK_BITS: {
              NVM_fuse_write (address, payload[6]);
              break;
            }
          case MTYPE_FLASH:
          case MTYPE_BOOT_FLASH:
          case MTYPE_EEPROM_XMEGA:
          case MTYPE_USERSIG: {
              NVM_buffered_write(address, buffer_length, buff_size, write_cmnd, payload);
              break;
            }
        }

        break;
      }

    case CMD_ERASE_CHIP: {
        const uint8_t erase_type = payload[1];
        const uint16_t address = payload[2] | (payload[3] << 8);
        switch (erase_type) {
          case 0: {
              UPDI::write_key(UPDI::Chip_Erase);
              updi_send_cmd(CMD_RST);
              updi_send_cmd(CMD_ENTER_PROGMODE);
              break;
            }
          case 4:
          case 5: {
              NVM::wait<false>();
              UPDI::sts_b(address, 0xFF);
              NVM::command<false>(NVM::ER);
              break;
            }
          case 6:
          case 7: {
              break;
            }
        }
        break;
      }
  }
}

void uplaodSketch() {
  /*const char* SERVER = "10.144.1.242";  // Set your correct hostname
    const unsigned short SERVER_PORT = 8080;     // Commonly 80 (HTTP) | 443 (HTTPS)
    const char* PATH = "/slave.hex";

    HttpClient client(wifiClient, SERVER, SERVER_PORT);

    Serial.print("Get the update file:");

    client.get(PATH);

    int statusCode = client.responseStatusCode();
    Serial.print("Update status code: ");
    Serial.println(statusCode);
    if (statusCode != 200) {
    client.stop();
    Serial.print("Cannot get the update file!");
    return;
    }

    long length = client.contentLength();
    if (length == HttpClient::kNoContentLengthHeader) {
    client.stop();
    Serial.println("Server didn't provide Content-length header. Can't continue with update.");
    return;
    }
    Serial.print("Server returned update file of size ");
    Serial.print(length);
    Serial.println(" bytes");

    byte b;
    char file_buf[length];
    int i = 0;
    while (length > 0) {
    if (!client.readBytes(&b, 1)) break;

    file_buf[i] = b;

    i++;
    length--;
    }

    client.stop();

    if (length > 0) {
    Serial.print("Timeout downloading update file at ");
    Serial.print(length);
    Serial.println(" bytes. Can't continue with update.");
    return;
    }

    Serial.println("Sketch update apply and reset.");
    delay(1000);

    int line = 1;

    for (int i = 0; i < sizeof(file_buf); i++)
    if (file_buf[i] == '\n') line++;

    char flash_buf[line * 16];

    processFile(file_buf, sizeof(file_buf), line, flash_buf);*/

  /*Serial.println("Reboot");
    char p[1] = {0x01};
    sendCmd(Serial3, CMD_UPDATE_FIRM, p, 1);*/

  int l;
  char *p;
  UPDI_io::init();

  /*** ***/
  updi_send_cmd(CMD_SIGN_ON);

  /*** Enter programming mode ***/
  updi_send_cmd(CMD_ENTER_PROGMODE);
  Serial.println("[Done] Enter Programming Mode");

  /*** Read Device Signature ***/
  /*l = 16;
    p = (char *)malloc(16 * sizeof(char));
    updi_send_cmd(CMD_READ_SINGNATURE, p, l);
    Serial.print("RECEIVE BYTE: ");
    printHEX(p, l);
    free(p);
    Serial.println("[Done] Read Signature");*/

  /*** Erase Chip ***/
  l = 2;
  p = (char *)malloc(l * sizeof(char));
  p[1] = 0;

  updi_send_cmd(CMD_ERASE_CHIP, p, l);
  free(p);
  Serial.println("[Done] Erase Chip");

  /*** Write MEM ***/
  l = 512 + 6; //6 + 255(payload size)
  p = (char *)calloc(l, sizeof(char));
  p[1] = MTYPE_BOOT_FLASH;
  p[2] = l - 6;
  p[3] = l - 6 >> 8;
  p[4] = UPDI_FLASH_APP_START;
  p[5] = UPDI_FLASH_APP_START >> 8;
  for (int i = 0; i < l - 6; i++) p[6 + i] = i;

  updi_send_cmd(CMD_WRITE_MEM, p, l);
  free(p);
  Serial.println("[Done] Write Mem");

  /*** Read MEM ***/
  l = (255) + 2;
  p = (char *)malloc(l * sizeof(char));
  p[1] = l - 2; //size = 10
  p[2] = (l - 2) >> 8;
  p[3] = UPDI_FLASH_APP_START + 256;
  p[4] = UPDI_FLASH_APP_START + 256 >> 8;

  updi_send_cmd(CMD_READ_MEM, p, l);
  Serial.print("RECEIVE BYTE: ");
  printHEX(p, l + 1);
  free(p);
  Serial.println("[Done] Read Mem");

  /*** Leaving programming mode ***/
  updi_send_cmd(CMD_LEAVE_PROGMODE);
  Serial.println("[Done] Leave Programming Mode");

  //uploadHEXFile(flash_buf, sizeof(flash_buf));
}

/*void uploadHEXFile(char *buf, int length) {
  int page_num = length / 128 + (length % 128 > 0 ? 1 : 0);
  int count = 0;
  int adr = 0x00;

  Serial.print("Total pages # is ");
  Serial.println(page_num);

  for (int i = 0; i < page_num; i++) { //looping each page
    Serial.print("uploading page #");
    Serial.println(i + 1);

    // Set memory address
    char addrs[2] = {
      (adr & 0xff),
      ((adr >> 8) & 0xff),
    };

    updi_send_cmd(CMD_LOAD_ADDRESS, addrs, sizeof(addrs));

    char ret[2];
    readch(ret, sizeof(ret));
    printHEX(ret, sizeof(ret));
    delay(10);

    // Upload page
    char page[128];
    for (int j = 0; j < sizeof(page); j++) page[j] = 0xff;

    for (int j = 0; j < sizeof(page); j++) {
      if (count < length) {
        page[j] = buf[count];
        count++;
      }
    }

    for (int j = 0; j < sizeof(page); j++) Serial.print(page[j]);
    Serial.println();
    Serial.println("-----");

    updi_send_cmd(CMD_PROG_PAGE, page, sizeof(page));

    char ret2[2];
    readch(ret2, sizeof(ret2));
    printHEX(ret2, sizeof(ret2));
    delay(10);

    updi_send_cmd(CMD_READ_PAGE);

    char ret3[130];
    readch(ret3, sizeof(ret3));
    printHEX(ret3, sizeof(ret3));
    delay(10);

    adr += 0x40;
  }
  }*/
