/*
  configService.ino - Example sketch program to show the usage for ConfigurationService Component of ESP-32 SDK.
  Created by Lokesh H K, April 23, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include <ConfigurationService.h>
//Onboard LED Pin
int ledPin = 2;

ConfigurationService* configService;
KeyStore *store;
void setup() {

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  store = KeyStore :: getKeyStoreInstance();
  store->loadJSONConfiguration();
  store->initializeEEPROM();

  const char* WIFI_NAME = store->getWiFiSSID();
  const char* WIFI_PASSWORD = store->getWiFiPasswd();

  LOG("\nConnecting to %s", WIFI_NAME);
  //WiFi.disconnect();
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    LOG("\nTrying to connect to Wifi Network - %s", WIFI_NAME);
  }

  LOG("\nSuccessfully connected to WiFi network - %s", WIFI_NAME);
  LOG("\nIP address: ");
  Serial.println(WiFi.localIP());

  configService = new ConfigurationService();

  //Calling getDeviceInfo after initializing should return DeviceInformation
  configService->initialize(); // This internally sets device state as NEW
  LOG("\nDeviceInformation: %s", (configService->getDeviceInfo())->c_str());

  //Configuring the device for NEW Device
  //configService->configureDevice();
  //LOG("\nDevice State: %d", store->getDeviceState());

  //Configuring the device for PAIRED Device
  //store->setDeviceState(DEVICE_PAIRED);
  //configService->configureDevice();
  //LOG("\nDevice State: %d", store->getDeviceState());

  //Configuring the device for ACTIVE Device
  store->setDeviceState(DEVICE_ACTIVE);
  configService->configureDevice();
  LOG("\nDevice State: %d", store->getDeviceState());
}

void loop() {
  digitalWrite(ledPin, LOW);
  delay(1000);
  digitalWrite(ledPin, HIGH);
  delay(1000);
}
