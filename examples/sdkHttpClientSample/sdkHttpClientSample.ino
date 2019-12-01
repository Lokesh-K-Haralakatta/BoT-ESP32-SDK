/*
  sdkHttpClientSample.ino - Sample sketch to show case the ESP-32 SDK usage
  using Http Client Client Library
  Created by Lokesh H K, November 29, 2019.
  Released into the repository BoT-ESP32-SDK.

  To use the ESP-32 SDK, include BoT-ESP32-SDK.zip through Arduino IDE

  sdkHttpClientSample sketch is a htpp client program to interact with already
  running Webserver on some other board / cloud / url. Make a note of Webserver IP
  and update in this sketch before flashing on to board.

  Change Partition Scheme to No OTA (Large APP) in Arduino IDE -> Tools after
  connecting ESP-32 board to avoid the error message saying "Sketch is too big"
  before compiling and uploading Sketch to ESP-32


  Here is the sketch flow:

  1. Sketch has code to trigger actions with various frequencies like minutely,
     hourly, daily, monthly,half-yearly, yearly and always
  2. Define the actions in maker portal, pair and activate the device, add service
     in companion app, update the actionIDs properly before executing the sketch
  3. Update required action to be triggered based on the action frequency in the
     URI formation statement
  4. Webserver provides the endpoints /qrcode, /actions, /pairing and /action/actionID
     for direct interaction
  5. Sketch can retrieve the defined actions from the remote server for the
     configured MakerId
  6. Sketch can request server to pair the board if it's not already paired
  7. Sketch can request to activate the board if it's not already activated for payments
  8. Sketch triggers the minutely payment action repeatedly at the frequency of 65 seconds
*/

//Include required header files
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

//Custom WiFi Credentials
//#define WIFI_SSID "FINN"
//#define WIFI_PASSWD "Id4S7719G99XG1R"

#define WIFI_SSID "LJioWiFi"
#define WIFI_PASSWD "adgjmptw"

//Action ID with frequency as "always"
String actionIDAlways = String("E6509B49-5048-4151-B965-BB7B2DBC7905");

//Action ID with frequency as "minutely"
String actionIDMinutelyDev = String("A42ABD19-3226-47AB-8045-8129DBDF117E");
String actionIDMinutelyProd = String("02E07940-F173-4271-9640-D63D291C5164");

//Action ID with frequency as "hourly"
String actionIDHourly = String("749081B8-664D-4A15-908E-1C3F6590930D");

//Action ID with frequency as "daily"
String actionIDDaily = String("81F6011A-9AF0-45AE-91CD-9A0CDA81FA1F");

//Action ID with frequency as "weekly"
String actionIDWeekly = String("0BF5E8D2-9062-467E-BB19-88CB76D06F8E");

//Action ID with frequency as "monthly"
String actionIDMonthly = String("C257DB70-AE57-4409-B94E-678CB1567FA6");

//Action ID with frequency as "half-yearly"
String actionIDHYearly = String("D93F99E1-011B-4609-B04E-AEDBA98A7C5F");

//Action ID with frequency as "yearly"
String actionIDYearly = String("0097430C-FA78-4087-9B78-3AC7FEEF2245");

//Webserver IP Address
//Update the remote server IP before flashing the sketch on to board
const char* ip = "192.168.43.132";

//Webserver Port
const int port = 3001;

//Led Pin number to blink
int ledPin = 2;


//Flag to synchorize between request and response
bool responseReceived = false;

//Declare and Instantiate http client instance
HTTPClient httpClient;
WiFiClient client;

//Function to perform blink
void blinkLED()
{
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
  delay(1000);
}

//Function to request for actions from the Server using the end point /actions
void getActionsFromServer()
{
   String urlString = String("http://") + ip + ":" + port + "/actions";

  //Initialize HTTP Client Instance
  //if(httpClient.begin(client,"http://192.168.43.132:3001/actions")){
  if(httpClient.begin(client,urlString)){
    Serial.printf("\nsdkHttpClientSample: getActionsFromServer: \
    HttpClient Initialized...");
    int httpCode = httpClient.GET();
    Serial.printf("\nsdkHttpClientSample: getActionsFromServer: \
    httpCode from httpClient->GET(): %d",httpCode);

    if(httpCode > 0) {
      Serial.printf("\nsdkHttpClientSample: getActionsFromServer: \
      Response from httpClient->GET(): %s", (httpClient.getString()).c_str());
      responseReceived = true;
      blinkLED();
    }
    else {
      Serial.printf("\nsdkHttpClientSample: getActionsFromServer: \
      httpClient->GET() failed : %s",httpClient.errorToString(httpCode).c_str());
    }

    httpClient.end();
  }
  else {
    Serial.printf("\nsdkHttpClientSample: getActionsFromServer: \
    HttpClient Initialization failed...");
  }
}

