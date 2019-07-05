/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "Note8Hotspot";
const char* password = "yb15fsfsaqib";
const char* mqtt_server = "broker.hivemq.com";
const int port = 1883;
const char* TOPIC = "zumo/mcu/data";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//void callback(char* topic, byte* payload, unsigned int length) {
//  Serial.print("Message arrived [");
//  Serial.print(topic);
//  Serial.print("] ");
//  for (int i = 0; i < length; i++) {
//    Serial.print((char)payload[i]);
//  }
//  Serial.println();
//
//  // Switch on the LED if an 1 was received as first character
//  if ((char)payload[0] == '1') {
//    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
//    // but actually the LED is on; this is because
//    // it is active low on the ESP-01)
//  } else {
//    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
//  }
//
//}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      digitalWrite(2, LOW);
      digitalWrite(BUILTIN_LED, LOW);
      Serial.println("connected");
      // Once connected, publish an announcement...
     client.publish(TOPIC, "<< NodeMCU Connected to MQTT Broker >>");
      // ... and resubscribe
      // client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      digitalWrite(BUILTIN_LED, LOW);
      digitalWrite(2, HIGH);
      delay(5000);
      
    }
  }
}
void publishSerialData(char *serialData){
  if (!client.connected()) {
    digitalWrite(2, LOW);
    digitalWrite(2, HIGH);
    reconnect();
  }
  client.publish(TOPIC, serialData);
}

void setup() {
  pinMode(2, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(2, HIGH);
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, port);
  // client.setCallback(callback);
}

void loop() {
  digitalWrite(2, HIGH);
  if (!client.connected()) {
    digitalWrite(2, LOW);
    digitalWrite(2, HIGH);
    reconnect();
  }
  client.loop();
  
  if (Serial.available() > 0) {
    char bfr[101];
    memset(bfr,0, 101);
    Serial.readBytesUntil( '\n',bfr,100);
    publishSerialData(bfr);
  }
  
  digitalWrite(2, LOW);
}
