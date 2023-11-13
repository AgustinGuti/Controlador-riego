#include "webServer.h"

#define JSON_SIZE 128

ESP8266WebServer server(80); // Create an instance of the server on port 80

void handleSectorPost();
void handleSectorGet();
void handleSectorsGet();

void startWebServer()
{
    // TODO add authentication
    server.onNotFound([]()
                      { server.send(404, "text/plain", "Not found"); });
    server.on("/sectors", handleSectorsGet);
    server.on("/sectors/:id", handleSectorGet);
    server.on("/sectors/:id", HTTP_POST, handleSectorPost);

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

        jsonObject["sector"] = i + 1;
        jsonObject["duracion"] = programa.duracion;
        jsonObject["horaInicio"] = programa.horaInicio;
        jsonObject["dias"] = (int)programa.dias;
    }

    String response;
    serializeJson(jsonArray, response);

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

    jsonObject["duracion"] = programa.duracion;
    jsonObject["horaInicio"] = programa.horaInicio;
    jsonObject["dias"] = (int)programa.dias;

    String response;
    serializeJson(jsonObject, response);

    server.send(200, "application/json", response);
}

void handleSectorPost()
{
    // TODO ask if i should check watering times don't overlap
    String id = server.pathArg(0);
    int sector = atoi(id.c_str());

    if (sector < 1 || sector > SECTOR_QTY)
    {
        server.send(404, "text/plain", "Not found");
        return;
    }

    DynamicJsonDocument jsonDocument(JSON_SIZE);
    DeserializationError error = deserializeJson(jsonDocument, server.arg("plain"));

    Serial.println(server.arg("plain"));

    if (error)
    {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
        server.send(400, "text/plain", "Bad Request - Invalid JSON");
        return;
    }

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