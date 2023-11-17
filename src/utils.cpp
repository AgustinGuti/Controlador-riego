#include "utils.h"

uint calculateProgramOffset(char sector)
{
    // 1 char for all enabled byte
    return sizeof(char) + (sector - 1) * sizeof(Programa);
}