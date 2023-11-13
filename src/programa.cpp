#include "programa.h"

Programa fromJson(JsonObject jsonObject)
{
    Programa programa;
    programa.duracion = jsonObject["duracion"].as<unsigned int>();
    programa.horaInicio = jsonObject["horaInicio"].as<unsigned int>();
    programa.dias = jsonObject["dias"].as<unsigned int>();
    return programa;
}

bool shouldWater(Programa *programa, DayTime *dayTime)
{
    return programa->dias & (1 << dayTime->day) && getDayMinutes(dayTime) >= programa->horaInicio && getDayMinutes(dayTime) < programa->horaInicio + programa->duracion;
}

bool operator==(const Programa &p1, const Programa &p2)
{
    return p1.duracion == p2.duracion && p1.horaInicio == p2.horaInicio && p1.dias == p2.dias;
}

bool operator!=(const Programa &p1, const Programa &p2)
{
    return !(p1 == p2);
}
