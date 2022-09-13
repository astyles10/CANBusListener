/******************************************************************************
 * demo: CAN-BUS Shield, receive data with check mode
 * send data coming to fast, such as less than 10ms, you can use this way
 * Jansion, 2015.5.27
 ******************************************************************************/

#include <SPI.h>

#include <iterator>
// #include <algorithm>
#include <map>
// #include <vector>

#include "df_can.h"

typedef struct options {
  bool enableLogging = false;
  options() = default;
} options;

enum SerialCommands { LIST_IDS = 'd', SELECT_ID = 's', TOGGLE_LOGGING = 'l' };

const int SPI_CS_PIN = 10;
MCPCAN CAN(SPI_CS_PIN);
std::map<uint32_t, bool> CANIdMap;
// FILE logFile;
// std::vector<uint32_t> CANIds;
static uint32_t selectedCANId = 0;

void setup() {
  Serial.begin(115200);
  connectToCANBus();
}

void loop() {
  handleCommandReceive();
  handleCanMessage();
}

void connectToCANBus() {
  while (true) {
    Serial.println("Attempting to connect to CAN BUS...");
    CAN.init();
    if (CAN_OK == CAN.begin(CAN_500KBPS)) {
      Serial.println("CAN BUS Shield init ok!");
      break;
    } else {
      delay(1000);
    }
  };
}

void handleCommandReceive() {
  if (Serial.available() > 0) {
    int command = Serial.read();
    switch (command) {
      case LIST_IDS:
        listKnownCanIds();
        break;
      case SELECT_ID:
        SelectActiveCanId();
        break;
      case TOGGLE_LOGGING:
        break;
      default:
        break;
    }
    Serial.print("Received command: ");
    Serial.println(Serial.read());
  }
}

void handleCanMessage() {
  unsigned char len = 0;
  unsigned char buf[8];
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    if (CANIdMap.find(CAN.getCanId()) == CANIdMap.end()) {
    // if (std::find(CANIds.begin(), CANIds.end(), CAN.getCanId())) {
      Serial.print("Found new CAN ID: ");
      Serial.println(CAN.getCanId(), HEX);
      CANIdMap.insert({CAN.getCanId(), false});
      // CANIds.push_back(CAN.getCanId());
    }

    CAN.readMsgBuf(&len, buf);
    for (int i = 0; i < len; i++) {
      Serial.print(buf[i], HEX);
      Serial.print("\t");
    }
    Serial.println();
  }
}

void listKnownCanIds() {
  Serial.println("List of known CAN ids:");
  for (const auto& canId : CANIdMap) {
    Serial.println(canId.first);
  }
}

void SelectActiveCanId() {
  Serial.println("Select a CAN ID:");
  for (const auto& canId : CANIdMap) {
    Serial.println(canId.first);
  }
  while (!Serial.available()) {
    // wait for input
  }
  char buffer[8];
  size_t len;
  Serial.readBytesUntil('\n', buffer, len);
  
  // Serial.readString()
}
