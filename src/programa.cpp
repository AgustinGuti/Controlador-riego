#include "programa.h"

Programa fromJson(JsonObject jsonObject)
{
    Programa programa;
    programa.horaInicio = jsonObject["startTime"].as<unsigned int>();
    programa.dias = 0;
    programa.dias |= jsonObject["enabled"] != 0 << 7;
    programa.manual = 0;
    programa.manual |= jsonObject["manual"] != 0 << 7;
    programa.manual |= jsonObject["isOn"] != 0 << 6;
    int dayBitmask = jsonObject["days"].as<unsigned int>();
    for (int j = 0; j < 7; j++)
    {
        programa.dias |= (dayBitmask & (1 << j)) ? (1 << j) : 0;
    }
    JsonArray jsonArray = jsonObject["sectorDurations"].as<JsonArray>();
    for (int i = 0; i < SECTOR_QTY; i++)
    {
        programa.sectorDurations[i] = jsonArray[i].as<unsigned int>();
    }

    return programa;
}