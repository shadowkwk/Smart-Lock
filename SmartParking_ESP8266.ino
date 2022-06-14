#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "HTTPSRedirect.h"

const char *GScriptId = "AKfycbz-ro7Q5IMcts_W3Avygxi7-1fOjS8M91FO8gqeUpFsKbWW-WqhYVWF2PNiDnzjbUFMfg";
String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload1 = "";
const char* host = "script.google.com";
const int httpsPort = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client1 = nullptr;
int T1 = 0;
int T2 = 0;
int command;

const char* ssid = "LAPTOP-F53JT4F7-8109";
const char* password =  "1Bp0-536";
const char* mqttServer = "farmer.cloudmqtt.com";
const int mqttPort = 11313;
const char* mqttUser = "kenny";
const char* mqttPassword = "1991";
WiFiClient  espClient;
PubSubClient client(espClient);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  client.publish("RFID", "Hello from ESP8266");
  client.subscribe("RFID");

  client1 = new HTTPSRedirect(httpsPort);
  client1->setInsecure();
  client1->setPrintResponseBody(true);
  client1->setContentTypeHeader("application/json");
  Serial.println("Connecting to ");
  Serial.println(host);
  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client1->connect(host, httpsPort);
    if (retval == 1) {
      flag = true;
      Serial.println("Connected");
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag) {
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    return;
  }
  delete client1;    // delete HTTPSRedirect object
  client1 = nullptr;
}


void callback(char* topic, byte* payload, unsigned int length) {
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0)
  {
    command = Serial.read();
    if (command == 48) { //wrong rfid
      T1 = 0;
      T2 = 1;
    }
    if (command == 49) {	//correct rfid
      T1 = 1;
      T2 = 1;
    }
    if (command == 50) {  //correct pw
      T1 = 1;
      T2 = 2;
    }
    if (command == 51) {  //incorrect pw
      T1 = 0;
      T2 = 2;
    }
    if (command == 48 || command == 49 || command == 50 || command == 51) {
      static bool flag = false;
      if (!flag) {
        client1 = new HTTPSRedirect(httpsPort);
        client1->setInsecure();
        flag = true;
        client1->setPrintResponseBody(true);
        client1->setContentTypeHeader("application/json");
      }
      if (client1 != nullptr) {
        if (!client1->connected()) {
          client1->connect(host, httpsPort);
        }
      }
      else {
        Serial.println("Error creating client object!");
      }

      // Create json object string to send to Google Sheets
      payload1 = payload_base + "\"" + T1 + "," + T2  + "\"}";

      // Publish data to Google Sheets
      //Serial.println("Publishing data...");
      //Serial.println(payload);
      if (client1->POST(url, host, payload1)) {
        // do stuff here if publish was successful
      }
      else {
        // do stuff here if publish was not successful
        Serial.println("Error while connecting");
      }
      delay(2000);
    }
  }

  client.loop();
}
