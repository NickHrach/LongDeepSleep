#include "LongDeepSleep.h"
#include "MockGlobalInstances.h"

NTPClient ntp;
const char *ssid="NOT IMPORTANT";
const char *password="NOT EMPLTY";

#define SIMULATE_ESP_RESTART do {ESP.moc_restart(5);\
			delete lds; \
			LongDeepSleep *lds= new LongDeepSleep(ssid, password, &ntp);} while(0)

int testStandardCases() {
	int error=0;

    std::cout << "Starting Tests\n";

    ntp.setMockTime(1749379060); // Sun Jun 08 2025 10:37:40 GMT+0000

    LongDeepSleep *lds= new LongDeepSleep(ssid, password, &ntp);

    if (lds->checkWakeUp()==LongDeepSleep::OTHER_WAKE_UP_REASON) {
         std::cout <<"[Test 1.1] Correct: Woken up from reset or restart.\n";
    } else {
         std::cout <<"[Test 1.1] Error: not correctly started without deep sleep.\n";
		error=-1;
    }

//TBD:
	//ESP.prepareforMockdeepsleep(...);
    lds->performLongDeepSleep(5000); // sleep for 5 seconds
	
	// After every long deep sleep we need to recreate the clas, as it gows through a simulated restart:
	SIMULATE_ESP_RESTART;
		
	if (lds->checkWakeUp()==LongDeepSleep::DEEP_SLEEP_DONE) {
      std::cout << "[Test 1.2] Correct: woke up from deep sleep\n";
	}
	else {
	  std::cout << "[Test 1.2] Error : Did not return correctly after deep sleep check\n";
	  error=-1;
	}
	
	unsigned long now = ntp.getEpochTime();
	unsigned long target = now + 8*3600; // 7 hours later
	lds->performLongDeepSleepUntil(target); // sleep for 5 seconds
	// We check 6 times// 3 times 
	SIMULATE_ESP_RESTART;
	if (lds->checkWakeUp()==-1) {
      std::cout << "[Test 1.3.1] Correct: deep sleep extended\n";
	}
	else {
	  std::cout << "[Test 1.3.1] Error : Did not return correctly after deep sleep check\n";
	  error=-1;
	}
	SIMULATE_ESP_RESTART;
	if (lds->checkWakeUp()==-1) {
      std::cout << "[Test 1.3.2] Correct: deep sleep extended\n";
	}
	else {
	  std::cout << "[Test 1.3.2] Error: Did not return correctly after deep sleep check\n";
	  error=-1;
	}
	SIMULATE_ESP_RESTART;
	if (lds->checkWakeUp()==-1) {
      std::cout << "[Test 1.3.3] Correct: deep sleep extended\n";
	}
	else {
	  std::cout << "[Test 1.3.3] Error: Did not return correctly after deep sleep check\n";
	  error=-1;
	}
	// ok, now it should be extended after checking time. We need to set the absolute time to target + something
	// then next 3 times should work and then return with message deepsleppUntil is done:
	ntp.setMockTime(target + 65);
	SIMULATE_ESP_RESTART;
	if (lds->checkWakeUp()==-1) {
      std::cout << "[Test 1.3.4] Correct: deep sleep extended\n";
	}
	else {
	  std::cout << "[Test 1.3.4] Error: Did not return correctly after deep sleep check\n";
	  error=-1;
	}
	SIMULATE_ESP_RESTART;
	if (lds->checkWakeUp()==-1) {
      std::cout << "[Test 1.3.5] Correct: deep sleep extended\n";
	}
	else {
	  std::cout << "[Test 1.3.5] Error: Did not return correctly after deep sleep check\n";
	  error=-1;
	}
	SIMULATE_ESP_RESTART;
	if (lds->checkWakeUp()==LongDeepSleep::DEEP_SLEEP_UNTIL_DONE) {
      std::cout << "[Test 1.3.6] Correct: deepSleepUntil done\n";
	}
	else {
	  std::cout << "[Test 1.3.6] Error: Did not return correctly after deep sleep check\n";
	  error=-1;
	}
	
	return error;
}

// TBD: test 
	// with wifi problems
	// with timeserver not responding

int main() {
	int error=0;
    error |= testCase();
    return error;
}
