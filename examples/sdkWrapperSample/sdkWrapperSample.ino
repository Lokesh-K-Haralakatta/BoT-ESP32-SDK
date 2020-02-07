/*
  sdkWrapperSample.ino - Sample sketch to show case the ESP-32 SDK usage through SDKWrapper.
  Created by Lokesh H K, July 03, 2019.
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
   Arduino IDE / Webserver onto ESP32 board
*/

/*

   Sketch also has the feature of Web OTA Update for the firmware and deep sleep

   For Web OTA Update, please make sure to providde the valid web server URL where
   the required firmware is available otherwise comment out the lines in the sketch

   For Deep Sleep Mode, please make sure to connect Push Button / Capacitive Button
   to GPIO33 Pin. ESP32 board gets wake up when the button is pressed, does the required
   actions provided and goes back to sleep until the button gets pressed again.
*/

/*
  There are 2 intervals used in the sketch:

  Payment Actions trigger interval:  Default is 5 minutes

  OTA Update interval: Default is 10 mins
*/

/*
  Here is the sketch flow:

  1. Gets makerID from the provided configuration
  2. Initializes the configuration, connects to given WiFi Network, initialize NTPClient
  3. Get current time in Epoch Seconds and initialize action trigger and OTA update epoch times
  3. Pairs the device using BLE with the FINN Application, if device is not already paired
  4. Gets the actions defined at provided makerID portal
  5. Gets the latest time in Epoch seconds and checks the elapsed seconds since from last
     action trigger and OTA update
  6. If the elapsed time since from last action trigger is >= 5 mins, then triggers the provided actionID
  7. If the elapsed time since from last OTA update is >= 10 mins, then request for new firware update,
     updates the ESP32 board with new firware received, restarts the Board
  8. Flows to enable the board for deep sleep mode until button is pressed next

  */



  #include <Storage.h>
  #include <Webserver.h>
  #include <SDKWrapper.h>
  #include <ESP32httpUpdate.h>

  //Custom WiFi Credentials
  #define WIFI_SSID "FINN"
  #define WIFI_PASSWD "xxxxxxxxxxxxxxx"

  //Declare service variables
  KeyStore *store = NULL;
  Webserver *server = NULL;
  SDKWrapper *sdk = NULL;

  //Action ID with frequency as "minutely"
  String actionIDMinutely = String("81F6011A-9AF0-45AE-91CD-9A0CDA81FA1F");

  //Variable to hold given deviceID value
  const char* deviceID = NULL;

  // Vriable to keep track of boot count
  RTC_DATA_ATTR unsigned long bootCount = 1;

  // Varibale to store last time action was triggered
  RTC_DATA_ATTR unsigned long previousActionTriggerEpoch = 0;

  // Action trigger interval in seconds
  RTC_DATA_ATTR const long actionTriggerInterval = 5*60;

  // Firwamre update trigger interval in seconds
  RTC_DATA_ATTR const long firmwareUpdateInterval = 10*60;

  // Varibale to store last time firmware update was triggered
  RTC_DATA_ATTR unsigned long previousOTAUpdateEpoch = 0;

  //Instance variables used to get current time
  WiFiUDP ntpUDP;
  NTPClient *timeClient;

  // Method to retrieve new firmware from remote server
  bool requestNewFirmware() {
    if((WiFi.status() == WL_CONNECTED)) {
        t_httpUpdate_return ret = ESPhttpUpdate.update("http://10.26.16.181:8080/ota");
        switch(ret) {
            case HTTP_UPDATE_FAILED:
                debugE("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                return false;
                break;
            case HTTP_UPDATE_NO_UPDATES:
                debugW("HTTP_UPDATE_NO_UPDATES");
                return false;
                break;
            case HTTP_UPDATE_OK:
                debugI("HTTP_UPDATE_OK");
                return true;
                break;
        }
    }
  }

//Function that prints the reason by which ESP32 has been awaken from sleep
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
    case 1  : debugI("\nWakeup caused by external signal using RTC_IO"); break;
    case 2  : debugI("\nWakeup caused by external signal using RTC_CNTL"); break;
    case 3  : debugI("\nWakeup caused by timer"); break;
    case 4  : debugI("\nWakeup caused by touchpad"); break;
    case 5  : debugI("\nWakeup caused by ULP program"); break;
    default : debugI("\nWakeup was not caused by deep sleep"); break;
  }
}

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

      server = Webserver::getWebserverInstance(loadConfig);

      //Instantiate Webserver by using the custom WiFi credentials
      //server = Webserver::getWebserverInstance(loadConfig,WIFI_SSID, WIFI_PASSWD,BoT_DEBUG);

      //Enable board to connect to WiFi Network
      server->connectWiFi();

      //Instantiate SDK Wrapper
      sdk = new SDKWrapper();

     if(server->isWiFiConnected()){
        //Get NTPClient Instance
        timeClient = new NTPClient(ntpUDP);

        //Get latest time update from internet
        while(!timeClient->update()) {
          timeClient->forceUpdate();
        }

        //Initialize the epoch times if it's firt time boot
        if( bootCount == 1){
           previousActionTriggerEpoch = timeClient->getEpochTime();
           previousOTAUpdateEpoch = previousActionTriggerEpoch;
        }
     }

      //Pair and Activate the device for first time
      if(sdk->pairAndActivateDevice()){
         debugI("\nsdkWrapperSample: Device is Paired and Activated for Autonomous Payments");
         //Get actions from BoT Server
         if(server->isWiFiConnected()){
           String* actions = sdk->getActions();
           //If actions are present, they are in JSON String
           if(actions != NULL){
             DynamicJsonBuffer jsonBuffer;
             JsonArray& actionsArray = jsonBuffer.parseArray(*actions);
             if(actionsArray.success()){
                 int actionsCount = actionsArray.size();
                 debugI("\nsdkWrapperSample :: JSON Actions array parsed successfully");
                 debugI("\nsdkWrapperSample :: Number of actions returned: %d", actionsCount);
             }
             jsonBuffer.clear();
            }
            else {
              debugE("\nsdkWrapperSample :: Failed in retrieveing actions from server");
            }
          }
        }
      else {
         debugI("\nsdkWrapperSample: Device is not Paired");
      }
    }
    else {
      LOG("\nsdkWrapperSample: MakerID can not be NULL!");
    }
  }

 void loop(){
   unsigned long currentEpochTime = previousActionTriggerEpoch;
   if(bootCount != 1) {
      currentEpochTime = timeClient->getEpochTime();
   }

   unsigned long elapsedSecondsSinceLastActionTrigger = currentEpochTime - previousActionTriggerEpoch;
   unsigned long elapsedSecondsSinceLastOTAUpdate = currentEpochTime - previousOTAUpdateEpoch;

   if( elapsedSecondsSinceLastActionTrigger >= actionTriggerInterval) {
     debugI("\nAvalable free heap before action trigger: %lu",ESP.getFreeHeap());
     if(server->isWiFiConnected()){
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
         debugI("\nAvailable free heap after action trigger: %lu",ESP.getFreeHeap());
         // save the action trigger time
         currentEpochTime = timeClient->getEpochTime();
         previousActionTriggerEpoch = currentEpochTime;
         elapsedSecondsSinceLastActionTrigger = currentEpochTime - previousActionTriggerEpoch;
       }
       else {
         debugI("\nsdkWrapperSample: Device State is not active to trigger the action, Try pairing the device again:");
         sdk->pairAndActivateDevice();
       }
     }
     else {
       debugW("\nsdkWrapperSample :: Board not connected to WiFi, try connecting again!");
       //Enable board to connect to WiFi Network
       server->connectWiFi();
     }
   }

   if( elapsedSecondsSinceLastOTAUpdate >= firmwareUpdateInterval) {
     debugI("\nAvailable free heap before firmware update trigger: %lu",ESP.getFreeHeap());
     if(server->isWiFiConnected()){
        debugI("\nStarting the firmware update onto ESP32 board....");
        if(requestNewFirmware()){
          // save the firmware update trigger time
          currentEpochTime = timeClient->getEpochTime();
          previousOTAUpdateEpoch = currentEpochTime;
          elapsedSecondsSinceLastOTAUpdate = currentEpochTime - previousOTAUpdateEpoch;
        }
        debugI("\nAvailable free heap after firmware update trigger: %lu",ESP.getFreeHeap());
     }
     else {
        debugE("\nBoard not connected to WiFi Network to request for latest available firmware, trying to connect to network!!!");
        server->connectWiFi();
     }
    }

   debugI("\nElapsed Seconds since last action trigger: %ld", elapsedSecondsSinceLastActionTrigger);
   debugI("\nElapsed Seconds since last firmware update trigger: %ld", elapsedSecondsSinceLastOTAUpdate);

   #ifndef DEBUG_DISABLED
     Debug.handle();
   #endif

   //Print the wakeup reason for ESP32
   print_wakeup_reason();

   debugI("\n Boot Count: %ld", bootCount);
   bootCount++;

   //Configure GPIO33 as ext0 wake up source for HIGH logic level
   esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1);

   //Go to sleep now
   esp_deep_sleep_start();

 }
