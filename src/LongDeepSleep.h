#ifndef _LONGDEEPSLEEP_H_
#define _LONGDEEPSLEEP_H_

#ifdef _MOCK_TEST_
#include "MockTestSetup.h"
#else
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <include/WiFiState.h>
#endif
/**
 * @brief A class for managing long deep sleep cycles with RTC memory and optimized WiFi reconnection on ESP8266.
 *
 * This class allows deep sleep durations beyond the hardware-imposed ~3.5-hour limit
 * by using RTC memory to persist sleep state between wake-ups.
 */
class LongDeepSleep
{
public:

    /**
     * @brief Return codes for checkWakeUp().
     */
    enum {
        DEEP_SLEEP_DONE = 0,              ///< Wake-up completed, proceed. WiFi not yet restored.
        DEEP_SLEEP_UNTIL_DONE = 1,        ///< Wake-up completed, WiFi and time server already available.
        OTHER_WAKE_UP_REASON = -1,        ///< Wake-up due to other reason (reset, power-on, etc.).
        RTC_MEMORY_CHECK_FAILED = -2,     ///< RTC memory is invalid or corrupted.
        ABSOLUTE_TIME_CHECK_FAILED = -3   ///< Wake-up for absolute time failed due to time server/WiFi issues.
    };
	
	
	/**
     * @brief Return codes for performLongDeepSleepUntil(int).
     */
	enum {
		OK = 0, 						///< Will never be seen as then the ESP will go into deep sleep
		NO_TIME_CLIENT = -1, 			///< No time client specified when creating class longdeepsleep, hence this function is not available.
		NO_ABSOLUE_TIME_AVAILABLE = -2, ///<  time client was specified, but it was not possible to determine an absolute time, hence this function is not available.
		SPECIFIED_TIME_IN_PAST = 1 		///< The specified until time stamp lies in the past, hence sleep not required, do whatever you want to do then.
	};

/**
     * @brief Differrent types of workarounds for deepsleep problems with clone modules
     */
	enum CloneWorkaround{
		NO_WORKAROUND = 0, 				///< This is standard solution, no workaround is applied.
		WEMOS_D1_V3_CLONE = 1, 			///< This worked for my Wemos D1 mini V3 clones
		ESP01_MODULE = 2, 			    ///< esp01 modules, not tested yet. Just copy/pasted code from here: https://github.com/nokxs/esp8266-alternative-deep-sleep
		UNKNOWN = 3
	};


   /**
     * @brief Second constructor for clone modules with deep sleep problems
     * 
     * @param param_cloneWorkaround. This is to selected the clone workaround. Use one of the enums form CloneWorkaround.
     * @param ssid WiFi SSID. (optional in case no absolute time for deep sleep is required.)
     * @param password WiFi password. (optional in case no absolute time for deep sleep is required.)
     * @param ntpClient pointer to a configured NTPClient instance. (optional in case no absolute time for deep sleep is rquired.)
     */
    LongDeepSleep(CloneWorkaround param_cloneWorkaround, const char* ssid = nullptr, const char* password = nullptr, NTPClient* ntpClient = nullptr);

    /**
     * @brief Constructor.
     * 
     * @param ssid WiFi SSID. (optional in case no absolute time for deep sleep is required.)
     * @param password WiFi password. (optional in case no absolute time for deep sleep is required.)
     * @param ntpClient pointer to a configured NTPClient instance. (optional in case no absolute time for deep sleep is rquired.)
     */
    LongDeepSleep(const char* ssid = nullptr, const char* password = nullptr, NTPClient* ntpClient = nullptr)
	:LongDeepSleep(NO_WORKAROUND,ssid,password,ntpClient){}

    /**
     * @brief Checks whether the device woke up from deep sleep and handles state. 
     * 
     * Depending on prior sleep configuration, may re-enter deep sleep if targeted sleep time is not elapsed (relative) or reached (absolute).
     * 
     * @param toleranceSec Allowed wake-up deviation in seconds (in case woke up a little bit earlier.)
     * @param failureSleepSecs If >0, sleep this long again when wifi connection or time server (only if specified) update fails. This parameter is only relevant in case we got with performLongDeepSleepUntil method into deep sleep and woke up to get into this function.
     * @return One of the enum codes above.
     */
    int checkWakeUp(uint32_t toleranceSec = 0, uint16_t failureSleepSecs = 0);

    /**
     * @brief Sleep for a relative amount of time (seconds).
     * 
     * Deep sleep is broken into cycles of up to ~3.5 hours.
     * 
     * @param sleepTimeSec Time to sleep in seconds.
     */
    void performLongDeepSleep(uint64_t sleepTimeSec);

