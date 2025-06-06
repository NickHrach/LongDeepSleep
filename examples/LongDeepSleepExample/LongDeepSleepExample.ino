#include "LongDeepSleep.h"
#include "SwitchableSerial.h"

const char* ssid = "YOUR SSID";
const char* password = "YOUR WiFi password";
WiFiUDP ntpUDP;
// Here a local server is sued as time server, replace by your own (local response is usually faster) or a global one.
// ege one of these:
// time-b-g.nist.gov: 129.6.15.29
// ptbtime1.ptb.de: 192.53.103.108
NTPClient timeClient(ntpUDP, "192.168.178.1", 1*60*60, 1*60*60000); // middle Europe timezone

LongDeepSleep lds(ssid,password,&timeClient);

	// My measurements for LongDeepSleepExample (with debug on) on a Wemos D1:
	// ~60ms when directly extending long deep sleep
	// ~1250ms when checking time from timeserver and then returning into deepsleep in case Wifi can be directly restored
	// ~4500ms when checking time from timeserver and then returning into deepsleep in case Wifi cannot be directly restored
  // debug messages off save around 10ms in all cases.
  // TBD: Need to make measurems of power cosnumption for different devices. 
void setup() {
  // This line should always be the first and will be called directly after wakeup:
  int wakeupResult=lds.checkWakeUp(0);
  if (wakeupResult==LongDeepSleep::DEEP_SLEEP_DONE)
  {
    D_println("Woke up from long deep sleep!");
  }
  else
  {
    D_println("returned because of other reason");
  }
  // Just in case we woke up w/o wifi re-established.
  lds.restoreWifi();
  D_println("=========================");
  D_println("Calling long deep sleep in INIT");
	
  // Determine value for absolute time 0:30 Uhr
  unsigned hour=3;
  unsigned minute=0;
  unsigned long now = timeClient.getEpochTime();
  unsigned long target = (now / 86400) * 86400 + 3600 * hour + 60 * minute; // 03:00 Uhr im Winter und 2:00 im Sommer
  if (target <= now) target += 86400; // If we are already passed this time today, add another day to wake up tomorrow.

  lds.performLongDeepSleepUntil(target);
  // In case this does not work, e.g. the time server did not response (cpuld be checked above before already), the ...SleepUntil function will return instead of going into deepsleep.
  
 
   // let's go int a relative long deep sleep then:
  lds.performLongDeepSleep(5*3600); // 5 hours will wake up 2 times, first to extend deep sleep, second to get here again.

	// Will never see this line1
}

void loop() {
  /// Nothing to be done here
}
