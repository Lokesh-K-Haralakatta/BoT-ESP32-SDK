/*
  BoTService.h - BoTService Class and Methods to make Secure HTTP Calls to BoT Service.
  Created by Lokesh H K, April 12, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#include "BoTService.h"

BoTService :: BoTService(const char* host, const char* uri, const int port){
  hostURL = new char[strlen(host)+1];
  strcpy(hostURL,host);

  uriPath = new char[strlen(uri)+1];
  strcpy(uriPath,uri);

  PORT = port;

  httpClient = new HTTPClient();

  store = KeyStore :: getKeyStoreInstance();
}

BoTService :: ~BoTService(){
  delete httpClient;
  delete uriPath;
  delete hostURL;
}

const char* BoTService :: mbedtlsError(int errnum) {
  char buffer[200];
  mbedtls_strerror(errnum, buffer, sizeof(buffer));
  return buffer;
}

String BoTService :: encodeJWT(const char* header, const char* payload) {
  char* base64Header = new char[strlen(header)+1];

  base64url_encode(
    (unsigned char *)header,   // Data to encode.
    strlen(header),            // Length of data to encode.
    base64Header);             // Base64 encoded data.

  char* base64Payload = new char[strlen(payload)+1];
  base64url_encode(
    (unsigned char *)payload,  // Data to encode.
    strlen(payload),           // Length of data to encode.
    base64Payload);            // Base64 encoded data.

  char* headerAndPayload = new char[strlen(base64Header)+strlen(base64Payload)+1];
  sprintf((char*)headerAndPayload, "%s.%s", base64Header, base64Payload);

  delete base64Header;
  delete base64Payload;

  mbedtls_pk_context pk_context;
  mbedtls_pk_init(&pk_context);
  int rc = mbedtls_pk_parse_key(
             &pk_context,
             (unsigned char *)store->getDevicePrivateKey(),
             strlen((const char*)store->getDevicePrivateKey()) + 1,
             nullptr,
             0);
  if (rc != 0) {
    LOG("\nBoTService :: encodeJWT: Failed to mbedtls_pk_parse_key: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
    return "";
  }

  mbedtls_rsa_context *rsa;

  rsa = mbedtls_pk_rsa(pk_context);

  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_ctr_drbg_init(&ctr_drbg);
  mbedtls_entropy_init(&entropy);

  const char* pers = "MyEntropy";
  mbedtls_ctr_drbg_seed(
    &ctr_drbg,
    mbedtls_entropy_func,
    &entropy,
    (const unsigned char*)pers,
    strlen(pers));

  char digest[32];
  rc = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), (unsigned char*)headerAndPayload, strlen((char*)headerAndPayload), (unsigned char*)digest);
  if (rc != 0) {
    LOG("\nBoTService :: encodeJWT: Failed to mbedtls_md: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
    return "";
  }

  char oBuf[500];
  size_t retSize;
  rc = mbedtls_pk_sign(&pk_context, MBEDTLS_MD_SHA256, (unsigned char*)digest, sizeof(digest), (unsigned char*)oBuf, &retSize, mbedtls_ctr_drbg_random, &ctr_drbg);
  if (rc != 0) {
    printf("Failed to mbedtls_pk_sign: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
    return "";
  }

  char base64Signature[600];

  base64url_encode((unsigned char *)oBuf, retSize, base64Signature);

  char* retData = new char[strlen((char*)headerAndPayload) + 1 + strlen((char*)base64Signature) + 1];

  sprintf(retData, "%s.%s", headerAndPayload, base64Signature);
  delay(100);
  mbedtls_pk_free(&pk_context);
  return retData;
}

String BoTService :: get(const char* endPoint){

  String *fullURI = new String(uriPath);
  fullURI->concat(endPoint);

  if(WiFi.status() == WL_CONNECTED){
    httpClient->begin(hostURL,PORT,fullURI->c_str());
    httpClient->addHeader("makerID", store->getMakerID());
    httpClient->addHeader("deviceID", store->getDeviceID());

    int httpCode = httpClient->GET();
    const char* errorMSG = httpClient->errorToString(httpCode).c_str();
    delete fullURI;

    if(httpCode > 0) {

      LOG("\nBoTService :: get: HTTP GET with endPoint %s, return code: %d\n", endPoint, httpCode);
      String payload = httpClient->getString();
      httpClient->end();

      if(httpCode == HTTP_CODE_OK) {
        int firstDoTIdx = payload.indexOf(".");
        if (firstDoTIdx != -1) {
          int secDoTIdx = payload.indexOf(".",firstDoTIdx+1);
          String encodedPayload = payload.substring(firstDoTIdx+1 , secDoTIdx);
          return(decodePayload(encodedPayload));
        }
      }
      else {
        LOG("\nBoTService :: get: HTTP GET with endpoint %s, failed, error: %s\n", endPoint, errorMSG);
        return errorMSG;
      }
    }
    else {
      LOG("\nBoTService :: get: HTTP GET with endPoint %s, failed, error: %s\n", endPoint, errorMSG);
      return errorMSG;
    }
  }
  else {
    LOG("\nBoTService :: get: Board Not Connected to WiFi...");
    return "Board Not Connected to WiFi...";
  }
}

String BoTService :: decodePayload(String encodedPayload){
  int payloadLength = encodedPayload.length() + 1;
  unsigned char decoded[payloadLength];
  String ss(encodedPayload);

  base64url_decode((char *)ss.c_str(), ss.length() , decoded);
  LOG("\nBoTService :: decodePayload: Decoded String: %s", decoded);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(decoded);
  String botValue = root["bot"];
  LOG("\nBoTService :: decodePayload: pairStatus after decode: %s",botValue.c_str());

  return botValue;
}

String BoTService :: post(const char* endPoint, const char* payload){

  char* jwtHeader = "{\"alg\":\"RS256\",\"typ\":\"JWT\"}";
  String botJWT = encodeJWT(jwtHeader, payload);
  String body = (String)"{\"bot\": \"" + botJWT +   "\"}";

  String *fullURI = new String(uriPath);
  fullURI->concat(endPoint);

  if(WiFi.status() == WL_CONNECTED){
    httpClient->begin(hostURL,PORT,fullURI->c_str());
    httpClient->addHeader("makerID", store->getMakerID());
    httpClient->addHeader("deviceID", store->getDeviceID());
    httpClient->addHeader("Content-Type", "application/json");
    httpClient->addHeader("Connection","keep-alive");
    httpClient->addHeader("Content-Length",String(body.length()));

    int httpCode = httpClient->POST(body);
    const char* errorMSG = httpClient->errorToString(httpCode).c_str();
    delete fullURI;

    if(httpCode > 0) {

      LOG("\nBoTService :: post: HTTP POST with endPoint %s, return code: %d\n", endPoint, httpCode);
      String payload = httpClient->getString();
      httpClient->end();

      if(httpCode == HTTP_CODE_OK) {
        int firstDoTIdx = payload.indexOf(".");
        if (firstDoTIdx != -1) {
          int secDoTIdx = payload.indexOf(".",firstDoTIdx+1);
          String encodedPayload = payload.substring(firstDoTIdx+1 , secDoTIdx);
          return(decodePayload(encodedPayload));
        }
      }
      else {
        LOG("\nBoTService :: post: HTTP POST with endpoint %s, failed, error: %s\n", endPoint, errorMSG);
        return errorMSG;
      }
    }
    else {
      LOG("\nBoTService :: post: HTTP POST with endPoint %s, failed, error: %s\n", endPoint, errorMSG);
      return errorMSG;
    }
  }
  else {
    LOG("\nBoTService :: post: Board Not Connected to WiFi...");
    return "Board Not Connected to WiFi...";
  }
}
