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

void setup() {

  store = KeyStore :: getKeyStoreInstance();
  store->loadJSONConfiguration();

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  //Get WiFi Credentials from given configuration
  //const char* WIFI_NAME = store->getWiFiSSID();
  //const char* WIFI_PASSWORD = store->getWiFiPasswd();

  //Provide custom WiFi Credentials
  const char* WIFI_NAME = "LJioWiFi";
  const char* WIFI_PASSWORD = "adgjmptw";

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

  //Enable HTTPS for BoTService Calls
  //Override HTTPS value given in configuration.json
  store->setHTTPS(true);

  //Create BoT Service Instance
  bot = new BoTService();

  //GET Pairing Status
  LOG("\nPair Status: %s", bot->get("/pair").c_str());

  //Deallocate
  delete bot;

  //Create BoT Service Instance
  bot = new BoTService();

  //GET Actions defined in Maker Portal
  LOG("\nActions: %s", bot->get("/actions").c_str());

  //Deallocate
  delete bot;

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

  //Create BoT Service Instance
  bot = new BoTService();

  LOG("\nResponse from triggering action: %s", bot->post("/actions",payload).c_str());

  //Deallocate
  delete bot;
}

void loop() {
  digitalWrite(ledPin, LOW);
  delay(1000);
  digitalWrite(ledPin, HIGH);
  delay(1000);
}
