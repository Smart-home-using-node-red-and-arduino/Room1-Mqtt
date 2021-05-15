#include "credentials.c"

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define LED 2

 
WiFiClient espClient;
PubSubClient client(espClient);
 
void setup() {
  pinMode(LED,OUTPUT);
  Serial.begin(115200);

  // get credentials from c function
  // function is declared in credentials.c 
  struct credentials cred = getCredentials();
  Serial.println("credentials");
  Serial.println(cred.ssid);
  Serial.println(cred.password);
  Serial.println(cred.mqtt_host_ip);
  Serial.println(cred.mqtt_port);
 
  WiFi.begin(cred.ssid, cred.password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.print("Connected to WiFi :");
  Serial.println(WiFi.SSID());
 
  client.setServer(cred.mqtt_host_ip, cred.mqtt_port);
  client.setCallback(mqtt_callback);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("nodeMcu")) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.println(client.state());  //If you get state 5: mismatch in configuration
      delay(2000);
 
    }
  }
 
  client.publish("esp/test", "Hello from ESP8266");
  client.subscribe("esp/test");
 
}
 
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");

  String message;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];  //Conver *byte to String
  }
   Serial.print(message);
  if(message == "#on") {digitalWrite(LED,LOW);}   //LED on  
  if(message == "#off") {digitalWrite(LED,HIGH);} //LED off
 
  Serial.println();
  Serial.println("-----------------------");  
}
 
void loop() {
  client.loop();
}
