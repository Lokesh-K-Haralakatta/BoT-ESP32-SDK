/*
  Storage.cpp - KeyStore Class Methods definition to retrieve and store configuration data for BoT Service
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
  https = NULL;
  makerID = NULL;
  deviceID = NULL;
  altDeviceID = NULL;
  queueID = NULL;
  privateKey = NULL;
  publicKey = NULL;
  apiKey = NULL;
  caCert = NULL;
  jsonCfgLoadStatus = NOT_LOADED;
  privateKeyLoadStatus = NOT_LOADED;
  publicKeyLoadStatus = NOT_LOADED;
  apiKeyLoadStatus = NOT_LOADED;
  caCertLoadStatus = NOT_LOADED;
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
  LOG("\nKeyStore :: loadJSONConfiguration: Loading given configuration from file - %s",JSON_CONFIG_FILE);
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

    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject(buffer);
    if(! json.success()){
      jsonCfgLoadStatus = NOT_LOADED;
      LOG("\nKeyStore :: loadJSONConfiguration: Failed to parse json configuration data");
      return;
    }

    const char* ssid = json["wifi_ssid"] | "wifi_ssid";
    if(ssid != nullptr){
      LOG("\nKeyStore :: loadJSONConfiguration: Parsed WiFi SSID from configuration: %s",ssid);
      wifiSSID = new String(ssid);
    }

    const char* passwd = json["wifi_passwd"] | "wifi_passwd";
    if(passwd != nullptr){
      LOG("\nKeyStore :: loadJSONConfiguration: Pasred WiFi Password from configuration: %s",passwd);
      wifiPASSWD = new String(passwd);
    }

    const char* httpsFlag = json["https"] | "true";
    if(httpsFlag != nullptr){
      LOG("\nKeyStore :: loadJSONConfiguration: Pasred HTTPS Flag from configuration: %s",httpsFlag);
      https = new String(httpsFlag);
    }

    const char* mId = json["maker_id"] | "maker_id";
    if(mId != nullptr){
      LOG("\nKeyStore :: loadJSONConfiguration: Pasred MakerID from configuration: %s",mId);
      makerID = new String(mId);
    }

    const char* dId = json["device_id"] | "device_id";
    if(dId != nullptr){
      LOG("\nKeyStore :: loadJSONConfiguration: Pasred deviceID from configuration: %s",dId);
      deviceID = new String(dId);
    }

    const char* adId = json["alt_device_id"] | "alt_device_id";
    if(adId != nullptr){
      LOG("\nKeyStore :: loadJSONConfiguration: Pasred alternate deviceID from configuration: %s",adId);
      altDeviceID = new String(adId);
    }

    const char* qId = json["queue_id"] | "queue_id";
    if(qId != nullptr){
      LOG("\nKeyStore :: loadJSONConfiguration: Pasred queueID from configuration: %s",qId);
      queueID = new String(qId);
    }

    delete buffer;
    jsonCfgLoadStatus = LOADED;
    LOG("\nKeyStore :: loadJSONConfiguration: Configuration loaded from %s file",JSON_CONFIG_FILE);
  }
}

void KeyStore :: setHTTPS(const bool httpsFlag){
  if(https != NULL)
    delete https;
    
  if(httpsFlag)
    https = new String("true");
  else
    https = new String("false");
}

const bool KeyStore :: getHTTPS(){
  if(https != NULL && https->equalsIgnoreCase("true"))
    return true;
  else
    return false;
}

const char* KeyStore :: getWiFiSSID(){
  return (wifiSSID != NULL) ? wifiSSID->c_str():NULL;
}

const char* KeyStore :: getWiFiPasswd(){
  return (wifiPASSWD != NULL) ? wifiPASSWD->c_str():NULL;
}

const char* KeyStore :: getMakerID(){
  return (makerID != NULL) ? makerID->c_str() : NULL;
}

const char* KeyStore :: getDeviceID(){
  return (deviceID != NULL) ? deviceID->c_str() : NULL;
}

const char* KeyStore :: getAlternateDeviceID(){
  return (altDeviceID != NULL) ? altDeviceID->c_str() : NULL;
}

const char* KeyStore :: getQueueID(){
  return (queueID != NULL) ? queueID->c_str() : NULL;
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

bool KeyStore :: isCACertLoaded(){
  return((caCertLoadStatus == LOADED)?true:false);
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
  if(!isCACertLoaded()){
    loadFileContents(CA_CERT_FILE,4);
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
      case 4: caCert = new String(buffer);
              caCertLoadStatus = LOADED;
              break;
    }

    delete buffer;
    LOG("\nKeyStore :: loadFileContents: Key Contents loaded from file - %s", filePath);
}

const char* KeyStore :: getDevicePrivateKey(){
  if(isPrivateKeyLoaded()){
    return privateKey->c_str();
  }
  return NULL;
}

const char* KeyStore :: getDevicePublicKey(){
  if(isPublicKeyLoaded()){
    return publicKey->c_str();
  }
  return NULL;
}

const char* KeyStore :: getAPIPublicKey(){
  if(isAPIKeyLoaded()){
    return apiKey->c_str();
  }
  return NULL;
}

const char* KeyStore :: getCACert(){
  if(isCACertLoaded()){
    return caCert->c_str();
  }
  return NULL;
}

std::vector <struct Action>  KeyStore :: retrieveActions(){
  //Clear previous actions details
  if(!actionsList.empty()){
    actionsList.clear();
    LOG("\nKeyStore :: retrieveActions: Cleared contents of previous actions present in ActionsList");
  }

  if(!SPIFFS.begin(true)){
    LOG("\nKeyStore :: retrieveActions: An Error has occurred while mounting SPIFFS");
    return actionsList;
  }

  if(SPIFFS.exists(ACTIONS_FILE)){
    File file = SPIFFS.open(ACTIONS_FILE, FILE_READ);
    if(!file){
      LOG("\nKeyStore :: retrieveActions: There was an error opening the file - %s for reading action details", ACTIONS_FILE);
      return actionsList;
    }

    DynamicJsonBuffer jb;
    JsonArray& actionsArray = jb.parseArray(file);
    file.close();

    if(actionsArray.success()){
      LOG("\nKeyStore :: retrieveActions: JSON Array parsed from the file - %s", ACTIONS_FILE);
      int actionsCount = actionsArray.size();
      for(byte i=0 ; i < actionsCount; i++){
        const char* actionID = actionsArray[i]["actionID"];
        const char* frequency = actionsArray[i]["frequency"];
        const unsigned long ltt = (actionsArray[i]["time"]).as<unsigned long>();
        LOG("\nKeyStore :: retrieveActions: Action - %d Details: %s - %s - %lu",i+1,actionID,frequency,ltt);

        struct Action actionItem;
        actionItem.actionID = new char[strlen(actionID)+1];
        actionItem.actionFrequency = new char[strlen(frequency)+1];
        strcpy(actionItem.actionID,actionID);
        strcpy(actionItem.actionFrequency,frequency);
        actionItem.triggeredTime = ltt;

        actionsList.push_back(actionItem);
      }
      return actionsList;
    }
    else {
      LOG("\nKeyStore :: retrieveActions: Error while parsing retrieved JSON Array from the file - %s", ACTIONS_FILE);
      return actionsList;
    }
  }
  else {
    LOG("\nFile - %s does not exist to read action details",ACTIONS_FILE);
    return actionsList;
  }
}

bool KeyStore :: saveActions(std::vector <struct Action> aList){
  if(!SPIFFS.begin(true)){
    LOG("\nKeyStore :: saveActions: An Error has occurred while mounting SPIFFS");
    return false;
  }

  File file = SPIFFS.open(ACTIONS_FILE, FILE_WRITE);
  if(!file){
    LOG("\nKeyStore :: saveActions: There was an error opening the file - %s for saving actions", ACTIONS_FILE);
    return false;
  }

  DynamicJsonBuffer jb;
  JsonArray& actionsArray = jb.createArray();

  LOG("\nKeyStore :: saveActions: Actions given to save to the file - %s : %d", ACTIONS_FILE,aList.size());
  for (std::vector<struct Action>::iterator i = aList.begin() ; i != aList.end(); ++i){
    JsonObject& obj = jb.createObject();
    obj["actionID"] = i->actionID;
    obj["frequency"] = i->actionFrequency;
    obj["time"] = i->triggeredTime;
    actionsArray.add(obj);
    LOG("\nKeyStore :: saveActions: %s : %s : %lu -- Added to actions array", i->actionID, i->actionFrequency, i->triggeredTime);
  }

  LOG("\nKeyStore :: saveActions: Number of actions in actionsArray to save to file : %d", actionsArray.size());
  int nBytes = actionsArray.measureLength();
  actionsArray.printTo(file);
  LOG("\nKeyStore :: saveActions: Number of bytes written to %s: %d",ACTIONS_FILE,nBytes);

  file.close();
  return true;
}
