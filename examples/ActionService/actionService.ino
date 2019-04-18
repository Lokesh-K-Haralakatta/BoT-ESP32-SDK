/*
  actionService.ino - Example sketch program to show the usage for ActionService Component of ESP-32 SDK.
  Created by Lokesh H K, April 17, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include <ActionService.h>

//Onboard LED Pin
int ledPin = 2;

ActionService* actService;
KeyStore* store;

void setup() {

  store = KeyStore :: getKeyStoreInstance();
  store->loadJSONConfiguration();

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

  //bot = new BoTService(host,uri,port);
  actService = new ActionService();

  //GET Actions for given device from BoT Service
  if(actService->getActions() != ""){
    LOG("\nActions retrieval from server is success..");
  }
  else {
    LOG("\nActions retrieval from server failed...");
  }

  //Trigger an action defined with the deviceID
  const char* actionID = "E6509B49-5048-4151-B965-BB7B2DBC7905";
  LOG("\nResponse from triggering action: %s", actService->triggerAction(actionID).c_str());

}

void loop() {
  digitalWrite(ledPin, LOW);
  delay(1000);
  digitalWrite(ledPin, HIGH);
  delay(1000);
}
