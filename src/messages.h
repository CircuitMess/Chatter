#ifndef messages_h
#define messages_h
#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <RTClib.h>

extern StaticJsonDocument<8000> messages;

void clearAllMessages();
void loadMessages();
void addMessage(char *text, bool sendOrReceiveFlag, DateTime time);
void saveMessages();

#endif