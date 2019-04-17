/*
  pairingService.ino - Example sketch program to show the usage for
                       PairingService and ActivationService Components of ESP-32 SDK.
  Created by Lokesh H K, April 16, 2019.
  Released into the repository BoT-ESP32-SDK.
*/
#include <PairingService.h>
#include <Storage.h>

//Onboard LED Pin
int ledPin = 2;

PairingService* ps;
KeyStore* store;

void setup() {

  store = KeyStore :: getKeyStoreInstance();
  store->loadJSONConfiguration();
  store->initializeEEPROM();

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

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

  ps = new PairingService();

  //Call pairDevice to pair the deviceID present in JSON Cofiguration

  //For already paired deviceID, we should see pairing successful message
  //Followed by activation successful message
  //Device State after pairDevice should be DEVICE_ACTIVE i.e. 2

  //For new deviceID, set the device state to DEVICE_NEW
  //We should see waiting for 10 times to see the response status as false
  //Device State after pairDevice should be DEVICE_NEW i.e. 0

  //For all other states, the pairDevice method simply returns immediately
  //store->setDeviceState(DEVICE_PAIRED);
  //store->setDeviceState(DEVICE_ACTIVE);
  //store->setDeviceState(DEVICE_MULTIPAIR);
  //Device State after pairDevice should be same as before

  store->setDeviceState(DEVICE_NEW);
  LOG("\n Given deviceID in configuration: %s", store->getDeviceID());
  LOG("\n Device State stored in EEPROM: %d", store->getDeviceState());
  LOG("\n Now trying to pair the device, followed by activating the device");

  //Should behave as mentioned above
  ps->pairDevice();

  LOG("\n Device State after pairDevice return: %d", store->getDeviceState());
}

void loop() {
  digitalWrite(ledPin, LOW);
  delay(1000);
  digitalWrite(ledPin, HIGH);
  delay(1000);
}
