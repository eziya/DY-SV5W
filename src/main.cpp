#include <Arduino.h>

// Define constants
const uint8_t CMD_PREFIX = 0xAA;
const uint8_t CMD_STATUS = 0x01;
const uint8_t CMD_PLAY = 0x02;
const uint8_t CMD_PAUSE = 0x03;
const uint8_t CMD_STOP = 0x04;
const uint8_t CMD_PREV = 0x05;
const uint8_t CMD_NEXT = 0x06;
const uint8_t CMD_VOL_SET = 0x13;
const uint8_t CMD_VOL_UP = 0x14;
const uint8_t CMD_VOL_DOWN = 0x15;
const uint8_t CMD_PLAY_MODE = 0x18;
const uint8_t CMD_EQ = 0x1A;

const uint8_t DEF_DATA_LEN = 0x1;
const uint8_t DEF_VOLUME = 0x5;
const uint8_t DEF_EQ = 0x01;
const uint8_t DEF_PLAY_MODE = 0x00;

const uint32_t TIMEOUT = 500;

void ConfigureDefaultSettings(void);
void SendCommand(uint8_t command);
uint8_t ReadPlayStatus(void);

static bool ManualStop = false;

void setup() {
  // Initialize serial ports
  Serial.begin(115200);  // Initialize Serial port
  Serial1.begin(9600);   // Initialize Serial1 port
  delay(100);
  ConfigureDefaultSettings();  // Set default volume, EQ settings
}

void loop() {
  static uint32_t prevMillis = 0;
  uint32_t currMillis = millis();

  if (Serial.available() > 0) {         // Check if data is available on Serial
    char receivedChar = Serial.read();  // Read data
    receivedChar = tolower(receivedChar);

    Serial.println("Message Received: " + String(receivedChar));

    switch (receivedChar) {
      case 'p':
        Serial.println("Playing...");
        ManualStop = false;
        SendCommand(CMD_PLAY);
        break;
      case 'a':
        Serial.println("Paused...");
        SendCommand(CMD_PAUSE);
        break;
      case 's':
        Serial.println("Stopping...");
        ManualStop = true;
        SendCommand(CMD_STOP);
        break;
      case 'v':
        Serial.println("Previous...");
        SendCommand(CMD_PREV);
        break;
      case 'n':
        Serial.println("Next...");
        SendCommand(CMD_NEXT);
        break;
      case '+':
        Serial.println("Volume Up...");
        SendCommand(CMD_VOL_UP);
        break;
      case '-':
        Serial.println("Volume Down...");
        SendCommand(CMD_VOL_DOWN);
        break;
      default: break;
    }
  } else {    
    // Check the play status every second
    if (currMillis - prevMillis > 1000) {
      prevMillis = currMillis;
      
      SendCommand(CMD_STATUS);
      uint8_t playStatus = ReadPlayStatus();
      Serial.print(playStatus);

      // Play next track if playback has stopped
      if (playStatus == 0 && ManualStop == false) {
        Serial.println("Next...");
        SendCommand(CMD_NEXT);
      }
    }
  }
}

void ConfigureDefaultSettings() {
  uint8_t checksum;

  // Setting Play Mode, currently not functioning
  checksum = (uint8_t)(CMD_PREFIX + CMD_PLAY_MODE + DEF_DATA_LEN + DEF_PLAY_MODE);
  Serial1.write(CMD_PREFIX);
  Serial1.write(CMD_PLAY_MODE);
  Serial1.write(DEF_DATA_LEN);
  Serial1.write(DEF_PLAY_MODE);
  Serial1.println(checksum);
  Serial.println("Set Play Mode as Repeat...");
  delay(100);

  // Setting EQ
  checksum = (uint8_t)(CMD_PREFIX + CMD_EQ + DEF_DATA_LEN + DEF_EQ);
  Serial1.write(CMD_PREFIX);
  Serial1.write(CMD_EQ);
  Serial1.write(DEF_DATA_LEN);
  Serial1.write(DEF_EQ);
  Serial1.println(checksum);
  Serial.println("Set EQ as POP...");
  delay(100);

  // Setting Volume
  checksum = (uint8_t)(CMD_PREFIX + CMD_VOL_SET + DEF_DATA_LEN + DEF_VOLUME);
  Serial1.write(CMD_PREFIX);
  Serial1.write(CMD_VOL_SET);
  Serial1.write(DEF_DATA_LEN);
  Serial1.write(DEF_VOLUME);
  Serial1.write(checksum);
  Serial.println("Set Volume as DEF_VOLUME...");
  delay(100);
}

void SendCommand(uint8_t command) {
  uint8_t checksum = (uint8_t)(CMD_PREFIX + command);
  Serial1.write(CMD_PREFIX);
  Serial1.write(command);
  Serial1.write(0x00);
  Serial1.write(checksum);
}

uint8_t ReadPlayStatus() {  
  uint32_t startTime = millis();
  uint8_t status;

  while (Serial1.available() < 5) {
    if (millis() - startTime > TIMEOUT) {
      return 0xFF; // Return error code on timeout
    }
  }

  for (int i = 0; i < 3; i++) {
    Serial1.read(); // The first 3 bytes are header, command, length.
  }
  status = Serial1.read(); // The 4th byte is status
  
  Serial1.read(); // Checksum

  return status;
}
