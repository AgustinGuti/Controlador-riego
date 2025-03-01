#ifndef RIEGOSEMANA_H
#define RIEGOSEMANA_H

#include <ArduinoJson.h>

class RiegoDia
{
public:
    unsigned char dia;
    unsigned int startTime; // Start time in minutes from 00:00
    unsigned char programId;
    bool active;
};

class RiegoSemana
{
public:
    RiegoDia dias[7];
};

RiegoDia riegoDiaFromJson(JsonObject jsonObject);
void buildRiegoDiaObject(JsonObject *jsonObject, RiegoDia riegoDia);
RiegoSemana buildInitialRiegoSemana();

#endif