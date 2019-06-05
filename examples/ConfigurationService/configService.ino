/*
  configService.ino - Example sketch program to show the usage for ConfigurationService Component of ESP-32 SDK.
  Created by Lokesh H K, April 23, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include <ConfigurationService.h>
#include <Webserver.h>

ConfigurationService* configService;
KeyStore *store;
Webserver *server;

void setup() {

  store = KeyStore :: getKeyStoreInstance();
  store->loadJSONConfiguration();
  store->initializeEEPROM();

  //Get WiFi Credentials from given configuration
  //const char* WIFI_SSID = store->getWiFiSSID();
  //const char* WIFI_PASSWD = store->getWiFiPasswd();

  //Provide custom WiFi Credentials
  const char* WIFI_SSID = "LJioWiFi";
  const char* WIFI_PASSWD = "adgjmptw";

  //Override HTTPS
  store->setHTTPS(true);

  //Instantiate Webserver by using the custom WiFi credentials
  bool loadConfig = false;
  int logLevel = BoT_INFO;
  server = new Webserver(loadConfig,WIFI_SSID, WIFI_PASSWD,logLevel);

  //Enable board to connect to WiFi Network
  server->connectWiFi();

  configService = new ConfigurationService();
}

void loop() {
  //Proceed further if board connects to WiFi Network
  if(server->isWiFiConnected()){
    //Calling getDeviceInfo after initializing should return DeviceInformation
    configService->initialize(); // This internally sets device state as NEW
    debugI("\nDeviceInformation: %s", (configService->getDeviceInfo())->c_str());

    //Configuring the device for NEW Device
    configService->configureDevice();
    debugI("\nDevice State after configure for NEW Device: %d", store->getDeviceState());

    //Configuring the device for PAIRED Device
    store->setDeviceState(DEVICE_PAIRED);
    configService->configureDevice();
    debugI("\nDevice State after configure for PAIRED Device: %d", store->getDeviceState());

    //Configuring the device for ACTIVE Device
    store->setDeviceState(DEVICE_ACTIVE);
    configService->configureDevice();
    debugI("\nDevice State after configure for ACTIVE Device: %d", store->getDeviceState());
  }
  else {
    LOG("\nconfigService: ESP-32 board not connected to WiFi Network, try again");
    //Enable board to connect to WiFi Network
    server->connectWiFi();
  }

  #ifndef DEBUG_DISABLED
    Debug.handle();
  #endif

  delay(1*60*1000);
}
