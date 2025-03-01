#include "riegoSemana.h"

RiegoDia riegoDiaFromJson(JsonObject jsonObject)
{
    RiegoDia riegoDia;
    riegoDia.dia = static_cast<unsigned char>(jsonObject["day"].as<int>());
    riegoDia.startTime = static_cast<unsigned int>(jsonObject["startTime"].as<int>());
    riegoDia.programId = static_cast<char>(jsonObject["programId"].as<int>());
    riegoDia.active = jsonObject["active"].as<bool>();

    return riegoDia;
}

RiegoSemana buildInitialRiegoSemana()
{
    RiegoSemana riegoSemana;
    for (int i = 0; i < 7; i++)
    {
        riegoSemana.dias[i].dia = i;
        riegoSemana.dias[i].startTime = 0;
        riegoSemana.dias[i].programId = 0;
        riegoSemana.dias[i].active = false;
    }

    return riegoSemana;
}

String toString(RiegoDia riegoDia, int day)
{
    String string = "Day: ";
    string += day;
    string += " Start time: ";
    string += riegoDia.startTime;
    string += " Program id: ";
    string += riegoDia.programId;
    string += " Active: ";
    string += riegoDia.active;

    return string;
}

void buildRiegoDiaObject(JsonObject *jsonObject, RiegoDia riegoDia)
{
    (*jsonObject)["day"] = riegoDia.dia;
    (*jsonObject)["startTime"] = riegoDia.startTime;
    (*jsonObject)["programId"] = riegoDia.programId;
    (*jsonObject)["active"] = riegoDia.active;
}