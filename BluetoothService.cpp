/*
BluetoothService.cpp - Contains class and methods for BLE communication with the companion application
Created by Lokesh H K, April 29, 2019.
Released into the repository BoT-ESP32-SDK.
*/

#include "BluetoothService.h"

bool BluetoothService :: clientConnected = false;

BluetoothService :: BluetoothService(){
  deviceName = NULL;
  store = KeyStore :: getKeyStoreInstance();
  bleServer = NULL;
  bleService = NULL;
  bleDeviceCharacteristic = NULL;
  bleDeviceInfoCharacteristic = NULL;
  bleNetworkCharacteristic = NULL;
  bleConfigureCharacteristic = NULL;
}

void BluetoothService :: setClientConnected(bool status){
  clientConnected = status;
}

bool BluetoothService :: isBLEClientConnected(){
  return ((clientConnected)?true:false);
}

void BluetoothService :: initializeBLE(const char* dName){
  if(dName != NULL){
    deviceName = new char[strlen(dName)+1];
    strcpy(deviceName,dName);
  }
  else {
    deviceName = new char[15];
    strcpy(deviceName,"BoT-ESP-32");
  }

  LOG("\nBluetoothService :: initializeBLE: DeviceName set to %s", deviceName);

  store->loadJSONConfiguration();
  store->retrieveAllKeys();

  BLEDevice::init(deviceName);

  bleServer = BLEDevice::createServer();
  bleServer->setCallbacks(new BoTServerCallbacks());

  bleService = bleServer->createService(SERVICE_UUID);

  LOG("\nBluetoothService :: initializeBLE: BLE Server and BLE Service setup done");

  bleDeviceCharacteristic = bleService->createCharacteristic(DEVICE_CHARACTERISTIC_UUID,
                                                            BLECharacteristic::PROPERTY_READ);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& doc = jsonBuffer.createObject();
  doc["deviceID"] = store->getDeviceID();
  doc["makerID"] = store->getMakerID();
  doc["publicKey"] = store->getDevicePublicKey();

  char dInfo[1024];
  doc.printTo(dInfo);

  LOG("\nBluetoothService :: initializeBLE: %s", dInfo);
  bleDeviceCharacteristic->setValue(std::string(dInfo));

  LOG("\nBluetoothService :: initializeBLE: Setting Device Characteristic is done");

  bleDeviceInfoCharacteristic = bleService->createCharacteristic(DEVICE_INFO_CHARACTERISTIC_UUID,
                                                                BLECharacteristic::PROPERTY_READ);
  bleDeviceInfoCharacteristic->setValue("{}");
  LOG("\nBluetoothService :: initializeBLE: Setting Device Info Characteristic is done");

  bleNetworkCharacteristic = bleService->createCharacteristic(DEVICE_NETWORK_CHARACTERISTIC_UUID,
                                                                BLECharacteristic::PROPERTY_READ);
  bleNetworkCharacteristic->setValue("{}");
  LOG("\nBluetoothService :: initializeBLE: Setting Network Characteristic is done");

  bleConfigureCharacteristic = bleService->createCharacteristic(CONFIGURE_CHARACTERISTIC_UUID,
                                                                                BLECharacteristic::PROPERTY_READ |
                                                                                BLECharacteristic::PROPERTY_WRITE);
  bleConfigureCharacteristic->setValue("{}");
  LOG("\nBluetoothService :: initializeBLE: Setting Configuration Characteristic is done");

  bleService->start();
  LOG("\nBluetoothService :: initializeBLE: BLE Service Started");

  bleServer->getAdvertising()->addServiceUUID(bleService->getUUID());
  bleServer->getAdvertising()->start();
  LOG("\nBluetoothService :: initializeBLE: BLE Server Started Advertising, we should see in companion device");
}

void BluetoothService :: deInitializeBLE(){
  LOG("\nBluetoothService :: deInitializeBLE: Stopping and Clearing BLE Service");

  bleServer->getAdvertising()->stop();
  LOG("\nBluetoothService :: deInitializeBLE: Advertising stopped...");

  bleService->stop();
  LOG("\nBluetoothService :: deInitializeBLE: BLE Service stopped...");

  bleServer->removeService(bleService);
  LOG("\nBluetoothService :: deInitializeBLE: BLE Service removed from BLE Server...");

  if(bleConfigureCharacteristic != NULL) delete bleConfigureCharacteristic;
  if(bleNetworkCharacteristic != NULL) delete bleNetworkCharacteristic;
  if(bleDeviceInfoCharacteristic != NULL) delete bleDeviceInfoCharacteristic;
  if(bleDeviceCharacteristic != NULL) delete bleDeviceCharacteristic;
  LOG("\nBluetoothService :: deInitializeBLE: All characteristics memory freed...");

  if(bleService != NULL) delete bleService;
  LOG("\nBluetoothService :: deInitializeBLE: BLE Service instance freed...");

  if(bleServer != NULL) delete bleServer;
  LOG("\nBluetoothService :: deInitializeBLE:  BLE Server instance freed...");

  BLEDevice::deinit(true);
  LOG("\nBluetoothService :: deInitializeBLE: BLE Device deinitialized...");

  if(deviceName != NULL) delete deviceName;
  LOG("\nBluetoothService :: deInitializeBLE: Memory used for deviceName freed...");

  LOG("\nBluetoothService :: deInitializeBLE: Done...");
}
