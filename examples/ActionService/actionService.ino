/*
  actionService.ino - Example sketch program to show the usage for ActionService Component of ESP-32 SDK.
  Created by Lokesh H K, April 17, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include <ActionService.h>
#include <Webserver.h>

ActionService* actService;
KeyStore* store;
Webserver *server;

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

  actService = new ActionService();
}

void loop() {
  //Proceed further if board connects to WiFi Network
  if(server->isWiFiConnected()){
    //GET Actions for given device from BoT Service
    if(actService->getActions() != NULL){
      debugI("\nactionService: Actions retrieval from server is success..");
    }
    else {
      debugE("\nactionService: Actions retrieval from server failed...");
    }

    //Trigger an action defined with the deviceID
    const char* actionID = "A42ABD19-3226-47AB-8045-8129DBDF117E";
    debugI("\nactionService: Response from triggering action: %s", actService->triggerAction(actionID).c_str());

  }
  else {
    LOG("\nactionService: ESP-32 board not connected to WiFi Network, try again");
    //Enable board to connect to WiFi Network
    server->connectWiFi();
  }

  #ifndef DEBUG_DISABLED
    Debug.handle();
  #endif

  delay(1*60*1000);
}
