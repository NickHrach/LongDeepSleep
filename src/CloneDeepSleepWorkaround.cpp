#include "c_types.h"
#include "LongDeepSleep.h"
uint32_t*RTC= (uint32_t *)0x60000700;
#define ets_delay_us ((void (*)(int))0x40002ecc)

// Adapt the following to your own needs, depending on however you make use of RTC memory somewhere else
#define RTC_MEMORY_OFFSET LongDeepSleep::UsedRTCDataSize()

// I just observed that my device sometimes immediately woke up when the sleep_time_us value was too high.
// This max value always worked during my tests, although it might be a little bit lower than required
// It's 12216795648 usec = 3hrs + 23 mins + ~37 secs
// Be aware that this workaround has no RTC clock adjustment for temperature drifts!
const uint64_t MAX_RTC1_VALUE=0x80000000;
extern const uint64_t WADeepsleep_WEMOS_D1_mini_v3_MAX_SLEEP_TIME=(MAX_RTC1_VALUE/45)<<8;

void WADeepsleep_WEMOS_D1_mini_v3(uint64_t sleep_time_us)
{
	uint32_t wakeupReason=5;
	ESP.rtcUserMemoryWrite((RTC_MEMORY_OFFSET+4)>>2, (uint32_t*)&wakeupReason, sizeof(wakeupReason));
    RTC[4] = 0;
    *RTC = 0;
    RTC[1]=100;
    RTC[3] = 0x10010;
    RTC[6] = 8;
    RTC[17] = 4;
    RTC[2] = 1<<20;
    ets_delay_us(10);
	uint64_t target = RTC[7] + (45 * (sleep_time_us >> 8));
	if (target > MAX_RTC1_VALUE)
	{
		target = MAX_RTC1_VALUE;
	}
    RTC[1] = target;
    RTC[3] = 0x640C8;
    RTC[4]= 0;
    RTC[6] = 0x18;
    RTC[16] = 0x7F;
    RTC[17] = 0x20;
    RTC[39] = 0x11;
    RTC[40] = 0x03;
    RTC[2] |= 1<<20;
    __asm volatile ("waiti 0");
}

// Workes only once, then the restartflag is cleared! 
// this should be called as early as possible in your code, keep the result somewhere else and use it for any further required restart reason usage
bool isComingFromDeepsleep_WEMOS_D1_mini_v3()
{
	uint32_t wakeupReason=0;
	ESP.rtcUserMemoryRead((RTC_MEMORY_OFFSET+4)>>2, (uint32_t*)&wakeupReason, sizeof(wakeupReason));
	bool deepSleepWakeup=(wakeupReason==5);
	wakeupReason=0;
	ESP.rtcUserMemoryWrite((RTC_MEMORY_OFFSET+4)>>2, (uint32_t*)&wakeupReason, sizeof(wakeupReason));
	return deepSleepWakeup;
}

// This here is for ESP-01 modules. Never tested as I don't have these available.
#define ets_wdt_disable ((void (*)(void))0x400030f0)

#define _R (uint32_t *)0x60000700
void WADeepsleep_ESP01(uint64_t time)
{
	ets_wdt_disable();
	*(_R + 4) = 0;
	*(_R + 17) = 4;
	*(_R + 1) = *(_R + 7) + 5;
	*(_R + 6) = 8;
	*(_R + 2) = 1 << 20;
	ets_delay_us(10);
	*(_R + 39) = 0x11;
	*(_R + 40) = 3;
	*(_R) &= 0xFCF;
	*(_R + 1) = *(_R + 7) + (45*(time >> 8));
	*(_R + 16) = 0x7F;
	*(_R + 2) = 1 << 20;
	__asm volatile ("waiti 0");
}