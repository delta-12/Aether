#include "random.h"

#include <stdint.h>
#include <stdlib.h>

void a_Random_Seed(void)
{
    uint32_t seed;
    srand((unsigned int)(uintptr_t)&seed);
}

uint32_t a_Random_Get32(void)
{
    return (uint32_t)(rand() % UINT32_MAX);
}
