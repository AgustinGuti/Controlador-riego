#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "programa.h"
#include "webServer.h"
#include "utils.h"

#define EEPROM_SIZE 512
#define CHECK_INTERVAL 500 // Todo change to 5 * 1000 * 60 // 5 minutes

#define OVERLAP_TIME 1 * 1000 // TODO change to 10 seconds

#define TURN_ON(X) digitalWrite((X), LOW) // X is the pin number
#define TURN_OFF(X) digitalWrite((X), HIGH)

signed char runningProgram = -1;
bool stopProgram = false;

int manualSector = -1;
unsigned int sectorDuration = 0;
bool stopManualSector = false;

void endProgram(unsigned long currentMillis);
void handleProgram(unsigned long currentMillis);

// TODO make configurable
const char *ssid = "TP-Link_EBB6";
const char *password = "76450853";

int outputPins[SECTOR_QTY] = {D0, D1, D2, D3, D4, D5, D6, D7};
int outputPumpPin = D8; // TODO find a suitable pin, this one prevents the board from booting

uint addr = 0;

unsigned long previousMillis = 0;

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

  startWebServer();

  for (int i = 0; i < SECTOR_QTY; i++)
  {
    pinMode(outputPins[i], OUTPUT);
    TURN_OFF(outputPins[i]);
  }
  pinMode(outputPumpPin, OUTPUT);
  TURN_OFF(outputPumpPin);

  memset(&sectorsStatus, 0, sizeof(sectorsStatus));
}

unsigned long currentSectorStartTime = 0;

bool isWaitingManual = false;
bool isWaiting = false;
bool isPumpOn = false;

void loop()
{
  handleWebServer();

  unsigned long currentMillis = millis();

  // If a new program was received, and there is no program running, start it.
  // Else, if there is a program running, check if it finished / change zones.
  if (receivedChange || (currentMillis - previousMillis >= CHECK_INTERVAL) || isWaiting || isWaitingManual)
  {
    if (receivedChange)
    {
      receivedChange = false;
    }
    if (stopProgram)
    {
      Serial.println("Stopping program");
      // isEnding = true;
      endProgram(currentMillis);
      stopProgram = false;
      return;
    }
    /*
    if (manualSector > -1 && sectorDuration > 0)
    {
      if (!isPumpOn)
      {
        TURN_ON(outputPumpPin);
        isPumpOn = true;
        isWaitingManual = true;
      }

      // Espero 10 segundos antes de prender el siguiente sector luego de prender la bomba
      if (isWaitingManual && previousMillis + OVERLAP_TIME < currentMillis)
      {
        isWaitingManual = false;
      }
      else
      {
        isWaitingManual = true;
        return;
      }

      if (((sectorsStatus.sectors[manualSector] + (currentSectorStartTime - currentMillis) / 1000 / 60) < sectorDuration) && stopManualSector == false)
      {
        if (currentSectorStartTime == 0)
        {
          currentSectorStartTime = millis();
        }
        TURN_ON(outputPins[manualSector]);
      }
      else
      {
        TURN_OFF(outputPumpPin);
        isPumpOn = false;
        isWaitingManual = true;
        if (isWaitingManual && previousMillis + OVERLAP_TIME < currentMillis)
        {
          TURN_OFF(outputPins[manualSector]);
          sectorsStatus.sectors[manualSector] = 0;
          manualSector = -1;
          sectorDuration = 0;
          currentSectorStartTime = 0;
          Serial.println("Manual sector finished");
          isWaitingManual = false;
        }
        else
        {
          isWaitingManual = true;
          return;
        }
      }
    }*/
    if (runningProgram > -1)
    {
      handleProgram(currentMillis);
    }
    previousMillis = currentMillis;
  }
}

void handleProgram(unsigned long currentMillis)
{
  // Prendo la bomba si no esta prendida
  if (!isPumpOn)
  {
    TURN_ON(outputPumpPin);
    isPumpOn = true;
    delay(OVERLAP_TIME);
  }
  for (int i = sectorsStatus.currentSector; i < SECTOR_QTY; i++)
  {
    Programa programa = settings.programs[(int)runningProgram];

    if (currentSectorStartTime == 0)
    {
      currentSectorStartTime = currentMillis;
    }
    // TODO divide by 1000. 100 is for faster testing
    if ((currentMillis - currentSectorStartTime) / 100 / 60 < programa.sectorDurations[i])
    {
      TURN_ON(outputPins[i]);

      // Actualizo el tiempo que lleva prendido el sector
      sectorsStatus.sectors[sectorsStatus.currentSector] = (currentMillis - currentSectorStartTime) / 1000 / 60;

      Serial.println("Continuing sector " + String(i));
      // Si el sector no termino de regar, sigo regando
      break;
    }

    Serial.println("Ending sector " + String(i));

    // Me salteo los sectores que no tienen duración
    int previousOnSector = i;
    while (programa.sectorDurations[i + 1] == 0 && i < SECTOR_QTY - 1)
    {
      i++;
    }
    if (i == SECTOR_QTY - 1)
    {
      endProgram(currentMillis);
    }
    else
    {
      // Paso al siguiente sector. Dejo un intervalo de 10 segs con ambos abiertos
      TURN_ON(outputPins[i + 1]);
      Serial.println("Sector " + String(i + 1) + " started");
      delay(OVERLAP_TIME);
      TURN_OFF(outputPins[previousOnSector]);
      Serial.println("Sector " + String(previousOnSector) + " finished");

      sectorsStatus.sectors[sectorsStatus.currentSector] = 0;
      sectorsStatus.currentSector = i + 1;
      currentSectorStartTime = currentMillis;
    }
  }
}

void endProgram(unsigned long currentMillis)
{
  // Si ya regué todos los sectores, termino el programa
  TURN_OFF(outputPumpPin);
  isPumpOn = false;
  delay(OVERLAP_TIME);
  for (int i = 0; i < SECTOR_QTY; i++)
  {
    TURN_OFF(outputPins[i]);
  }
  sectorsStatus.currentSector = 0;
  memset(&sectorsStatus.sectors, 0, sizeof(sectorsStatus.sectors));
  currentSectorStartTime = 0;
  runningProgram = -1;
  Serial.println("Program finished");
}