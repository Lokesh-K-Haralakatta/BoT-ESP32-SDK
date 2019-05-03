/*
  startAsyncServer.ino - Example sketch program to show the usage for BoT-ESP32-SDK Server.
  Created by Lokesh H K, April 7, 2019.
  Released into the repository BoT-ESP32-SDK.
*/
#include <Webserver.h>
#define WIFI_SSID "LJioWiFi"
#define WIFI_PASSWD "adgjmptw"

Webserver server(false,WIFI_SSID, WIFI_PASSWD);
//Webserver server(true);
KeyStore* store;
HTTPClient* httpClient = NULL;

//Webserver Port
const int port = 3001;

void setup()
{
  server.connectWiFi();
  if(server.isWiFiConnected()){

    store = KeyStore::getKeyStoreInstance();
    store->initializeEEPROM();

    //Start Async Webserver on ESP32 board
    server.startServer();

    //Instantiate HTTPClient Instance
    httpClient = new HTTPClient();
  }
  else {
    LOG("\nESP-32 board not connected to WiFi Network");
  }

}

void loop()
{
   if(server.isServerAvailable()){
      //Retrieve defined actions for given makerID
      httpClient->begin((server.getBoardIP()).toString(),3001,"/actions");
      int httpCode = httpClient->GET();
      String payload = httpClient->getString();
      httpClient->end();

      if(httpCode == 200){
        LOG("\nActions Retrieved from BoT Service: %s", payload.c_str());
      }
      else {
        LOG("\nActions Retrieval failed with httpCode - %d", httpCode);
      }

      //For device state as NEW, /pairing will wait for maxPairAttempts and return the response as Not Paired
      //Make sure to have new unpaired deviceID in configuration file flashed to ESP32 board
      store->setDeviceState(DEVICE_NEW);
      httpClient->begin((server.getBoardIP()).toString(),port,"/pairing");
      httpCode = httpClient->GET();
      payload = httpClient->getString();
      httpClient->end();

      if(httpCode == 200){
         if(store->getDeviceState() == DEVICE_ACTIVE){
           LOG("\nDevice Activation Successful");
         }
         else
           LOG("\nDevice Not Activated, try again");
     }
     else {
        LOG("\nCalling /pairing failed with httpCode - %d", httpCode);
     }
    }
    else {
        LOG("\nstartAsyncServer: Webserver not available on ESP32 board!");
    }
  delay(5*60*1000);
}
