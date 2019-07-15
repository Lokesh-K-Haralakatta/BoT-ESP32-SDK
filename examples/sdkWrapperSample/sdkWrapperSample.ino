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

/* Change Partition Scheme to No OTA (Large APP) in Arduino IDE -> Tools after connecting ESP-32 board
   to avoid the error message saying "Sketch is too big" before compiling and uploading Sketch to ESP-32 */

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

  //Custom WiFi Credentials
  #define WIFI_SSID "LJioWiFi"
  #define WIFI_PASSWD "adgjmptw"

  //Declare service variables
  KeyStore *store = NULL;
  Webserver *server = NULL;
  SDKWrapper *sdk = NULL;

  //Action ID with frequency as "minutely"
  String actionIDMinutely = String("A42ABD19-3226-47AB-8045-8129DBDF117E");

  //Variable to hold given deviceID value
  const char* deviceID = NULL;

  //Variable to keep track of action triggered
  int triggerCount = 0;

  //Flag to check existence of the action
  bool actionFound = false;

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
      //server = new Webserver(loadConfig);

      //Instantiate Webserver by using the custom WiFi credentials
      loadConfig = false;
      int logLevel = BoT_INFO;
      server = new Webserver(loadConfig,WIFI_SSID, WIFI_PASSWD,logLevel);

      //Enable board to connect to WiFi Network
      server->connectWiFi();

      //Instantiate SDK Wrapper
      sdk = new SDKWrapper();

      //Pair and Activate the device for first time
      if(sdk->pairAndActivateDevice()){
         debugI("\nsdkWrapperSample: Device is Paired and Activated for Autonomous Payments");
         //Get actions from BoT Server
         if(server->isWiFiConnected()){
           String* actions = sdk->getActions();
           //If actions are present, they are in JSON String
           //Parse them to check given action is there or not
           if(actions != NULL){
             DynamicJsonBuffer jsonBuffer;
             JsonArray& actionsArray = jsonBuffer.parseArray(*actions);
             if(actionsArray.success()){
                 int actionsCount = actionsArray.size();
                 debugI("\nsdkWrapperSample :: JSON Actions array parsed successfully");
                 debugI("\nsdkWrapperSample :: Number of actions returned: %d", actionsCount);
                 //Check whether given action is present in returned actions list
                 for(byte i=0 ; i < actionsCount; i++){
                    const char* actionID = actionsArray[i]["actionID"];
                    const char* frequency = actionsArray[i]["frequency"];
                    if(actionIDMinutely.equals(actionID)){
                      debugI("\nsdkWrapperSample :: Action Found in actionsArray");
                      debugI("\nsdkWrapperSample :: Action ID: %s", actionID);
                      debugI("\nsdkWrapperSample :: Action Frequency: %s", frequency);
                      actionFound = true;
                      break;
                    }
                  }
              }
              jsonBuffer.clear();
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
   debugI("\nAvalable free heap at the beginning of loop: %lu",ESP.getFreeHeap());
   if(server->isWiFiConnected()){
     int dState = store->getDeviceState();
     debugI("\nsdkWrapperSample :: Device State -> %s",store->getDeviceStatusMsg());
     //Check for the device state, should be active to trigger the action
     if(dState >= DEVICE_ACTIVE){
       //Trigger the action if it's defined with the maker portal
       if(actionFound){
         debugI("\nsdkWrapperSample :: Triggering action - %s", actionIDMinutely.c_str());
         if(sdk->triggerAction(actionIDMinutely.c_str())){
           triggerCount++;
           debugI("\nsdkWrapperSample :: Triggering action successful for %d times",triggerCount);
         }
         else {
           debugE("\nsdkWrapperSample :: Triggering action failed!");
         }
       }
       else {
         debugW("\nsdkWrapperSample :: Action - %s not found in maker portal", actionIDMinutely.c_str());
       }
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

   //Put delay to reclaim the released memory
   delay(1000);
   debugI("\nAvalable free heap at the end of loop: %lu",ESP.getFreeHeap());

   #ifndef DEBUG_DISABLED
     Debug.handle();
   #endif

   //Introduce delay of 1 min
   delay(1*60*1000);

 }
