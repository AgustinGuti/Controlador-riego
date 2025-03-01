#ifndef PROGRAMA_H
#define PROGRAMA_H

#include <ArduinoJson.h>
#include "utils.h"

#define SECTOR_QTY 8
#define PROGRAM_QTY 3

class Programa
{
public:
    unsigned char sectorOrderPositions[SECTOR_QTY]; // orden de riego de los sectores. Por ejemplo, [2,3,1] significa que se riega el sector 2, luego el 3 y por último el 1
    unsigned char sectorDurations[SECTOR_QTY];      // duración de cada sector en minutos. Si es 0, el sector no se riega
};

Programa fromJson(JsonObject jsonObject);
String toString(Programa programa, int program);
void buildSectorObject(JsonObject *jsonObject, Programa programa);
void buildProgramObject(JsonObject *jsonObject, Programa programa, int programId);

#endif