#include "webServer.h"

#define JSON_ALL_SIZE 1024
#define JSON_PROGRAM_SIZE 256

ESP8266WebServer server(80); // Create an instance of the server on port 80

void handleProgramPost();
void handleProgramGet();
void handleProgramsGet();
void handleProgramStart();
void handleProgramStop();
void handleSetEnabled();

bool receivedChange = false;

void startWebServer()
{
    // TODO add authentication
    server.onNotFound([]()
                      { server.send(404, "text/plain", "Not found"); });
    server.on("/programs", handleProgramsGet);
    server.on("/program", handleProgramGet);
    server.on("/program", HTTP_POST, handleProgramPost);
    server.on("/program/start", HTTP_POST, handleProgramStart);
    server.on("/program/stop", HTTP_POST, handleProgramStop);
    server.on("/enable", HTTP_POST, handleSetEnabled);

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
    (*jsonObject)["startTime"] = programa.horaInicio;
    (*jsonObject)["enabled"] = IS_ENABLED(programa) == 1;
    (*jsonObject)["manual"] = IS_MANUAL(programa) == 1;
    (*jsonObject)["isOn"] = IS_ON(programa) == 1;
    JsonArray jsonArray = jsonObject->createNestedArray("sectorDurations");
    for (int i = 0; i < SECTOR_QTY; i++)
    {
        int duration = programa.sectorDurations[i];
        jsonArray.add(duration);
    }

    int dayBitmask = 0;
    for (int j = 0; j < 7; j++)
    {
        dayBitmask |= (programa.dias & (1 << j)) ? (1 << j) : 0;
    }
    (*jsonObject)["days"] = dayBitmask;
}

void handleProgramsGet()
{
    DynamicJsonDocument jsonDocument(JSON_ALL_SIZE);
    JsonObject jsonGeneralObject = jsonDocument.to<JsonObject>();
    jsonGeneralObject["enabled"] = settings.allEnabled;
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

    DynamicJsonDocument jsonDocument(JSON_PROGRAM_SIZE);
    JsonObject jsonObject = jsonDocument.to<JsonObject>();

    buildProgramObject(&jsonObject, program);

    String response;
    serializeJson(jsonObject, response);

    Serial.println("Sector get response:" + response);

    server.send(200, "application/json", response);
}

// TODO add authentication
void handleProgramPost()
{
    Serial.println("Sector post");
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

    DynamicJsonDocument jsonDocumentAnswer(JSON_PROGRAM_SIZE); // TODO calculate size
    JsonArray jsonArray = jsonDocumentAnswer.to<JsonArray>();

    JsonObject jsonObject = jsonArray.createNestedObject();

    if (programToRun == -1)
    {
        programToRun = program;
    }
    else
    {
        jsonObject["error"] = "Program already running";
        String response;
        serializeJson(jsonArray, response);

        server.send(400, "application/json", response);
        return;
    }

    jsonObject["program"] = program + 1;

    String response;
    serializeJson(jsonArray, response);

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

    DynamicJsonDocument jsonDocumentAnswer(JSON_PROGRAM_SIZE); // TODO calculate size
    JsonArray jsonArray = jsonDocumentAnswer.to<JsonArray>();

    JsonObject jsonObject = jsonArray.createNestedObject();

    if (programToRun == program)
    {
        stopProgram = true;
    }
    else
    {
        jsonObject["error"] = "Program not running";
        String response;
        serializeJson(jsonArray, response);

        server.send(400, "application/json", response);
        return;
    }

    jsonObject["program"] = program + 1;

    String response;
    serializeJson(jsonArray, response);

    receivedChange = true;
    server.send(200, "application/json", response);
}

void handleSetEnabled()
{
    DynamicJsonDocument jsonDocument(JSON_PROGRAM_SIZE); // TODO calculate size
    DeserializationError error = deserializeJson(jsonDocument, server.arg("plain"));

    if (error)
    {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
        server.send(400, "text/plain", "Bad Request - Invalid JSON");
        return;
    }

    Serial.println(server.arg("plain"));
    Serial.println(jsonDocument["enabled"].as<bool>());

    settings.allEnabled = jsonDocument["enabled"].as<bool>();

    Serial.println("Saving to EEPROM");

    DynamicJsonDocument jsonDocumentAnswer(JSON_PROGRAM_SIZE); // TODO calculate size
    JsonArray jsonArray = jsonDocumentAnswer.to<JsonArray>();

    JsonObject jsonObject = jsonArray.createNestedObject();
    jsonObject["enabled"] = settings.allEnabled;

    String response;
    serializeJson(jsonArray, response);

    Serial.println("Sectors get response:" + response);

    receivedChange = true;
    server.send(200, "application/json", response);
}