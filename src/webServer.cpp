#include "webServer.h"

#define JSON_ALL_SIZE 1024
#define JSON_PROGRAM_SIZE 256

ESP8266WebServer server(80); // Create an instance of the server on port 80

void handleSectorGet();
void handleProgramPost();
void handleProgramGet();
void handleProgramsGet();
void handleProgramStart();
void handleProgramStop();
void handleSectorStart();
void handleSectorStop();
int calculateRemainingTime(int duration);

bool receivedChange = false;

void startWebServer()
{
    // TODO add authentication
    server.onNotFound([]()
                      { server.send(404, "text/plain", "Not found"); });
    server.on("/program", HTTP_POST, handleProgramPost);
    server.on("/program/start", HTTP_POST, handleProgramStart);
    server.on("/program/stop", HTTP_POST, handleProgramStop);
    server.on("/sector/start", HTTP_POST, handleSectorStart);
    server.on("/sector/stop", HTTP_POST, handleSectorStop);
    server.on("/sector", handleSectorGet);
    server.on("/programs", handleProgramsGet);
    server.on("/program", handleProgramGet);

    ElegantOTA.begin(&server); // Start ElegantOTA
    server.begin();            // Start the server
    Serial.println("Server listening");
}

void handleWebServer()
{
    ElegantOTA.loop();
    server.handleClient();
}

void buildProgramObject(JsonObject *jsonObject, int program)
{
    Programa programa = settings.programs[program];

    (*jsonObject)["program"] = program + 1;
    (*jsonObject)["isOn"] = runningProgram == program;
    JsonArray jsonArray = jsonObject->createNestedArray("sectorDurations");
    for (int i = 0; i < SECTOR_QTY; i++)
    {
        int duration = programa.sectorDurations[i];
        jsonArray.add(duration);
    }
}

void buildSectorObject(JsonObject *jsonObject)
{
    if (manualSector == -1 && runningProgram == -1)
    {
        (*jsonObject)["sector"] = -1;
        (*jsonObject)["isOn"] = false;
        (*jsonObject)["duration"] = 0;
        (*jsonObject)["remaining"] = 0;
    }
    else if (manualSector == -1)
    {
        Programa programa = settings.programs[(int)runningProgram];
        int sector = sectorsStatus.currentSector;
        int duration = programa.sectorDurations[sector];
        (*jsonObject)["sector"] = sector + 1;
        (*jsonObject)["isOn"] = true;
        (*jsonObject)["duration"] = duration;
        (*jsonObject)["remaining"] = calculateRemainingTime(duration);
    }
    else
    {
        int sector = manualSector;
        (*jsonObject)["sector"] = sector + 1;
        (*jsonObject)["isOn"] = true;
        (*jsonObject)["duration"] = sectorDuration;
        (*jsonObject)["remaining"] = calculateRemainingTime(sectorDuration);

        Serial.println("Start:" + String(currentSectorStartTime));
    }
}

int calculateRemainingTime(int duration)
{
    int remaining;
    if (currentSectorStartTime == 0)
    {
        remaining = duration;
    }
    else
    {
        remaining = duration - (millis() - currentSectorStartTime) / 1000 / 60;
        if (remaining < 0)
        {
            remaining = 0;
        }
    }
    return remaining;
}

void handleSectorGet()
{
    DynamicJsonDocument jsonDocument(JSON_PROGRAM_SIZE);
    JsonObject jsonObject = jsonDocument.to<JsonObject>();

    buildSectorObject(&jsonObject);

    String response;
    serializeJson(jsonObject, response);

    Serial.println("Sector get response:" + response);

    server.send(200, "application/json", response);
}

void handleProgramsGet()
{
    DynamicJsonDocument jsonDocument(JSON_ALL_SIZE);
    JsonObject jsonGeneralObject = jsonDocument.to<JsonObject>();
    JsonArray jsonArray = jsonGeneralObject.createNestedArray("programs");

    for (int i = 0; i < PROGRAM_QTY; i++)
    {
        JsonObject jsonObject = jsonArray.createNestedObject();

        buildProgramObject(&jsonObject, i);

        String debug;
        serializeJson(jsonObject, debug);
    }

    String response;
    serializeJson(jsonGeneralObject, response);

    server.send(200, "application/json", response);
}

void handleProgramGet()
{
    String id = server.arg(0);
    int program = atoi(id.c_str());
    Serial.println("Sector get:" + String(program));
    if (program < 1 || program > SECTOR_QTY)
    {
        server.send(404, "text/plain", "Not found");
        return;
    }

    program--;

    DynamicJsonDocument jsonDocument(JSON_PROGRAM_SIZE);
    JsonObject jsonObject = jsonDocument.to<JsonObject>();

    buildProgramObject(&jsonObject, program);

    String response;
    serializeJson(jsonObject, response);

    Serial.println("Sector get response:" + response);

    server.send(200, "application/json", response);
}

