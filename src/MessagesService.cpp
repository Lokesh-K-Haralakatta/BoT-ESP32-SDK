/*
    Usage
    Start a getMessages() loop AFTER the device is paired & status to CORE is updated.
    When there is a message, CORE will respond with payload, see Payload structure. If there is no message, the response will be empty.
    The CORE expects a trigger action from IoT after the message is received.
    To finish the process, after custom actions (read sensors and do actuator stuff), triggerAction with actionID and pass the customerID as alternativeID (aid)

  MessagesService.h - Class and Methods to get messages (bottalk) with BoT Service
  Created by Ercan Bozoglu, September 30, 2012.
  Released into the repository BoT-ESP32-SDK.
*/

#include "MessagesService.h"

  /**
   * Payload structure
   data class MessagePojo(
           val deviceID: String,
           val payload: String,
           val messageID: String,
           val event: String?,
           val delivered: Int
                         ) {
       var payloadModel: PayloadModel? = null

       data class PayloadModel(
               val actionID: String?,
               val customerID: String?,
               val deviceID: String?
                              )
   }
   **/
MessagesService :: MessagesService(){
  bot = BoTService :: getBoTServiceInstance();
}
    /**
     * @returns pair with actionID and customerID
     * TODO in the future, CORE is going to send list of messages (queued). Now it expects object, single message.
     */
    std::pair MessagesService :: getMessages(){
        String* response = bot->get(MESSAGES_END_POINT);
        debugD("\nMessagesService :: getMessages : %s", response->c_str());
        DynamicJsonBuffer jsonBuffer;
        JsonObject& messagePayload = jsonBuffer.parseObject(receivedString->c_str());
        JsonObject& payloadPayload = jsonBuffer.parseObject(messagePayload.get<const char*>("payload"));

        String* actionID = new String(payloadPayload.get<const char*>("actionID"));
        String* customerID = new String(messagePayload.get<const char*>("customerID"));
        // TODO replace pair with string array if not supported.
        return std::make_pair(actionID, customerID);
    }
}
