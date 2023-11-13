#include "utils.h"

uint calculateProgramOffset(char sector)
{
    return (sector - 1) * sizeof(Programa);
}