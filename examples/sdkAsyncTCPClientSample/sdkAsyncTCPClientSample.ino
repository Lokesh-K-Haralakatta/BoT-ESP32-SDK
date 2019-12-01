/*
  sdkAsyncTCPClientSample.ino - Sample sketch to show case the ESP-32 SDK usage
  using Async TCP Client Library
  Created by Lokesh H K, November 28, 2019.
  Released into the repository BoT-ESP32-SDK.

  To use the ESP-32 SDK, include BoT-ESP32-SDK.zip through Arduino IDE

  sdkAsyncTCPClientSample sketch is a tcp client program to interact with already
  running Webserver on some other board / cloud / url. Make a note of Webserver IP
  and update in this sketch before flashing on to board.

  Change Partition Scheme to No OTA (Large APP) in Arduino IDE -> Tools after
  connecting ESP-32 board to avoid the error message saying "Sketch is too big"
  before compiling and uploading Sketch to ESP-32


  Here is the sketch flow:

  1. Sketch has code to trigger actions with various frequencies like minutely,
     hourly, daily, monthly,half-yearly, yearly and always
  2. Define the actions in maker portal, pair and activate the device add service
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
#include <AsyncTCP.h>
#include <WiFi.h>

//Custom WiFi Credentials
#define WIFI_SSID "FINN"
#define WIFI_PASSWD "Id4S7719G99XG1R"

//Declare client instance variable
AsyncClient *client_tcp = new AsyncClient;

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
const char* ip = "10.26.16.32";

//Webserver Port
const int port = 3001;

//Led Pin number to blink
int ledPin = 2;

//Flag to synchorize between request and response
bool responseReceived = false;

//Function to perform blink
void blinkLED()
{
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
  delay(1000);
}

//Function to request for actions from the Server using the end point /actions
void getActionsFromServer(void *arg)
{
  AsyncClient *client = reinterpret_cast<AsyncClient *>(arg);
  // We now create a URI for the request
  String requestString = String("GET /actions HTTP/1.1\r\n http://") +
       client->remoteIP().toString() + ":3001/ \r\n Connection: close\r\n\r\n";

  const char* serverReqStr = requestString.c_str();
  Serial.printf("\nsdkAsyncTCPClientSample: getActionsFromServer: \
  Server Request: %s",serverReqStr);

  // Send request to get actions from server
  size_t nBytes = client->write(serverReqStr);
  Serial.printf("\nsdkAsyncTCPClientSample: getActionsFromServer: \
  Amount of bytes written to Server: %d",nBytes);
}

//Function to request to trigger action to the Server using the end point /action
void triggerAction(void *arg1){
  AsyncClient *client = reinterpret_cast<AsyncClient *>(arg1);
  // We now create a URI for the request
  // Update the required actionId in the URI formation below
  String requestString = String("GET /action?actionID=") +
         actionIDMinutelyProd.c_str() + " HTTP/1.1\r\n http://" +
         client->remoteIP().toString() + ":3001/ \r\n Connection: close\r\n\r\n";

  const char* serverReqStr = requestString.c_str();
  Serial.printf("\nsdkAsyncTCPClientSample: triggerAction: Server Request: %s",serverReqStr);

  //Send action trigger request to server
  //size_t nBytes = client->write("GET /action?actionID=E6509B49-5048-4151-B965-BB7B2DBC7905
  //HTTP/1.1\r\n http://10.26.16.126:3001/ \r\n Connection: reuse\r\n\r\n");
  size_t nBytes = client->write(serverReqStr);
  Serial.printf("\nsdkAsyncTCPClientSample: triggerAction: \
  Amount of bytes written to Server: %d",nBytes);

}

//Function to request for pairing from the Server using the end point /pairing
void devicePair(void *arg){
  AsyncClient *client = reinterpret_cast<AsyncClient *>(arg);
  // We now create a URI for the request
  String requestString = String("GET /pairing HTTP/1.1\r\n http://") +
        client->remoteIP().toString() + ":3001/ \r\n Connection: close\r\n\r\n";
  const char* serverReqStr = requestString.c_str();
  Serial.printf("\nsdkAsyncTCPClientSample: devicePair: Server Request: %s",serverReqStr);
  // Send pairing request to server
  size_t nBytes = client->write(serverReqStr);
  Serial.printf("\nsdkAsyncTCPClientSample: devicePair: \
  Amount of bytes written to Server: %d",nBytes);
}

//Function to request for activate device using the end point /activate
void deviceActivate(void *arg){
  AsyncClient *client = reinterpret_cast<AsyncClient *>(arg);
  // We now create a URI for the request
  String requestString = String("GET /activate HTTP/1.1\r\n http://") +
       client->remoteIP().toString() + ":3001/ \r\n Connection: close\r\n\r\n";
  const char* serverReqStr = requestString.c_str();
  Serial.printf("\nsdkAsyncTCPClientSample: deviceActivate: Server Request: %s",serverReqStr);
  // Send activate request to server
  size_t nBytes = client->write(serverReqStr);
  Serial.printf("\nsdkAsyncTCPClientSample: deviceActivate: \
  Amount of bytes written to Server: %d",nBytes);
}

//Async function to handle data received from server
void handleData(void *arg, AsyncClient *client, void *data, size_t len)
{
  Serial.printf("\nsdkAsyncTCPClientSample: handleData: Data received from %s \n",
                                         client->remoteIP().toString().c_str());
  Serial.write((uint8_t *)data, len);
  //Indicate the response received
  blinkLED();

  //Set responseReceived flag
  responseReceived = true;

  //Close the client connection
  client_tcp->close(true);
}

//Async function to notify on client connected to server
void onConnect(void *arg, AsyncClient *client)
{
  Serial.printf("\nsdkAsyncTCPClientSample: onConnect: Async TCP Client Connected \
  to Webserver at port: %d", port);
  //getActionsFromServer(client);
  //devicePair(client);
  //deviceActivate(client);
  triggerAction(client);
}

//Async function to notify on client disconnected from server
void onDisconnect(void *arg, AsyncClient *client){
  Serial.printf("\nsdkSample: onDisconnect: Async TCP Client Disconnected from Webserver");
}

//Setup function for the board
void setup()
{
  //initialization
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(115200);

  //Attach async callback functions for the client
  client_tcp->onData(handleData, client_tcp);
  client_tcp->onConnect(onConnect, client_tcp);
  client_tcp->onDisconnect(onDisconnect, client_tcp);

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
      Serial.printf("\nTCP Client connecting to remote server on IP: %s..",ip);
      client_tcp->connect(ip, port);

        while(responseReceived != true){
          Serial.printf("\nWaiting to receive response from server....");
          delay(5000);
        }
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

  //Introduce delay of 65 seconds
  delay(65*1000);
}
