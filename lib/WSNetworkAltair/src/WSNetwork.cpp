#include "WSNetwork.h"
#include <VirtualTimer.h>

static uint8_t recBuffer[256];
static uint8_t attendPending;

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
}

bool WSNetwork::begin()
{
  Mesh.begin();
  Mesh.openEndpoint(11, receiveMessage);
  return true;
}

bool WSNetwork::begin(uint16_t addr)
{
  Mesh.begin(addr);
  Mesh.openEndpoint(11, receiveMessage);
  return true;
}

int WSNetwork::connect()
{
  return 0;
}

int WSNetwork::read(unsigned char* buffer, int len, unsigned long timeout_ms)
{
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
  // forming packet
	static TxPacket packet;
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
  static VirtualTimer timer;
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

uint16_t WSNetwork::getAddress()
{
  return Mesh.getShortAddr();
}
