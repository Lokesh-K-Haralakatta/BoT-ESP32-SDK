/*
  ConfigurationService.cpp - Class and Methods to configure the device specifics
                          and prepare environment to perform required operation
  Created by Lokesh H K, April 22, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include "ConfigurationService.h"

ConfigurationService :: ConfigurationService(){
  store = KeyStore :: getKeyStoreInstance();
  pairService = new PairingService();
  activateService = new ActivationService();
  actionService = new ActionService();
}

void ConfigurationService :: initialize(){
  store->loadJSONConfiguration();
  store->initializeEEPROM();
  store->retrieveAllKeys();
  store->generateAndSaveQRCode();
  //Check device pair type and assign state accordingly
  if(store->isDeviceMultipair())
    store->setDeviceState(DEVICE_MULTIPAIR);
  else
    store->setDeviceState(DEVICE_NEW);
  debugD("\nConfigurationService :: initialize: Device State: %d",store->getDeviceState());
  debugD("\nConfigurationService :: initialize: Configuration successfully initialized");
}

void ConfigurationService :: configureDevice(){
  switch (store->getDeviceState()) {
      case DEVICE_NEW:
          debugD("\nConfigurationService :: configureDevice: Device not paired yet, Initializing pairing...");
          pairService->pairDevice();
          break;
      case DEVICE_PAIRED:
          debugD("\nConfigurationService :: configureDevice: Device paired but not activated, Initializing activation process...");
          activateService->activateDevice();
          break;
      case DEVICE_ACTIVE:
          debugD("\nConfigurationService :: configureDevice: Device is already active");
          debugD("\nConfigurationService :: configureDevice: %s", (actionService->getActions())->c_str());
          break;
  }
}
