/*
  ActivationService.cpp - Class and Methods to send activation request and
                    poll for activation status with BoT Service
  Created by Lokesh H K, April 17, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include "ActivationService.h"

ActivationService :: ActivationService(){
  store = KeyStore :: getKeyStoreInstance();
}

bool ActivationService :: pollActivationStatus(){
  debugD("\nActivationService :: pollActivationStatus: Started polling BoT for activation status for the device...");
  int counter = 1;
  String response;
  do {
    debugD("\nActivationService :: pollActivationStatus: Checking activation status, attempt %d of %d", counter,MAXIMUM_TRIES);
    response = sendActivationRequest();
    if(response.equals("")){
      return true;
    }
    ++counter;
    delay(POLLING_INTERVAL_IN_MILLISECONDS);
  }while(counter <= MAXIMUM_TRIES);

   return false;
}

String ActivationService :: sendActivationRequest(){
  const char* deviceID = store->getDeviceID();

  DynamicJsonBuffer jsonBuffer;
  JsonObject& doc = jsonBuffer.createObject();
  JsonObject& botData = doc.createNestedObject("bot");
  botData["deviceID"] = deviceID;

  char payload[100];
  doc.printTo(payload);
  debugD("\nActivationService :: sendActivationRequest: Minified JSON payload to send: %s", payload);

  BoTService *bot = new BoTService();
  String response = bot->post(ACTIVATION_END_POINT,payload);
  debugD("\nActivationService :: sendActivationRequest: Response from bot->post: %s",response.c_str());
  delete bot;

  return response;
}

void ActivationService :: activateDevice(){
  store->initializeEEPROM();
  if(pollActivationStatus() == true){
    store->setDeviceState(DEVICE_ACTIVE);
    debugI("\nActivationService :: activateDevice: Activation successful. Triggering actions enabled");
  }
  else {
    debugW("\nActivationService :: activateDevice: Unable to activate device, try activating again");
  }
}
