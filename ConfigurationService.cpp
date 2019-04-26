/*
  ConfigurationService.cpp - Class and Methods to configure the device specifics
                          and prepare environment to perform required operation
  Created by Lokesh H K, April 22, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include "ConfigurationService.h"

ConfigurationService :: ConfigurationService(){
  store = KeyStore :: getKeyStoreInstance();
  deviceInfo = NULL;
  pairService = new PairingService();
  activateService = new ActivationService();
  actionService = new ActionService();
}

void ConfigurationService :: initialize(){
  store->loadJSONConfiguration();
  store->initializeEEPROM();
  store->retrieveAllKeys();
  //generateAndSaveQRCode();
  store->setDeviceState(DEVICE_NEW);
  LOG("\nConfigurationService :: initialize: Configuration successfully initialized");
}

String* ConfigurationService :: getDeviceInfo(){
  if(deviceInfo != NULL){
    delete deviceInfo;
    deviceInfo = NULL;
  }

  if(store->isJSONConfigLoaded() && store->isPublicKeyLoaded()){
    LOG("\nConfigurationService :: getDeviceInfo: Getting device specific data");
    const char* deviceID = store->getDeviceID();
    const char* makerID = store->getMakerID();
    const char* publicKey = store->getDevicePublicKey();

    DynamicJsonBuffer jsonBuffer;
    JsonObject& doc = jsonBuffer.createObject();
    doc["deviceID"] = deviceID;
    doc["makerID"] = makerID;
    doc["publicKey"] = publicKey;

    if (store->getDeviceState() == DEVICE_MULTIPAIR) {
      doc["multipair"] = 1;
      doc["alternativeID"] = store->getAlternateDeviceID();
    }

    char dInfo[1024];
    doc.printTo(dInfo);
    LOG("\nConfigurationService :: getDeviceInfo: Data: %s", dInfo);
    LOG("\nConfigurationService :: getDeviceInfo: Length: %d", strlen(dInfo));

    deviceInfo = new String(dInfo);
  }
  return deviceInfo;
}

void ConfigurationService :: configureDevice(){
  switch (store->getDeviceState()) {
      case DEVICE_NEW:
          LOG("\nConfigurationService :: configureDevice: Device not paired yet, Initializing pairing...");
          pairService->pairDevice();
          break;
      case DEVICE_PAIRED:
          LOG("\nConfigurationService :: configureDevice: Device paired but not activated, Initializing activation process...");
          activateService->activateDevice();
          break;
      case DEVICE_ACTIVE:
          LOG("\nConfigurationService :: configureDevice: Device is already active");
          LOG("\nConfigurationService :: configureDevice: %s", (actionService->getActions()).c_str());
          break;
  }
}
