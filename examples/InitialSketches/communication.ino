
#include <ArduinoJson.h>
#include "base64url.h"

String makerID_variable = "185d8549-0091-463e-90fe-eda6ae15dc91";
String deviceID_variable = "edfc7678-cacf-44f0-a2c6-1be15abef444";
String actionID_variable = "77189283-A963-4E2E-BD12-18D1681A00EE";
const char* host = "api-dev.bankingofthings.io";  // Server URL


String getPairing() {

  String i_status = getJSON ("pair");
  Serial.println("HTTP Pairing Status is:" + i_status);
  return i_status;
}

void getActions() {
  String i_actions = getJSON ("actions");
  Serial.println("HTTP Actions Retrieved:" + i_actions);
}

void postAction(String action, String value) {
  StaticJsonBuffer<200> jsonBuffer;
  String actionJSONString = "";
  JsonObject& actionJSON = jsonBuffer.createObject();
  JsonObject& bot = actionJSON.createNestedObject("bot");
  bot["deviceID"] = "edfc7678-cacf-44f0-a2c6-1be15abef444";
  bot["actionID"] = "77189283-A963-4E2E-BD12-18D1681A00EE";
  bot["queueID"] = "qqqqqqqqqqqqq";
  bot["value"] = "0.3";
  actionJSON.printTo(actionJSONString);

  Serial.println("actionJSON:" + (String)actionJSONString);

  //actionJSON.prettyPrintTo(Serial);
  //String jwtHeader = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9";
  char* jwtHeader = "{\"alg\":\"RS256\",\"typ\":\"JWT\"}";
  int jsonLength = actionJSONString.length();
  char jwtPayload[jsonLength + 1] ;
  actionJSONString.toCharArray(jwtPayload, jsonLength + 1);

  //  String macInput = jwtHeader + "." + jwtPayload;
  //String signature = hashRS256(macInput, privateKey);
  String botJWT = encodeJWT(jwtHeader, jwtPayload);
  Serial.println("JWT bot:" + botJWT);
  String actionBody_encodedJWT = (String)"{\"bot\": \"" + botJWT +   "\"}";
  String i_action_response = postJSON ("actions", actionBody_encodedJWT);
  Serial.println("HTTP Action Triggered:" + i_action_response);

  if (i_action_response.indexOf("OK") != -1)
    blinkLED();
}


String postJSON(String path, String body) {
  client.setCACert(bot_root_ca);
  if (!client.connect(host, 443)) {
    Serial.println("HTTP Client: connection failed");
    return "";
  }

  String url = "/bot_iot/" + path;
  String httpRq = "";

  httpRq = String("POST ") + "https://" + host + url + " HTTP/1.1\r\n" +
           "Host: " + host + "\r\n" +
           "Content-Type: " + "application/json\r\n" +
           "Connection :" + "keep-alive\r\n" +
           "Content-Length: " + (String)body.length() + "\r\n" +
           "makerID: " + makerID_variable + "\r\n" +
           "deviceID: " + deviceID_variable + "\r\n" +
           "\r\n" +
           body ;

  Serial.println("HTTP Client is making POST request");
  client.print(httpRq);

  Serial.println(httpRq);
  delay(5);

  unsigned long timeout = millis();
  while (client.available() == 0) {
    //Serial.println("HTTP Client: client seems unavailable ...");
    if (millis() - timeout > 5000) {
      Serial.println("HTTP Client: >>> Client Timeout !");
      client.stop();
      return "";
    }
  }
  String prefix = "eyJhbGciOiJSUzI1NiJ9.";

  // Read all the lines of the reply from server and print them to Serial
  Serial.println("HTTP Client receiving POST response: ");
  while (client.available()) {
     char c = client.read();
      Serial.write(c);
//    String line = client.readStringUntil('\r');
//    Serial.print(line);
//    if (line.indexOf(prefix) != -1) {
//      int prefixLength = prefix.length() + 1;
//      String encodedPayload = line.substring(prefixLength , line.substring(prefixLength, line.length()).indexOf(".") + prefixLength + 1);
//      Serial.println("JWT received:" + encodedPayload);
//      return decodePayload(encodedPayload);
//    }
    return "";
  }
  client.stop();
}


void doithere() {
  client.setCACert(bot_root_ca);
  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, 443))
    Serial.println("Connection failed!");
  else {
    Serial.println("Connected to server!");
    // Make a HTTP request:
    client.println("GET https://api-dev.bankingofthings.io/bot_iot/pairing HTTP/1.0");
    client.println("Host: api-dev.bankingofthings.io");
    client.println("Connection: close");
    client.println();

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }
    client.stop();
  }
}


String getJSON(String path) {
  //doithere();
  client.setCACert(bot_root_ca);

  if (!client.connect(host, 443)) {
    Serial.println("HTTP Client: connection failed");
    return "";
  }

  String url = "/bot_iot/" + path;

  String httpRq = "";

  httpRq = String("GET ") + "https://" + host + url + " HTTP/1.1\r\n" +
           "Host: " + host + "\r\n" +
           "makerID: " + makerID_variable + "\r\n" +
           "deviceID: " + deviceID_variable + "\r\n" +
           "Connection: close\r\n\r\n";


  Serial.println("HTTP Client is making GET request");
  client.print(httpRq);

  Serial.println(httpRq);

  unsigned long timeout = millis();
  while (client.available() == 0) {
    //Serial.println("HTTP Client: client seems unavailable ...");
    if (millis() - timeout > 5000) {
      Serial.println("HTTP Client: >>> Client Timeout !");
      client.stop();
      return "";
    }
  }
  String prefix = "eyJhbGciOiJSUzI1NiJ9.";

  // Read all the lines of the reply from server and print them to Serial
  Serial.println("HTTP Client receiving GET response: ");
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
    if (line.indexOf(prefix) != -1) {
      int prefixLength = prefix.length() + 1;
      String encodedPayload = line.substring(prefixLength , line.substring(prefixLength, line.length()).indexOf(".") + prefixLength + 1);

      return decodePayload(encodedPayload);
    }
  }
}



String decodePayload(String encodedPayload) {
  int payloadLength = encodedPayload.length() + 1;
  unsigned char decoded[payloadLength];
  String ss(encodedPayload);
  base64url_decode((char *)ss.c_str(), ss.length() , decoded);
  // b64_decode( decoded , (char *)ss.c_str() , ss.length() );
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(decoded);
  String actions = root["bot"];
  Serial.println(actions);
  return actions;
}

void startWifi() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("WIFI: Connecting WIFI:" + String(ssid));
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WIFI: Connected to the WiFi network! IP:");
  Serial.println("WIFI:" + WiFi.localIP());

}
