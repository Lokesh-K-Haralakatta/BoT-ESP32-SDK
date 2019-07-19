/*
  keyStore.ino - Example sketch program to show the usage for configuration retrieval, keys retrieval
    Certificates retrieveal and store/retrieve actions for BoT Service.
  Created by Lokesh H K, April 9, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include <Storage.h>
#include <Webserver.h>
#define ACTIONS_COUNT 7

KeyStore* store;
Webserver *server;
//Vector to hold action items
std::vector <struct Action> actionsList;

//Vector to hold offline action items
std::vector <struct OfflineActionMetadata> olActionsList;

//Initialize actions to save to file
const char* actionIds[ACTIONS_COUNT] = {  "E6509B49-5048-4151-B965-BB7B2DBC7905",
                                          "E6509B49-5048-4151-B965-BB7B2DBC7906",
                                          "E6509B49-5048-4151-B965-BB7B2DBC7907",
                                          "E6509B49-5048-4151-B965-BB7B2DBC7908",
                                          "C257DB70-AE57-4409-B94E-678CB1567FA6",
                                          "D93F99E1-011B-4609-B04E-AEDBA98A7C5F",
                                          "0097430C-FA78-4087-9B78-3AC7FEEF2245" };

const char* frequencies[ACTIONS_COUNT] = { "minuetly", "hourly", "daily", "weekly", "monthly", "half-yearly", "yearly" };

const unsigned long lastTrigTime[ACTIONS_COUNT] = { 1557225825,1557225886,1557225936,1557225987,1557226000,1557226100, 1557226200 };

void buildActionsList(){
  clearActionsList();
  for(int i=0; i<ACTIONS_COUNT; i++){
    struct Action item;
    item.actionID = new char[strlen(actionIds[i])+1];
    strcpy(item.actionID,actionIds[i]);
    item.actionFrequency = new char[strlen(frequencies[i])+1];
    strcpy(item.actionFrequency,frequencies[i]);
    item.triggeredTime = lastTrigTime[i];

    actionsList.push_back(item);
  }
}

void clearActionsList(){
  std::vector<struct Action>::iterator i;
  while(!actionsList.empty()){
    i = actionsList.begin();
    delete i->actionID;
    delete i->actionFrequency;
    actionsList.erase(i);
  }
}

void buildOfflineActionsList(){
  const char* deviceID = store->getDeviceID();
  const char* makerID = store->getMakerID();
  const char* queueID = store->generateUuid4();
  const char* altID = store->getAlternateDeviceID();

  clearOlActionsList();
  for(int i=0; i<ACTIONS_COUNT; i++){
    struct OfflineActionMetadata item;
    item.actionID = new char[strlen(actionIds[i])+1];
    strcpy(item.actionID,actionIds[i]);
    item.deviceID = new char[strlen(deviceID)+1];
    strcpy(item.deviceID,deviceID);
    item.makerID = new char[strlen(makerID)+1];
    strcpy(item.makerID,makerID);
    item.queueID = new char[strlen(queueID)+1];
    strcpy(item.queueID,queueID);
    if(altID != NULL){
      item.alternateID = new char[strlen(altID)+1];
      strcpy(item.alternateID,altID);
    }
    item.offline = 1;
    item.value = 0.0;
    item.timestamp = millis();

    if(store->isDeviceMultipair()){
      item.multipair = 1;
    }
    else {
      item.multipair = 0;
    }
    olActionsList.push_back(item);
  }
}

void clearOlActionsList(){
  std::vector<struct OfflineActionMetadata>::iterator i;
  while(!olActionsList.empty()){
    i = olActionsList.begin();
    delete i->actionID;
    delete i->deviceID;
    delete i->queueID;
    delete i->makerID;
    if(i->alternateID != NULL){
      delete i->alternateID;
    }
    olActionsList.erase(i);
  }
}

void setup(){
  store = KeyStore :: getKeyStoreInstance();

  store->loadJSONConfiguration();
  store->initializeEEPROM();

  //Get WiFi Credentials from given configuration
  //const char* WIFI_SSID = store->getWiFiSSID();
  //const char* WIFI_PASSWD = store->getWiFiPasswd();

  //Provide custom WiFi Credentials
  const char* WIFI_SSID = "LJioWiFi";
  const char* WIFI_PASSWD = "adgjmptw";

  //Instantiate Webserver by using the custom WiFi credentials
  bool loadConfig = false;
  int debugILevel = BoT_DEBUG;
  server = new Webserver(loadConfig,WIFI_SSID, WIFI_PASSWD,debugILevel);

  //Enable board to connect to WiFi Network
  server->connectWiFi();

}

void loop(){
  debugI("\nkeyStore: Available free heap at the beginning of loop: %lu",ESP.getFreeHeap());
  //Proceed further if board connects to WiFi Network
  if(server->isWiFiConnected()){
    if(store->isJSONConfigLoaded()){
      debugI("\n WiFi SSID: %s", store->getWiFiSSID());
      debugI("\n WiFi Passwd: %s", store->getWiFiPasswd());
      debugI("\n Maker ID: %s", store->getMakerID());
      debugI("\n Device ID: %s", store->getDeviceID());
      debugI("\n Device Name: %s", store->getDeviceName());
      debugI("\n Altternate Device ID: %s", store->getAlternateDeviceID());
    }

    //Set DeviceState based on pair type
    if(store->isDeviceMultipair()){
      debugI("\n Device is Multipair enabled, setting state to DEVICE_MULTIPAIR");
      store->setDeviceState(DEVICE_MULTIPAIR);
    }
    else {
      debugI("\n Device is not Multipair enabled, setting state to DEVICE_NEW");
      store->setDeviceState(DEVICE_NEW);
    }
    debugI("\n Device State Value: %d",store->getDeviceState());
    debugI("\n Device Status Msg: %s",store->getDeviceStatusMsg());

    //Explicitly set Device name
    store->setDeviceName("keystore-device");
    debugI("\n Device Name after reset: %s", store->getDeviceName());

    store->retrieveAllKeys();

    if(store->isPrivateKeyLoaded()){
      debugI("\n Private Key Contents: \n%s\n", store->getDevicePrivateKey());
    }

    if(store->isPublicKeyLoaded()){
      debugI("\n Public Key Contents: \n%s\n", store->getDevicePublicKey());
    }

    if(store->isAPIKeyLoaded()){
      debugI("\n API Key Contents: \n%s\n", store->getAPIPublicKey());
    }

    if(store->isCACertLoaded()){
      debugI("\n CA Certificate Contents: \n%s\n", store->getCACert());
    }

    debugI("\n DeviceInformation: %s", (store->getDeviceInfo())->c_str());

    //Build action items and add to actionsList
    buildActionsList();

    if(store->saveActions(actionsList)){
      debugI("\n %d actions saved to file - %s", actionsList.size(),ACTIONS_FILE);
    }
    else {
      debugI("\n Failed in saving actions to file - %s", ACTIONS_FILE);
    }

    //Clear actions items from actionsList
    clearActionsList();

    std::vector <struct Action> retActionsList = store->retrieveActions();
    if(retActionsList.empty()){
      debugI("\n No Actions retrieved from the file - %s", ACTIONS_FILE);
    }
    else {
      debugI("\n Actions retrieved from the file - %s : %d", ACTIONS_FILE,retActionsList.size());
      for (std::vector<struct Action>::iterator i = retActionsList.begin() ; i != retActionsList.end(); ++i){
        debugI("\n %s : %s : %lu", i->actionID, i->actionFrequency, i->triggeredTime);
      }
    }

    //Check QR Code generation and saving functionality
    if(store->isQRCodeGeneratedandSaved()){
      debugI("\n QR Code is already generated and saved to SPIFFS, resetting it's status");
      store->resetQRCodeStatus();
      store->generateAndSaveQRCode();
    }
    else {
      debugI("\n QR Code is not generated and saved to SPIFFS, now generating and saving to SPIFFS");
      store->generateAndSaveQRCode();
    }

    //Offline actions should not be present at this point
    if(store->offlineActionsExist()){
      debugE("\n Offline Actions are available in the file - %s", OFFLINE_ACTIONS_FILE);
    }
    else {
      debugI("\n Offline Actions are not available in the file - %s", OFFLINE_ACTIONS_FILE);
    }

    //Build offline action items and add to olActionsList
    buildOfflineActionsList();

    if(store->saveOfflineActions(olActionsList)){
      debugI("\n %d offline actions saved to file - %s", olActionsList.size(),OFFLINE_ACTIONS_FILE);
    }
    else {
      debugE("\n Failed in saving offline actions to file - %s", OFFLINE_ACTIONS_FILE);
    }

    //Clear offline actions items from olActionsList
    clearOlActionsList();

    //Offline actions should be present at this point
    if(store->offlineActionsExist()){
      debugI("\n Offline Actions are available in the file - %s", OFFLINE_ACTIONS_FILE);
    }
    else {
      debugE("\n Offline Actions are not available in the file - %s", OFFLINE_ACTIONS_FILE);
    }

    std::vector <struct OfflineActionMetadata> retOfflineActionsList = store->retrieveOfflineActions(true);
    if(retOfflineActionsList.empty()){
      debugI("\n No Offline Actions retrieved from the file - %s", OFFLINE_ACTIONS_FILE);
    }
    else {
      debugI("\n Offline Actions retrieved from the file - %s : %d", OFFLINE_ACTIONS_FILE,retOfflineActionsList.size());
      for (std::vector<struct OfflineActionMetadata>::iterator i = retOfflineActionsList.begin() ; i != retOfflineActionsList.end(); ++i){
        debugI("\n %d : %s : %s : %s : %s : %d : %lu", i->offline, i->makerID, i->deviceID, i->actionID, i->queueID, i->multipair, i->timestamp);
      }
    }

    //Offline actions should not be present at this point
    if(store->offlineActionsExist()){
      debugE("\n Offline Actions are available in the file - %s", OFFLINE_ACTIONS_FILE);
    }
    else {
      debugI("\n Offline Actions are not available in the file - %s", OFFLINE_ACTIONS_FILE);
    }
  }
  else {
    LOG("\nkeyStore: ESP-32 board not connected to WiFi Network, try again");
    //Enable board to connect to WiFi Network
    server->connectWiFi();
  }

  debugI("\nkeyStore: Available free heap at the end of loop: %lu",ESP.getFreeHeap());

  #ifndef DEBUG_DISABLED
    Debug.handle();
  #endif

  delay(1*60*1000);
}
