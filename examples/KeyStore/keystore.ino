/*
  keyStore.ino - Example sketch program to show the usage for configuration retrieval and store for BoT Service.
  Created by Lokesh H K, April 9, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include <Storage.h>

KeyStore store;

void setup(){
  //Serial.begin(115200);
  store.loadJSONConfiguration();
  if(store.isJSONConfigLoaded()){
    LOG("\n WiFi SSID: %s", store.getWiFiSSID());
    LOG("\n WiFi Passwd: %s", store.getWiFiPasswd());
    LOG("\n Maker ID: %s", store.getMakerID());
    LOG("\n Device ID: %s", store.getDeviceID());
  }
  store.retrieveAllKeys();
  if(store.isPrivateKeyLoaded()){
    LOG("\n Private Key Contents: \n%s\n", store.getDevicePrivateKey());
  }
  if(store.isPublicKeyLoaded()){
    LOG("\n Public Key Contents: \n%s\n", store.getDevicePublicKey());
  }
  if(store.isAPIKeyLoaded()){
    LOG("\n API Key Contents: \n%s\n", store.getAPIPublicKey());
  }
}

void loop(){

}
