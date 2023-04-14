#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

float sensor;
char sensorOut[20];

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 102); //Ethernet Shield
IPAddress server(192, 168, 7, 244); //Server

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

void setup()
{
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
}

void loop()
{
  sensor = random(20, 40);
  client.loop();
  dtostrf(sensor,2,2,sensorOut);
  Serial.println(sensorOut);
  if (client.connect("MillC1", "faizi", "mqtt-faizi")) {
    client.publish("SPINDO/UNIT6/PRODUKSI/TEST", sensorOut);
  }
  delay(1000);
}
