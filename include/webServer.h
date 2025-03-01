#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>
#include <EEPROM.h>
#include "settings.h"
#include "programa.h"

extern Settings settings;

extern bool receivedChange;

void startWebServer();

void handleWebServer();

#endif