/*
  bleService.ino - Example sketch program to show the usage of Bluetooth Service.
  Created by Lokesh H K, April 30, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include <BluetoothService.h>

BluetoothService *ble;

void setup(){
  ble = new BluetoothService();
  ble->initializeBLE("BoT-ESP32");
}

void loop(){
  if(ble->isBLEClientConnected()){
    LOG("\nBLE Client Status: Connected");
  }
  else {
    LOG("\nBLE Client Status: Disconnected");
  }
  
  delay(2000);
}
