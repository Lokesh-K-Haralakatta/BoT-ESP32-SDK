/*
  botService.ino - Example sketch program to show the usage for BoTService Component of ESP-32 SDK.
  Created by Lokesh H K, April 12, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include <BoTService.h>
#include <Storage.h>
#include <Webserver.h>

BoTService* bot;
KeyStore* store;
Webserver *server = NULL;

void setup() {

  store = KeyStore :: getKeyStoreInstance();
  store->loadJSONConfiguration();

  //Get WiFi Credentials from given configuration
  //const char* WIFI_SSID = store->getWiFiSSID();
  //const char* WIFI_PASSWD = store->getWiFiPasswd();

  //Provide custom WiFi Credentials
  const char* WIFI_SSID = "LJioWiFi";
  const char* WIFI_PASSWD = "adgjmptw";

  //Override HTTPS
  store->setHTTPS(true);

  //Instantiate Webserver by using the custom WiFi credentials
  bool loadConfig = false;
  int logLevel = BoT_INFO;
  server = new Webserver(loadConfig,WIFI_SSID, WIFI_PASSWD,logLevel);

  //Enable board to connect to WiFi Network
  server->connectWiFi();

  //Get BoT Service Instance
  bot = BoTService::getBoTServiceInstance();

}

void loop() {
  #ifndef DEBUG_DISABLED
    Debug.handle();
  #endif

  //Proceed further if board connects to WiFi Network
  if(server->isWiFiConnected()){
    //GET Pairing Status
    debugI("\nPair Status: %s", bot->get("/pair")->c_str());
    //GET Actions defined in Maker Portal
    debugI("\nActions: %s", bot->get("/actions")->c_str());

    //Prepare JSON Data to trigger an Action through POST call
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& doc = jsonBuffer.createObject();
    JsonObject& botData = doc.createNestedObject("bot");
    botData["deviceID"] = store->getDeviceID();
    botData["actionID"] = "A42ABD19-3226-47AB-8045-8129DBDF117E";
    botData["queueID"] = store->generateUuid4();

    char payload[200];
    doc.printTo(payload);
    debugI("\nMinified JSON Data to trigger Action: %s", payload);

    debugI("\nResponse from triggering action: %s", bot->post("/actions",payload).c_str());
  }
  else {
  LOG("\nsdkSample: ESP-32 board not connected to WiFi Network, try again");
  //Enable board to connect to WiFi Network
  server->connectWiFi();
  }

  delay(1*60*1000);
}
