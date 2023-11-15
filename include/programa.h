#ifndef PROGRAMA_H
#define PROGRAMA_H

#include <ArduinoJson.h>
#include "daytime.h"
#include "utils.h"

class Programa
{
public:
    int duracion;   // en minutos
    int horaInicio; // en minutos
    char dias;      // "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday":
                    // Los bits 0-6 representan los días de la semana, donde 0 es el domingo, siendo 0 el bit menos significativo.
                    // Aprovecho el bit 7 para indicar si el programa está habilitado o no
};

Programa fromJson(JsonObject jsonObject);
bool shouldWater(Programa *programa, DayTime *dayTime);
bool isSectorEnabled(Programa *programa);
bool operator==(const Programa &p1, const Programa &p2);
bool operator!=(const Programa &p1, const Programa &p2);

#endif