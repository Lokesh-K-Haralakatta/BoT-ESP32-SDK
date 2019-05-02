# BoT-ESP32-SDK

This read me contains the detailed steps to work with **FINN - Banking of Things Service** on ESP-32 Dev Kit Module using BoT-ESP32-SDK library.

### Getting Started instructions for ESP-32 Dev Kit Module
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

- **Setting up of BoT-ESP32-SDK Library on Arduino IDE**
  - Download ZIP from repository and install through Arduino IDE
  - The required configuration file and keys are available in the path `Arduino/libraries/BoT-ESP32-SDK/data` directory
  - Update the required configuration in `configuration.json` file and replace the key-pair files by retaining the same file names
  - Sample example sketch - sdkSample.ino is available in the path `Arduino/libraries/BoT-ESP32-SDK/examples/SDKSample` directory

- **Steps to execute sdkSample as it's purpose is to trigger the given action for every 1 minute**
  - Copy over the contents of `Arduino/libraries/BoT-ESP32-SDK/examples/SDKSample/sdkSample.ino` into Arduino IDE Sketches directory
  - Copy over `Arduino/libraries/BoT-ESP32-SDK/data` into sdkSample sketch data directory
  - Update the configuration details and key-pair details in the files present in data directory
  - Change Partition Scheme from `Default` to `No OTA(Large APP)` in Arduino IDE -> Tools to avoid compilation error
  - Compile and Upload sketch to ESP32 board using Arduino IDE
  - Flash data directory contents using Arduino IDE -> Tools -> ESP32 Sketch Data Upload option
  - Open Serial Monitor Window in Arduino IDE to observe the sketch flow
  - Pair the new device through Companion Application using Bluetooth Communication, if device is not yet paired
  - Once device is paired, observe the action getting triggered for every 1 minute
