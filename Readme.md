# Arduino Library for Long Duration Deep Sleep on ESP8266 and ESP32
Smart Deep Sleep Library for ESP8266 – with potential ESP32 compatibility (not tested yet)

## Motivation

This library was created to support battery-powered ESP8266 based arduino projects needing powerful timer depending deep-sleep solutions.
In my case it was an e-paper display project that needs to wake up once per day, retrieve updated content from a server via wifi, update the display content accordingly, and then go back to deep sleep. The project focuses on **time-based wakeups** rather than IO-triggered ones.

During development, I encountered several challenges related to deep sleep on ESP8266 and ESP32 boards:

1. **Limited sleep duration on ESP8266**:  
   The ESP8266 can only sleep for about 3.5 hours at a time. To support longer intervals, persistent state needs to be saved across sleep cycles to decide whether to go back to sleep or perform the actual task.

2. **Sleep timing drift**:  
   Deep sleep durations are not very accurate due to the temperature-dependent behavior of the internal real-time counter (RTC), which is not a realt-time clock! This leads to deviations from the intended sleep duration.

3. **WiFi should remain off after most wakeups**:  
   To save energy, WiFi should stay off during most wake cycles. However, reactivating WiFi later becomes unreliable without specific handling. Most of the online available solutions did not work reliably when perform tests with my devices, hence I combined multiple of them.

4. **WiFi reconnection is slow and power-hungry**:  
   By default, the device performs a full network scan before reconnecting, which consumes time and power. Same here, I tested multiple solutions that somtimes worked and sometimes did not. Integrated the one, that always worked for my devices.

### ✅ Solutions implemented in this library

- **RTC memory usage**:  
  Persistent state is stored in RTC memory, allowing coordinated long-duration sleep using multiple shorter cycles on ESP8266.

- **Absolute time-based sleep scheduling**:  
  A time server (e.g. NTP) is used to determine the absolute target wake-up time, compensating for RTC drifts. I recommend to use a local device as time servers to keep answer times as fast as possible, but also public global once can be used.

- **Selective WiFi activation**:  
  The system only enables WiFi when absolutely necessary, significantly reducing power usage during wake-up.

- **Fast WiFi reconnection**:  
  A dedicated function restores the WiFi connection quickly and reliably without scanning all networks again, except the quick connect does not work and make a new scan necessary.

- **Support Wemos D1 clones with weak flash leading to "zombie mode" after deep sleep**:  
  Seems as if a large number of these boards are available and lead to this issue reported here https://github.com/esp8266/Arduino/issues/6007.
  When recenlty be confronted with one of these I found a solution for it and added it to this library. As it might not work for otheres there is an option to add additional workarounds. There is a second constructor of class LongDeepSleep now with an additional first parameter taking an enum value "CloneWorkAround".
---

This library abstracts these patterns into a simple interface, making it easier to build reliable, ultra-low-power projects based on ESP8266 (and ESP32).

## Usage

Using `LongDeepSleep` is simple and efficient. After including the library and (optionally) setting up WiFi and an NTP time client, you can schedule very long deep sleep periods with absolute or relative timing.

### 🔧 Setup

1. Create an `NTPClient` (can be local or public), only required for absolute deep sleep feature.
2. Instantiate a `LongDeepSleep` class object with your WiFi credentials and NTP client.
3. Call `checkWakeUp()` as the very first line in `setup()` to determine the wake-up reason and perform deep sleep extensions if required.
4. Then perform the task you want to do after wake-up. You can even distinguish between different wake-up reasons (reset button, errors, long deep sleep end)
5. Use `performLongDeepSleep()` or `performLongDeepSleepUntil()` accordingly to go into next long deep sleep.

### 📌 Key Functions

- `checkWakeUp()`:  
  Checks the wake-up reason and handles internal state. Must be called first.

- `performLongDeepSleep(unsigned long seconds)`:  
  Puts the device into deep sleep for a long period (may be broken into chunks, bu user does not need to care).

