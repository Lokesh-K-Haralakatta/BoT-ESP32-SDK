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
    debugE("\nControllerService :: getActions: %s", body);
    request->send(503, "application/json", body);
  }
  else {
    const char* responseString = response.c_str();
    debugD("\nControllerService :: getActions: %s", responseString);
    request->send(200, "application/json", responseString);
  }
}

void ControllerService :: getQRCode(AsyncWebServerRequest *request){
  //Variables to hold return info
  int returnCode = -1;
  String contentType = "";
  String responseMsg = "";

  /*
  const char* qrCodeGenURL = "https://api.qrserver.com/v1/create-qr-code/?size=150x150&data=";
  store->retrieveAllKeys();
  const char* dInfo = (store->getDeviceInfo())->c_str();
  debugD("\nControllerService :: getQRCode: Device Info: %s",dInfo);
  String qrCodeGenLink = String(qrCodeGenURL);
  qrCodeGenLink.concat(dInfo);
  */
  String qrCodeGenLink = "https://api.qrserver.com/v1/create-qr-code/?size=150x150&data=%7B%22deviceID%22%3A%22eb25d0ba-2dcd-4db2-8f96-a4fbe54dbffc%22%2C%22name%22%3A%22BoT-ESP-32%22%2C%22makerID%22%3A%22469908A3-8F6C-46AC-84FA-4CF1570E564B%22%2C%22publicKey%22%3A%22ssh-rsa%20AAAAB3NzaC1yc2EAAAADAQABAAABAQC5hDDJ9mvJj77rV2fm6cXpklEq2lO7TDYVBWvnVdP5JJrPfwW3XGBk%2Ft7S9jmuxcq%2BwGep%2F1YELMCGenXt%2FM8Qhy0694m9gSB8aqOiNo9EC9%2BWRRjAwpV7ObeJex8EiuqP8eUe9INfTATPS3GCHfqnUJc%2Fufw652bA5HFdD3no3Vvnp0iuJwKiitvVy26mrcqhayXqDM5uzNGFLZof9On%2FGwfcDcpkKhL4LNtvWutB80M3BxY2G8UL1vT0QILln37Mm5lIHPt7JCrN8vVqwT5fBCuej5khEUsOMb9i5bzjF26CEyepZPgr%2FxdRk8sxHsCok%2F0W23zzf4iLDtVyXZJp%20lokeshkot%40hcl.com%5Cn%22%2C%22multipair%22%3A0%7D";
  debugD("\nControllerService :: getQRCode: qrCodeGenLink: %s",qrCodeGenLink.c_str());

  //request->redirect(qrCodeGenLink);
  HTTPClient* httpClient = new HTTPClient();
  httpClient->begin(qrCodeGenLink,store->getQRCACert());
  int httpCode = httpClient->GET();
  if(httpCode > 0){
    debugD("\nControllerService :: getQRCode: HTTP return code for GET call to generate QR Code: %d",httpCode);
    if(httpCode == HTTP_CODE_OK){
      uint8_t* buffer = NULL;
      debugD("\nControllerService :: getQRCode: QR Code generation is successful");
      int len = httpClient->getSize();
      debugD("\nControllerService :: getQRCode: Content-length: %d",len);
      WiFiClient* stream = httpClient->getStreamPtr();
      if(httpClient->connected() && (len>0 || len == -1)){
        size_t size = stream->available();
        debugD("\nControllerService :: getQRCode: QR Code image size: %u",size);
        if(size){
          buffer = new uint8_t[size];
          int rc = stream->readBytes(buffer,size);
          debugD("\nControllerService :: getQRCode: Amount of bytes read into buffer: %d",rc);
          returnCode = 200;
          contentType = contentType + "image/png";
          responseMsg = responseMsg + String((char*)buffer);
          request->send(200, "image/png", (char*)buffer);
          delete buffer;
        }
        else {
          debugD("\nControllerService :: getQRCode: QR Code Data not available");
          returnCode = 404;
          contentType = contentType + "text/plain";
          responseMsg = responseMsg + "QR Code Data not available";
          //request->send(404, "text/plain", "QR Code Data not available");
        }
      }
      else {
        debugD("\nControllerService :: getQRCode: No Content Available");
        returnCode = 204;
        contentType = contentType + "text/plain";
        responseMsg = responseMsg + "No Content Available";
        //request->send(204, "text/plain", "No Content Available");
      }
    }
    else {
      debugD("\nControllerService :: getQRCode: HTTP GET Call to generate QR Code Failed with code: %d",httpCode);
      returnCode = httpCode;
      contentType = contentType + "text/plain";
      responseMsg = responseMsg + HTTPClient::errorToString(httpCode);
      //request->send(httpCode, "text/plain", "HTTP GET Call to generate QR Code Failed");
    }
  }
  else {
    debugD("\nControllerService :: getQRCode: HTTP GET Call to generate QR Code Failed with code: %d",httpCode);
    returnCode = httpCode;
    contentType = contentType + "text/plain";
    responseMsg = responseMsg + HTTPClient::errorToString(httpCode);
    //request->send(503, "text/plain", "Server Error in generating QR Code");
  }
  //Cleanup HTTP Client Resources
  httpClient->end();
  delete httpClient;
  //Send back the response
  request->send(returnCode, contentType.c_str(), responseMsg.c_str());
}

