#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include "programa.h"
#include "riegoSemana.h"
#include "utils.h"

uint calculateProgramOffset(char sector);
uint calculateRiegoSemanaOffset(char day);

class Settings
{
public:
    Programa programs[PROGRAM_QTY];
    RiegoSemana semana;
};

boolean validateSettings(Settings *settings);

#endif