#include "credentials.c"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <Ticker.h> // ticker library to check mqtt connection status ( if disconnected then try to reconnect )


#define LED 2
#define relay D1

 
WiFiClient espClient;
PubSubClient client(espClient);
Ticker check_Mqtt_connection; // Check if the connection is lost try to reconnect ( maybe mqtt server was shutdown unexpectedly or restarted! )

void checkMqttServer();
 
void setup() {
  pinMode(LED,OUTPUT);
  pinMode(relay,OUTPUT);
  Serial.begin(115200);

  check_Mqtt_connection.attach(180, checkMqttServer); // check mqtt server connection every 3 minutes (3*60 seconds)

  // get credentials from c function
  // function is declared in credentials.c 
  struct credentials cred = getCredentials();

  // print credentials for debug purposes
  /* Serial.println("credentials");
  Serial.println(cred.ssid);
  Serial.println(cred.password);
  Serial.println(cred.mqtt_host_ip);
  Serial.println(cred.mqtt_port);  */
 
  WiFi.begin(cred.ssid, cred.password);
 
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi..");
    digitalWrite(LED,LOW);
    delay(300);
    digitalWrite(LED,HIGH);
    delay(300);
  }
  Serial.print("Connected to WiFi :");
  Serial.println(WiFi.SSID());
  digitalWrite(LED,LOW);
 
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
 
  client.publish("room1", "room1 nodeMCU connected!");
  client.subscribe("room1/lamp");
  client.subscribe("room1/BuiltInLamp");
 
}

void checkMqttServer(){
  while (!client.connected()) {
    Serial.println("Reconnecting to MQTT...");

    if (client.connect("nodeMcu")) {
 
      Serial.println("connected");
    } else {
 
      Serial.print("failed with state ");
      Serial.println(client.state());  //If you get state 5: mismatch in configuration
      delay(10000);
    }
  }
}
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
 
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
   
    Serial.print("Message:");

    String message;
    for (int i = 0; i < length; i++) {
      message = message + (char)payload[i];  //Conver *byte to String
    }
//   Serial.print(message); // print the message, For debug purposes

   if( strstr(topic, "room1/BuiltInLamp") ){
       // turn built in led on or off
      if(message == "#on") {
        digitalWrite(LED,LOW);
        Serial.println("nodeMCU built in lamp turned on");
      }
      else if(message == "#off") {
        digitalWrite(LED,HIGH);
        Serial.println("nodeMCU built in lamp turned off");
      }else{
        Serial.print("Unsupported query: ");
        Serial.println(message);
      }
   }else if( strstr(topic,"room1/lamp") ){
      // change relay connected to 
      // we used one channel solid state relay (g3mb-202p relay)
      if(message == "#on") {
        digitalWrite(relay,LOW);
        Serial.println("room1 lamp turned on");
      }
      else if(message == "#off") {
        digitalWrite(relay,HIGH);
        Serial.println("room1 lamp turned off");
      }else{
        Serial.print("Unsupported query: ");
        Serial.println(message);
      }
   }else{
      Serial.print("Unsupported Topic: ");
      Serial.println(topic);
   }
 
  Serial.println();
  Serial.println("-----------------------");  
}
 
void loop() {
  client.loop();
}
