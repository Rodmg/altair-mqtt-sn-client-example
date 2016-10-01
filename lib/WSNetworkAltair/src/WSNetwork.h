#ifndef WSNETWORK_H
#define WSNETWORK_H

#include <Arduino.h>
#include <Mesh.h>
#include <Wire.h>
#include "Storage.h"

#define PAIR_REQ_INTERVAL 5000

class WSNetwork
{
private:
  bool pairMode;
  uint8_t randomId;
  unsigned long lastPairReqSent;
public:
  WSNetwork();
  bool begin();
  bool begin(uint16_t addr, uint16_t pan = 0x0001, uint8_t * key = NULL);
  int connect();
  int read(unsigned char* buffer, int len, unsigned long timeout_ms);
  int write(unsigned char* buffer, int len, unsigned long timeout);
  int disconnect();
  bool sleep();

  void yield();
  bool enterPairMode();
  bool enterNormalMode();
  bool inPairMode();
  void sendPairReq();
  void loop();
  uint16_t getAddress();
  void setKey(uint8_t * key = NULL);
};

#endif //WSNETWORK_H
