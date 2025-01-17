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

boolean validateSettings(Settings *settings)
{
    boolean changes = false;
    char savedOrder[SECTOR_QTY];
    bool duplicateFound = false;
    for (int i = 0; i < PROGRAM_QTY; i++)
    {
        for (int j = 0; j < SECTOR_QTY; j++)
        {
            if (settings->programs[i].sectorDurations[j] > 30)
            {
                settings->programs[i].sectorDurations[j] = 30;
                changes = true;
            }
            if (!duplicateFound)
            {
                if (settings->programs[i].sectorOrderPositions[j] > SECTOR_QTY - 1)
                {
                    settings->programs[i].sectorOrderPositions[j] = j;
                    changes = true;
                }
                savedOrder[j] = settings->programs[i].sectorOrderPositions[j];
                for (int k = 0; k < j; k++)
                {
                    if (k != j && savedOrder[j] == savedOrder[k])
                    {
                        duplicateFound = true;
                    }
                }
            }
        }
    }

    if (duplicateFound)
    {
        for (int i = 0; i < PROGRAM_QTY; i++)
        {
            for (int j = 0; j < SECTOR_QTY; j++)
            {
                settings->programs[i].sectorOrderPositions[j] = j;
            }
        }
        changes = true;
    }

    return changes;
}