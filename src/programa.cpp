#include "programa.h"

int calculateRemainingTime(int duration);

Programa fromJson(JsonObject jsonObject)
{
    Programa programa;
    JsonArray jsonArray = jsonObject["sectorDurations"].as<JsonArray>();
    for (int i = 0; i < SECTOR_QTY; i++)
    {
        unsigned char duration = static_cast<unsigned char>(jsonArray[i].as<int>());
        programa.sectorDurations[i] = duration;
    }
    JsonArray jsonArray2 = jsonObject["sectorOrder"].as<JsonArray>();
    for (int i = 0; i < SECTOR_QTY; i++)
    {
        unsigned char sector = static_cast<unsigned char>(jsonArray2[i].as<int>());
        programa.sectorOrderPositions[i] = sector;
    }

    return programa;
}

String toString(Programa programa, int program)
{
    String string = "Id: ";
    string += program;
    string += " Sector durations: [";
    for (int i = 0; i < SECTOR_QTY; i++)
    {
        string += programa.sectorDurations[i];
        string += " ";
    }
    string += "]";
    string += " Sector order: [";
    for (int i = 0; i < SECTOR_QTY; i++)
    {
        string += programa.sectorOrderPositions[i];
        string += " ";
    }
    string += "]";

    return string;
}

void buildProgramObject(JsonObject *jsonObject, Programa programa, int programId)
{
    (*jsonObject)["program"] = programId + 1;
    (*jsonObject)["isOn"] = runningProgram == programId;
    JsonArray jsonArray = jsonObject->createNestedArray("sectorDurations");
    for (int i = 0; i < SECTOR_QTY; i++)
    {
        int duration = programa.sectorDurations[i];
        jsonArray.add(duration);
    }
    JsonArray jsonArray2 = jsonObject->createNestedArray("sectorOrder");
    for (int i = 0; i < SECTOR_QTY; i++)
    {
        int sector = programa.sectorOrderPositions[i];
        jsonArray2.add(sector);
    }
}

void buildSectorObject(JsonObject *jsonObject, Programa programa)
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
        // Programa programa = settings.programs[(int)runningProgram];
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