void ControllerService :: pairDevice(AsyncWebServerRequest *request){
  store->initializeEEPROM();
  DynamicJsonBuffer jsonBuffer;
  JsonObject& doc = jsonBuffer.createObject();
  char body[100];

  if(store->getDeviceState() != DEVICE_NEW){
    doc["message"] = "Device is already paired";
    doc.printTo(body);
    debugW("\nControllerService :: pairDevice: %s", body);
    request->send(403, "application/json", body);
  }
  else {
    pairService->pairDevice();
    int deviceState = store->getDeviceState();
    debugD("\nControllerService :: pairDevice: Device state after return from pairService->pairDevice() : %d",deviceState);
    if( deviceState != DEVICE_NEW){
      doc["message"] = "Device pairing successful";
      doc.printTo(body);
      debugD("\nControllerService :: pairDevice: %s", body);
      request->send(200, "application/json", body);
    }
    else {
      doc["message"] = "Unable to pair device";
      doc.printTo(body);
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
    debugE("\nControllerService :: triggerAction: %s", body);
    request->send(403, "application/json", body);
  }
  else {
    JsonObject& jsonObj = json.as<JsonObject>();

    if(jsonObj.containsKey("actionID") == false){
      doc["message"] = "Missing parameter `actionID`";
      doc.printTo(body);
      debugE("\nControllerService :: triggerAction: %s", body);
      request->send(400, "application/json", body);
    }
    else if((store->getDeviceState() == DEVICE_MULTIPAIR) &&
            (jsonObj.containsKey("alternativeID") == false)){
      doc["message"] = "Missing parameter `AlternativeID`";
      doc.printTo(body);
      debugE("\nControllerService :: triggerAction: %s", body);
      request->send(400, "application/json", body);
    }
    else {
      const char* actionID = (jsonObj.containsKey("actionID"))?jsonObj.get<const char*>("actionID"):NULL;
      const char* value = (jsonObj.containsKey("value"))?jsonObj.get<const char*>("value"):NULL;
      const char* altID = (jsonObj.containsKey("alternativeID"))?jsonObj.get<const char*>("alternativeID"):NULL;

      String response = actionService->triggerAction(actionID, value, altID);
      debugD("\nControllerService :: triggerAction: Response: %s", response.c_str());

      if(response.indexOf("OK") != -1) {
        doc["message"] = "Action triggered successful";
        doc.printTo(body);
        debugD("\nControllerService :: triggerAction: %s", body);
        request->send(200, "application/json", body);
      }
      else if(response.indexOf("Action not found") != -1){
        doc["message"] = "Action not triggered as its not found";
        doc.printTo(body);
        debugE("\nControllerService :: triggerAction: %s", body);
        request->send(404, "application/json", body);
      }
      else {
        doc["message"] = "Action triggerring failed, check parameters and try again";
        doc.printTo(body);
        debugE("\nControllerService :: triggerAction: %s", body);
        request->send(503, "application/json", body);
      }
    }
  }
}
