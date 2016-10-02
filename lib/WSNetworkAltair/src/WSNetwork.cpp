#include "WSNetwork.h"
#include <VirtualTimer.h>

static uint8_t recBuffer[256];
static uint8_t attendPending;

static TxPacket packet;
static VirtualTimer timer;

#define ENC_KEY_SIZE 16
static uint8_t voidKey[ENC_KEY_SIZE] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

static bool isVoidKey(uint8_t * key)
{
  if(!key) return true;
  for(uint8_t i = 0; i < ENC_KEY_SIZE; i++)
  {
    if(key[i] != voidKey[i]) return false;
  }
  return true;
}

bool sendConfirmed = false;
uint8_t sendStatus;

static void meshSendConfirm(TxPacket *req)
{
  sendConfirmed = true;
  sendStatus = req->status;
	// Here, you could check req->status
	// For knowing if the packet was succesfully sent.
}

static void receiveMessage(RxPacket *ind)
{
  memcpy(recBuffer, ind->data, ind->size);
  attendPending = ind->size;
  //dbg
  // Serial.print("Received: ");
  // Serial.println(ind->size);
  // end dbg
	return true;
}

WSNetwork::WSNetwork()
{
  attendPending = 0;
  pairMode = false;
  lastPairReqSent = 0;
  meshInited = false;
  randomSeed(analogRead(7));
  randomId = random(0xFF);
}

bool WSNetwork::begin()
{
  uint8_t addr = Storage.getAddr();
  uint8_t pan = Storage.getPan();
  uint8_t key[ENC_KEY_SIZE];
  Storage.getKey(key);
  if(addr == 0 || addr == 0xFF) return enterPairMode();
  if(pan == 0 || pan == 0xFF) return enterPairMode();
  return begin(addr, pan, key);
}

bool WSNetwork::begin(uint16_t addr, uint16_t pan = 0x0001, uint8_t * key = NULL)
{
  // Shouldn't call Mesh.begin multiple times, or unexpected things happen
  if(!meshInited)
  {
    Mesh.begin(addr);
    Mesh.openEndpoint(11, receiveMessage);
    meshInited = true;
  }

  Mesh.setAddr(addr);
  Mesh.setPanId(pan);
  setKey(key);
  return true;
}

int WSNetwork::connect()
{
  return 0;
}

int WSNetwork::read(unsigned char* buffer, int len, unsigned long timeout_ms)
{
  // Dont allow normal operation in pair mode
  if(pairMode) return 0;
  if(!attendPending) return 0;
  //Serial.print("Reading: ");
  //Serial.println(attendPending);
  uint8_t n = attendPending;
  memcpy(buffer, recBuffer, attendPending);
  attendPending = 0;
  return n;
}

int WSNetwork::write(unsigned char* buffer, int len, unsigned long timeout)
{
  // Dont allow normal operation in pair mode
  if(pairMode) return 0;
  // forming packet
	packet.dstAddr = HUB;
	packet.dstEndpoint = 11;
	packet.srcEndpoint = 11;
	// Request acknowledge, use only if not sending to BROADCAST
	packet.options = NWK_OPT_ACK_REQUEST;
	packet.data = buffer;
	packet.size = len;
	packet.confirm = meshSendConfirm;

  sendConfirmed = false;
	Mesh.sendPacket(&packet);
  timer.countdown_ms(timeout);
  while(!sendConfirmed && !timer.expired())
  {
    yield();
  }

  if(timer.expired()) len = 0; // Timeout
  else if(sendStatus != NWK_SUCCESS_STATUS) len = 0; // There was an error sending

  return len;
}

int WSNetwork::disconnect()
{
  return 0;
}

bool WSNetwork::sleep()
{
  while(Mesh.busy()) yield();
  Mesh.sleep();
}

void WSNetwork::yield()
{
  Mesh.loop();
}

bool WSNetwork::enterPairMode()
{
  pairMode = true;
  return begin(0x0000, 0x0000, NULL);
}

bool WSNetwork::enterNormalMode()
{
  pairMode = false;
  return begin();
}

bool WSNetwork::inPairMode()
{
  return pairMode;
}

void WSNetwork::sendPairReq()
{
  uint8_t buffer[3] = { 3, 0x03, randomId };
  uint8_t len = 3;
  // forming packet
	packet.dstAddr = HUB;
	packet.dstEndpoint = 11;
	packet.srcEndpoint = 11;
	// Request acknowledge, use only if not sending to BROADCAST
	packet.options = NWK_OPT_ACK_REQUEST;
	packet.data = buffer;
	packet.size = len;
	packet.confirm = meshSendConfirm;

  sendConfirmed = false;
	Mesh.sendPacket(&packet);
  timer.countdown_ms(500);
  while(!sendConfirmed && !timer.expired())
  {
    yield();
  }

  if(timer.expired()) len = 0; // Timeout
  else if(sendStatus != NWK_SUCCESS_STATUS) len = 0; // There was an error sending
}

uint16_t WSNetwork::getAddress()
{
  return Mesh.getShortAddr();
}

// Only needed for pair mode
void WSNetwork::loop()
{
  if(pairMode)
  {
    unsigned long now = millis();
    if(lastPairReqSent + PAIR_REQ_INTERVAL < now)
    {
      sendPairReq();
      lastPairReqSent = now;
    }

    // Handle PAIRRES messages
    if(!attendPending) return;

    // Check if its addressed to us and has encryption key
    if(attendPending >= 21 && recBuffer[0] == 21 && recBuffer[1] == 0x03 && recBuffer[2] == randomId)
    {
      // Save given address
      Storage.setAddr(recBuffer[3]);
      Storage.setPan(recBuffer[4]);
      Storage.setKey(&recBuffer[5]);
      enterNormalMode();
    }
    // No encryption key
    if(attendPending >= 5 && recBuffer[0] == 5 && recBuffer[1] == 0x03 && recBuffer[2] == randomId)
    {
      // Save given address
      Storage.setAddr(recBuffer[3]);
      Storage.setPan(recBuffer[4]);
      Storage.setKey(voidKey);
      enterNormalMode();
    }

  }
}

void WSNetwork::setKey(uint8_t * key)
{
  if(isVoidKey(key)) Mesh.setSecurityEnabled(false);
  else
  {
    Mesh.setSecurityKey(key);
    Mesh.setSecurityEnabled(true);
  }
}
