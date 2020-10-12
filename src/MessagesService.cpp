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
        store = KeyStore :: getKeyStoreInstance();
}
/**
 * @returns pair with actionID and customerID
 * TODO in the future, CORE is going to send list of messages (queued). Now it expects object, single message.
 */
String* MessagesService :: getMessages(){
        String* messages = bot->get(MESSAGES_END_POINT);
        debugD("\nMessagesService :: getMessages : %s", messages->c_str());
        if(messages->indexOf("[") != -1 && messages->indexOf("]") != -1) {
                DynamicJsonBuffer jsonBuffer;
                JsonArray& messagesArray = jsonBuffer.parseArray(*messages);
                if(messagesArray.success()) {
                        int messagesCount = messagesArray.size();
                        debugD("\MessagesService :: getMessages: JSON Messages array parsed successfully");
                        debugD("\MessagesService :: getMessages: Number of messages returned: %d", messagesCount);

                        //Process the new set of messages
                        for(byte i=0; i < messagesCount; i++) {
                                const char* actionID = messagesArray[i]["actionID"];
                                const char* customerID = messagesArray[i]["customerID"];
                                const char* deviceID = messagesArray[i]["deviceID"];
                                const char* event = messagesArray[i]["event"];
                                debugD("\nID: action: %s customer: %s device: %s event: %s", actionID, customerID, deviceID, event);
                                //Trigger Offline Action
                                String* actionResponse = triggerAction(actionID,0);

                        }
                        debugI("\nActionService :: getMessages: Added %d messages returned from server ");
                        return messages;
                }
                else {
                        debugE("\MessagesService :: getMessages: JSON Messages array parsed failed!");
                        debugW("\MessagesService :: getMessages: use locally stored actions, if available");
                        debugW("\nActionService :: getMessages: Local messages count: %d", messagesArray.size());
                        return NULL;
                }
        }
        else {
                debugE("\MessagesService :: getMessages: Could not retrieve messages from server");
                debugW("\MessagesService :: getMessages: use locally stored messages, if available");

                return NULL;
        }
}

String* MessagesService :: triggerAction(const char* actionID,  const double value){
        //Action triggering logic goes here
        DynamicJsonBuffer jsonBuffer;
        JsonObject& doc = jsonBuffer.createObject();
        JsonObject& botData = doc.createNestedObject("bot");
        botData["deviceID"] = store->getDeviceID();
        botData["actionID"] = actionID;

        if (store->isDeviceMultipair()) {
                botData["alternativeID"] = store->getAlternateDeviceID();
        }

        if (value > 0.0) {
                botData["value"] = value;
        }

        char payload[200];
        doc.printTo(payload);
        jsonBuffer.clear();
        debugI("\nActionService : triggerAction: Minified JSON payload to trigger action: %s", payload);
        String* postResponse = bot->post(ACTIONS_END_POINT,payload);
        if(postResponse != NULL) {
                debugI("\nActionService : triggerAction: Post response %s",postResponse->c_str());
        }
        else {
                debugI("\nActionService : triggerAction: Post response is NULL");
        }
        return(postResponse);
}


