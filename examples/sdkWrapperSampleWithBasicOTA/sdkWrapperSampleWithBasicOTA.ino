/*
  sdkWrapperSampleWithBaicOTA.ino - Sample sketch to show case the ESP-32 SDK usage through SDKWrapper.
  and also the sketch update through Basic Arduino OTA

  Created by Lokesh H K, February 01 2020.
  Released into the repository BoT-ESP32-SDK.
*/

/*
  Sample assumes that We have valid private-public key pair for the device, api public key.
  Also we have valid makerID, deviceID defined.
  All these information is provided through files present in data directory of the sample.
  Update the required configuration, key-pair for the device in the data directory
  Make sure to flash all the files present in data directory onto ESP32 board using Arduino IDE.
*/

/*
  To use the ESP-32 SDK, include BoT-ESP32-SDK.zip through Arduino IDE
*/

/* Change Partition Scheme to Minimal SPIFFS (1.9 MB App with OTA/190 KB SPIFFS)
   in Arduino IDE -> Tools after connecting ESP-32 board to facilitate OTA through
   Arduino IDE onto ESP32 board
/*
  Here is the sketch flow:

  1. Gets makerID from the provided configuration
  2. Initializes the configuration, connects to given WiFi Network
  3. Pairs the device using BLE with the FINN Application
  4. Gets the actions defined at provided makerID portal
  5. Triggers the provided actionID at an interval of 1 minute it if's found in maker portal

  */



  #include <Storage.h>
  #include <Webserver.h>
  #include <SDKWrapper.h>
  #include <ESPmDNS.h>
  #include <WiFiUdp.h>
  #include <ArduinoOTA.h>

  //Custom WiFi Credentials
  #define WIFI_SSID "FINN"
  #define WIFI_PASSWD "Id4S7719G99XG1R"

  //Declare service variables
  KeyStore *store = NULL;
  Webserver *server = NULL;
  SDKWrapper *sdk = NULL;

  //Action ID with frequency as "minutely"
  String actionIDMinutely = String("A42ABD19-3226-47AB-8045-8129DBDF117E");

  //Variable to hold given deviceID value
  const char* deviceID = NULL;

  // Varibale store last time action was triggered
  unsigned long previousMillis = 0;

  // Action trigger interval in millis
  const long interval = 1*60*1000;

  void setup()
  {
    //Get KeyStore Instance
    store = KeyStore::getKeyStoreInstance();

    //Load the given configuration details from the SPIFFS
    store->loadJSONConfiguration();

    //Initialize EEPROM to load previous device state if any
    store->initializeEEPROM();

    //Get the given makerID from the configuration
    const char* makerID = store->getMakerID();

    //Get the given deviceID from the configuration
    deviceID = store->getDeviceID();

    //Continue if makerID is available
    if(makerID != NULL){
      //Variable to flag whether to load WiFi credentials from given configuration or not
      bool loadConfig = true;

      //Override HTTPS
      //store->setHTTPS(true);

      //Instantiate Webserver by using WiFi credentials from configuration
      //server = Webserver::getWebserverInstance(loadConfig);

      //Instantiate Webserver by using the custom WiFi credentials
      loadConfig = false;
      int logLevel = BoT_INFO;
      server = Webserver::getWebserverInstance(loadConfig,WIFI_SSID, WIFI_PASSWD,logLevel);

      //Enable board to connect to WiFi Network
      server->connectWiFi();

      //Instantiate SDK Wrapper
      sdk = new SDKWrapper();
    }
    else {
      LOG("\nsdkWrapperSample: MakerID can not be NULL!");
    }

    ArduinoOTA
    .onStart([]() {

      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      debugI("Start updating %s",type);
    })
    .onEnd([]() {
      debugI("OTA Upload Done!!!");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      debugI("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      debugE("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) debugE("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) debugE("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) debugE("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) debugE("Receive Failed");
      else if (error == OTA_END_ERROR) debugE("End Failed");
    });

  ArduinoOTA.begin();
  }

 void loop(){
  ArduinoOTA.handle();
  delay(1);

  unsigned long currentMillis = millis();
  unsigned long elapsedMillis = currentMillis - previousMillis;

   if(server->isWiFiConnected()){
     if( elapsedMillis >= interval) {
      // save the action trigger time
      previousMillis = currentMillis;
      debugI("\nElapsed Millis since last action trigger: %ld", elapsedMillis);

      debugI("\nAvalable free heap at the beginning of action trigger: %lu",ESP.getFreeHeap());

     int dState = store->getDeviceState();
     debugI("\nsdkWrapperSample :: Device State -> %s",store->getDeviceStatusMsg());
     //Check for the device state, should be active to trigger the action
     if(dState >= DEVICE_ACTIVE){
       //Trigger the action added with the paired device
       debugI("\nsdkWrapperSample :: Triggering action - %s", actionIDMinutely.c_str());
      if(sdk->triggerAction(actionIDMinutely.c_str())){
        debugI("\nsdkWrapperSample :: Triggering action successful...");
      }
      else {
        debugE("\nsdkWrapperSample :: Triggering action failed!");
      }
      debugI("\nAvalable free heap at the end of action trigger: %lu",ESP.getFreeHeap());
     }
     else {
       debugI("\nsdkWrapperSample: Device State is not active to trigger the action, Try pairing the device again:");
       sdk->pairAndActivateDevice();
     }
    }
  }
   else {
     debugW("\nsdkWrapperSample :: Board not connected to WiFi, try connecting again!");
     //Enable board to connect to WiFi Network
     server->connectWiFi();
   }

   #ifndef DEBUG_DISABLED
     Debug.handle();
   #endif
 }
