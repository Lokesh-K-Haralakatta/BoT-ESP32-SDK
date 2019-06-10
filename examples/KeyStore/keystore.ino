/*
  keyStore.ino - Example sketch program to show the usage for configuration retrieval, keys retrieval
    Certificates retrieveal and store/retrieve actions for BoT Service.
  Created by Lokesh H K, April 9, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include <Storage.h>
#include <Webserver.h>

KeyStore* store;
Webserver *server;

void setup(){
  store = KeyStore :: getKeyStoreInstance();

  store->loadJSONConfiguration();

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

    if(store->isQRCACertLoaded()){
      debugI("\n QR Code API CA Certificate Contents: \n%s\n", store->getQRCACert());
    }

    debugI("\nDeviceInformation: %s", (store->getDeviceInfo())->c_str());

    //Initialize actions to save to file
    const char* id1 = "E6509B49-5048-4151-B965-BB7B2DBC7905";
    const char* freq1 = "minuetly";
    const unsigned long ltt1 = 1557225825;

    const char* id2 = "E6509B49-5048-4151-B965-BB7B2DBC7906";
    const char* freq2 = "hourly";
    const unsigned long ltt2 = 1557225886;

    const char* id3 = "E6509B49-5048-4151-B965-BB7B2DBC7907";
    const char* freq3 = "daily";
    const unsigned long ltt3 = 1557225936;

    const char* id4 = "E6509B49-5048-4151-B965-BB7B2DBC7908";
    const char* freq4 = "weekly";
    const unsigned long ltt4 = 1557225987;

    std::vector <struct Action> actionsList;

    struct Action item1;
    item1.actionID = new char[strlen(id1)+1];
    item1.actionFrequency = new char[strlen(freq1)+1];

    strcpy(item1.actionID,id1);
    strcpy(item1.actionFrequency,freq1);
    item1.triggeredTime = ltt1;
    actionsList.push_back(item1);

    struct Action item2;
    item2.actionID = new char[strlen(id2)+1];
    item2.actionFrequency = new char[strlen(freq2)+1];

    strcpy(item2.actionID,id2);
    strcpy(item2.actionFrequency,freq2);
    item2.triggeredTime = ltt2;
    actionsList.push_back(item2);

    struct Action item3;
    item3.actionID = new char[strlen(id3)+1];
    item3.actionFrequency = new char[strlen(freq3)+1];

    strcpy(item3.actionID,id3);
    strcpy(item3.actionFrequency,freq3);
    item3.triggeredTime = ltt3;
    actionsList.push_back(item3);

    struct Action item4;
    item4.actionID = new char[strlen(id4)+1];
    item4.actionFrequency = new char[strlen(freq4)+1];
    strcpy(item4.actionID,id4);
    strcpy(item4.actionFrequency,freq4);
    item4.triggeredTime = ltt4;
    actionsList.push_back(item4);

    if(store->saveActions(actionsList)){
      debugI("\n %d actions saved to file - %s", actionsList.size(),ACTIONS_FILE);
    }
    else {
      debugI("\n Failed in saving actions to file - %s", ACTIONS_FILE);
    }

    actionsList = store->retrieveActions();
    if(actionsList.empty()){
      debugI("\n No Actions retrieved from the file - %s", ACTIONS_FILE);
    }
    else {
      debugI("\n Actions retrieved from the file - %s : %d", ACTIONS_FILE,actionsList.size());
      for (std::vector<struct Action>::iterator i = actionsList.begin() ; i != actionsList.end(); ++i){
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
  }
  else {
    LOG("\nkeyStore: ESP-32 board not connected to WiFi Network, try again");
    //Enable board to connect to WiFi Network
    server->connectWiFi();
  }
  #ifndef DEBUG_DISABLED
    Debug.handle();
  #endif

  delay(1*60*1000);
}
