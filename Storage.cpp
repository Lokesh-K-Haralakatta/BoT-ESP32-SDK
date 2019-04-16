/*
  Storage.h - KeyStore Class Methods definition to retrieve and store configuration data for BoT Service
  Created by Lokesh H K, April 9, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include "Storage.h"

KeyStore* KeyStore::store = NULL;

KeyStore* KeyStore:: getKeyStoreInstance(){
  if(store == NULL){
    store = new KeyStore();
  }

  return store;
}

KeyStore :: KeyStore(){
  Serial.begin(115200);
  wifiSSID = NULL;
  wifiPASSWD = NULL;
  makerID = NULL;
  deviceID = NULL;
  queueID = NULL;
  privateKey = NULL;
  publicKey = NULL;
  apiKey = NULL;
  jsonCfgLoadStatus = NOT_LOADED;
  privateKeyLoadStatus = NOT_LOADED;
  publicKeyLoadStatus = NOT_LOADED;
  apiKeyLoadStatus = NOT_LOADED;
}

void KeyStore :: setDeviceState(int state){
  EEPROM.write(DEVICE_STATE_ADDR, state);
  EEPROM.commit();
}

void KeyStore :: resetDeviceState(){
  EEPROM.write(DEVICE_STATE_ADDR, DEVICE_NEW);
  EEPROM.commit();
}

const int KeyStore :: getDeviceState(){
  return EEPROM.read(DEVICE_STATE_ADDR);
}

bool KeyStore :: isJSONConfigLoaded(){
  return((jsonCfgLoadStatus == LOADED)?true:false);
}

void KeyStore :: loadJSONConfiguration(){
  if(!isJSONConfigLoaded()){
    if(!SPIFFS.begin(true)){
      jsonCfgLoadStatus = NOT_LOADED;
      LOG("\nKeyStore :: loadJSONConfiguration: An Error has occurred while mounting SPIFFS");
      return;
    }

    File file = SPIFFS.open(JSON_CONFIG_FILE);
    if(!file){
      jsonCfgLoadStatus = NOT_LOADED;
      LOG("\nKeyStore :: loadJSONConfiguration: Failed to open file - %s for reading configuration",JSON_CONFIG_FILE);
      return;
    }

    size_t size = file.size();
    /*if (size > 1024){
      jsonCfgLoadStatus = NOT_LOADED;
      LOG("\nKeyStore :: loadJSONConfiguration: Configuration content is too large, make sure its < 1024 bytes");
      return;
    }*/

    char *buffer = new char[size];
    file.readBytes(buffer,size);
    file.close();

    StaticJsonBuffer <1024>jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject(buffer);
    if(! json.success()){
      jsonCfgLoadStatus = NOT_LOADED;
      LOG("\nKeyStore :: loadJSONConfiguration: Failed to parse json configuration data");
      return;
    }

    const char* ssid = json["wifi_ssid"];
    wifiSSID = new String(ssid);
    const char* passwd = json["wifi_passwd"];
    wifiPASSWD = new String(passwd);
    const char* mId = json["maker_id"];
    makerID = new String(mId);
    const char* dId = json["device_id"];
    deviceID = new String(dId);
    const char* qId = json["queue_id"];
    queueID = new String(qId);
    jsonCfgLoadStatus = LOADED;
    LOG("\nKeyStore :: loadJSONConfiguration: Configuration loaded from %s file",JSON_CONFIG_FILE);
  }
}

const char* KeyStore :: getWiFiSSID(){
  return wifiSSID->c_str();
}

const char* KeyStore :: getWiFiPasswd(){
  return wifiPASSWD->c_str();
}

const char* KeyStore :: getMakerID(){
  return makerID->c_str();
}

const char* KeyStore :: getDeviceID(){
  return deviceID->c_str();
}

const char* KeyStore :: getQueueID(){
  return queueID->c_str();
}

bool KeyStore :: isPrivateKeyLoaded(){
  return((privateKeyLoadStatus == LOADED)?true:false);
}

bool KeyStore :: isPublicKeyLoaded(){
  return((publicKeyLoadStatus == LOADED)?true:false);
}

bool KeyStore :: isAPIKeyLoaded(){
  return((apiKeyLoadStatus == LOADED)?true:false);
}

void KeyStore :: initializeEEPROM(){
  EEPROM.begin(EEPROM_SIZE);
}

void KeyStore :: retrieveAllKeys(){
  if(!isPrivateKeyLoaded()){
    loadFileContents(PRIVATE_KEY_FILE,1);
  }
  if(!isPublicKeyLoaded()){
    loadFileContents(PUBLIC_KEY_FILE,2);
  }
  if(!isAPIKeyLoaded()){
    loadFileContents(API_KEY_FILE,3);
  }
}
void KeyStore :: loadFileContents(const char* filePath, byte kType){
    if(!SPIFFS.begin(true)){
      LOG("\nKeyStore :: loadFileContents: An Error has occurred while mounting SPIFFS");
      return;
    }

    File file = SPIFFS.open(filePath);
    if(!file){
      LOG("\nKeyStore :: loadFileContents: Failed to open file - %s for reading key contents", filePath);
      return;
    }

    size_t size = file.size();
    /*if (size > 1024){
      LOG("\nKeyStore :: loadFileContents: Size Key content is %d bytes, make sure its < 1024 bytes for file - %s", filePath);
      return;
    }*/

    char *buffer = new char[size+1];
    file.readBytes(buffer,size);
    buffer[size] = '\0';
    /*int i=0;
    while(file.available()){
        buffer[i++] = file.read();
    }
    buffer[i]='\0';*/

    file.close();

    switch(kType){
      case 1: privateKey = new String(buffer);
              privateKeyLoadStatus = LOADED;
              break;
      case 2: publicKey = new String(buffer);
              publicKeyLoadStatus = LOADED;
              break;
      case 3: apiKey = new String(buffer);
              apiKeyLoadStatus = LOADED;
              break;
    }

    delete buffer;
    LOG("\nKeyStore :: loadFileContents: Key Contents loaded from file - %s", filePath);
}

const char* KeyStore :: getDevicePrivateKey(){
  return privateKey->c_str();
}

const char* KeyStore :: getDevicePublicKey(){
  return publicKey->c_str();
}

const char* KeyStore :: getAPIPublicKey(){
  return apiKey->c_str();
}
