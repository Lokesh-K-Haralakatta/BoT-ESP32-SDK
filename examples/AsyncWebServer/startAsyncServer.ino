/*
  startAsyncServer.ino - Example sketch program to show the usage for BoT-ESP32-SDK Server.
  Created by Lokesh H K, April 7, 2019.
  Released into the repository BoT-ESP32-SDK.
*/
#include <Webserver.h>
#define WIFI_SSID "LJioWiFi"
#define WIFI_PASSWD "adgjmptw"

//Webserver server(false,WIFI_SSID, WIFI_PASSWD);
Webserver server(true);
KeyStore* store;
void setup()
{
  server.connectWiFi();
  if(server.isWiFiConnected()){

    store = KeyStore::getKeyStoreInstance();
    store->initializeEEPROM();
    //For device state as NEW, /pairing will wait for maxPairAttempts and return the response
    //store->setDeviceState(DEVICE_NEW);

    //For triggering the action, hit /actions with post method

    //For device with NEW, PAIRED state, we should see 403 Device not activated message as response
    //store->setDeviceState(DEVICE_NEW);

    //For device with MULTIPAIR state, json missing alternateID should get 400 Missing parameter
    //store->setDeviceState(DEVICE_MULTIPAIR);

    //For ACTIVE state device, missing actionID in json body should return 400 Missing parameter
    store->setDeviceState(DEVICE_ACTIVE);

    //For Active state device, json body containing invalid actionID, we should see 404

    //For invalid combination of makerID, device ID and actionID, we should see 503 code

    //If everything OK, we should see 200 Ok with Action triggerred message

    server.startServer();
  }
  else {
    LOG("\nESP-32 board not connected to WiFi Network");
  }

}

void loop()
{
  /*if(server.isServerAvailable()){
    server.blinkLED();
  }*/

}
