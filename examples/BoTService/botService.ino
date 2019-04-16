/*
  botService.ino - Example sketch program to show the usage for BoTService Component of ESP-32 SDK.
  Created by Lokesh H K, April 12, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include <BoTService.h>
#include <Storage.h>

//Onboard LED Pin
int ledPin = 2;

BoTService* bot;
KeyStore* store;

const char* host = "api-dev.bankingofthings.io";
int port = 80;
const char* uri = "/bot_iot";

void setup() {

  store = KeyStore :: getKeyStoreInstance();
  store->loadJSONConfiguration();

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  const char* WIFI_NAME = store->getWiFiSSID();
  const char* WIFI_PASSWORD = store->getWiFiPasswd();

  LOG("\nConnecting to %s", WIFI_NAME);
  //WiFi.disconnect();
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    LOG("\nTrying to connect to Wifi Network - %s", WIFI_NAME);
  }

  LOG("\nSuccessfully connected to WiFi network - %s", WIFI_NAME);
  LOG("\nIP address: ");
  Serial.println(WiFi.localIP());

  //bot = new BoTService(host,uri,port);
  bot = new BoTService();

  //Sample GET calls
  LOG("\nPair Status: %s", bot->get("/pair").c_str());
  LOG("\nActions: %s", bot->get("/actions").c_str());

  //Prepare JSON Data to trigger an Action through POST call
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& doc = jsonBuffer.createObject();
  JsonObject& botData = doc.createNestedObject("bot");
  botData["deviceID"] = store->getDeviceID();
  botData["actionID"] = "E6509B49-5048-4151-B965-BB7B2DBC7905";
  botData["queueID"] = store->getQueueID();

  char payload[200];
  doc.printTo(payload);
  LOG("\nMinified JSON Data to trigger Action: %s", payload);

  LOG("\nResponse from trigering action: %s", bot->post("/actions",payload).c_str());
}

void loop() {
  digitalWrite(ledPin, LOW);
  delay(1000);
  digitalWrite(ledPin, HIGH);
  delay(1000);
}
