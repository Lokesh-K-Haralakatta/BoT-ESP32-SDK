/*
  botService.ino - Example sketch program to show the usage for BoTService Component of ESP-32 SDK.
  Created by Lokesh H K, April 12, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include <BoTService.h>

//WiFi Network Credentials
const char* WIFI_NAME= "PJioWiFi";
const char* WIFI_PASSWORD = "qwertyuiop";

//Onboard LED Pin
int ledPin = 2;

BoTService* bot;

const char* host = "api-dev.bankingofthings.io";
int port = 80;
const char* uri = "/bot_iot";

void setup() {

  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  LOG("\nConnecting to %s", WIFI_NAME);
  WiFi.disconnect();
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    LOG("\nTrying to connect to Wifi Network - %s", WIFI_NAME);
  }

  LOG("\nSuccessfully connected to WiFi network - %s", WIFI_NAME);
  LOG("\nIP address: ");
  Serial.println(WiFi.localIP());

  bot = new BoTService(host,uri,port);
  //const char* mID = "469908A3-8F6C-46AC-84FA-4CF1570E564B";
  //const char* dID = "eb25d0ba-2dcd-4db2-8f96-a4fbe54dbffc";

  LOG("\nPair Status: %s", bot->get("/pair").c_str());
  LOG("\nActions: %s", bot->get("/actions").c_str());
}

void loop() {
  digitalWrite(ledPin, LOW);
  delay(1000);
  digitalWrite(ledPin, HIGH);
  delay(1000);
}
