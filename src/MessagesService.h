/*
  MessagesService.h - Class and Methods to get messages (bottalk) with BoT Service
  Created by Ercan Bozoglu, September 30, 2012.
  Released into the repository BoT-ESP32-SDK.
*/

#ifndef MessagesService_h
#define MessagesService_h
#include "BoTESP32SDK.h"
#include "BoTService.h"
#include "Webserver.h"
#include "Storage.h"
#define MESSAGES_END_POINT "/messages"
#define MINUTE_IN_SECONDS 60
#define HOUR_IN_SECONDS (MINUTE_IN_SECONDS * 60)
#define DAY_IN_SECONDS (HOUR_IN_SECONDS * 24)
#define WEEK_IN_SECONDS (DAY_IN_SECONDS * 7)
#define MONTH_IN_SECONDS (WEEK_IN_SECONDS * 4)
#define HALF_YEAR_IN_SECONDS (WEEK_IN_SECONDS * 26)
#define YEAR_IN_SECONDS (WEEK_IN_SECONDS * 52)

class MessagesService {
  public:
    ~MessagesService();
    static MessagesService* getMessagesServiceInstance();
    String* getMessages();
  private:
    BoTService *bot;
    MessagesService();
    static MessagesService* instance;
};
#endif
