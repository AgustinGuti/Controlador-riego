#ifndef PROGRAMA_H
#define PROGRAMA_H

#include <ArduinoJson.h>
#include "daytime.h"

class Programa
{
public:
    int duracion;   // en minutos
    int horaInicio; // en minutos
    char dias;      // "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday": Los bits 0-6 representan los días de la semana, donde 0 es el domingo, siendo 0 el bit menos significativo.
};

Programa fromJson(JsonObject jsonObject);
bool shouldWater(Programa *programa, DayTime *dayTime);
bool operator==(const Programa &p1, const Programa &p2);
bool operator!=(const Programa &p1, const Programa &p2);

#endif