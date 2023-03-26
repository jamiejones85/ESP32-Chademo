#ifndef CHADEMO_WEB_SERVER_H
#define CHADEMO_WEB_SERVER_H
#include <Arduino.h>
#include "Globals.h"
#include "ESPAsyncWebServer.h"
#include "ArduinoJson.h"

class ChademoWebServer
{
  public:
    ChademoWebServer(EESettings& settings);
    void setup();
    void execute();
    void broadcast(const char * message);
    AsyncWebSocket& getWebSocket();
  private:
    void toJson(EESettings& settings, DynamicJsonDocument &root);
    void fromJson(EESettings& settings, JsonObject &doc);
    EESettings& settings;
};
#endif
