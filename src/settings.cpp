#include "settings.h"

// TODO modularize to programs part
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

uint calculateProgramOffset(char sector)
{
    return (sector) * sizeof(Programa);
}

uint calculateRiegoSemanaOffset(char day)
{
    return sizeof(Programa) * PROGRAM_QTY + day * sizeof(RiegoDia);
}