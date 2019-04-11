# BoT-ESP32-SDK

This read me contains the detailed steps to work with **FINN - Banking of Things Service** on ESP-32 Dev Kit Module using BoT-ESP32-SDK library.

### Setting up of ESP-32 Dev Module
    - Hardware Requirements
      - ESP-32 Devkit Board
    - Software Requirements
      - [Setup Arduino IDE](https://www.arduino.cc/en/Guide/HomePage)
      - [Install ESP32 Library through Arduino IDE](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-mac-and-linux-instructions/)
      - [First Use of ESP32 Devkit Board](https://startingelectronics.org/articles/ESP32-WROOM-testing/)

### Setting up of additional plugins and libraries required for BoT-ESP32-SDK
    - Install [Arduino ESP32 Filesystem Uploader](https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/)
    - Download Zip [AsyncTCP Library](https://github.com/me-no-dev/AsyncTCP) and include in Arduino IDE
    - Download Zip [ESPAsyncWebServer Library](https://github.com/me-no-dev/ESPAsyncWebServer) and include in Arduino IDE
    - Download Zip [ArduinoJson Version 5.13.0](https://github.com/bblanchon/ArduinoJson/releases/tag/v5.13.0) and include in Arduino IDE

### Setting up of BoT-ESP32-SDK Library on Arduino IDE
    - Download ZIP from repository and install through Arduino IDE
    - The required configuration file and keys are available in the path `Arduino/libraries/BoT-ESP32-SDK/data` directory
    - Update the required configuration in `configuration.json` file and replace the key-pair files by retaining the same file names
    - Sample example sketches are also available in the path `Arduino/libraries/BoT-ESP32-SDK/examples` directory
