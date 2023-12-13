#include "programa.h"

Programa fromJson(JsonObject jsonObject)
{
    Programa programa;
    JsonArray jsonArray = jsonObject["sectorDurations"].as<JsonArray>();
    for (int i = 0; i < SECTOR_QTY; i++)
    {
        unsigned char duration = static_cast<unsigned char>(jsonArray[i].as<int>());
        programa.sectorDurations[i] = duration;
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

    return string;
}