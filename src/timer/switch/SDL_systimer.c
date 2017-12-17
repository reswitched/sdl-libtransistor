#include "../../SDL_internal.h"

#if SDL_TIMER_SWITCH

#include<libtransistor/types.h>
#include<libtransistor/svc.h>

#include "SDL_timer.h"

static SDL_bool ticks_started = SDL_FALSE;
static Uint64 init_system_ticks = 0;

void
SDL_TicksInit(void)
{
    if (ticks_started) {
        return;
    }
    init_system_ticks = SDL_GetPerformanceCounter();
    ticks_started = SDL_TRUE;
}

void
SDL_TicksQuit(void)
{
    ticks_started = SDL_FALSE;
}

Uint32
SDL_GetTicks(void)
{
    if (!ticks_started) {
        SDL_TicksInit();
    }

    return (SDL_GetPerformanceCounter() - init_system_ticks) * 1000 / SDL_GetPerformanceFrequency();
}

Uint64
SDL_GetPerformanceCounter(void)
{
    return svcGetSystemTick();
}

Uint64
SDL_GetPerformanceFrequency(void)
{
    return 19200000;
}

void
SDL_Delay(Uint32 ms)
{
    svcSleepThread(((Uint64) ms) * 1000000); // milliseconds to nanoseconds
}

#endif /* SDL_TIMER_SWITCH */

/* vi: set ts=4 sw=4 expandtab: */
