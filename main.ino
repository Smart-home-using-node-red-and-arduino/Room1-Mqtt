#include "credentials.c"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <string.h>
#include <Ticker.h> // ticker library to check mqtt connection status ( if disconnected then try to reconnect )


#define LED 2
#define relay D1


// get credentials from c function
// function is declared in credentials.c 
struct credentials cred = getCredentials();



 
WiFiClient espClient;
PubSubClient client(espClient);
Ticker check_Mqtt_connection; // Check if the connection is lost try to reconnect ( maybe mqtt server was shutdown unexpectedly or restarted! )

void checkMqttServer();
 
void setup() {
  pinMode(LED,OUTPUT);
  pinMode(relay,OUTPUT);
  Serial.begin(115200);

  check_Mqtt_connection.attach(180, checkMqttServer); // check mqtt server connection every 3 minutes (3*60 seconds)



  // print credentials for debug purposes
  /* Serial.println("credentials");
  Serial.println(cred.ssid);
  Serial.println(cred.password);
  Serial.println(cred.mqtt_host_ip);
  Serial.println(cred.mqtt_port);  */

  // setup OTA
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(cred.ssid, cred.password);
 
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
//  Serial.println(WiFi.SSID());  // print for debug 
  digitalWrite(LED,LOW);
 
  client.setServer(cred.mqtt_host_ip, cred.mqtt_port);
  client.setCallback(mqtt_callback);
 
  while (!client.connected()) {
    digitalWrite(LED,HIGH);
    Serial.println("Connecting to MQTT...");

    if (client.connect("nodeMcu")) {
 
      Serial.println("connected");  
 
    } else {
      digitalWrite(LED,LOW);
      
      Serial.print("failed with state ");
      Serial.println(client.state());  //If you get state 5: mismatch in configuration
      delay(500);
      digitalWrite(LED,HIGH);
      delay(500);
 
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
  ArduinoOTA.handle();
  client.loop();
}
