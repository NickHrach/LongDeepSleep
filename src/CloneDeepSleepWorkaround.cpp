#include "c_types.h"
#include "LongDeepSleep.h"
uint32_t*RTC= (uint32_t *)0x60000700;
#define ets_delay_us ((void (*)(int))0x40002ecc)

// Adapt the following to your own needs, depending on however you make use of RTC memory somewhere else
#define RTC_MEMORY_OFFSET LongDeepSleep::UsedRTCDataSize()

void WADeepsleep_WEMOS_D1_mini_v3(uint64_t sleep_time_us)
{
	uint32_t wakeupReason=5;
	ESP.rtcUserMemoryWrite((RTC_MEMORY_OFFSET+4)>>2, (uint32_t*)&wakeupReason, sizeof(wakeupReason));
	Serial.flush();
    RTC[4] = 0;
    *RTC = 0;
    RTC[1]=100;
    RTC[3] = 0x10010;
    RTC[6] = 8;
    RTC[17] = 4;
    RTC[2] = 1<<20;
    ets_delay_us(10);
    RTC[1]= RTC[7] + (45 * (sleep_time_us >> 8));
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
boolean isComingFromDeepsleep_WEMOS_D1_mini_v3()
{
	uint32_t wakeupReason=0;
	ESP.rtcUserMemoryRead((RTC_MEMORY_OFFSET+4)>>2, (uint32_t*)&wakeupReason, sizeof(wakeupReason));
	boolean deepSleepWakeup=(wakeupReason==5);
	wakeupReason=0;
	ESP.rtcUserMemoryWrite((RTC_MEMORY_OFFSET+4)>>2, (uint32_t*)&wakeupReason, sizeof(wakeupReason));
	return deepSleepWakeup;
}