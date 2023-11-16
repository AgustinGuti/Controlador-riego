#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "daytime.h"
#include "programa.h"
#include "webServer.h"
#include "utils.h"
#include "LittleFS.h"

#define EEPROM_SIZE 512
#define CHECK_INTERVAL 5 * 1000 * 60 // 5 minutes

// TODO Replace with your network credentials

const char *ssid = "TP-Link_EBB6";
const char *password = "76450853";

Programa programas[SECTOR_QTY];
bool allEnabled;

int outputPins[SECTOR_QTY] = {D1, D2, D3, D4, D5, D6, D7, D8};

uint addr = 0;

unsigned long previousMillis = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting: Version" + String(VERSION));

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(addr, allEnabled);
  EEPROM.get(addr + sizeof(allEnabled), programas);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  IPAddress staticIP(192, 168, 1, 200); // Set your desired static IP address
  IPAddress gateway(192, 168, 1, 1);    // Set your router's IP address
  IPAddress subnet(255, 255, 255, 0);   // Set your subnet mask

  WiFi.config(staticIP, gateway, subnet);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  timeInit();
  startWebServer();

  for (int i = 0; i < SECTOR_QTY; i++)
  {
    // TODO MAX 12mA per pin, use relay / transistor
    pinMode(outputPins[i], OUTPUT);
  }
}

void loop()
{
  handleWebServer();
  unsigned long currentMillis = millis();

  if (receivedChange || (currentMillis - previousMillis >= CHECK_INTERVAL))
  {
    previousMillis = currentMillis;
    DayTime dayTime;
    getDayTime(&dayTime);
    Serial.print("Day:");
    Serial.println(dayTime.day);
    Serial.print("Hour:");
    Serial.println(dayTime.hour);
    Serial.print("Minutes:");
    Serial.println(dayTime.minutes);

    for (int i = 0; i < SECTOR_QTY; i++)
    {
      Programa programa = programas[i];

      if (shouldWater(&programa, &dayTime))
      {
        digitalWrite(outputPins[i], HIGH);
      }
      else
      {
        digitalWrite(outputPins[i], LOW);
      }
    }
  }
}
