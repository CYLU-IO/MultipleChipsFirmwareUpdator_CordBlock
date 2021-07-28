bool hexConv (const char * (& pStr), byte & b) {
  if (!isxdigit (pStr [0]) || !isxdigit (pStr [1])) return true;

  b = *pStr++ - '0';

  if (b > 9) b -= 7;

  b <<= 4;

  byte b1 = *pStr++ - '0';

  if (b1 > 9) b1 -= 7;

  b |= b1;

  return false;
}

bool processLine (const char *pLine, byte *dataBuffer) {
  byte hexBuffer[21];

  if (*pLine++ != ':') return true;

  int bytesInLine = 0;

  // convert entire line from ASCII into binary
  while (isxdigit (*pLine)) {
    if (hexConv (pLine, hexBuffer [bytesInLine++])) return true;
  }

  if (bytesInLine < 5) return true;

  // sumcheck it
  byte sumCheck = 0;

  for (int i = 0; i < (bytesInLine - 1); i++) sumCheck += hexBuffer[i];

  // 2's complement
  sumCheck = ~sumCheck + 1;

  // check sumcheck
  if (sumCheck != hexBuffer [bytesInLine - 1]) return true;

  // length of data (eg. how much to write to memory)
  byte len = hexBuffer [0];

  // the data length should be the number of bytes, less
  //   length / address (2) / transaction type / sumcheck
  if (len != (bytesInLine - 5)) return true;

  for (int i = 0; i < len; i++) dataBuffer[i] = hexBuffer[4 + i];

  return false;
}

void processFile(char *file_buf, int length, int line, char *flash_buf) {
  char line_buf[43];
  byte data_buf[16];

  int count = 0;
  int flash_count = 0;

  for (int j = 0; j < sizeof(line_buf); j++) line_buf[j] = 0xFF;
  for (int j = 0; j < (line * 16); j++) flash_buf[j] = 0xFF;

  for (int i = 0; i < length; i++) {
    if (file_buf[i] == '\n') {
      for (int j = 0; j < sizeof(data_buf); j++) data_buf[j] = 0xFF; //init data buffer

      processLine((char*)line_buf, data_buf);

      for (int j = 0; j < sizeof(data_buf); j++) {
        flash_buf[flash_count] = data_buf[j];
        flash_count++;
      }

      count = 0;
      for (int j = 0; j < sizeof(line_buf); j++) line_buf[j] = 0xFF;
    } else {
      line_buf[count] = file_buf[i];
      count++;
    }
  }
}

void printHEX(char *arr, int length) {
  for (int i = 0; i < length; i++) {
    Serial.print(arr[i], HEX);
    Serial.print(' ');

    //if (arr[i] == '\n') Serial.println();
  }

  Serial.println();
}
