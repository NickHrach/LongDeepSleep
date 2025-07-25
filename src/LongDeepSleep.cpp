#include "LongDeepSleep.h"
#define _DEBUG_ 1 
#include "SwitchableSerial.h"
#include "CloneDeepSleepWorkaround.h"
constexpr uint64_t MICROSECONDS_PER_SECOND = 1000000ULL;

  LongDeepSleep::LongDeepSleep(CloneWorkaround param_cloneWorkaround, const char* ssid, const char* password, NTPClient* ntpClient)
    :wifiSsid(ssid),wifiPassword(password),timeClient(ntpClient), cloneWorkaround(param_cloneWorkaround)
  {
    RTCmemoryValid=readFromRTCmemory();
	rtcData.rebootCounter++;
  }
  
  int LongDeepSleep::checkWakeUp(uint32_t toleranceSec, uint16_t failureSleepSecs)
  {
	// same baud rate as boot process info
    // Be aware that this Serial.Begin will only be executed, if Serial was not already begun with any other valid baudrate!
	// See MAKRO definition for details in "SwitchableSerial.h"
	D_init(74880);
    D_println(F("\n----------------------------------"));
	D_print(F("resetCounter:")); D_println(rebootCounter());
    D_print(F("millis:")); D_println(millis());
	bool isComingFromDeepSleep=false;
	if (cloneWorkaround != NO_WORKAROUND)
	{
		D_println("Check for workaround reboot reason");
		if (cloneWorkaround == WEMOS_D1_V3_CLONE)
		{
			isComingFromDeepSleep=isComingFromDeepsleep_WEMOS_D1_mini_v3();
		}
		// Add here other workarounds that require sepcial reset reason detection
		if (isComingFromDeepSleep)
		{
			D_println(F("restart reason: Workaround DeepSleep wakeup detected."));
		}
	}
	if (!isComingFromDeepSleep) // only true in case a workaround got this result.
	{
		struct rst_info *rstInfo = system_get_rst_info();
		D_print(F("restart reason:"));
		D_println(rstInfo->reason);
		// Return false in case this is not wake up from deep sleep
		if (rstInfo->reason!=5) return OTHER_WAKE_UP_REASON;
	}
	
    if (!RTCmemoryValid) 
    {
      // Oops, RTC memory compromised, let's restart as if we did not get regularly out of deep sleep.
	  // Actually, I never saw this in my tests!
      D_println(F("RTC memory content failed CRC check!"));
      return RTC_MEMORY_CHECK_FAILED;
    }
  
    if (rtcData.remainingSleepTimeUs==0) 
		// We could check for tolerance value also, but then Wifi would not work. We need another deepsleep with WAKE_RF_DEFAULT, hence need to proceed in all cases
    {
		// check for absolute time target
        if (rtcData.sleepUntilEpocheTime == 0)
        {
            // no absolute time specified, we are done
            D_println(F("long relative deep sleep done."));
			// Be aware that Wifi is not restored here before we return.
            return DEEP_SLEEP_DONE;
        }
        D_println(F("relative long sleep done. Checking absolute time ..."));
        // get WLAN up and contact time server to get current time
        restoreWifi(failureSleepSecs);
        // here, we obviously did not go into deepsleep again, hence we need to check the time server
        unsigned long now = timeClient->getEpochTime();
		// timezone corrections might lead to a value > 0, hence allow a 2 days offset.
        if (now < (48u*3600u)) return ABSOLUTE_TIME_CHECK_FAILED;
        // calculate diff time is our sleep time in secs
        int64_t sleepTimeSec = rtcData.sleepUntilEpocheTime;
		
		D_print("Epoche Target:");D_println(sleepTimeSec);
		D_print("Epoche now:");D_println(now);
		sleepTimeSec = sleepTimeSec-now;
		//D_print("Epoche diff:");D_println(sleepTimeSec);
		
        if (sleepTimeSec<=toleranceSec)
        {
			// Yes, we are there!
          D_println(F("Absolute time elapsed."));
          return DEEP_SLEEP_UNTIL_DONE;
        }
		char buffer[30];
		D_print(F("Target time:"));D_println(timeClient->time2string(rtcData.sleepUntilEpocheTime,buffer,false));
		D_print(F("Still remaining time in secs:"));D_println(sleepTimeSec);
        // otherwise perform LongDeepSleep again for rest of time
        saveRTCAndCallLongDeepSleep(sleepTimeSec * MICROSECONDS_PER_SECOND);
        // Never reach this point
		return -1;
    }
    D_print(F("Long deep sleep not finally done. Remaining sleep time is:"));D_print(rtcData.remainingSleepTimeUs/MICROSECONDS_PER_SECOND);D_println(F(" secs."));
    saveRTCAndCallLongDeepSleep(rtcData.remainingSleepTimeUs);
    return -1; // this will never happen as the last function call will end in deep sleep.
  }

  uint64_t LongDeepSleep::deepSleepMax()
  {
	  if (cloneWorkaround == WEMOS_D1_V3_CLONE)
	  {
		  return WADeepsleep_WEMOS_D1_mini_v3_MAX_SLEEP_TIME;
	  }
	  return ESP.deepSleepMax()-10000000ULL;
  }

  void LongDeepSleep::saveRTCAndCallLongDeepSleep(uint64_t sleepTimeUs)
  {
	D_println(F("saveRTCAndCallLongDS"));
    uint64_t maxValue= deepSleepMax();
  
    if (sleepTimeUs>maxValue)
    {
      rtcData.remainingSleepTimeUs=sleepTimeUs-maxValue;
      sleepTimeUs=maxValue;
    } 
    else
    {
      rtcData.remainingSleepTimeUs=0;
    }
    releaseWifi();
    writeRTCmemory();
    D_print(F("Going into DEEP SLEEP FOR USECS:"));
    D_println(sleepTimeUs);
    D_print(F("Total uptime (ms):"));
    D_println(millis());
    D_println(F("#####################"));
    /*
	// Used this block to see how much runtime differs, when debug messages are not printed to serial.
	// During my tests it was about 10ms only.
	// My measurements for LongDeepSleepExample (with debug on)
	// ~60ms when directly extending long deep sleep
	// ~1250ms when checking time from timeserver and then returning into deepsleep in case Wifi can be directly restored
	// ~4500ms when checking time from timeserver and then returning into deepsleep in case Wifi cannot be directly restored and needs to perform a scan
    Serial.begin(74880);
    Serial.print(F("\nTotal uptime (ms):"));
    Serial.println(millis());
    Serial.println(F("#####################"));
    */
	// next line is for test&debug purposes only:
	/*
	sleepTimeUs=5000000;
	D_println(F(" !!!!! ATTENTION: SLEEPING ONLY 5 SECS for TEST PURPOSES !!!!"));
	*/
	D_flush();
	if (cloneWorkaround!=NO_WORKAROUND)
	{
		D_print(F("Workaround:"));D_println(cloneWorkaround);
	}
	if (cloneWorkaround == WEMOS_D1_V3_CLONE)
	{
		WADeepsleep_WEMOS_D1_mini_v3(sleepTimeUs);
	}
	else if (cloneWorkaround == WEMOS_D1_V3_CLONE)
	{
		WADeepsleep_ESP01(sleepTimeUs);	
	}
	else // if no workaround then use standard implementation
	{
		if (rtcData.remainingSleepTimeUs)
		  ESP.deepSleep(sleepTimeUs, RF_DISABLED);
		else
		  ESP.deepSleep(sleepTimeUs, RF_DEFAULT);
	}
  // does not return
  }

  void LongDeepSleep::performLongDeepSleep(uint64_t sleepTimeSec)
  {
	D_print(F("arranging long deep sleep for secs:"));D_println(sleepTimeSec);
    rtcData.sleepUntilEpocheTime=0;
    saveRTCAndCallLongDeepSleep(sleepTimeSec*MICROSECONDS_PER_SECOND); // This takes usec as parameter
  }

  int LongDeepSleep::performLongDeepSleepUntil(uint64_t epocheTime)
  {
    if (timeClient==NULL) return NO_TIME_CLIENT;
    unsigned long now = timeClient->getEpochTime();
	D_print(F("NOW:"));D_println(now);
    // let's assume there is time gone since ntpTime was initialized, but never got an absolute update
	// Also consider shifts by user defined timezones
    if (now < (48u*3600u)) return NO_ABSOLUE_TIME_AVAILABLE;
    
	// return iin case we already passed the specified time.
	if (epocheTime<now) return SPECIFIED_TIME_IN_PAST;
	
    // write target time to struct so we can check against it later when woken up
    rtcData.sleepUntilEpocheTime=epocheTime;
      
    // calculate diff time is our sleep time in secs
    uint64_t sleepTimeSec = epocheTime - now;
	char buffer[30];
	D_print(F("perform long Sleep until:"));D_println(timeClient->time2string(epocheTime,buffer,false));

    // call performLongDeepSleep
    saveRTCAndCallLongDeepSleep(sleepTimeSec*MICROSECONDS_PER_SECOND);
	return OK;
  }

  void LongDeepSleep::restoreWifi(uint16_t failureSleepSecs)
  {
    
    // immediately return in case we are already connected.
    if (WiFi.status() == WL_CONNECTED) return;

    bool quickConnect=false;
 
	D_println(F("Restoring Wifi ..."));
	if (!WiFi.resumeFromShutdown(rtcData.wifiState))
	{
		D_println(F("Cannot restore Wifi")); 
	}
	else
	{
		quickConnect=true;
	}
    
	if (!quickConnect)
    {
      D_printf("Try to connect to Wifi %s\n", wifiSsid);
      // The RTC data was not valid, so make a regular connection
       //resetWifi();
      WiFi.persistent(false);
      WiFi.mode(WIFI_STA);
      WiFi.begin(wifiSsid, wifiPassword);
    }

    int retries = 0;
    int wifiStatus = WiFi.status();
    while( wifiStatus != WL_CONNECTED ) 
    {
      retries++;
      // print a dot every 50 msec
    #ifdef _DEBUG_
      if ((retries%20)==0) {D_print(F("."));D_print(wifiStatus);}
    #endif

      if (quickConnect && (retries == maxQuickConnectCycles)) 
      {
        // Quick connect is not working, reset WiFi and try regular connection
        D_println(F("\nQuick connect failed."));
        D_printf("Try to connect to Wifi %s\n", wifiSsid);
        resetWifi();
        WiFi.begin(wifiSsid,wifiPassword);
      }
      if( retries == maxWifiWaitCycles ) 
      {
        // Giving up Wifi connection
        D_println(F("\nNot able to connect to Wifi."));
        WiFi.disconnect(true);
        delay(1);
        WiFi.mode(WIFI_OFF);
        if (failureSleepSecs>0)
        {
          D_printf("Try again in %u seconds.", failureSleepSecs);
          
          // no need to recalculate anything or rewrite RTC memory, hence calling directly ESP.deepSleep.
          ESP.deepSleep(failureSleepSecs*MICROSECONDS_PER_SECOND, RF_DEFAULT );
        }
		else
		{
			D_println("Giving up without Wifi connection");
			return;
		}
      } 
      delay(10); // one cylce is 10ms.
      wifiStatus = WiFi.status();
    }
    D_println(F("\nSuccessfully connected to Wifi."));
    D_print(F("IP Address: "));D_println(WiFi.localIP());
    D_print(F("Gateway IP: "));D_println(WiFi.gatewayIP());
    D_print(F("BSSID: "));D_println(WiFi.BSSIDstr());
    D_print(F("Channel: "));D_println(WiFi.channel());

    if (timeClient)
    {
      timeClient->begin();
      if (!timeClient->forceUpdate())
      {
		// try again:
		delay(10);
		timeClient->begin();
		if (!timeClient->forceUpdate())
		{
			D_println(F("Time server not reachable!"));
			if (failureSleepSecs>0)
			{
			  D_printf("Try again in %u seconds.", failureSleepSecs);
			  
			  // no need to recalculate anything or rewrite RTC memory, hence calling directly ESP.deepSleep.
			  ESP.deepSleep(failureSleepSecs*MICROSECONDS_PER_SECOND, RF_DEFAULT );
			}
		}
      }
    #ifdef _DEBUG_
      char timeStr[50];
      timeClient->getFormattedDateTime(timeStr, true);
      D_print(F("Current time: "));
      D_println(timeStr);
    #endif
    }
  }

  void LongDeepSleep::releaseWifi()
  {
    if (WiFi.isConnected())
    {
		D_println(F("Wifi save and shutdown"));
		WiFi.shutdown(rtcData.wifiState);
	}
  }

  void LongDeepSleep::resetWifi()
  {
    D_println(F("reset Wifi HW"));
    WiFi.disconnect();
    delay(5);
    WiFi.forceSleepBegin();
    delay(5);
    WiFi.forceSleepWake();
    delay(5);
  }

  void LongDeepSleep::resetRTCdata()
 {
	rtcData.rebootCounter=0;
	rtcData.remainingSleepTimeUs=0;
	rtcData.sleepUntilEpocheTime = 0;
 }
	
  bool LongDeepSleep::readFromRTCmemory()
  {
    D_println(F("read Info from RTC memory"));
    if (ESP.rtcUserMemoryRead(0,(uint32_t*)&rtcData, sizeof(rtcData))) 
    {
      // Calculate the CRC of what we just read from RTC memory, but skip the first 4 bytes as that's the checksum itself.
      uint32_t crc = calculateCRC32( ((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4 );
      if (crc == rtcData.crc32)
      {
        D_println(F("CRC check ok."));
        return true;
      }
      else
      {
        D_println(F("CRC check failed."));
		resetRTCdata();
        return false;
      }
    }
    
    D_println(F("Reading RTC memory failed."));
    return false;
  }

  void LongDeepSleep::writeRTCmemory()
  {
    D_println(F("write Info to RTC memory"));
    rtcData.crc32 = calculateCRC32( ((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4 );
    ESP.rtcUserMemoryWrite( 0, (uint32_t*)&rtcData, sizeof( rtcData ) );
  }

  uint32_t LongDeepSleep::calculateCRC32( const uint8_t *data, size_t length ) 
  {
    uint32_t crc = 0xffffffff;
    while( length-- ) 
    {
      uint8_t c = *data++;
      for( uint32_t i = 0x80; i > 0; i >>= 1 ) 
      {
        bool bit = crc & 0x80000000;
        if( c & i ) 
        {
          bit = !bit;
        }

        crc <<= 1;
        if( bit ) 
        {
          crc ^= 0x04c11db7;
        }
      }
    }
  return crc;
}