    /**
     * @brief Sleep until an absolute epoch time.
     * 
     * Requires a wifi connection and an updated NTPClient with valid time.
     * 
     * @param epocheTime Target epoch time (in seconds).
     */
    int performLongDeepSleepUntil(uint64_t epocheTime);

    /**
     * @brief Restores WiFi after wake-up from deep sleep.
     * 
     * Optimizes for minimal connection time. Falls back to deep sleep if reconnect fails.
     * 
     * @param failureSleepSecs If >0, sleep this long again when wifi connection or time server (only if specified) update fails. Users might need to call this as it's only automatically called when woken up the last time after the system went into deep sleep via function performLongDeepSleepUntil. 
     */
    void restoreWifi(uint16_t failureSleepSecs = 0);

    /**
     * @brief Releases WiFi to conserve power.
     * 
     * Saves connection state to RTC for fast reconnect. Might be used by users in their part, when a larger part might be executed without WiFi and energy consumption is really critical. Otherwise this is called automatically within the performLongDeepSleep-functions.
     */
    void releaseWifi();

    /**
     * @brief Customizes connection wait cycles.
     * 
     * One cycle is 10 ms. Default is 20/2000 = 200ms/20s.
	 * It's not recommended to change these values, but funciton is provided for completeness.
     * 
     * @param quickConnectCycles Max cycles to reconnect via saved BSSID/channel.
     * @param wifiWaitCycles Max cycles to establish normal WiFi.
     */
    inline void changeWaitCycles(uint16_t quickConnectCycles, uint16_t wifiWaitCycles)
    {
        maxQuickConnectCycles = quickConnectCycles;
        maxWifiWaitCycles = wifiWaitCycles;
    }
	
	/**
     * @brief Returns a number that is increased in each call of checkWakeUp and also saved in RTC memory.
     * 
     * In case you want to know how often the ESP reboots until you manually reset the counter, you can get the current value via this function.
	 * The counter is increased by one every time the constructor of this class is called after the value was restored from RTC memory.
	 * Please note that you need to manually reset the value by function resetRebootCounter() whenever you need to.
     */
	uint32_t rebootCounter(){return rtcData.rebootCounter;}
	/**
     * @brief Resets the reboot counter to any other value.
     * 
     * This function resets an internal counter to a specified value. Default is zero. 
	 * The counter is increased by one every time the constructor of this class is called after the value was restored from RTC memory.
     */
	void resetRebootCounter(uint32_t resetValue=0){rtcData.rebootCounter=resetValue;}
	
	/**
     * @brief Returns RTC memory size used by this library.
     * 
     * Returns the size of the RTC memory that is used internally. You can use this to offset your own RTC memory usage in case you need to store something else to survive reboots.
	 * Be aware that the workarounds for weak ESP8266 modules need another 4 bytes of memory.
	 * They are using the same function as well. You might have a look into file CloneDeepSleepWorkaround.cpp for a hint how to use it.
     */
	static uint32_t UsedRTCDataSize(){return sizeof(UsedRTCData);}

private:
    void saveRTCAndCallLongDeepSleep(uint64_t sleepTimeSec);
    bool readFromRTCmemory();
    void writeRTCmemory();
    void saveWifiInformation();
    void resetWifi();
	void resetRTCdata();
    
	uint32_t calculateCRC32(const uint8_t *data, size_t length);

    bool RTCmemoryValid = false;
    int sleepTimeUntilWifiReconnectSec = 60 * 60;

    uint16_t maxQuickConnectCycles = 20;
    uint16_t maxWifiWaitCycles = 2000;

    const char* wifiSsid;
    const char* wifiPassword;
    NTPClient* timeClient;
	
	CloneWorkaround cloneWorkaround = NO_WORKAROUND;

    /**
     * @brief RTC memory structure to persist state across deep sleep.
     */
    struct UsedRTCData{
        uint32_t crc32;                        ///< CRC checksum.
        uint32_t rebootCounter;				   ///< counts numbers of reboots. Is increased in constructor after reading from RTC memory. Needs to be reset by user by calling resetRebootCounter()
		uint64_t remainingSleepTimeUs;         ///< Remaining sleep time in microseconds.
        uint32_t sleepUntilEpocheTime;     	   ///< Absolute wake time (epoch).
        WiFiState wifiState;                   ///< State needed to quickly reconnect to WiFi.
    } rtcData;
};

#endif
