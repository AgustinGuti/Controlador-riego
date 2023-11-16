#include "programa.h"

extern const char *dayNames[];

Programa fromJson(JsonObject jsonObject)
{
    Programa programa;
    programa.duracion = jsonObject["duration"].as<unsigned int>();
    programa.horaInicio = jsonObject["startTime"].as<unsigned int>();
    programa.dias = 0;
    programa.dias |= jsonObject["enabled"] != 0 << 7;
    programa.manual = 0;
    programa.manual |= jsonObject["manual"] != 0 << 7;
    programa.manual |= jsonObject["isOn"] != 0 << 6;
    for (int j = 0; j < 7; j++)
    {
        programa.dias |= (jsonObject["days"][dayNames[j]] != 0) << j;
    }
    return programa;
}

bool shouldWater(Programa *programa, DayTime *dayTime)
{
    return (IS_MANUAL(*programa) && IS_ON(*programa)) ||
           (!IS_MANUAL(*programa) && IS_ENABLED(*programa) &&
            IS_DAY_SET(*programa, dayTime->day) &&
            getDayMinutes(dayTime) >= programa->horaInicio && getDayMinutes(dayTime) < programa->horaInicio + programa->duracion);
}

bool operator==(const Programa &p1, const Programa &p2)
{
    return p1.duracion == p2.duracion && p1.horaInicio == p2.horaInicio && p1.dias == p2.dias;
}

bool operator!=(const Programa &p1, const Programa &p2)
{
    return !(p1 == p2);
}
