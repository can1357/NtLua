#include <intrin.h>
#include "crt.h"

void abort()
{
	__debugbreak();
}

char* getenv()
{
	return "virtual://";
}

typedef long clock_t;

extern "C" clock_t clock()
{
    ULARGE_INTEGER TickCount;

    while (TRUE)
    {
        TickCount.HighPart = (ULONG)SharedUserData->TickCount.High1Time;
        TickCount.LowPart = SharedUserData->TickCount.LowPart;

        if (TickCount.HighPart == (ULONG)SharedUserData->TickCount.High2Time) break;

        YieldProcessor();
    }

    return (UInt32x32To64(TickCount.LowPart, SharedUserData->TickCountMultiplier) >> 24) +
        (UInt32x32To64(TickCount.HighPart, SharedUserData->TickCountMultiplier) << 8);
}

extern "C" time_t _time64(time_t* time)
{
    LARGE_INTEGER SystemTime;

    do
    {
        SystemTime.HighPart = SharedUserData->SystemTime.High1Time;
        SystemTime.LowPart = SharedUserData->SystemTime.LowPart;
    } while (SystemTime.HighPart != SharedUserData->SystemTime.High2Time);

    // https://www.gamedev.net/forums/topic/565693-converting-filetime-to-time_t-on-windows/
    return SystemTime.QuadPart / 10000000ULL - 11644473600ULL;
}