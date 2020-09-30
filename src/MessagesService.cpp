/*
  MessagesService.h - Class and Methods to get messages (bottalk) with BoT Service
  Created by Ercan Bozoglu, September 30, 2012.
  Released into the repository BoT-ESP32-SDK.
*/

#include "MessagesService.h"

MessagesService :: MessagesService(){
  store = KeyStore :: getKeyStoreInstance();
  bot = BoTService :: getBoTServiceInstance();
}

    String* MessagesService :: getMessages(){
      String* response = bot->get(MESSAGES_END_POINT);
      debugD("\nMessagesService :: getMessages : %s", response->c_str());
      return response;
    }
}
