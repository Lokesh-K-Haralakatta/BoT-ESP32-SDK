/*
  sdkWrapperSampleWithWebOTA.ino - Sample sketch to show case the ESP-32 SDK usage through SDKWrapper.
  and also the sketch update through Web Update

  Created by Lokesh H K, February 01 2020.
  Released into the repository BoT-ESP32-SDK.
*/

/*
  Sample assumes that We have valid private-public key pair for the device, api public key.
  Also we have valid makerID, deviceID defined.
  All these information is provided through files present in data directory of the sample.
  Update the required configuration, key-pair for the device in the data directory
  Make sure to flash all the files present in data directory onto ESP32 board using Arduino IDE.
*/

/*
  To use the ESP-32 SDK, include BoT-ESP32-SDK.zip through Arduino IDE
*/

/* Change Partition Scheme to Minimal SPIFFS (1.9 MB App with OTA/190 KB SPIFFS)
   in Arduino IDE -> Tools after connecting ESP-32 board to facilitate OTA through
   Web onto ESP32 board
/*
  Here is the sketch flow:

  1. Gets makerID from the provided configuration
  2. Initializes the configuration, connects to given WiFi Network
  3. Pairs the device using BLE with the FINN Application
  4. Gets the actions defined at provided makerID portal
  5. Triggers the provided actionID at an interval of 1 minute it if's found in maker portal

  */

  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>
  #include <Update.h>

  #include <Storage.h>
  #include <Webserver.h>
  #include <SDKWrapper.h>

  //Custom WiFi Credentials
  #define WIFI_SSID "WiFi-SSID-Name"
  #define WIFI_PASSWD "WiFi-SSID-Password"

  //Declare service variables
  KeyStore *store = NULL;
  Webserver *server = NULL;
  SDKWrapper *sdk = NULL;

  //Action ID with frequency as "minutely"
  String actionIDMinutely = String("A42ABD19-3226-47AB-8045-8129DBDF117E");

  const char* host = "esp32";
  WebServer otaServer(80);

  //Variable to hold given deviceID value
  const char* deviceID = NULL;

 // Varibale store last time action was triggered
 unsigned long previousMillis = 0;

 // Action trigger interval in millis
 const long interval = 1*60*1000;

  /* Style */
String style =
"<style>#file-input,input{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}"
"input{background:#f1f1f1;border:0;padding:0 15px}body{background:#3498db;font-family:sans-serif;font-size:14px;color:#777}"
"#file-input{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}"
"#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:0%;height:10px}"
"form{background:#fff;max-width:258px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}"
".btn{background:#3498db;color:#fff;cursor:pointer}</style>";

/* Login page */
String loginIndex =
"<form name=loginForm>"
"<h1>ESP32 Login</h1>"
"<input name=userid placeholder='User ID'> "
"<input name=pwd placeholder=Password type=Password> "
"<input type=submit onclick=check(this.form) class=btn value=Login></form>"
"<script>"
"function check(form) {"
"if(form.userid.value=='admin' && form.pwd.value=='admin')"
"{window.open('/serverIndex')}"
"else"
"{alert('Error Password or Username')}"
"}"
"</script>" + style;

