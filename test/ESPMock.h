#ifndef _ESP_MOCK_
#define _ESP_MOCK_

#include <cstring>
#include <iostream>
#include <chrono>

struct rst_info
{
	uint16_t reason;
};

class MockESP {
private:
    static constexpr size_t RTC_MEM_SIZE = 1024;
    uint8_t rtcMemory[RTC_MEM_SIZE] = {0};
    uint64_t maxDeepSleepDuration = 3.5 * 3600 * 1000000ULL; // ~3.5 Stunden in Mikrosekunden

public:
	void moc_restart(int reason=6)
	{
		std::cout << "[MockESP] (Re-)starting\n";
		moc_resettime();
		_system_get_rst_info.reason=reason;
	}
	
	void moc_resettime()
	{
		start_time = std::chrono::steady_clock::now();
	}

	MockESP(){moc_restart();}
	
    // Simuliert das Lesen aus dem RTC-Speicher
    bool rtcUserMemoryRead(int offset, uint32_t* data, size_t length) {
        if (offset != 0 || length > RTC_MEM_SIZE) {
            return false;
        }
        memcpy(data, rtcMemory, length);
        std::cout << "[MockESP] rtcUserMemoryRead: " << length << " bytes\n";
        return true;
    }

    // Simuliert das Schreiben in den RTC-Speicher
    bool rtcUserMemoryWrite(int offset, uint32_t* data, size_t length) {
        if (offset != 0 || length > RTC_MEM_SIZE) {
            return false;
        }
        memcpy(rtcMemory, data, length);
        std::cout << "[MockESP] rtcUserMemoryWrite: " << length << " bytes\n";
        return true;
    }

    // Gibt die maximale DeepSleep-Dauer zurück (~3,5 Stunden)
    uint64_t deepSleepMax() {
        return maxDeepSleepDuration;
    }

    // Simuliert das Einschlafen in den DeepSleep-Modus
    // Diese Methode endet idealerweise mit einem Programmabbruch oder Reset in echter Hardware
    void deepSleep(uint64_t time_us, int mode) {
        std::cout << "[MockESP] deepSleep called for " << time_us << " usec with mode " << mode << "\n";
        std::cout << "[MockESP] (Simulation: Programm does not really go into sleep.)\n";
        
		// set restart reason to return from deep sleep for next test steps.
		_system_get_rst_info.reason=5;
    }
	
	rst_info _system_get_rst_info;
    std::chrono::steady_clock::time_point start_time;
};

extern MockESP ESP;

extern unsigned long millis();

extern rst_info* system_get_rst_info();

extern void delay(unsigned);

#endif
