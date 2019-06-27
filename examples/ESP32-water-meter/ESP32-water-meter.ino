/*
  esp32-water-meter.ino - Sketch to measure water consumption volume using Water Flow Sensor.
  Created by Lokesh H K, June 25, 2019.
  Available in github repository BoT-ESP32-SDK/examples/ESP32-water-meter.ino
*/

/*
  Sample assumes that we have valid private-public key pair for the device, api public key.
  Also we have valid makerID, deviceID, actionID defined.
  All these information is provided through files present in data directory of the sample.
  Update the required configuration, key-pair for the device in the data directory
  Make sure to flash all the files present in data directory onto ESP32 board using Arduino IDE.
*/

/*
  Sketch uses the ESP-32 SDK, refer to https://github.com/BankingofThings/BoT-ESP32-SDK to setup SDK and it's prerequisites
  Sketch also uses Flowmeter library to simulate Flow sensor, download and include throguh Arduino IDE from the link https://github.com/sekdiy/FlowMeter
*/

/* Change Partition Scheme to No OTA (Large APP) in Arduino IDE -> Tools after connecting ESP-32 board
   to avoid the error message saying "Sketch is too big" before compiling and uploading Sketch to ESP-32 */

/*
  Here is the sketch flow:

  1. Gets makerID from the provided configuration
  2. Initializes the configuration, internally it waits for pairing and device gets activated
  3. Starts the Async Webserver on port 3001 on ESP32 board
  4. Sketch uses simulated Flow Sensor to measure the water consumption
  5. Logic to calculate water consumption runs on Core-1 of ESP-32
  6. Logic to trigger notifications runs in separate task on Core-0
  7. Logic to trigger payment runs in separate task on Core-0
*/

#include <Storage.h>
#include <Webserver.h>
#include <BoTService.h>
#include <FlowMeter.h>

//Custom WiFi Credentials
#define WIFI_SSID "LJioWiFi"
#define WIFI_PASSWD "adgjmptw"

//Declare service variables
KeyStore *store = NULL;
Webserver *server = NULL;

//Webserver Port
const int port = 3001;

//Variable to hold given deviceID value
const char* deviceID = NULL;

//Tasks handles
TaskHandle_t nTask;
TaskHandle_t pTask;

// let's provide our own sensor properties, including calibration points for error correction
FlowSensorProperties flowSensor = {60.0f, 4.5f, {1.2, 1.1, 1.05, 1, 1, 1, 1, 0.95, 0.9, 0.8}};

// let's pretend there's a flow sensor connected to pin 3
FlowMeter Meter = FlowMeter(3, flowSensor);

//Define notify threshold in liters
const int INTERVAL_THRESHOLD = 25;
const int NOTIFY_1 = INTERVAL_THRESHOLD;
const int NOTIFY_2 = INTERVAL_THRESHOLD * 2;
const int NOTIFY_3 = INTERVAL_THRESHOLD * 3;
const int NOTIFY_4 = INTERVAL_THRESHOLD *4;

//Define push notification uuids present as generated in maker portal
char* NOTIFY_25_LTRS = "E3A5CE36-C7F8-4835-BF54-31305D21FB9D";
char* NOTIFY_50_LTRS = "E3D056DC-F68C-4EB1-B66A-D8401038C0B0";
char* NOTIFY_75_LTRS = "4007A480-FEAA-4CF5-B950-1285BBCAF2A4";
char* NOTIFY_100_LTRS = "8E4BFF62-98FD-485F-902F-1CBD4360CE65";
char* NOTIFY_100_LTRS_PAYMENT = "66CA601D-D4DA-4DB8-AE4D-9ED38138A738";

//Notify flag
bool notify = false;
int notifyID = 0;

//Action flag
bool action = false;

//Previous consumption in liters
int prevConsumedLtrs = 0;

//Present consumption in liters
int consumedLtrs = 0;

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
    store->setHTTPS(true);

    //Instantiate Webserver by using WiFi credentials from configuration
    //server = new Webserver(loadConfig);

    //Instantiate Webserver by using the custom WiFi credentials
    loadConfig = false;
    int logLevel = BoT_INFO;
    server = new Webserver(loadConfig,WIFI_SSID, WIFI_PASSWD,logLevel);

    //Enable board to connect to WiFi Network
    server->connectWiFi();

    //Start the Async Webserver on ESP32 board to initialize, configure the device
    server->startServer();

    //create a task to trigger notification, with priority 1 and executed on core 0
    xTaskCreatePinnedToCore(
                    notificationTask,   /* Task function. */
                    "Notify Task",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &nTask,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */
    delay(500);

    //create a task to trigger payment, with priority 1 and executed on core 0
    xTaskCreatePinnedToCore(
                    paymentTask,   /* Task function. */
                    "Payment Task",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &pTask,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 1 */
    delay(500);

    Meter.reset();
  }
  else {
    LOG("\nsdkSample: MakerID can not be NULL!");
  }
}

