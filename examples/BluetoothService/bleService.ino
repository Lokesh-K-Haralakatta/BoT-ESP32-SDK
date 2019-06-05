/*
  bleService.ino - Example sketch program to show the usage of Bluetooth Service.
  Created by Lokesh H K, April 30, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include <BluetoothService.h>
#include <Webserver.h>

BluetoothService *ble;
Webserver *server;
KeyStore* store;
int counter = 1;

void setup(){
  store = KeyStore :: getKeyStoreInstance();
  //Provide custom WiFi Credentials
  const char* WIFI_SSID = "LJioWiFi";
  const char* WIFI_PASSWD = "adgjmptw";

  //Instantiate Webserver by using the custom WiFi credentials
  bool loadConfig = false;
  int logLevel = BoT_DEBUG;
  server = new Webserver(loadConfig,WIFI_SSID, WIFI_PASSWD,logLevel);

  //Enable board to connect to WiFi Network
  server->connectWiFi();

}

void loop(){
  //Proceed further if board connects to WiFi Network
  if(server->isWiFiConnected()){
    ble = new BluetoothService();
    if(counter == 1)
      ble->initializeBLE(NULL);
    else {
      String bleDeviceName = "BoT-ESP32-" + String(counter);
      ble->initializeBLE(bleDeviceName.c_str());
    }

    while(!ble->isBLEClientConnected()){
      debugI("\nbleService: Waiting for BLE Client to connect");
      if(ble->isBLEClientConnected()) break;
      delay(3000);
    }
    while(ble->isBLEClientConnected()){
      debugI("\nbleService: Waiting for BLE Client to disconnect");
      if(!ble->isBLEClientConnected()) break;
      delay(3000);
    }
    ble->deInitializeBLE();
    delete ble;
    counter++;
  }
  else {
    LOG("\nbleService: ESP-32 board not connected to WiFi Network, try again");
    //Enable board to connect to WiFi Network
    server->connectWiFi();
  }

  #ifndef DEBUG_DISABLED
    Debug.handle();
  #endif

  delay(2000);
}