void handleProgramPost()
{
    Serial.println("Program post");
    String id = server.arg(0);
    int program = atoi(id.c_str());

    if (program < 1 || program > PROGRAM_QTY)
    {
        server.send(404, "text/plain", "Not found");
        return;
    }

    program--;

    DynamicJsonDocument jsonDocument(JSON_PROGRAM_SIZE);
    DeserializationError error = deserializeJson(jsonDocument, server.arg("plain"));

    if (error)
    {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
        server.send(400, "text/plain", "Bad Request - Invalid JSON");
        return;
    }

    Serial.println(server.arg("plain"));

    Programa programa = fromJson(jsonDocument.as<JsonObject>());

    Serial.println(toString(programa, program));

    Serial.println("Saving to EEPROM");
    // TODO uncomment after testing
    //    EEPROM.put(calculateProgramOffset(program), programas);

    settings.programs[program] = programa;
    receivedChange = true;
    server.send(200, "text/plain", "OK");
}

void handleProgramStart()
{
    String id = server.arg(0);
    int program = atoi(id.c_str());
    Serial.println("Sector start:" + String(program));
    if (program < 1 || program > SECTOR_QTY)
    {
        server.send(404, "text/plain", "Not found");
        return;
    }

    program--;

    DynamicJsonDocument jsonDocument(JSON_PROGRAM_SIZE);
    JsonObject jsonObject = jsonDocument.to<JsonObject>();

    Serial.println("Running program:" + String(runningProgram));
    if (runningProgram == -1)
    {
        runningProgram = program;
    }

    buildProgramObject(&jsonObject, program);

    String response;
    serializeJson(jsonObject, response);

    Serial.println("Sector start response:" + response);

    receivedChange = true;
    server.send(200, "application/json", response);
}

void handleProgramStop()
{
    String id = server.arg(0);
    int program = atoi(id.c_str());
    Serial.println("Sector stop:" + String(program));
    if (program < 1 || program > SECTOR_QTY)
    {
        server.send(404, "text/plain", "Not found");
        return;
    }

    program--;

    DynamicJsonDocument jsonDocument(JSON_PROGRAM_SIZE);
    JsonObject jsonObject = jsonDocument.to<JsonObject>();

    if (runningProgram == program)
    {
        stopProgram = true;
        runningProgram = -1;
    }

    buildProgramObject(&jsonObject, program);

    String response;
    serializeJson(jsonObject, response);

    receivedChange = true;
    server.send(200, "application/json", response);
}

void handleSectorStart()
{
    String id = server.arg(0);
    int sector = atoi(id.c_str());
    Serial.println("Sector start:" + String(sector));
    if (sector < 1 || sector > SECTOR_QTY)
    {
        server.send(404, "text/plain", "Not found");
        return;
    }

    sector--;

    DynamicJsonDocument jsonDocument(JSON_PROGRAM_SIZE);
    JsonObject jsonObject = jsonDocument.to<JsonObject>();

    if (manualSector == -1 && runningProgram == -1)
    {
        DynamicJsonDocument jsonDocument(JSON_PROGRAM_SIZE);
        DeserializationError error = deserializeJson(jsonDocument, server.arg("plain"));

        if (error)
        {
            Serial.print("Failed to parse JSON: ");
            Serial.println(error.c_str());
            server.send(400, "text/plain", "Bad Request - Invalid JSON");
            return;
        }

        sectorDuration = jsonDocument["duration"].as<unsigned int>();
        Serial.println("Sector duration:" + String(sectorDuration));
        if (sectorDuration == 0)
        {
            sectorDuration = 0;
            jsonObject["error"] = "Duration must be greater than 0";
            String response;
            serializeJson(jsonObject, response);

            server.send(400, "application/json", response);
            return;
        }

        manualSector = sector;
    }
    else
    {
        jsonObject["error"] = "Sector already running";
        String response;
        serializeJson(jsonObject, response);

        server.send(400, "application/json", response);
        return;
    }

    buildSectorObject(&jsonObject);

    String response;
    serializeJson(jsonObject, response);

    receivedChange = true;
    server.send(200, "application/json", response);
}

void handleSectorStop()
{
    String id = server.arg(0);
    int sector = atoi(id.c_str());
    Serial.println("Sector stop:" + String(sector));
    if (sector < 1 || sector > SECTOR_QTY)
    {
        server.send(404, "text/plain", "Not found");
        return;
    }

    sector--;

    DynamicJsonDocument jsonDocument(JSON_PROGRAM_SIZE);
    JsonObject jsonObject = jsonDocument.to<JsonObject>();

    if (manualSector == sector)
    {
        stopManualSector = true;
        manualSector = -1;
    }
    else
    {
        jsonObject["error"] = "Sector not running";
        String response;
        serializeJson(jsonObject, response);

        server.send(400, "application/json", response);
        return;
    }

    buildSectorObject(&jsonObject);

    String response;
    serializeJson(jsonObject, response);

    receivedChange = true;
    server.send(200, "application/json", response);
}