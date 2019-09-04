# BoT-ESP32-SDK

This read me contains the detailed steps to work with **FINN - Banking of Things Service** on ESP-32 Dev Kit Module using BoT-ESP32-SDK library.

## Supported Features
   | Sl. No        | SDK Feature                                | Status    | Remarks |
   | :-----------: |:-------------------------------------------| :---------| :-------|
   |        1      | Pairing through Bluetooth Low Energy (BLE) | Supported | Supported in both modes - Webserver as well as in SDK Module |
   |        2      | Pairing through QR Code                    | Supported | Supported only in Webserver mode through end point /qrcode to get generated QRCode for device to be paired |
   |        3      | Secured HTTP with BoT Service              | Supported | SDK has got an option of enable / disable https at runtime
   |        4      | Remote Debug                               | Supported | Enable to access the ESP-32 board through telnet client. This can be disabled for production |
   |        5      | Logging                                    | Supported | There are 4 different log levels supported for SDK - BoT_INFO, BoT_WARN, BoT_DEBUG and BoT_ERROR |
   |        6      | Offline Actions                            | Supported | Enables saving the actions when there is no internet connectivity available and processing with following request |
   
## Getting Started instructions for ESP-32 Dev Kit Module
- **Setting up of ESP-32 Dev Module**
  - Hardware Requirements
    - [ESP-32 Devkit Board](https://www.espressif.com/en/products/hardware/esp32-devkitc/overview)
    - Laptop with Windows, Linux or Mac OS
  - Software Requirements
    - [Setup Arduino IDE](https://www.arduino.cc/en/Guide/HomePage)
    - [Install ESP32 Library through Arduino IDE](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-mac-and-linux-instructions/)
    - [First Use of ESP32 Devkit Board](https://startingelectronics.org/articles/ESP32-WROOM-testing/)

- **Setting up of additional plugins and libraries required for BoT-ESP32-SDK**
  - Install [Arduino ESP32 Filesystem Uploader](https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/)
  - Download Zip [AsyncTCP Library](https://github.com/me-no-dev/AsyncTCP) and include in Arduino IDE
  - Download Zip [ESPAsyncWebServer Library](https://github.com/me-no-dev/ESPAsyncWebServer) and include in Arduino IDE
  - Download Zip [ArduinoJson Version 5.13.0](https://github.com/bblanchon/ArduinoJson/releases/tag/v5.13.0) and include in Arduino IDE
  - Download Zip [NTP Client](https://github.com/taranais/NTPClient/releases) and include in Arduino IDE
  - Download Zip [ESP32Ping](https://github.com/marian-craciunescu/ESP32Ping) and include in Arduino IDE
  - Install [RemoteDebug](https://www.arduinolibraries.info/libraries/remote-debug) either through Arduino IDE Libraries or by downloading latest ZIP and including in Arduino IDE

- **Setting up of BoT-ESP32-SDK Library on Arduino IDE**
  - Download ZIP from repository and install through Arduino IDE
  - The required configuration file and keys are available in the path `Arduino/libraries/BoT-ESP32-SDK/data` directory
  - Update the required configuration in `configuration.json` file and replace the key-pair files by retaining the same file names
  - Webserver endpoints usage sample sketch - `sdkSample.ino` is available in the path `Arduino/libraries/BoT-ESP32-SDK/examples/sdkSample` directory
  - SDKWrapper methods usage sample sketch - `sdkWrapperSample.ino` is available in the path `Arduino/libraries/BoT-ESP32-SDK/examples/sdkWrapperSample` directory
  - Make sure the line `#define DEBUG_DISABLED true` is commented in the file `BoTESP32SDK.h` to have Remote Redug feature enabled

- **Steps to execute sdkSample as it's purpose is to trigger the given action for every 1 minute**
  - Copy over the contents of `Arduino/libraries/BoT-ESP32-SDK/examples/sdkSample` into Arduino IDE Sketches directory
  - Copy over `Arduino/libraries/BoT-ESP32-SDK/data` into sdkSample sketch data directory
  - Update the configuration details and key-pair details in the files present in data directory
  - Change Partition Scheme from `Default` to `No OTA(Large APP)` in Arduino IDE -> Tools to avoid compilation error
  - Sketch has code to trigger actions with various frequencies like minutely, hourly, daily, monthly,half-yearly, yearly and always
  - Remove the comments for required action to be triggered based on the action frequency
  - Define the actions in [Maker Portal](https://maker.bankingofthings.io/login), update the actionIDs properly before executing the sketch
  - Compile and Upload sketch to ESP32 board using Arduino IDE
  - Upload data directory contents using `Arduino IDE -> Tools -> ESP32 Sketch Data Upload` option onto ESP-32 board
  - Wait for couple of seconds, the sketch should start running and connect to specified WiFi Network present in Sketch / Configuration
  - After ESP-32 board successfully connecting to specified WiFi Network, make a note of it's IP Address
  - Specified deviceID present in `configuration.json` is new, then SDK internally generates and saves QR Code for the device
  - Pair the new device through [FINN Mobile Application](https://docs.bankingofthings.io/mobile-app) using BLE or using saved QR Code
  - The QR Code can be accessed using the webserver's end point `/qrcode` running on ESP-32 board
  - Add the required service(s) in the [FINN Mobile Application](https://docs.bankingofthings.io/mobile-app) while pairing the device to enable action(s) to be triggered from the device
  - Open Serial Monitor Window in Arduino IDE to observe the sketch flow or SDK also supports RemoteDebug feature use `telnet ipAddr`
  - Once device is paired, observe the action getting triggered for every 1 minute
  
- **Steps to execute sdkWrapperSample as it's purpose is to directly call SDKWrapper methods to getActions and triggerAction for every 1 minute without using Async Webserver end points**
  - Copy over the contents of `Arduino/libraries/BoT-ESP32-SDK/examples/sdkWrapperSample` into Arduino IDE Sketches directory
  - Copy over `Arduino/libraries/BoT-ESP32-SDK/data` into sdkWrapperSample sketch data directory
  - Update the configuration details and key-pair details in the files present in data directory
  - Change Partition Scheme from `Default` to `No OTA(Large APP)` in Arduino IDE -> Tools to avoid compilation error
  - Define the actions in [Maker Portal](https://maker.bankingofthings.io/login), update the actionID properly before executing the sketch
  - Compile and Upload sketch to ESP32 board using Arduino IDE
  - Upload data directory contents using `Arduino IDE -> Tools -> ESP32 Sketch Data Upload` option onto ESP-32 board
  - Wait for couple of seconds, the sketch should start running and connect to specified WiFi Network present in Sketch / Configuration
  - After ESP-32 board successfully connecting to specified WiFi Network, make a note of it's IP Address
  - Specified deviceID present in `configuration.json` is new, then SDK internally kicks of BLE pairing and indefinitely waits for pairing get done through [FINN Mobile Application](https://docs.bankingofthings.io/mobile-app) followed by activating the device for triggering the actions
  - Pair the new device through [FINN Mobile Application](https://docs.bankingofthings.io/mobile-app) using BLE feature
  - Add service in [FINN Mobile Application](https://docs.bankingofthings.io/mobile-app) for the device to enable action trigger
  - Open Serial Monitor Window in Arduino IDE to observe the sketch flow or SDK also supports RemoteDebug feature use `telnet ipAddr`
  - Once device is paired, observe the action getting triggered for every 1 minute

### Guidelines to develop sketch using ESP-32 SDK
- **Configuration Format**
  - The required configuration details for the sketch are expected to be provided through file `configuration.json`
  - The `configuration.json` is expected to be present in `data` directory with in the sketch directory
  - The `configuration.json` is expected to contain below given mandatory key-value pairs:
    - WiFi SSID Name
    - WiFi SSID Password
    - Maker ID
    - Device ID
  - The key-value pairs required for Multipair functionality are optional. Be default, the multipair functionaity is disabled. To enable, explicity 2 parameters need to be provided through `configuration.json`
    - Multipair flag value as true
    - Value for Alternative Device Id
  - By default, the HTTPS feature is enabled. To disable HTTPS and have only HTTP to communicate with BoT Service, explicitly it has to be speciifed through `https` parameter set to false in `configuration.json`
  - Below given is sample snippet of `configuration.json` file including all key-value pairs:
      ```
        {
	          "wifi_ssid": "PJioWiFi",
	          "wifi_passwd": "qwertyuiop",
	          "https": "true",
	          "maker_id": "469908A3-8F6C-46AC-84FA-4CF1570E564B",
	          "device_id": "eb25d0ba-2dcd-4db2-8f96-a4fbe54dbffc",
	          "multipair": "false",
	          "alt_device_id": "KA-04 ME-3475"
        }

      ```
- **Device Key-Pair for secure data exchange**
  - Generate RSA Key pair on any of the system by referring to [Generating new SSH Key](https://help.github.com/en/enterprise/2.16/user/articles/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent#generating-a-new-ssh-key)
  - Copy the contents of private key (id_rsa) to the file `private.key` into sketch data directory
  - Copy the contents of public key (id_rsa.pub) to the file `public.key` into sketch data directory
  - Copy the contents of BoT Service public key to the file `api.pem` into sketch data directory

- **Secure HTTP (HTTPS) Feature**
  - ESP-32 SDK supports HTTPS by default with the BoT Service Calls
  - To have Secure HTTP Communication between BoT Service and Sketch, we need CA Certificate
  - Export GoDaddy Root Certificate from trusted certificate store present in browser to file `cacert.cer`
  - Place the file `cacert.cer` into sketch data directory and flash onto ESP-32 Board along with other files
  - SDK enables HTTPS by default, we have also have an option of disabling HTTPS through configuration
  - In order to disable HTTPS, specify the flag `"https": "false"` in `configuration.json` as shown above and flash onto ESP32 board
  - SDK also has a method `setHTTPS(bool value)` in KeyStore class to override HTTPS flag value provided through `configuration.json`

- **Loading configuration details from `configuration.json` in sketch**
  - To perform the required actions, first step is to load configuration from `configuration.json` file
  - To load configuration, we need an instance to `KeyStore` Class present in `Storage.h` of ESP-32 SDK
  - Below given code snippet illustrates loading the configuration from `configuration.json` file in sketch code
      ```
        #include <Storage.h>
        ...
        KeyStore* store = KeyStore::getKeyStoreInstance(); // Get an instance of KeyStore Class
        store->loadJSONConfiguration(); // Load given configuration from `configuration.json` file
        store->initializeEEPROM(); // Initialize EEPROM to get/update device state in the workflow

      ```

- **Enable Remote Debug Feature and handle in sketch**
  - SDK Supports Remote Debug feature through telnet which helps in code debugging over WiFi through telnet and also helps in setting different log levels
  - Comment the line `#define DEBUG_DISABLED true` in file `BoTESP32SDK.h` to enable Remote Debug Feature
      ```
        //RemoteDebug Specifics go here

        //#define DEBUG_DISABLED true
        #include <RemoteDebug.h>

      ```
  - The supported log levels are defined in file `BoTESP32SDK.h`
      ```
        //Debug levels for RemoteDebug feature
        //Effective only if RemoteDebug is enabled
        #define BoT_DEBUG 2
        #define BoT_INFO 3
        #define BoT_WARNING 4
        #define BoT_ERROR 5

      ```
  - The supported macros to log the messages based on specified log level are:
    - `debugD` => Debug Message
    - `debugI` => Info Message
    - `debugW` => Warning Message
    - `debugE` => Error Message
  - The default loglevel set in SDK is `BoT_INFO`
  - Call `Debug.handle()` at the end of `loop()` function in the sketch as shown below to transfer current log messages to telnet client over WiFi:
      ```
        #ifndef DEBUG_DISABLED
          Debug.handle();
        #endif

      ```
  - To view the log messages remotely, get the ESP-32 board's IP address after connecting to WiFi Network and connect through telnet from remote system
  - The log levels can also be controlled remotely through telnet by specifying appropriate command from the telnet client window followed by pressing enter key
  - We can also completely disable the log messages to reduce the overread for Production / Release by uncommenting the line `#define DEBUG_DISABLED true` in file `BoTESP32SDK.h`

- **Connecting ESP-32 board to WiFi Network and Starting AsyncWebServer on ESP-32 board**
  - ESP-32 board need to be connected to WiFi Network provided through `configuration.json` file or Customized Network present in the sketch before carrying out any further tasks
  - In either case, we need an instance to `Webserver` Class present in `Webserver.h` of ESP-32 SDK
  - Below code snippet shows an instance to `Webserver` Class that can be connected to custom WiFi Network
      ```
        #include <Webserver.h>
        #define WIFI_SSID "LJioWiFi"
        #define WIFI_PASSWD "adgjmptw"
        ...
        Webserver *server = new Webserver(false,WIFI_SSID, WIFI_PASSWD);
        ...

      ```
  - Below code snippet shows an instance to `Webserver` Class that can be connected to configured WiFi Network
      ```
        #include <Webserver.h>
        ...
        Webserver *server = new Webserver(true);
        ...

      ```
  - As we have an instance to `Webserver` Class, next is to invoke member function `connectWiFi()` to make ESP32 board get connected to set WiFi Network within `Webserver` instance.
  - Next, we have member function `isWiFiConnected()` to invoke to make sure ESP32 board connected to WiFi Network
  - As we have confirmation on ESP-32 connected to WiFi Network, next step is to invoke member function `startServer()` to start AsyncWebserver on ESP-32 board
  - The AsyncWebserver on ESP-32 board provides below list of end points
    - `/pairing`: Used to wait for change of device state and activate the device
    - `/actions`: Used to retrieve the list of the actions defined for the makerID as well as trigger an action
    - `/qrcode`: Used to access the generated and saved QR Code for the device
  - Above sequence of steps are depicted in below given code snippet
      ```
        .......
        server->connectWiFi();
        if(server->isWiFiConnected()){

          store = KeyStore::getKeyStoreInstance();
          store->initializeEEPROM();

          //Start Async Webserver on ESP32 board on predefined port 3001
          server->startServer();
          .......
        }

      ```
  - The call to `server->startServer` internally makes call to BLE initialization enabling device pairing through Bluetooth with the companion application trying to communicate from iOS / Android device and also generates and saves QR Code for the device onto SPIFFS
  - Hit the endpoint `/qrcode` to get access to device QRCode and pair the device using FINN APP from iOS / Android device
  - Device gets paired successfully, then call to activate the device is made and the device is ready to trigger the actions if device activation is successful
  - Device fails in getting paired, then we can re-attempt to pair the device using the end point `/pairing` defined by the webserver

- **Retrieve defined actions for the makerID from BoT Service using AsyncWebserver end point `/actions`**
  - After successful start of AsyncWebserver, we can retrieve the actions defined for the provided `makerID`
  - Below given code snippet shows simple HTTPClient code to retrieve actions from BoT Service
      ```
        .......
           if(server->isServerAvailable()){
             //Retrieve defined actions for given makerID
             httpClient->begin((server->getBoardIP()).toString(),3001,"/actions");
             //Set HTTP Call timeout as 1 min
             httpClient->setTimeout(1*60*1000);

             int httpCode = httpClient->GET();
             String payload = httpClient->getString();
             httpClient->end();

             if(httpCode == 200){
               debugI("\nstartAsyncServer: Actions Retrieved from BoT Service: %s", payload.c_str());
             }
             else {
               debugI("\nstartAsyncServer: Actions Retrieval failed with httpCode - %d", httpCode);
             }
        .......

      ```
  - The complete sketch is available at path `examples/AsyncWebServer/startAsyncServer.ino`

- **Pairing the device followed by activating it to trigger actions using AsyncWebserver end point `/pairing`**
  - We can activate the paired device by hitting the defined end point `/pairing` to trigger actions
  - Below given code snippet shows simple HTTPClient code to hit `/pairing` end point
      ```
        .......
        if(server->isServerAvailable()){
          httpClient->begin((server->getBoardIP()).toString(),port,"/pairing");
          //Set HTTP Call timeout as 1 min
          httpClient->setTimeout(1*60*1000);

          httpCode = httpClient->GET();
          payload = httpClient->getString();
          httpClient->end();

          if(httpCode == 200){
            if(store->getDeviceState() == DEVICE_ACTIVE){
              debugI("\nstartAsyncServer: Device Activation Successful");
            }
            else {
              debugI("\nstartAsyncServer: Device Not Activated, try again");
            }
          else {
            debugI("\nstartAsyncServer: Calling /pairing failed with httpCode - %d", httpCode);
          }
        }
        else {
          LOG("\nWebserver not available on ESP32 board!");
        }
        .......

      ```
  - The complete sketch is available at path `examples/AsyncWebServer/startAsyncServer.ino`
  
- **Triggering the defined action using AsyncWebserver end point `/actions`**
  - We can trigger the defined actions periodically using the `/actions` end point
  - Below given code snippet shows simple HTTPClient making POST call on `/actions`
      ```
        .....
        //Check for Webserver availability to trigger the action
        if(server->isServerAvailable()){
          //Check for the device state, should be active
          if(store->getDeviceState() >= DEVICE_ACTIVE){
            //Instantiate HTTP Client to send HTTP Request to trigger the action
            httpClient = new HTTPClient();
            httpClient->begin((server->getBoardIP()).toString(),port,"/actions");

            //Prepare body with actionID
            String body = (String)"{\"actionID\": \"" + actionID +   "\"";

            //Add alternativeID if device is DEVICE_MULTIPAIR
            if(store->isDeviceMultipair()){
              body.concat(",\"alternativeID\": \"" + String(store->getAlternateDeviceID()) +"\"");
            }
            body.concat(" } ");
            debugI("\nsdkSample: triggerAction Body contents: %s",body.c_str());

            //Set required headers for HTTP Call
            httpClient->addHeader("Content-Type", "application/json");
            httpClient->addHeader("Content-Length",String(body.length()));

            //Set HTTP Call timeout as 1 min
            httpClient->setTimeout(1*60*1000);

            //Call HTTP Post to submit action
            int httpCode = httpClient->POST(body);

            //Get response body contents
            String payload = httpClient->getString();

            //Check for successful triggerring of given action
            if(httpCode == 200){
               debugI("\nsdkSample: Action submitted to server...");
            }
            else {
               debugE("\nsdkSample: Action submission failed with httpCode - %d and message: %s", httpCode, payload.c_str());
            }

            //End http
            httpClient->end();

            //Deallocate memory allocated for httpClient
            delete httpClient;

            ......
          }
          .......

      ```
  - Complete sketch is available at the path `examples/sdkSample/sdkSample.ino`

- **Consuming ESP-32 SDK using SDKWrapper Class**
  - SDK also supports it's direct usage as another module / library through the SDKWrapper Class Methods
  - SDKWrapper Class provides following methods, those can be directly used in the sketch bypassing the dependency on Webserver
    - `pairAndActivateDevice`: Used to pair the device with FINN Mobile Application through BLE
    - `getActions`: Used to retrieve all the available actions defined at the provided maker portal in JSON String format
    - `triggerAction`: Used to trigger an action for Single Pair as well as for Multipair device
  - The sample workflow for using the SDKWrapper Class methods are showcased in the example sketch `sdkWrapperSample.ino` available at the path `examples/sdkWrapperSample`
  
- **Must do steps before compiling and uploading sketch to ESP-32 board**
  - Make sure the data directory contents are proper for the sketch
  - Flash the data directory contents onto board from `Arduino IDE -> Tools -> ESP32 Sketch Data Upload` Option
  - Change the partition scheme from Default to No OTA (Large APP) using `Arduino IDE -> Tools` menu
  - Make sure all the required dependent libraries for ESP-32 SDK are installed through Arduino IDE