- `performLongDeepSleepUntil(unsigned long targetEpochTime)`:  
  Sleeps until a specific target time (UNIX timestamp, seconds since 1970-01-01, using multiple sleep cycles if needed.

### 🧪 Examples
#### Super Simple relative long deep sleep
```cpp
#include "LongDeepSleep.h"
#define _DEBUG_ 1 
#include "SwitchableSerial.h"

// this instance handles all long deep sleep related stuff:
// No Wifi or ntp client required if only using relative long deep sleep:
LongDeepSleep lds();

void setup() {
  int wakeupResult = lds.checkWakeUp();
  
  if (wakeupResult == LongDeepSleep::DEEP_SLEEP_DONE) {
    D_println("Woke up from long deep sleep!");
  } else {
    D_println("Woke up for another reason or determined an error!");
  } 
  
  \\ do whatever you need here before returning to deepsleep.

  lds.performLongDeepSleep(12 * 3600); // Sleep for apro. 12 hours. Can be significantly (1-2hours) shorter beacuse of RTC drifts!
}

void loop() {
  // nothing here
}
```

#### Wait until a specific absolute time
```cpp
#include "LongDeepSleep.h"
#define _DEBUG_ 1 
#include "SwitchableSerial.h"
#include <WiFiUdp.h>
#include <NTPClient.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

WiFiUDP ntpUDP;
// Local or public NTP server, replace IP Address and your local time zone settings
NTPClient timeClient(ntpUDP, "192.168.178.1", 3600, 60000); 

// this instance handles all long deep sleep related stuff:
LongDeepSleep lds(ssid, password, &timeClient);

void setup() {
  int wakeupResult = lds.checkWakeUp();

  if (wakeupResult == LongDeepSleep::DEEP_SLEEP_UNTIL_DONE) {
    D_println("Woke up from long deep sleep until absolute time!");
  } else if (wakeupResult == LongDeepSleep::DEEP_SLEEP_DONE) {
    D_println("Woke up from long deep sleep relative time!");
  } else {
    D_println("Woke up for another reason or determined an error!");
  } 

  // When returning from deep sleep w/o wifi reactiving which is only done when returning from ...sleepUntil().
  lds.restoreWifi();

  // Determine target time (e.g. 03:00 next day)
  unsigned long now = timeClient.getEpochTime();
  unsigned long target = (now / 86400) * 86400 + 3 * 3600;
  if (target <= now) target += 86400; // Next day, if we already passed the time today

  lds.performLongDeepSleepUntil(target);
  // We should not reach this point, except for WiFi connection or time server updated failed, then ...
  // Fallback: relative deep sleep (5 hours)
  lds.performLongDeepSleep(5 * 3600);
}

void loop() {
  // nothing here
}
```

### 🧪 See examples/LongDeepSleepExample.ino in the repository for the full working sketch including debug notes and timing measurements.
Or for detailed explanation look at the automatically generated Doxygen dokumentation: https://nickhrach.github.io/LongDeepSleep/classLongDeepSleep.html

## 🔗 Related Links & Resources
- https://www.bakke.online/index.php/2017/06/24/esp8266-wifi-power-reduction-avoiding-network-scan/

	A series of articles about avoiding Wifi Scans after restart from deep sleep. This solution did not work for me reliably when performing multiple deep sleeps without restoring Wifi, hence I decided to use WiFi.shutdown and WiFi.resumeFromShutdown instead. But I used the described solution for storing information in RTC memory.
- https://salvatorelab.com/2023/01/wemos-d1-mini-deep-sleep-current-draw/
	
    A great overview of power consumption by different versions of Wemos D1 mini modules in deep sleep. 
- https://randomnerdtutorials.com/esp8266-nodemcu-date-time-ntp-client-server-arduino/
	
    Overview over NTP client usage. Most gateway devices like Fritzbox are also able to be used as local time server. 
	
- https://github.com/esp8266/Arduino/issues/6007.
  
  Issue thread about ESP clones not waking up after deep sleep. There seem to be many different devices needing different solutions. I only integrated one solution that worked for my specific "Wemos D1 mini V3" clones. I am open to add more in case you provide the tested solution.