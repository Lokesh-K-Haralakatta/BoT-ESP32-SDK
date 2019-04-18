/*
  Actionervice.h - Class and Methods to trigger and get actions with BoT Service
  Created by Lokesh H K, April 17, 2019.
  Released into the repository BoT-ESP32-SDK.
*/

#ifndef ActionService_h
#define ActionService_h
#include "BoTESP32SDK.h"
#include "BoTService.h"
#include "Storage.h"
#define ACTIONS_END_POINT "/actions"
#define MINUTE_IN_SECONDS 60
#define HOUR_IN_SECONDS (MINUTE_IN_SECONDS * 60)
#define DAY_IN_SECONDS (HOUR_IN_SECONDS * 24)
#define WEEK_IN_SECONDS (DAY_IN_SECONDS * 7)
#define MONTH_IN_SECONDS (WEEK_IN_SECONDS * 4)
#define HALF_YEAR_IN_SECONDS (WEEK_IN_SECONDS * 26)
#define YEAR_IN_SECONDS (WEEK_IN_SECONDS * 52)

struct Action{
  char* actionID;
  char* actionFrequency;
  unsigned long triggeredTime;
};

class ActionService {
  public:
    ActionService();
    String triggerAction(const char* actionID, const char* value = NULL, const char* altID = NULL);
    String getActions();
  private:
    BoTService *bot;
    KeyStore *store;
    WiFiUDP ntpUDP;
    NTPClient *timeClient;
    std::vector <struct Action> actionsList;
    bool isValidAction(const char* actionID);
    bool isValidActionFrequency(const struct Action*);
};
#endif
