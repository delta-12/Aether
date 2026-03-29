#include "tick.h"

#if defined(AETHER_TICK_USER)
extern a_Tick_Ms_t a_TickUser_GetTick(void);
#elif defined(ARDUINO)
#include "Arduino.h"
#else
#include <time.h>

#define A_TICK_MILLISECONDS_TO_SECONDS     1000U
#define A_TICK_NANOSECONDS_TO_MILLISECONDS 1000000U
#endif /* AETHER_TICK_USER */

a_Tick_Ms_t a_Tick_GetTick(void)
{
#if defined(AETHER_TICK_USER)
    return a_TickUser_GetTick();
#elif defined(ARDUINO)
    return millis();
#else
    a_Tick_Ms_t     tick = A_TICK_MS_MAX;
    struct timespec time;

    if (0 != timespec_get(&time, TIME_UTC))
    {
        tick = (a_Tick_Ms_t)(time.tv_sec * A_TICK_MILLISECONDS_TO_SECONDS + time.tv_nsec / A_TICK_NANOSECONDS_TO_MILLISECONDS);
    }

    return tick;
#endif /* AETHER_TICK_USER */
}

a_Tick_Ms_t a_Tick_GetElapsed(const a_Tick_Ms_t start)
{
    return a_Tick_GetTick() - start;
}
