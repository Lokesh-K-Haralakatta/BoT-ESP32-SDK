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
  store->loadJSONConfiguration();
  store->retrieveAllKeys();
  debugD("\nBluetoothService :: initializeBLE: Loaded config and all keys from KeyStore");

  if(dName != NULL){
    deviceName = new char[strlen(dName)+1];
    strcpy(deviceName,dName);
  }
  else {
    const char* dName = store->getDeviceName();
    deviceName = new char[strlen(dName)+1];
    strcpy(deviceName,dName);
  }
  debugD("\nBluetoothService :: initializeBLE: DeviceName set to %s", deviceName);

  BLEDevice::init(deviceName);
  debugD("\nBluetoothService :: initializeBLE: BLEDevice::init done");

  bleServer = BLEDevice::createServer();
  debugD("\nBluetoothService :: initializeBLE: BLEDevice::createServer done");

  bleServer->setCallbacks(new BoTServerCallbacks());
  debugD("\nBluetoothService :: initializeBLE: bleServer->setCallbacks done");

  bleService = bleServer->createService(SERVICE_UUID);
  debugD("\nBluetoothService :: initializeBLE: BLE Server and BLE Service setup done");

  bleDeviceCharacteristic = bleService->createCharacteristic(DEVICE_CHARACTERISTIC_UUID,
                                                            BLECharacteristic::PROPERTY_READ);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& doc = jsonBuffer.createObject();
  doc["deviceID"] = store->getDeviceID();
  doc["makerID"] = store->getMakerID();
  doc["publicKey"] = store->getDevicePublicKey();
  doc["multipair"] = 0;

  if(store->getDeviceState() == DEVICE_MULTIPAIR){
    doc["multipair"] = 1;
    doc["aid"] = store->getAlternateDeviceID();
  }
  
  char dInfo[1024];
  doc.printTo(dInfo);

  debugD("\nBluetoothService :: initializeBLE: %s", dInfo);
  bleDeviceCharacteristic->setValue(std::string(dInfo));

  debugD("\nBluetoothService :: initializeBLE: Setting Device Characteristic is done");

  bleDeviceInfoCharacteristic = bleService->createCharacteristic(DEVICE_INFO_CHARACTERISTIC_UUID,
                                                                BLECharacteristic::PROPERTY_READ);
  bleDeviceInfoCharacteristic->setValue("{}");
  debugD("\nBluetoothService :: initializeBLE: Setting Device Info Characteristic is done");

  bleNetworkCharacteristic = bleService->createCharacteristic(DEVICE_NETWORK_CHARACTERISTIC_UUID,
                                                                BLECharacteristic::PROPERTY_READ);
  bleNetworkCharacteristic->setValue("{}");
  debugD("\nBluetoothService :: initializeBLE: Setting Network Characteristic is done");

  bleConfigureCharacteristic = bleService->createCharacteristic(CONFIGURE_CHARACTERISTIC_UUID,
                                                                                BLECharacteristic::PROPERTY_READ |
                                                                                BLECharacteristic::PROPERTY_WRITE);
  bleConfigureCharacteristic->setValue("{}");
  debugD("\nBluetoothService :: initializeBLE: Setting Configuration Characteristic is done");

  bleService->start();
  debugD("\nBluetoothService :: initializeBLE: BLE Service Started");

  bleServer->getAdvertising()->addServiceUUID(bleService->getUUID());
  bleServer->getAdvertising()->start();
  debugI("\nBluetoothService :: initializeBLE: BLE Server Started Advertising, we should see bluetooth device with name - %s in companion application",deviceName);
}

void BluetoothService :: deInitializeBLE(){
  debugD("\nBluetoothService :: deInitializeBLE: Stopping and Clearing BLE Service");

  bleServer->getAdvertising()->stop();
  debugD("\nBluetoothService :: deInitializeBLE: Advertising stopped...");

  bleService->stop();
  debugD("\nBluetoothService :: deInitializeBLE: BLE Service stopped...");

  bleServer->removeService(bleService);
  debugD("\nBluetoothService :: deInitializeBLE: BLE Service removed from BLE Server...");

  if(bleConfigureCharacteristic != NULL) delete bleConfigureCharacteristic;
  if(bleNetworkCharacteristic != NULL) delete bleNetworkCharacteristic;
  if(bleDeviceInfoCharacteristic != NULL) delete bleDeviceInfoCharacteristic;
  if(bleDeviceCharacteristic != NULL) delete bleDeviceCharacteristic;
  debugD("\nBluetoothService :: deInitializeBLE: All characteristics memory freed...");

  if(bleService != NULL) delete bleService;
  debugD("\nBluetoothService :: deInitializeBLE: BLE Service instance freed...");

  if(bleServer != NULL) delete bleServer;
  debugD("\nBluetoothService :: deInitializeBLE:  BLE Server instance freed...");

  BLEDevice::deinit(true);
  debugD("\nBluetoothService :: deInitializeBLE: BLE Device deinitialized...");

  if(deviceName != NULL) delete deviceName;
  debugD("\nBluetoothService :: deInitializeBLE: Memory used for deviceName freed...");

  debugD("\nBluetoothService :: deInitializeBLE: Done...");
}