//Function to request to trigger action to the Server using the end point /action
void triggerAction(){

  String urlString = String("http://") + ip + ":" + port + "/action?actionID="+actionIDMinutelyProd.c_str();

  //Initialize HTTP Client Instance
  //if(httpClient.begin(client,"http://192.168.43.132:3001/action?actionID=A42ABD19-3226-47AB-8045-8129DBDF117E")){
  if(httpClient.begin(client,urlString)){
    Serial.printf("\nsdkHttpClientSample: triggerAction: \
    HttpClient Initialized...");

    int httpCode = httpClient.GET();
    Serial.printf("\nsdkHttpClientSample: triggerAction: \
    httpCode from httpClient->GET(): %d",httpCode);

    if(httpCode > 0) {
      Serial.printf("\nsdkHttpClientSample: triggerAction: \
      Respone from httpClient->GET(): %s", (httpClient.getString()).c_str());
      responseReceived = true;
      blinkLED();
    }
    else {
      Serial.printf("\nsdkHttpClientSample: triggerAction: \
      httpClient->GET() failed : %s",httpClient.errorToString(httpCode).c_str());
    }

    httpClient.end();
    //delete httpClient;
  }
  else {
    Serial.printf("\nsdkHttpClientSample: triggerAction: \
    HttpClient Initialization failed...");
  }
  responseReceived = true;
  blinkLED();

}

//Function to request for pairing from the Server using the end point /pairing
void devicePair(){
   String urlString = String("http://") + ip + ":" + port + "/pairing";

  //Initialize HTTP Client Instance
  //if(httpClient.begin(client,"http://192.168.43.132:3001/pairing")){
  if(httpClient.begin(client,urlString)){
    Serial.printf("\nsdkHttpClientSample: devicePair: \
    HttpClient Initialized...");
    int httpCode = httpClient.GET();
    Serial.printf("\nsdkHttpClientSample: devicePair: \
    httpCode from httpClient->GET(): %d",httpCode);

    if(httpCode > 0) {
      Serial.printf("\nsdkHttpClientSample: devicePair: \
      Response from httpClient->GET(): %s", (httpClient.getString()).c_str());
      responseReceived = true;
      blinkLED();
    }
    else {
      Serial.printf("\nsdkHttpClientSample: devicePair: \
      httpClient->GET() failed : %s",httpClient.errorToString(httpCode).c_str());
    }

    httpClient.end();
  }
  else {
    Serial.printf("\nsdkHttpClientSample: devicePair: \
    HttpClient Initialization failed...");
  }
}

//Function to request for activate device using the end point /activate
void deviceActivate(){
   String urlString = String("http://") + ip + ":" + port + "/activate";

  //Initialize HTTP Client Instance
  //if(httpClient.begin(client,"http://192.168.43.132:3001/activate")){
  if(httpClient.begin(client,urlString)){
    Serial.printf("\nsdkHttpClientSample: deviceActivate: \
    HttpClient Initialized...");
    int httpCode = httpClient.GET();
    Serial.printf("\nsdkHttpClientSample: deviceActivate: \
    httpCode from httpClient->GET(): %d",httpCode);

    if(httpCode > 0) {
      Serial.printf("\nsdkHttpClientSample: deviceActivate: \
      Response from httpClient->GET(): %s", (httpClient.getString()).c_str());
      responseReceived = true;
      blinkLED();
    }
    else {
      Serial.printf("\nsdkHttpClientSample: deviceActivate: \
      httpClient->GET() failed : %s",httpClient.errorToString(httpCode).c_str());
    }

    httpClient.end();
  }
  else {
    Serial.printf("\nsdkHttpClientSample: deviceActivate: \
    HttpClient Initialization failed...");
  }
}

//Setup function for the board
void setup()
{
  //initialization
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(115200);

  //Connect board to provided WiFi SSID
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);

  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.printf("\nTrying to Connect to WiFi SSID: %s ...", WIFI_SSID);
  }

  //Indicate WiFi connectivity by blinking onboard LED
  blinkLED();
  Serial.printf("\nBoard Connected to WiFi SSID: %s, assigned IP: %s", WIFI_SSID,
                                          (WiFi.localIP().toString()).c_str());
}

//Loop function for the board
void loop()
{
  Serial.printf("\nAvailable free heap at the beginning of loop: %lu",ESP.getFreeHeap());
  responseReceived = false;

  //Check for Webserver availability to trigger the action
  if(WiFi.status() == WL_CONNECTED){
    Serial.printf("\nBoard is connected to WiFi...");

    if( ip != NULL) {
      //Connect to remote server
      Serial.printf("\nGetting actions from server running on IP: %s", ip);
      //getActionsFromServer();
      //devicePair();
      //deviceActivate();
      triggerAction();
    }
    else {
      Serial.printf("\nPlease update the remote server IP address and flash again...");
    }

  }
  else {
    Serial.printf("\nBoard is not connected to WiFi, trying to connect now...");

    //Connect board to provided WiFi SSID
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.printf("\nTrying to Connect to WiFi SSID: %s ...", WIFI_SSID);
    }
    //Indicate WiFi connectivity by blinking onboard LED
    blinkLED();
    Serial.printf("\nBoard Connected to WiFi SSID: %s, assigned IP: %s",
                               WIFI_SSID, (WiFi.localIP().toString()).c_str());
  }

  Serial.printf("\nAvailable free heap at the end of loop: %lu",ESP.getFreeHeap());

  //Introduce delay of 65 seconds
  delay(65*1000);
}
