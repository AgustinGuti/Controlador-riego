#ifndef UTILS_H
#define UTILS_H

#define VERSION "0.1"
#define SECTOR_QTY 8
#define PROGRAM_QTY 3

extern signed char runningProgram;
extern bool stopProgram;

extern int manualSector;
extern unsigned int sectorDuration;
extern bool stopManualSector;

extern unsigned long currentSectorStartTime;

struct SectorsStatus
{
    char sectors[SECTOR_QTY];  // Time each sector has been on
    int currentSector;         // Current sector being watered
    int currentSectorPosition; // The position in the order array of the program
};

extern SectorsStatus sectorsStatus;

#endif