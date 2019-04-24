/*
  ControllerService.cpp - Class and Methods to interface between Webserver End Points
                          and the backend service components
  Created by Lokesh H K, April 24, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include "ControllerService.h"

ControllerService :: ControllerService(){
  store = KeyStore :: getKeyStoreInstance();
  pairService = new PairingService();
  actionService = new ActionService();
  configService = new ConfigurationService();
}

void ControllerService :: getActions(AsyncWebServerRequest *request){
  String response = actionService->getActions();
  DynamicJsonBuffer jsonBuffer;
  JsonObject& doc = jsonBuffer.createObject();

  if(response.equals("")){
    doc["message"] = "Unable to retrieve actions";
    char body[100];
    doc.printTo(body);
    LOG("\nControllerService :: getActions: %s", body);
    request->send(503, "application/json", body);
  }
  else {
    const char* responseString = response.c_str();
    LOG("\nControllerService :: getActions: %s", responseString);
    request->send(200, "application/json", responseString);
  }
}

void ControllerService :: pairDevice(AsyncWebServerRequest *request){
  store->initializeEEPROM();
  DynamicJsonBuffer jsonBuffer;
  JsonObject& doc = jsonBuffer.createObject();
  char body[100];

  if(store->getDeviceState() != DEVICE_NEW){
    doc["message"] = "Device is already paired";
    doc.printTo(body);
    LOG("\nControllerService :: pairDevice: %s", body);
    request->send(403, "application/json", body);
  }
  else {
    pairService->pairDevice();
    if(store->getDeviceState() != DEVICE_NEW){
      doc["message"] = "Device pairing successful";
      doc.printTo(body);
      LOG("\nControllerService :: pairDevice: %s", body);
      request->send(200, "application/json", body);
    }
    else {
      doc["message"] = "Unable to pair device";
      doc.printTo(body);
      LOG("\nControllerService :: pairDevice: %s", body);
      request->send(503, "application/json", body);
    }
  }
}
