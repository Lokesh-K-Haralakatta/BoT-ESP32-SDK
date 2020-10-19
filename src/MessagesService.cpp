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
#include "ActionService.h"

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

        store = KeyStore :: getKeyStoreInstance();
        bot = BoTService :: getBoTServiceInstance();
        actionService = ActionService :: getActionServiceInstance();
        debugI("\nMessagesService :: getMessages : init");
}
/**
 * @returns pair with actionID and customerID
 * TODO in the future, CORE is going to send list of messages (queued). Now it expects object, single message.
 */
void MessagesService :: getMessages(){
        debugI("\nMessagesService :: getMessages : start");
        String* message = bot->get(MESSAGES_END_POINT);
        debugI("\nMessagesService :: getMessages : %s", message->c_str());
        if(message->indexOf("{") != -1 && message->indexOf("}") != -1) {
                DynamicJsonBuffer jsonBuffer;
                JsonObject& messageJson = jsonBuffer.parseObject(*message);
                const char*  event = messageJson["event"];
                const char* payload = messageJson["payload"];
                const char* deviceID = messageJson["deviceID"];
                const char* messageID = messageJson["messageID"];
                debugI("\nMessagesService :: Message: %s, Event %s, deviceID: %s", messageID, event, deviceID);

                JsonObject& payloadJson = jsonBuffer.parseObject(payload);
                const char* actionID = payloadJson["actionID"];
                const char* customerID = payloadJson["customerID"];
                deviceID = payloadJson["deviceID"];
                debugI("\nMessagesService :: Payload actionID %s, customerID %s, deviceID: %s", actionID, customerID, deviceID);

                bool triggerResult = false;
                String* response = actionService->triggerAction(actionID,"");
                if(response != NULL) {
                        debugD("\nMessagesService :: triggerAction: Response: %s", response->c_str());
                        if(response->indexOf("OK") != -1) {
                                debugI("\nMessagesService :: triggerAction: Action triggered successful");
                                triggerResult = true;
                        }
                        else if(response->indexOf("Action not found") != -1) {
                                debugW("\nMessagesService :: triggerAction: Action not triggered as its not found");
                                triggerResult = false;
                        }
                        else {
                                debugE("\nMessagesService :: triggerAction: Action triggerring failed, check parameters and try again");
                                triggerResult = false;
                        }
                }
                else {
                        debugW("\nMessagesService :: triggerAction: Action not triggered as there is no Internet Available but saved as Offline Action");
                        triggerResult = true;
                }

        }
        else {
                debugE("\MessagesService :: getMessages: Could not retrieve messages from server");

                return;
        }
}




