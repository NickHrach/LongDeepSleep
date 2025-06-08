
#include "MockTestSetup.h"



/**
 * Berechnet das Delta in ms zwischen dem jetztigen Zeitpunkt und dem in init_moc() gespeicherten start_time.
 */
unsigned long millis() {
    auto now = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - ESP.start_time).count();
	// let's asume we are 20 times faster on test system than on target system
    return static_cast<unsigned long>(diff)*20;
}

MockESP ESP;


rst_info* system_get_rst_info(){return &ESP._system_get_rst_info;}

void delay(unsigned){}

WiFiMock WiFi;

SerialMock Serial;