void loop() {
  debugI("Water consumption measurement running on core: %d",xPortGetCoreID());
  debugI("Checking for threshold values...");

  // randomly change simulation period and pulse rate
  long frequency = random(flowSensor.capacity * flowSensor.kFactor);  // Hz
  long period = random(3 * frequency, 5000);                      // ms

  // simulate random flow meter pulses within a random period
  for (long i = 0; i < (long) ((float) period * (float) frequency / 1000.0f); i++) {
      Meter.count();
  }

  // wait that random period
  delay(period);

  // process the counted ticks
  Meter.tick(period);

  // Measure water consumption
  int totalConsumption = Meter.getTotalVolume();
  if(prevConsumedLtrs >= NOTIFY_4)
    consumedLtrs = totalConsumption - prevConsumedLtrs;
  else
    consumedLtrs = totalConsumption;

  debugI("Total up time: %f Seconds",Meter.getTotalDuration() / 1000.0f);
  debugI("Total water consumed so far: %d liters",totalConsumption);
  debugI("Consumed water in this interval: %d liters",consumedLtrs);
  if(consumedLtrs >= NOTIFY_1 && consumedLtrs < (NOTIFY_1 + (INTERVAL_THRESHOLD/10))){
     notify = true;
     notifyID = 1;
  }
  else if(consumedLtrs >= NOTIFY_2 && consumedLtrs < (NOTIFY_2 + (INTERVAL_THRESHOLD/10))){
     notify = true;
     notifyID = 2;
  }
  else if(consumedLtrs >= NOTIFY_3 && consumedLtrs < (NOTIFY_3 + (INTERVAL_THRESHOLD/10))){
     notify = true;
     notifyID = 3;
  }
  else if(consumedLtrs >= NOTIFY_4){
     notify = true;
     notifyID = 4;
     action = true;
     prevConsumedLtrs += NOTIFY_4;
     debugI("Previous consumed water in liters: %d", prevConsumedLtrs);
     consumedLtrs = consumedLtrs%NOTIFY_4;
  }

  #ifndef DEBUG_DISABLED
      Debug.handle();
  #endif
}

void notificationTask( void * pvParameters ){
  char *notifyUUID = NULL;
  for(;;){
    debugI("Notification Task running on core: %d",xPortGetCoreID());
    debugI("Checking for outstanding notifications to be triggered...");
    if(notify){
      if(server->isServerAvailable()){
        switch(notifyID){
          case 1: notifyUUID = NOTIFY_25_LTRS;
                  debugI(" %d liters of water consumed in this period", NOTIFY_1); break;
          case 2: notifyUUID = NOTIFY_50_LTRS;
                  debugI(" %d liters of water consumed in this period", NOTIFY_2); break;
          case 3: notifyUUID = NOTIFY_75_LTRS;
                  debugI(" %d liters of water consumed in this period", NOTIFY_3); break;
          case 4: notifyUUID = NOTIFY_100_LTRS;
                  debugI(" %d liters of water consumed in this period", NOTIFY_4); break;
          case 5: notifyUUID = NOTIFY_100_LTRS_PAYMENT;
                  debugI(" Autonomous payment done for %d liters of consumed water", NOTIFY_4); break;
        }

        BoTService* bot = NULL;
        //Prepare JSON Data to trigger an Action through POST call
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& doc = jsonBuffer.createObject();
        JsonObject& botData = doc.createNestedObject("bot");
        botData["deviceID"] = store->getDeviceID();
        botData["actionID"] = notifyUUID;
        botData["queueID"] = store->generateUuid4();

        char payload[200];
        doc.printTo(payload);
        debugI("Minified JSON Data to trigger Notification: %s", payload);

        //Create BoT Service Instance
        bot = new BoTService();

        //Hit End point to post
        String response = bot->post("/actions",payload);

        //Deallocate
        delete bot;

        //Turn Off Notify Flag if notification triggered successful
        if(response.indexOf("OK") != -1){
          notify = false;
          debugI("Notification triggered successfull, turned off notify flag");
        }
        else
          debugE("Notification trigger failed with response - %s",response.c_str());
      }
      else
        LOG("\nServer not available to trigger outstanding notification");
    }
    #ifndef DEBUG_DISABLED
      Debug.handle();
    #endif

    delay((INTERVAL_THRESHOLD/10)*1000);
  }
}

void paymentTask( void * pvParameters ){
  String actionUUID = String("0B41DF51-C027-4215-BE7C-2261DEAB7295");
  for(;;){
    debugI("Payment Task running on core: %d",xPortGetCoreID());
    debugI("Checking for outstanding payments to be triggered...");
    if(action){
      if(server->isServerAvailable()){
        debugI("Triggering payment for consumption of %d liters of water",NOTIFY_4);
        BoTService* bot = NULL;
        //Prepare JSON Data to trigger an Action through POST call
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& doc = jsonBuffer.createObject();
        JsonObject& botData = doc.createNestedObject("bot");
        botData["deviceID"] = store->getDeviceID();
        botData["actionID"] = actionUUID.c_str();
        botData["queueID"] = store->generateUuid4();

        char payload[200];
        doc.printTo(payload);
        debugI("Minified JSON Data to trigger payment: %s", payload);

        //Create BoT Service Instance
        bot = new BoTService();

        //Hit End point to post
        String response = bot->post("/actions",payload);

        //Deallocate
        delete bot;

        //Turn Off action flag if payment is successful
        if(response.indexOf("OK") != -1){
          action = false;
          debugI("Payment successful, turned off action flag");
          //Set notify flag to send payment done notification
          notify = true;
          notifyID = 5;
        }
        else
          debugE("Payment trigger failed with response - %s",response.c_str());
      }
      else
        LOG("\nServer not available to trigger outstanding payment");
    }
    #ifndef DEBUG_DISABLED
      Debug.handle();
    #endif

    delay((INTERVAL_THRESHOLD/10)*1000);
  }
}
