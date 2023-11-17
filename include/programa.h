#ifndef PROGRAMA_H
#define PROGRAMA_H

#include <ArduinoJson.h>
#include "utils.h"

#define SECTOR_QTY 8
#define PROGRAM_QTY 3

// X is a Programa
#define IS_MANUAL(X) ((X).manual & (1 << 7))
#define IS_ON(X) ((X).manual & (1 << 6))
#define IS_ENABLED(X) ((X).dias & (1 << 7))
#define IS_DAY_SET(X, Y) ((X).dias & (1 << (Y)))

class Programa
{
public:
    int horaInicio;                   // en minutos
    char dias;                        // "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday":
                                      // Los bits 0-6 representan los días de la semana, donde 0 es el domingo, siendo 0 el bit menos significativo.
                                      // Aprovecho el bit 7 para indicar si el programa está habilitado o no
    char manual;                      // primer bit indica si está en modo manual o no. Si está en modo manual, el segundo bit indica si está prendido o no (1 es prendido, 0 es apagado)
    char sectorDurations[SECTOR_QTY]; // duración de cada sector en minutos. Si es 0, el sector no se riega
};

class Settings
{
public:
    bool allEnabled;
    Programa programs[PROGRAM_QTY];
};

Programa fromJson(JsonObject jsonObject);

#endif