#ifndef UTILS_H_2579621476717839321
#define UTILS_H_2579621476717839321

#include <Arduino.h>
#include "programa.h"

#define VERSION "0.1"

extern char programToRun;
extern char runningProgram;
extern bool stopProgram;

uint calculateProgramOffset(char sector);

#endif