/* Server Index Page */
String serverIndex =
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
"<input type='file' name='update' id='file' onchange='sub(this)' style=display:none>"
"<label id='file-input' for='file'>   Choose file...</label>"
"<input type='submit' class=btn value='Update'>"
"<br><br>"
"<div id='prg'></div>"
"<br><div id='prgbar'><div id='bar'></div></div><br></form>"
"<script>"
"function sub(obj){"
"var fileName = obj.value.split('\\\\');"
"document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
"};"
"$('form').submit(function(e){"
"e.preventDefault();"
"var form = $('#upload_form')[0];"
"var data = new FormData(form);"
"$.ajax({"
"url: '/update',"
"type: 'POST',"
"data: data,"
"contentType: false,"
"processData:false,"
"xhr: function() {"
"var xhr = new window.XMLHttpRequest();"
"xhr.upload.addEventListener('progress', function(evt) {"
"if (evt.lengthComputable) {"
"var per = evt.loaded / evt.total;"
"$('#prg').html('progress: ' + Math.round(per*100) + '%');"
"$('#bar').css('width',Math.round(per*100) + '%');"
"}"
"}, false);"
"return xhr;"
"},"
"success:function(d, s) {"
"console.log('success!') "
"},"
"error: function (a, b, c) {"
"}"
"});"
"});"
"</script>" + style;

  void setup()
  {
    //Get KeyStore Instance
    store = KeyStore::getKeyStoreInstance();

    //Load the given configuration details from the SPIFFS
    store->loadJSONConfiguration();

    //Initialize EEPROM to load previous device state if any
    store->initializeEEPROM();

    //Get the given makerID from the configuration
    const char* makerID = store->getMakerID();

    //Get the given deviceID from the configuration
    deviceID = store->getDeviceID();

    //Continue if makerID is available
    if(makerID != NULL){
      //Variable to flag whether to load WiFi credentials from given configuration or not
      bool loadConfig = true;

      //Override HTTPS
      //store->setHTTPS(true);

      //Instantiate Webserver by using WiFi credentials from configuration
      //server = Webserver::getWebserverInstance(loadConfig);

      //Instantiate Webserver by using the custom WiFi credentials
      loadConfig = false;
      int logLevel = BoT_INFO;
      server = Webserver::getWebserverInstance(loadConfig,WIFI_SSID, WIFI_PASSWD,logLevel);

      //Enable board to connect to WiFi Network
      server->connectWiFi();

      //Instantiate SDK Wrapper
      sdk = new SDKWrapper();

      /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  otaServer.on("/", HTTP_GET, []() {
    otaServer.sendHeader("Connection", "close");
    otaServer.send(200, "text/html", loginIndex);
  });
  otaServer.on("/serverIndex", HTTP_GET, []() {
    otaServer.sendHeader("Connection", "close");
    otaServer.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  otaServer.on("/update", HTTP_POST, []() {
    otaServer.sendHeader("Connection", "close");
    otaServer.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = otaServer.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });

  otaServer.begin();

    }
    else {
      LOG("\nsdkWrapperSample: MakerID can not be NULL!");
    }
}

 void loop(){
  otaServer.handleClient();
  delay(1);

  unsigned long currentMillis = millis();
  unsigned long elapsedMillis = currentMillis - previousMillis;

   if(server->isWiFiConnected()){
     if( elapsedMillis >= interval) {
      // save the action trigger time
      previousMillis = currentMillis;
      debugI("\nElapsed Millis since last action trigger: %ld", elapsedMillis);

      debugI("\nAvalable free heap at the beginning of action trigger: %lu",ESP.getFreeHeap());

     int dState = store->getDeviceState();
     debugI("\nsdkWrapperSample :: Device State -> %s",store->getDeviceStatusMsg());
     //Check for the device state, should be active to trigger the action
     if(dState >= DEVICE_ACTIVE){
       //Trigger the action added with the paired device
       debugI("\nsdkWrapperSample :: Triggering action - %s", actionIDMinutely.c_str());
      if(sdk->triggerAction(actionIDMinutely.c_str())){
        debugI("\nsdkWrapperSample :: Triggering action successful...");
      }
      else {
        debugE("\nsdkWrapperSample :: Triggering action failed!");
      }
      debugI("\nAvalable free heap at the end of action trigger: %lu",ESP.getFreeHeap());
     }
     else {
       debugI("\nsdkWrapperSample: Device State is not active to trigger the action, Try pairing the device again:");
       sdk->pairAndActivateDevice();
     }
    }
  }
   else {
     debugW("\nsdkWrapperSample :: Board not connected to WiFi, try connecting again!");
     //Enable board to connect to WiFi Network
     server->connectWiFi();
   }

   #ifndef DEBUG_DISABLED
     Debug.handle();
   #endif
 }
