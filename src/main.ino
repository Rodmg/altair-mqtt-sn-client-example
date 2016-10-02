#include <MQTTSNClient.h>
#include <WSNetwork.h>
#include <VirtualTimer.h>

using namespace MQTTSN;

WSNetwork network;
Client client(network, 1000);

// Topic strings
const char lastWill[] = "lastwill";
const char publishTopic[] = "client/publish";
const char subscribeTopic[] = "client/subscribe";

// Timer for publishing example topic repeatedly
VirtualTimer timer;

// Pair support
#define PAIR_BTN 33
#define PAIR_LED 14

void setupPair()
{
  pinMode(PAIR_BTN, INPUT);
  pinMode(PAIR_LED, OUTPUT);
  digitalWrite(PAIR_LED, HIGH);
}

void attendPairToggle()
{
  if(!digitalRead(PAIR_BTN))
  {
    if(network.inPairMode())
    {
      network.enterNormalMode();
      digitalWrite(PAIR_LED, HIGH);
    }
    else
    {
      network.enterPairMode();
      digitalWrite(PAIR_LED, LOW);
    }
    delay(1000);
  }
  // Manage indicator LED state
  if(network.inPairMode()) digitalWrite(PAIR_LED, LOW);
  else digitalWrite(PAIR_LED, HIGH);
}


// Function that handles the "client/subscribe" topic
void subscribeHandler(struct MQTTSN::MessageData &msg)
{
  Serial.print("Got message, Topic: ");
  Serial.println(subscribeTopic);
  Serial.print("Message: ");
  for(uint8_t i = 0; i < msg.message.payloadlen; i++)
  {
    Serial.print(((char*)msg.message.payload)[i]);
  }
  Serial.println();
}

// Function for connecting with gateway
bool connectMqtt()
{
  Serial.println("Connecting MQTT...");
  // Set MQTT last will topic and blank message
  client.setWill(lastWill, NULL, 0);
  // Setup MQTT connection
  static MQTTSNPacket_connectData options = MQTTSNPacket_connectData_initializer;
  options.duration = 10; // Keep alive interval, Seconds
  options.cleansession = true;
  options.willFlag = true;
  int status = client.connect(options);

  if(status != SUCCESS) return false;
  // Clear any previous subscriptions (useful if we are reconnecting)
  client.clearSubscriptions();
  client.clearRegistrations();
  // We need to first register the topics that we may publish (a diference between this MQTT-SN implementation and MQTT)
  client.registerTopic(publishTopic, strlen(publishTopic));
  // Subscribe a function handler to a topic
  client.subscribe(subscribeTopic, QOS1, subscribeHandler);

  return true;
}

void yield()
{
  network.yield();
}

void setup()
{
  Serial.begin(9600);
  // Start with automatic address given by Aquila Mesh
  //char pass[16] = {1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6};
  //network.begin(0x07, 12, pass);
  network.begin();
  // Start timer for publishing "client/publish" every 5 seconds
  timer.countdown_ms(5000);
  setupPair();
}

void loop()
{
  // Attend network tasks
  yield();
  attendPairToggle();
  if(network.inPairMode()) {
    network.loop();
    return;
  }
  client.loop();
  if(!client.isConnected())
  {
    Serial.println("MQTT disconnected, trying to reconnect...");
    if(!connectMqtt()) return;
  }
  // Attend timer
  if(timer.expired())
  {
    char payload[] = "Hello world";
    bool retained = false;
    // Publish "client/publish"
    client.publish(publishTopic, payload, strlen(payload), QOS1, retained);
    // Restart timer
    timer.countdown_ms(5000);
    Serial.println("Published message");
  }

}
