#include <messages.h>
StaticJsonDocument<8000> messages;
void loadMessages()
{
    // SPIFFS.remove("/messages.json");
    if (!SPIFFS.exists("/messages.json"))
    {
        Serial.println("created");
        File file = SPIFFS.open("/messages.json", "w");
        JsonObject singleMessage = messages.createNestedObject();
        singleMessage["text"] = "testerino";
        singleMessage["time"] = 11902;
        singleMessage.end();
        serializeJson(messages, file);
        file.close();
        // messages.clear();
    }
    else
    {
        Serial.println("read from messages");

        File file = SPIFFS.open("/messages.json", "r");
        deserializeJson(messages, file);
        file.close();
    }
    // messages.add(singleMessage);
    // serializeJsonPretty(messages, Serial);
}
void saveMessages()
{
    serializeJsonPretty(messages, Serial);
    SPIFFS.remove("/messages.json");
    File file = SPIFFS.open("/messages.json", "w");
    serializeJson(messages, file);
    file.close();
}
void clearAllMessages()
{
    SPIFFS.remove("/messages.json");
    File file = SPIFFS.open("/messages.json", "w");
    file.close();
    messages.clear();
}
void addMessage(char *text, bool sendOrReceiveFlag, DateTime time)
{
    if (messages.size() > 20)
        messages.remove(0);
    JsonObject singleMessage = messages.createNestedObject();
    singleMessage["text"] = text;
    singleMessage["time"] = time.unixtime();
    singleMessage["sendOrReceiveFlag"] = sendOrReceiveFlag;
    singleMessage["messageSize"] = 1;
    singleMessage.end();
    saveMessages();
}

void addMessage(char *text, bool sendOrReceiveFlag, DateTime time, boolean messageType = 0)
{
    if (messages.size() > 20)
        messages.remove(0);
    JsonObject singleMessage = messages.createNestedObject();
    singleMessage["text"] = text;
    singleMessage["time"] = time.unixtime();
    singleMessage["sendOrReceiveFlag"] = sendOrReceiveFlag;
    if (messageType)
        singleMessage["messageSize"] = 3;
    else
        singleMessage["messageSize"] = 1;
    singleMessage.end();
    saveMessages();
}