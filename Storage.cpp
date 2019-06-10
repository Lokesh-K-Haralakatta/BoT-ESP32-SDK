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
  deviceName = NULL;
  deviceInfo = NULL;
  altDeviceID = NULL;
  privateKey = NULL;
  publicKey = NULL;
  apiKey = NULL;
  caCert = NULL;
  qrCACert = NULL;
  qrCodeStatus = false;
  jsonCfgLoadStatus = NOT_LOADED;
  privateKeyLoadStatus = NOT_LOADED;
  publicKeyLoadStatus = NOT_LOADED;
  apiKeyLoadStatus = NOT_LOADED;
  caCertLoadStatus = NOT_LOADED;
  qrCACertLoadStatus = NOT_LOADED;
}

String* KeyStore :: getDeviceInfo(){
  if(deviceInfo != NULL){
    debugD("\nKeyStore :: getDeviceInfo: Already collected device info");
    return deviceInfo;
  }

  if(isJSONConfigLoaded() && isPublicKeyLoaded()){
    debugD("\nKeyStore :: getDeviceInfo: Getting device specific data");
    const char* deviceID = getDeviceID();
    const char* deviceName = getDeviceName();
    const char* makerID = getMakerID();
    const char* publicKey = getDevicePublicKey();

    DynamicJsonBuffer jsonBuffer;
    JsonObject& doc = jsonBuffer.createObject();
    doc["deviceID"] = deviceID;
    doc["name"] = deviceName;
    doc["makerID"] = makerID;
    doc["publicKey"] = publicKey;
    doc["multipair"] = 0;
    if (getDeviceState() == DEVICE_MULTIPAIR) {
      doc["multipair"] = 1;
      doc["aid"] = getAlternateDeviceID();
    }

    char dInfo[1024];
    doc.printTo(dInfo);
    debugD("\nKeyStore :: getDeviceInfo: Data: %s", dInfo);
    debugD("\nKeyStore :: getDeviceInfo: Length: %d", strlen(dInfo));

    deviceInfo = new String(dInfo);
  }
  return deviceInfo;
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
    LOG("\nKeyStore :: loadJSONConfiguration: Loading given configuration from file - %s",JSON_CONFIG_FILE);
    if(!SPIFFS.begin(true)){
      jsonCfgLoadStatus = NOT_LOADED;
      LOG("\nKeyStore :: loadJSONConfiguration: An Error has occurred while mounting SPIFFS");
      return;
    }

  if(SPIFFS.exists(JSON_CONFIG_FILE)){
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

    const char* dName = json["device_name"] | "BoT-ESP-32";
    if(dName != nullptr){
      LOG("\nKeyStore :: loadJSONConfiguration: Pasred deviceName from configuration: %s",dName);
      deviceName = new String(dName);
    }

    const char* adId = json["alt_device_id"] | "alt_device_id";
    if(adId != nullptr){
      LOG("\nKeyStore :: loadJSONConfiguration: Pasred alternate deviceID from configuration: %s",adId);
      altDeviceID = new String(adId);
    }

    delete buffer;
    jsonCfgLoadStatus = LOADED;
    LOG("\nKeyStore :: loadJSONConfiguration: Configuration loaded from %s file",JSON_CONFIG_FILE);
   }
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

const char* KeyStore :: getDeviceName(){
  return (deviceName != NULL) ? deviceName->c_str() : NULL;
}

void KeyStore :: setDeviceName(const char* dName){
  if(deviceName != NULL){
    delete deviceName;
    deviceName = NULL;
  }
  if(dName != NULL)
    deviceName = new String(dName);
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

bool KeyStore :: isQRCACertLoaded(){
  return((qrCACertLoadStatus == LOADED)?true:false);
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
  if(!isQRCACertLoaded()){
    loadFileContents(QRC_CA_CERT,5);
  }
}

void KeyStore :: loadFileContents(const char* filePath, byte kType){
    if(!SPIFFS.begin(true)){
      #ifndef DEBUG_DISABLED
        debugE("\nKeyStore :: loadFileContents: An Error has occurred while mounting SPIFFS");
      #else
        LOG("\nKeyStore :: loadFileContents: An Error has occurred while mounting SPIFFS");
      #endif
      return;
    }

  if(SPIFFS.exists(filePath)){
    File file = SPIFFS.open(filePath);
    if(!file){
      #ifndef DEBUG_DISABLED
        debugE("\nKeyStore :: loadFileContents: Failed to open file - %s for reading key contents", filePath);
      #else
        LOG("\nKeyStore :: loadFileContents: Failed to open file - %s for reading key contents", filePath);
      #endif
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
      case 5: qrCACert = new String(buffer);
              qrCACertLoadStatus = LOADED;
              break;
    }

    delete buffer;
    #ifndef DEBUG_DISABLED
      debugD("\nKeyStore :: loadFileContents: Key Contents loaded from file - %s", filePath);
    #else
      LOG("\nKeyStore :: loadFileContents: Key Contents loaded from file - %s", filePath);
    #endif
  }
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

const char* KeyStore :: getQRCACert(){
  if(isQRCACertLoaded()){
    return qrCACert->c_str();
  }
  return NULL;
}

std::vector <struct Action>  KeyStore :: retrieveActions(){
  //Clear previous actions details
  if(!actionsList.empty()){
    actionsList.clear();
    debugD("\nKeyStore :: retrieveActions: Cleared contents of previous actions present in ActionsList");
  }

  if(!SPIFFS.begin(true)){
    debugE("\nKeyStore :: retrieveActions: An Error has occurred while mounting SPIFFS");
    return actionsList;
  }

  if(SPIFFS.exists(ACTIONS_FILE)){
    File file = SPIFFS.open(ACTIONS_FILE, FILE_READ);
    if(!file){
      debugE("\nKeyStore :: retrieveActions: There was an error opening the file - %s for reading action details", ACTIONS_FILE);
      return actionsList;
    }

    DynamicJsonBuffer jb;
    JsonArray& actionsArray = jb.parseArray(file);
    file.close();

    if(actionsArray.success()){
      debugD("\nKeyStore :: retrieveActions: JSON Array parsed from the file - %s", ACTIONS_FILE);
      int actionsCount = actionsArray.size();
      for(byte i=0 ; i < actionsCount; i++){
        const char* actionID = actionsArray[i]["actionID"];
        const char* frequency = actionsArray[i]["frequency"];
        const unsigned long ltt = (actionsArray[i]["time"]).as<unsigned long>();
        debugD("\nKeyStore :: retrieveActions: Action - %d Details: %s - %s - %lu",i+1,actionID,frequency,ltt);

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
      debugE("\nKeyStore :: retrieveActions: Error while parsing retrieved JSON Array from the file - %s", ACTIONS_FILE);
      return actionsList;
    }
  }
  else {
    debugE("\nFile - %s does not exist to read action details",ACTIONS_FILE);
    return actionsList;
  }
}

bool KeyStore :: saveActions(std::vector <struct Action> aList){
  if(!SPIFFS.begin(true)){
    debugE("\nKeyStore :: saveActions: An Error has occurred while mounting SPIFFS");
    return false;
  }

  File file = SPIFFS.open(ACTIONS_FILE, FILE_WRITE);
  if(!file){
    debugE("\nKeyStore :: saveActions: There was an error opening the file - %s for saving actions", ACTIONS_FILE);
    return false;
  }

  DynamicJsonBuffer jb;
  JsonArray& actionsArray = jb.createArray();

  debugD("\nKeyStore :: saveActions: Actions given to save to the file - %s : %d", ACTIONS_FILE,aList.size());
  for (std::vector<struct Action>::iterator i = aList.begin() ; i != aList.end(); ++i){
    JsonObject& obj = jb.createObject();
    obj["actionID"] = i->actionID;
    obj["frequency"] = i->actionFrequency;
    obj["time"] = i->triggeredTime;
    actionsArray.add(obj);
    debugD("\nKeyStore :: saveActions: %s : %s : %lu -- Added to actions array", i->actionID, i->actionFrequency, i->triggeredTime);
  }

  debugD("\nKeyStore :: saveActions: Number of actions in actionsArray to save to file : %d", actionsArray.size());
  int nBytes = actionsArray.measureLength();
  actionsArray.printTo(file);
  debugD("\nKeyStore :: saveActions: Number of bytes written to %s: %d",ACTIONS_FILE,nBytes);

  file.close();
  return true;
}

void KeyStore :: urlEncode(String& dInfo){
  //Replcae all {'s with %7B
  dInfo.replace("{","%7B");
  //Replace all "'s with %22
  dInfo.replace("\"","%22");
  //Replcae all :'s with %3A
  dInfo.replace(":","%3A");
  //Replace all ,'s with %2C
  dInfo.replace(",","%2C");
  //Replace all ' ' with %20
  dInfo.replace(" ","%20");
  //Replace all / with %2F
  dInfo.replace("\/","%2F");
  //Replace all +'s with %2B
  dInfo.replace("+","%2B");
  //Replace all @'s with %40
  dInfo.replace("@","%40");
  //Replace all \'s with %5C
  dInfo.replace("\\","%5C");
  //Replace all }'s with %7D
  dInfo.replace("}","%7D");

  //%7B%22deviceID%22%3A%22eb25d0ba-2dcd-4db2-8f96-a4fbe54dbffc%22%2C%22name%22%3A%22BoT-ESP-32%22%2C%22makerID%22%3A%22469908A3-8F6C-46AC-84FA-4CF1570E564B%22%2C%22publicKey%22%3A%22ssh-rsa%20AAAAB3NzaC1yc2EAAAADAQABAAABAQC5hDDJ9mvJj77rV2fm6cXpklEq2lO7TDYVBWvnVdP5JJrPfwW3XGBk%2Ft7S9jmuxcq%2BwGep%2F1YELMCGenXt%2FM8Qhy0694m9gSB8aqOiNo9EC9%2BWRRjAwpV7ObeJex8EiuqP8eUe9INfTATPS3GCHfqnUJc%2Fufw652bA5HFdD3no3Vvnp0iuJwKiitvVy26mrcqhayXqDM5uzNGFLZof9On%2FGwfcDcpkKhL4LNtvWutB80M3BxY2G8UL1vT0QILln37Mm5lIHPt7JCrN8vVqwT5fBCuej5khEUsOMb9i5bzjF26CEyepZPgr%2FxdRk8sxHsCok%2F0W23zzf4iLDtVyXZJp%20lokeshkot%40hcl.com%5Cn%22%2C%22multipair%22%3A0%7D";
}
bool KeyStore :: generateAndSaveQRCode(){
  //QR Code is already generated and saved to SPIFFS, just return true
  if(isQRCodeGeneratedandSaved()){
    debugD("\nKeyStore :: generateAndSaveQRCode: QR Code already generated and saved to SPIFFS");
    return qrCodeStatus;
  }

  //Otherwise generate QR Code and Save to SPIFFS, set qrCodeStatus to true
  bool qrCodeGenerated = false;
  bool qrCodeSaved = false;

  if(WiFi.status() == WL_CONNECTED){
  const char* qrCodeGenURL = "https://api.qrserver.com/v1/create-qr-code/?size=150x150&data=";
  store->retrieveAllKeys();
  const char* dInfo = (getDeviceInfo())->c_str();
  debugD("\nKeyStore :: generateAndSaveQRCode: Device Info: %s",dInfo);
  String dInfoStr = String(dInfo);
  urlEncode(dInfoStr);
  debugD("\nKeyStore :: generateAndSaveQRCode: URL Encoded Device Info: %s",dInfoStr.c_str());
  String qrCodeGenLink = String(qrCodeGenURL);
  qrCodeGenLink.concat(dInfoStr);
  debugD("\nKeyStore :: generateAndSaveQRCode: qrCodeGenLink: %s",qrCodeGenLink.c_str());

  //Hardcoded URL Encoded qrCodeGenLink for reference
  //String qrCodeGenLink = "https://api.qrserver.com/v1/create-qr-code/?size=150x150&data=%7B%22deviceID%22%3A%22eb25d0ba-2dcd-4db2-8f96-a4fbe54dbffc%22%2C%22name%22%3A%22BoT-ESP-32%22%2C%22makerID%22%3A%22469908A3-8F6C-46AC-84FA-4CF1570E564B%22%2C%22publicKey%22%3A%22ssh-rsa%20AAAAB3NzaC1yc2EAAAADAQABAAABAQC5hDDJ9mvJj77rV2fm6cXpklEq2lO7TDYVBWvnVdP5JJrPfwW3XGBk%2Ft7S9jmuxcq%2BwGep%2F1YELMCGenXt%2FM8Qhy0694m9gSB8aqOiNo9EC9%2BWRRjAwpV7ObeJex8EiuqP8eUe9INfTATPS3GCHfqnUJc%2Fufw652bA5HFdD3no3Vvnp0iuJwKiitvVy26mrcqhayXqDM5uzNGFLZof9On%2FGwfcDcpkKhL4LNtvWutB80M3BxY2G8UL1vT0QILln37Mm5lIHPt7JCrN8vVqwT5fBCuej5khEUsOMb9i5bzjF26CEyepZPgr%2FxdRk8sxHsCok%2F0W23zzf4iLDtVyXZJp%20lokeshkot%40hcl.com%5Cn%22%2C%22multipair%22%3A0%7D";
  //debugD("\nKeyStore :: generateAndSaveQRCode: qrCodeGenLink: %s",qrCodeGenLink.c_str());

  HTTPClient* httpClient = new HTTPClient();
  httpClient->begin(qrCodeGenLink,getQRCACert());
  //Set HTTP Call timeout to sufficient value
  httpClient->setTimeout(1*60*1000);
  int httpCode = httpClient->GET();
  if(httpCode > 0){
    debugD("\nKeyStore :: generateAndSaveQRCode: HTTP return code for GET call to generate QR Code: %d",httpCode);
    if(httpCode == HTTP_CODE_OK){
      uint8_t* buffer = NULL;
      debugI("\nKeyStore :: generateAndSaveQRCode: QR Code generation is successful");
      qrCodeGenerated = true;
      int len = httpClient->getSize();
      debugD("\nKeyStore :: generateAndSaveQRCode: Content-length: %d",len);
      WiFiClient* stream = httpClient->getStreamPtr();
      if(httpClient->connected() && (len>0 || len == -1)){
        size_t size = stream->available();
        debugD("\nKeyStore :: generateAndSaveQRCode: QR Code image size: %u",size);
        if(size){
          buffer = new uint8_t[size];
          int rc = stream->readBytes(buffer,size);
          debugD("\nKeyStore :: generateAndSaveQRCode: Amount of bytes read into buffer: %d",rc);
          //Save buffer data into SPIFFS
          qrCodeSaved = saveQRCode(buffer,size);
          delete buffer;
        }
        else {
          debugD("\nKeyStore :: generateAndSaveQRCode: QR Code Data not available to save to SPIFFS");
        }
      }
      else {
        debugD("\nKeyStore :: generateAndSaveQRCode: HTTP Client not connected OR No Content Available");
      }
    }
    else {
      debugD("\nKeyStore :: generateAndSaveQRCode: HTTP GET Call to generate QR Code Failed with code: %d",httpCode);
    }
  }
  else {
    debugD("\nKeyStore :: generateAndSaveQRCode: HTTP GET Call to generate QR Code Failed with code: %d",httpCode);
  }
  //Cleanup HTTP Client Resources
  httpClient->end();
  delete httpClient;
 }
 else
  debugE("\nKeyStore :: generateAndSaveQRCode: Board not connected to WiFi");

 qrCodeStatus = (qrCodeGenerated && qrCodeSaved);
 return qrCodeStatus;
}

bool KeyStore :: saveQRCode(const uint8_t* buffer, const size_t bufferSize){
  if(!SPIFFS.begin(true)){
    debugE("\nKeyStore :: saveQRCode: An Error has occurred while mounting SPIFFS");
    return false;
  }

  File file = SPIFFS.open(QRCODE_FILE, FILE_WRITE);
  if(!file){
    debugE("\nKeyStore :: saveQRCode: There was an error opening the file - %s for saving QR Code", QRCODE_FILE);
    return false;
  }

  int bytesWritten = file.write(buffer,bufferSize);
  debugD("\nKeyStore :: saveQRCode: Amount of bytes written to file - %s for saving QR Code: %d", QRCODE_FILE,bytesWritten);

  return ((bytesWritten>0)?true:false);
}

bool KeyStore :: isQRCodeGeneratedandSaved(){
  if(!SPIFFS.begin(true)){
    debugE("\nKeyStore :: isQRCodeGeneratedandSaved: An Error has occurred while mounting SPIFFS");
    return false;
  }

  qrCodeStatus = SPIFFS.exists(QRCODE_FILE);
  return qrCodeStatus;
}

bool KeyStore :: resetQRCodeStatus(){
  if(!SPIFFS.begin(true)){
    debugE("\nKeyStore :: resetQRCodeStatus: An Error has occurred while mounting SPIFFS");
    return false;
  }
  if(SPIFFS.remove(QRCODE_FILE)){
    qrCodeStatus = false;
    return true;
  }
  else
    return false;
}
