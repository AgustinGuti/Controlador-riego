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

const char *ssid = "SSID";
const char *password = "PASSWORD";

Programa programas[SECTOR_QTY];
int outputPins[SECTOR_QTY] = {D1, D2, D3, D4, D5, D6, D7, D8};

uint addr = 0;

unsigned long previousMillis = 0;

void setup()
{
  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(addr, programas);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  IPAddress staticIP(192, 168, 1, 10); // Set your desired static IP address
  IPAddress gateway(192, 168, 1, 1);   // Set your router's IP address
  IPAddress subnet(255, 255, 255, 0);  // Set your subnet mask

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

  if (currentMillis - previousMillis >= CHECK_INTERVAL)
  {
    previousMillis = currentMillis;
    DayTime dayTime;
    getDayTime(&dayTime);

    for (int i = 0; i < SECTOR_QTY; i++)
    {
      Programa programa = programas[i];
      Serial.println(dayTime.day);
      // TODO ask if i should check if the sector is already on
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
