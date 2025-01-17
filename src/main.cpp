#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "programa.h"
#include "webServer.h"
#include "utils.h"

#define EEPROM_SIZE 512
#define CHECK_INTERVAL 10 * 1000

#define RX0 1
#define TX0 3

#define OVERLAP_TIME 5 * 1000

#define TURN_ON(X) digitalWrite((X), LOW) // X is the pin number
#define TURN_OFF(X) digitalWrite((X), HIGH)

// DONE change to / 1000, 100 is for faster testing
// Checks if the sector should change. If the current time minus the start time is greater than the duration, change sector
#define SHOULD_CHANGE_SECTOR(CURR, START, DUR) ((CURR) - (START)) / 1000 / 60 >= (DUR)

signed char runningProgram = -1;
bool stopProgram = false;

int manualSector = -1;
unsigned int sectorDuration = 0;
bool stopManualSector = false;

void endProgram(unsigned long currentMillis);
void endSector();
void handleProgram(unsigned long currentMillis);
void handleSector(unsigned long currentMillis);

// TODO make configurable
const char *ssid = "TP-Link_EBB6";
const char *password = "76450853";

int outputPins[SECTOR_QTY] = {D0, D1, D2, D3, D4, D7, RX0, TX0};
int outputPumpPin = D6;
int outputTankValvePin = D5;

unsigned long previousMillis = 0;

SectorsStatus sectorsStatus;
Settings settings;

void setup()
{
	// Serial.begin(115200);

	EEPROM.begin(sizeof(settings));

	EEPROM.get(0, settings);

	boolean changes = validateSettings(&settings);
	if (changes)
	{
		EEPROM.put(0, settings);
		EEPROM.commit();
	}

	WiFi.mode(WIFI_STA);

	// TODO change for production
	IPAddress staticIP(192, 168, 0, 200); // Set your desired static IP address
	IPAddress gateway(192, 168, 0, 1);	  // Set your router's IP address
	IPAddress subnet(255, 255, 255, 0);	  // Set your subnet mask

	WiFi.config(staticIP, gateway, subnet);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
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

	pinMode(outputTankValvePin, OUTPUT);
	TURN_OFF(outputTankValvePin);

	memset(&sectorsStatus, 0, sizeof(sectorsStatus));
}

unsigned long currentSectorStartTime = 0;

bool isPumpOn = false;

void loop()
{
	handleWebServer();

	unsigned long currentMillis = millis();

	// If a new program was received, and there is no program running, start it.
	// Else, if there is a program running, check if it finished / change zones.
	if (receivedChange || (currentMillis - previousMillis >= CHECK_INTERVAL))
	{
		receivedChange = false;
		if (manualSector > -1 && sectorDuration > 0)
		{
			handleSector(currentMillis);
		}
		if (runningProgram > -1)
		{
			handleProgram(currentMillis);
		}
		previousMillis = currentMillis;
	}
}

void delayAndTurnPumpOn()
{
	if (!isPumpOn)
	{
		delay(OVERLAP_TIME);
		TURN_ON(outputTankValvePin);
		TURN_ON(outputPumpPin);
		isPumpOn = true;
	}
}

void handleSector(unsigned long currentMillis)
{
	if (stopManualSector)
	{
		endSector();
		return;
	}

	if (currentSectorStartTime == 0)
	{
		currentSectorStartTime = currentMillis;
	}
	if (SHOULD_CHANGE_SECTOR(currentMillis, currentSectorStartTime, sectorDuration))
	{
		endSector();
	}
	else
	{
		TURN_ON(outputPins[manualSector]);
		delayAndTurnPumpOn();
	}
}

void endSector()
{
	TURN_OFF(outputPumpPin);
	isPumpOn = false;
	delay(OVERLAP_TIME);

	for (int i = 0; i < SECTOR_QTY; i++)
	{
		TURN_OFF(outputPins[i]);
	}
	manualSector = -1;
	sectorDuration = 0;
	currentSectorStartTime = 0;
	stopManualSector = false;

	TURN_OFF(outputTankValvePin);
}

/**
 * Changes to the next sector in the program. Skips sectors with 0 duration.
 * If the next sector is the last one, ends the program.
 */
void changeToNextSector(int sector, int sectorOrderPosition, Programa *programa, unsigned long currentMillis)
{
	int previousOnSector = sector;
	while (programa->sectorDurations[programa->sectorOrderPositions[sectorOrderPosition + 1]] == 0 && sectorOrderPosition < SECTOR_QTY - 1)
	{
		sectorOrderPosition++;
	}
	if (sectorOrderPosition == SECTOR_QTY - 1)
	{
		endProgram(currentMillis);
	}
	else
	{
		// Paso al siguiente sector. Dejo un intervalo de 10 segs con ambos abiertos
		int nextSector = programa->sectorOrderPositions[sectorOrderPosition + 1];

		TURN_ON(outputPins[nextSector]);
		delay(OVERLAP_TIME);
		TURN_OFF(outputPins[previousOnSector]);

		sectorsStatus.sectors[sectorsStatus.currentSector] = 0;
		sectorsStatus.currentSector = nextSector;
		sectorsStatus.currentSectorPosition = sectorOrderPosition + 1;
		currentSectorStartTime = currentMillis;
	}
}

void handleProgram(unsigned long currentMillis)
{
	if (stopProgram)
	{
		endProgram(currentMillis);
		return;
	}

	for (int i = sectorsStatus.currentSectorPosition; i < SECTOR_QTY; i++)
	{
		Programa programa = settings.programs[(int)runningProgram];

		if (currentSectorStartTime == 0)
		{
			currentSectorStartTime = currentMillis;
		}

		int currentSector = programa.sectorOrderPositions[i];
		if (SHOULD_CHANGE_SECTOR(currentMillis, currentSectorStartTime, programa.sectorDurations[currentSector]))
		{
			changeToNextSector(currentSector, i, &programa, currentMillis);
		}
		else
		{
			sectorsStatus.currentSector = currentSector;
			TURN_ON(outputPins[currentSector]);
			delayAndTurnPumpOn();
			break;
		}
	}
}

void endProgram(unsigned long currentMillis)
{
	TURN_OFF(outputPumpPin);
	isPumpOn = false;
	delay(OVERLAP_TIME);
	for (int i = 0; i < SECTOR_QTY; i++)
	{
		TURN_OFF(outputPins[i]);
	}
	sectorsStatus.currentSector = 0;
	sectorsStatus.currentSectorPosition = 0;
	memset(&sectorsStatus.sectors, 0, sizeof(sectorsStatus.sectors));
	currentSectorStartTime = 0;
	runningProgram = -1;
	stopProgram = false;
	TURN_OFF(outputTankValvePin);
}