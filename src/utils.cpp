#include "utils.h"

uint calculateProgramOffset(char sector)
{
    return (sector) * sizeof(Programa);
}
