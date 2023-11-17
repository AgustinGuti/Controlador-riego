#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "programa.h"
#include "webServer.h"
#include "utils.h"
#include "LittleFS.h"

#define EEPROM_SIZE 512
#define CHECK_INTERVAL 5 * 1000 * 60 // 5 minutes

char programToRun = -1;
char runningProgram = -1;
bool stopProgram = false;

void endProgram();

// TODO make configurable
const char *ssid = "TP-Link_EBB6";
const char *password = "76450853";

int outputPins[SECTOR_QTY] = {D1, D2, D3, D4, D5, D6, D7, D8};
int outputPumpPin = D0;

uint addr = 0;

unsigned long previousMillis = 0;

struct SectorsStatus
{
  char sectors[SECTOR_QTY]; // Time each sector has been on
  char currentSector;       // Current sector being watered
};

SectorsStatus sectorsStatus;
Settings settings;

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting: Version" + String(VERSION));

  EEPROM.begin(EEPROM_SIZE);

  EEPROM.get(addr, settings);

  WiFi.mode(WIFI_STA);

  IPAddress staticIP(192, 168, 0, 200); // Set your desired static IP address
  IPAddress gateway(192, 168, 0, 1);    // Set your router's IP address
  IPAddress subnet(255, 255, 255, 0);   // Set your subnet mask

  WiFi.config(staticIP, gateway, subnet);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // TODO add local network if it cant connect to the internet

  // timeInit();
  startWebServer();

  for (int i = 0; i < SECTOR_QTY; i++)
  {
    // TODO MAX 12mA per pin, use relay / transistor
    pinMode(outputPins[i], OUTPUT);
  }
  pinMode(outputPumpPin, OUTPUT);

  memset(&sectorsStatus, 0, sizeof(sectorsStatus));
}

void loop()
{
  handleWebServer();
  unsigned long currentMillis = millis();

  // If a new program was received, and there is no program running, start it.
  // Else, if there is a program running, check if it finished / change zones.
  if (receivedChange || (currentMillis - previousMillis >= CHECK_INTERVAL))
  {
    previousMillis = currentMillis;
    if (stopProgram)
    {
      endProgram();
      stopProgram = false;
    }
    if ((programToRun > -1 && programToRun < PROGRAM_QTY && runningProgram == -1) || runningProgram > -1)
    {
      if (runningProgram == -1)
      {
        runningProgram = programToRun;
        programToRun = -1;
        digitalWrite(outputPumpPin, HIGH);

        Serial.print("Running program: ");
        Serial.println(runningProgram);

        for (int i = sectorsStatus.currentSector; i < SECTOR_QTY; i++)
        {
          Programa programa = settings.programs[(int)runningProgram];
          if (sectorsStatus.sectors[i] < programa.sectorDurations[i])
          {
            // Si el sector no termino de regar, sigo regando
            break;
          }
          else
          {
            if (i == SECTOR_QTY - 1)
            {
              endProgram();
            }
            else
            {
              // Paso al siguiente sector. Dejo un intervalo de 10 segs con ambos abiertos
              digitalWrite(outputPins[i + 1], HIGH);
              delay(10000);
              digitalWrite(outputPins[i], LOW);

              sectorsStatus.sectors[i] = 0;
              sectorsStatus.currentSector = i + 1;
            }
          }
        }
      }
    }
  }
}

void endProgram()
{
  // Si ya regué todos los sectores, termino el programa
  // TODO Preguntar, apago primero la bomba, despues la válvula, no?
  runningProgram = -1;
  digitalWrite(outputPumpPin, LOW);
  for (int i = 0; i < SECTOR_QTY; i++)
  {
    digitalWrite(outputPins[i], LOW);
  }
  Serial.println("Program finished");
}