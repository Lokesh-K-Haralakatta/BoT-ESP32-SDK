/*
  pairingService.ino - Example sketch program to show the usage for
                       PairingService and ActivationService Components of ESP-32 SDK.
  Created by Lokesh H K, April 16, 2019.
  Released into the repository BoT-ESP32-SDK.
*/
#include <PairingService.h>
#include <Storage.h>
#include <Webserver.h>

PairingService* ps;
KeyStore* store;
Webserver *server = NULL;

void setup() {

  store = KeyStore :: getKeyStoreInstance();
  store->loadJSONConfiguration();
  store->initializeEEPROM();

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
  int logLevel = BoT_ERROR;
  server = new Webserver(loadConfig,WIFI_SSID, WIFI_PASSWD,logLevel);

  //Enable board to connect to WiFi Network
  server->connectWiFi();

  ps = new PairingService();

  //Call pairDevice to pair the deviceID present in JSON Cofiguration

  //For already paired deviceID, we should see pairing successful message
  //Followed by activation successful message
  //Device State after pairDevice should be DEVICE_ACTIVE i.e. 2

  //For new deviceID, set the device state to DEVICE_NEW
  //We should see waiting for 3 times to see the response status as false
  //Device State after pairDevice should be DEVICE_NEW i.e. 0

  //For all other states, the pairDevice method simply returns immediately
  //store->setDeviceState(DEVICE_PAIRED);
  //store->setDeviceState(DEVICE_ACTIVE);
  //store->setDeviceState(DEVICE_MULTIPAIR);
  //Device State after pairDevice should be same as before
}

void loop() {
  //Proceed further if board connects to WiFi Network
  if(server->isWiFiConnected()){
    store->setDeviceState(DEVICE_NEW);
    debugI("\n Given deviceID in configuration: %s", store->getDeviceID());
    debugI("\n Device State stored in EEPROM: %d", store->getDeviceState());
    debugI("\n Now trying to pair the device, followed by activating the device");

    //Should behave as mentioned above
    ps->pairDevice();

    debugI("\n Device State after pairDevice return: %d", store->getDeviceState());
  }
  else {
  LOG("\nsdkSample: ESP-32 board not connected to WiFi Network, try again");
  //Enable board to connect to WiFi Network
  server->connectWiFi();
  }

  #ifndef DEBUG_DISABLED
    Debug.handle();
  #endif

  delay(1*60*1000);
}
