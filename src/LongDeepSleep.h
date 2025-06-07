#ifndef _LONGDEEPSLEEP_H_
#define _LONGDEEPSLEEP_H_
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <include/WiFiState.h>

class LongDeepSleep
{

public:

  enum 
  {
    DEEP_SLEEP_DONE = 0,      // wake up was done successfully, just proceed as planned. Wifi is not restored yet.
    DEEP_SLEEP_UNTIL_DONE = 0, // wake up was done successfully, just proceed as planned. Wifi and time server are already available!
    OTHER_WAKE_UP_REASON = 1, // The system was woken up for another reason, check system_get_rst_info()
    RTC_MEMORY_CHECK_FAILED =2, // RTC memory got corrupted or could not be read. Checking for long deep sleep is not possible
    ABSOLUTE_TIME_CHECK_FAILED = 3, // An absolute target time was specified and the calculated relative time elapsed, but Wifi re-connection was not successfull and/or time server was not reached. Absolute time check was not possible.  
  };


  LongDeepSleep(const char* ssid=0, const char* password=0, NTPClient* ntpClient=NULL);
  /* Check whether this was a wakeup out of deepsleep
    Returns true if the time elapsed and it was the end of a deep sleep
    Returns false if it was another wakeup reason (hard reset etc.)
    Does not return in case the deep sleep was not finalized and needed another deepsleep cycle.
    Paremeter tolarenceSec allows a wakeup before the elapsed time or the absolue time
    Indeed ESP.deepsleep does not exactly sleep the specified time as the RTC sleep time varies based on temperature and other influencing factors.
    In case an absolute time is given by calling PerformLongDeepSleepUntil this funtion will try to reach out for a time server via Wifi and then perform another sleep in case it is required.
    When going into deep sleep via PerformLongDeepSleep with a relative time this funtion will not use use Wifi

    parameter failureSleepSecs is relevant in case an absolute time for wakeup was given.
    In case it's >0 and no connection to timeserver is possible we will deep sleep for this amount of time again and then check again.
    This is usefull if you can't do anyhting meaningful anyway. If this value is 0, the function returns with value NO_ABSOLUTE_TIME_CHECK_POSSIBLE 
      */
  int checkWakeUp(uint32_t toleranceSec=0, uint16_t failureSleepSecs=0);

  /* sleep long in seconds
    The time will not exactly be met. ESP can only sleep for 3-4 hours. Even for such a short period it might wake up ~20 minutes earlier.
    As there is no absolute time in deep sleep mode available, we need to believe in the time being elapsed correctly.
    In case you need a more exact wake up, use PerformLongDeepSleepUntil instead
  */
  void performLongDeepSleep(uint64_t sleepTimeSec);

  /* sleep until time
  epocheTime is number of seconds that have elapsed since January 1, 1970 (midnight GMT); this might be shifted depending on how the ntpClient is setup. 
  Attention: This will return in case there is no ntpClient specified or the ntpClient is not returning a valid current time (no update was done to it with netowrk conenction after last reset.)
  */
  void performLongDeepSleepUntil(uint64_t epocheTime); 

  /*
    Wakes Wifi after Deepsleep. Tihs is required because we optimized Wifi usage for shortest wake-up and reduce power consumption.
    If it's not possible to get Wifi connection it goes into deepsleep in case failureSleepSecs is >0. 
  */
  void restoreWifi(uint16_t failureSleepSecs=0);
  /*
    writes Wifi informaiton to internal structure to be saved when going into next deep sleep and
    switches Wifi off in order to reduce power consumption for further program execution without Wifi.
    */

  /* This might be used to release Wifi earlier and perform other tasks, that do not need Wifi.
	It's recommended to use the NTP time client also before this in order to calculate
  */

  void releaseWifi();
  /*
    Change the default wait cycles for Wifi reconnection purposes.
    1 cycles is 10ms. 
    Dtandard values are 20 and 2000. I do not recommend to change these.
  */
  inline void changeWaitCycles(uint16_t quickConnectCycles, uint16_t wifiWaitCycles)
  {
    maxQuickConnectCycles=quickConnectCycles; 
    maxWifiWaitCycles=wifiWaitCycles;
  }

  private:
  void saveRTCAndCallLongDeepSleep(uint64_t sleepTimeSec);
  bool readFromRTCmemory();
  void writeRTCmemory();
  void saveWifiInformation();
  void resetWifi();
  
  uint32_t calculateCRC32(const uint8_t *data,size_t length);
  bool RTCmemoryValid=false;

  int sleepTimeUntilWifiReconnectSec=60*60; // One hour
  uint16_t maxQuickConnectCycles=20; // 1 cycle = 10ms. Wait max 200 ms to reconnect with old BSSi and channel
  uint16_t maxWifiWaitCycles=2000; // 1 cycle = 10ms. Wait max. 20 secs to reach Wifi

  const char* wifiSsid;
  const char* wifiPassword;
 
  NTPClient *timeClient;

  // Struct to save data in RTC memory which is kept alive during deep sleep
  // Needs to be a mutliply by 4 as RTC memory is organized in 4-bytes chunks.
  struct {
      //checksum
    uint32_t crc32;   // 4 bytes
    
    // sleeptime information
    uint64_t remainingSleepTimeUs;  // 8bytes
    uint32_t sleepUntilEpocheTime=0;  // 4 bytes

    // Wifi reestablishing optimization (in sum 8)
    // Wifi reestablishing optimization 
    WiFiState wifiState;
  } rtcData;
};

#endif