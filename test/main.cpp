#include "LongDeepSleep.h"
#include "MockGlobalInstances.h"
#include <gtest/gtest.h>

LongDeepSleep *lds=NULL;
NTPClient ntp;
const char *ssid="Simulatet Wifi SSID";
const char *password="Simulated Wifi password";


// in a real restart the lds object would be created in a plain environment, hence we destroy and re-create it:
#define SIMULATE_ESP_RESTART do {ESP.moc_restart(5);\
			delete lds; \
			lds= new LongDeepSleep(ssid, password, &ntp);} while(0)

TEST(LongDeepSleepTest, LongRelativeSleep) {
	lds= new LongDeepSleep(ssid, password, &ntp);
	int wakeup_reason=lds->checkWakeUp();
	EXPECT_EQ(wakeup_reason, LongDeepSleep::OTHER_WAKE_UP_REASON) ;
	lds->performLongDeepSleep(5000); // sleep for 5 seconds	
	SIMULATE_ESP_RESTART;
	wakeup_reason=lds->checkWakeUp();
	EXPECT_EQ(wakeup_reason, LongDeepSleep::DEEP_SLEEP_DONE) ;
}

TEST(LongDeepSleepTest, LongAbsoluteSleep) {
	ESP.moc_restart();
	ntp.setMockTime(0); 
	lds= new LongDeepSleep(ssid, password, &ntp);
	EXPECT_EQ(lds->performLongDeepSleepUntil(1000), LongDeepSleep::NO_ABSOLUE_TIME_AVAILABLE) << "Checked for missing absolute time";

	ntp.setMockTime(1749379060); // Sun Jun 08 2025 10:37:40 GMT+0000
	unsigned long now = ntp.getEpochTime();
	unsigned long target = now + 8*3600; // 8 hours later
	EXPECT_EQ(lds->performLongDeepSleepUntil(now-10), LongDeepSleep::SPECIFIED_TIME_IN_PAST) << "Checked for sleep until time in past";
	
	int return_value = lds->performLongDeepSleepUntil(target); 
	EXPECT_EQ(return_value, LongDeepSleep::OK) << "Checked for sleep until in 8 hours\n";
	// We check 3 times 
	SIMULATE_ESP_RESTART;
	EXPECT_EQ(lds->checkWakeUp(), -1);
	SIMULATE_ESP_RESTART;
	EXPECT_EQ(lds->checkWakeUp(), -1);
	SIMULATE_ESP_RESTART;
	EXPECT_EQ(lds->checkWakeUp(), -1);
	// ok, now it should be extended after checking time. We need to set the absolute time to target + something
	// then next 3 times should work and then return with message deepsleppUntil is done:
	ntp.setMockTime(target + 65);
	SIMULATE_ESP_RESTART;
	EXPECT_EQ(lds->checkWakeUp(), -1);
	SIMULATE_ESP_RESTART;
	EXPECT_EQ(lds->checkWakeUp(), -1);
	SIMULATE_ESP_RESTART;
	EXPECT_EQ(lds->checkWakeUp(), LongDeepSleep::DEEP_SLEEP_UNTIL_DONE) ;
}

TEST(LongDeepSleepTest, NoWifi) {	
    lds= new LongDeepSleep(ssid, password, &ntp);
	// Now: What happens when Wifi cannot connect after absolue sleep?
	unsigned long now = ntp.getEpochTime();
	unsigned long target = now + 1*3600; // 1 hour later can be done within one cycle.
	lds->performLongDeepSleepUntil(target);
	SIMULATE_ESP_RESTART;
	WiFi.setStatus(WL_CONNECT_FAILED);
	ntp.setMockTime(40); // No Wifi --> no absolutetime.
	int wakeup_reason=lds->checkWakeUp();
	EXPECT_EQ(wakeup_reason, LongDeepSleep::ABSOLUTE_TIME_CHECK_FAILED) ;
}

