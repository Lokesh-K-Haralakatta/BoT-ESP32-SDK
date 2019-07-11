/*
  ControllerService.cpp - Class and Methods to interface between Webserver End Points
                          and the backend service components
  Created by Lokesh H K, April 24, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include "ControllerService.h"

ControllerService :: ControllerService(){
  store = KeyStore :: getKeyStoreInstance();
}

void ControllerService :: getActions(AsyncWebServerRequest *request){
  ActionService* actionService = new ActionService();
  String* response = actionService->getActions();
  delete actionService;

  if(response == NULL){
    DynamicJsonBuffer jsonBuffer;
    JsonObject& doc = jsonBuffer.createObject();
    doc["message"] = "Unable to retrieve actions";
    char body[100];
    doc.printTo(body);
    jsonBuffer.clear();
    debugE("\nControllerService :: getActions: %s", body);
    request->send(503, "application/json", body);
  }
  else {
    const char* responseString = response->c_str();
    debugD("\nControllerService :: getActions: %s", responseString);
    request->send(200, "application/json", responseString);
  }
}

void ControllerService :: getQRCode(AsyncWebServerRequest *request){
  //Check QR Code generation qrCodeStatus
  bool qrCodeStatus = store->isQRCodeGeneratedandSaved();
  if(!qrCodeStatus){
    debugD("\nControllerService :: getQRCode: Generating QR Code and saving to SPIFFS");
    qrCodeStatus = store->generateAndSaveQRCode();
  }
  if(qrCodeStatus){
    debugI("\nControllerService :: getQRCode: QR Code exists on SPIFFS, serving through webresponse");
    request->send(SPIFFS,QRCODE_FILE,"image/svg+xml");
  }
  else{
    debugE("\nControllerService :: getQRCode: QR Code not available on SPIFFS, returning 404 as web response");
    request->send(404,"text/plain","QR Code not available on SPIFFS");
  }
}

void ControllerService :: pairDevice(AsyncWebServerRequest *request){
  store->initializeEEPROM();
  DynamicJsonBuffer jsonBuffer;
  JsonObject& doc = jsonBuffer.createObject();
  char body[100];

  if(store->getDeviceState() > DEVICE_NEW){
    doc["message"] = "Device is already paired";
    doc.printTo(body);
    jsonBuffer.clear();
    debugW("\nControllerService :: pairDevice: %s", body);
    request->send(403, "application/json", body);
  }
  else {
    PairingService* pairService = new PairingService();
    pairService->pairDevice();
    delete pairService;

    int deviceState = store->getDeviceState();
    debugD("\nControllerService :: pairDevice: Device state after return from pairService->pairDevice() : %d",deviceState);
    if( deviceState != DEVICE_NEW){
      doc["message"] = "Device pairing successful";
      doc.printTo(body);
      jsonBuffer.clear();
      debugD("\nControllerService :: pairDevice: %s", body);
      request->send(200, "application/json", body);
    }
    else {
      doc["message"] = "Unable to pair device";
      doc.printTo(body);
      jsonBuffer.clear();
      debugE("\nControllerService :: pairDevice: %s", body);
      request->send(503, "application/json", body);
    }
  }
}

void ControllerService :: triggerAction(AsyncWebServerRequest *request, JsonVariant &json){
  store->initializeEEPROM();
  DynamicJsonBuffer jsonBuffer;
  JsonObject& doc = jsonBuffer.createObject();
  char body[100];

  if(store->getDeviceState() < DEVICE_ACTIVE){
    doc["message"] = "Device not activated";
    doc.printTo(body);
    jsonBuffer.clear();
    debugE("\nControllerService :: triggerAction: %s", body);
    request->send(403, "application/json", body);
  }
  else {
    JsonObject& jsonObj = json.as<JsonObject>();

    if(jsonObj.containsKey("actionID") == false){
      doc["message"] = "Missing parameter `actionID`";
      doc.printTo(body);
      jsonBuffer.clear();
      debugE("\nControllerService :: triggerAction: %s", body);
      request->send(400, "application/json", body);
    }
    else if((store->getDeviceState() == DEVICE_MULTIPAIR) &&
            (jsonObj.containsKey("alternativeID") == false)){
      doc["message"] = "Missing parameter `AlternativeID`";
      doc.printTo(body);
      jsonBuffer.clear();
      debugE("\nControllerService :: triggerAction: %s", body);
      request->send(400, "application/json", body);
    }
    else {
      const char* actionID = (jsonObj.containsKey("actionID"))?jsonObj.get<const char*>("actionID"):NULL;
      const char* value = (jsonObj.containsKey("value"))?jsonObj.get<const char*>("value"):NULL;
      const char* altID = (jsonObj.containsKey("alternativeID"))?jsonObj.get<const char*>("alternativeID"):NULL;

      ActionService* actionService = new ActionService();
      String response = actionService->triggerAction(actionID, value, altID);
      delete actionService;
      debugD("\nControllerService :: triggerAction: Response: %s", response.c_str());

      if(response.indexOf("OK") != -1) {
        doc["message"] = "Action triggered successful";
        doc.printTo(body);
        jsonBuffer.clear();
        debugD("\nControllerService :: triggerAction: %s", body);
        request->send(200, "application/json", body);
      }
      else if(response.indexOf("Action not found") != -1){
        doc["message"] = "Action not triggered as its not found";
        doc.printTo(body);
        jsonBuffer.clear();
        debugE("\nControllerService :: triggerAction: %s", body);
        request->send(404, "application/json", body);
      }
      else {
        doc["message"] = "Action triggerring failed, check parameters and try again";
        doc.printTo(body);
        jsonBuffer.clear();
        debugE("\nControllerService :: triggerAction: %s", body);
        request->send(503, "application/json", body);
      }
    }
  }
}
