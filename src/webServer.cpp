#include "webServer.h"

#define JSON_SIZE 128

ESP8266WebServer server(80); // Create an instance of the server on port 80

void handleSectorPost();
void handleSectorGet();
void handleSectorsGet();
void handleSetEnabled();

extern const char *dayNames[];

void startWebServer()
{
    // TODO add authentication
    server.onNotFound([]()
                      { server.send(404, "text/plain", "Not found"); });
    server.on("/sectors", handleSectorsGet);
    server.on("/sectors/:id", handleSectorGet);
    server.on("/sectors/:id", HTTP_POST, handleSectorPost);
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

void handleSectorsGet()
{
    DynamicJsonDocument jsonDocument(JSON_SIZE);
    JsonArray jsonArray = jsonDocument.to<JsonArray>();

    for (int i = 0; i < SECTOR_QTY; i++)
    {
        Programa programa = programas[i];

        JsonObject jsonObject = jsonArray.createNestedObject();
        jsonObject["enabled"] = allEnabled;

        JsonObject sectorsObject = jsonObject.createNestedObject("sectors");
        sectorsObject["sector"] = i + 1;
        sectorsObject["duracion"] = programa.duracion;
        sectorsObject["horaInicio"] = programa.horaInicio;
        sectorsObject["enabled"] = isSectorEnabled(&programa);
        JsonObject diasObject = sectorsObject.createNestedObject("dias");
        for (int j = 0; j < 7; j++)
        {
            diasObject[dayNames[j]] = (programa.dias & (1 << j)) != 0;
        }
    }

    String response;
    serializeJson(jsonArray, response);

    Serial.println("Sectors get response:" + response);

    server.send(200, "application/json", response);
}

void handleSectorGet()
{
    String id = server.pathArg(0);
    int sector = atoi(id.c_str());
    if (sector < 1 || sector > SECTOR_QTY)
    {
        server.send(404, "text/plain", "Not found");
        return;
    }

    Programa programa = programas[sector];

    DynamicJsonDocument jsonDocument(JSON_SIZE);
    JsonObject jsonObject = jsonDocument.to<JsonObject>();

    jsonObject["sector"] = sector;
    jsonObject["duracion"] = programa.duracion;
    jsonObject["horaInicio"] = programa.horaInicio;
    jsonObject["enabled"] = isSectorEnabled(&programa);
    JsonObject diasObject = jsonObject.createNestedObject("dias");
    for (int j = 0; j < 7; j++)
    {
        diasObject[dayNames[j]] = (programa.dias & (1 << j)) != 0;
    }

    String response;
    serializeJson(jsonObject, response);

    Serial.println("Sector get response:" + response);

    server.send(200, "application/json", response);
}
// TODO add authentication
void handleSectorPost()
{
    String id = server.pathArg(0);
    int sector = atoi(id.c_str());

    if (sector < 1 || sector > SECTOR_QTY)
    {
        server.send(404, "text/plain", "Not found");
        return;
    }

    DynamicJsonDocument jsonDocument(JSON_SIZE);
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
    if (programa != programas[sector])
    {
        Serial.println("Saving to EEPROM");
        // TODO uncomment after testing
        //    EEPROM.put(calculateProgramOffset(sector), programas);
    }

    programas[sector] = programa;
    server.send(200, "text/plain", "OK");
}

void handleSetEnabled()
{
    DynamicJsonDocument jsonDocument(JSON_SIZE);
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

    allEnabled = jsonDocument["enabled"].as<bool>();

    Serial.println("Saving to EEPROM");

    DynamicJsonDocument jsonDocument(JSON_SIZE);
    JsonArray jsonArray = jsonDocument.to<JsonArray>();

    JsonObject jsonObject = jsonArray.createNestedObject();
    jsonObject["enabled"] = allEnabled;

    String response;
    serializeJson(jsonArray, response);

    Serial.println("Sectors get response:" + response);

    server.send(200, "application/json", response);
}