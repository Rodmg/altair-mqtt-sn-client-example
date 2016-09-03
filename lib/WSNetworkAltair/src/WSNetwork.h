#ifndef WSNETWORK_H
#define WSNETWORK_H

#include <Arduino.h>
#include <Mesh.h>
#include <Wire.h>

class WSNetwork
{
public:
  WSNetwork();
  bool begin();
  bool begin(uint16_t addr);
  int connect();
  int read(unsigned char* buffer, int len, unsigned long timeout_ms);
  int write(unsigned char* buffer, int len, unsigned long timeout);
  int disconnect();
  bool sleep();

  void yield();
  uint16_t getAddress();
};

#endif //WSNETWORK_H
