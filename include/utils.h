#ifndef UTILS_H_2579621476717839321
#define UTILS_H_2579621476717839321

#include <Arduino.h>
#include "programa.h"

#define SECTOR_QTY 8
#define VERSION "0.1"

static const char *dayNames[] = {"sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"};

uint calculateProgramOffset(char sector);

#endif
