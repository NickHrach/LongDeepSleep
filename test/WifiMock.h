#ifndef _WiFiMockClass_
#define _WiFiMockClass_

enum wl_status_t {
    WL_IDLE_STATUS = 0,
    WL_NO_SSID_AVAIL,
    WL_SCAN_COMPLETED,
    WL_CONNECTED,
    WL_CONNECT_FAILED,
    WL_CONNECTION_LOST,
    WL_DISCONNECTED
} ;

enum WiFiMode_t {
    WIFI_OFF = 0,
    WIFI_STA = 1,
    WIFI_AP = 2,
    WIFI_AP_STA = 3
};

enum RFMode {
    RF_DEFAULT = 0, // RF_CAL or not after deep-sleep wake up, depends on init data byte 108.
    RF_CAL = 1,      // RF_CAL after deep-sleep wake up, there will be large current.
    RF_NO_CAL = 2,   // no RF_CAL after deep-sleep wake up, there will only be small current.
    RF_DISABLED = 4 // disable RF after deep-sleep wake up, just like modem sleep, there will be the smallest current.
};

struct WiFiState
{
	int dummy;
};

class WiFiMock{
public:
     wl_status_t currentStatus = WL_CONNECTED;
    bool autoReconnect = true;
    bool isPersistent = true;
    WiFiMode_t currentMode = WIFI_OFF;

    void begin(const char* ssid, const char* password) {
        Serial.printf("[MOCK] WiFi.begin(%s, %s)\n", ssid, password);
        
    }

    wl_status_t status() {
        //Serial.println("[MOCK] WiFi.status()");
        return currentStatus;
    }

    void disconnect(bool wifioff = false) {
        Serial.printf("[MOCK] WiFi.disconnect(%s)\n", wifioff ? "true" : "false");
        currentStatus = WL_DISCONNECTED;
        if (wifioff) currentMode = WIFI_OFF;
    }

    bool isConnected() {
        Serial.println("[MOCK] WiFi.isConnected()");
        return currentStatus == WL_CONNECTED;
    }

    void setAutoReconnect(bool reconnect) {
        Serial.printf("[MOCK] WiFi.setAutoReconnect(%s)\n", reconnect ? "true" : "false");
        autoReconnect = reconnect;
    }

    void persistent(bool persistentSetting) {
        Serial.printf("[MOCK] WiFi.persistent(%s)\n", persistentSetting ? "true" : "false");
        isPersistent = persistentSetting;
    }

    void mode(WiFiMode_t mode) {
        Serial.printf("[MOCK] WiFi.mode(%d)\n", mode);
        currentMode = mode;
    }

    // Optional for tests
    void setStatus(wl_status_t status) {
        currentStatus = status;
    }

    WiFiMode_t getMode() const {
        return currentMode;
    }
	
    bool resumeFromShutdown(const WiFiState& state) {
        Serial.println("[MOCK] WiFi.resumeFromShutdown()");
        return true; 
    }

    // Simuliere shutdown
    void shutdown(WiFiState& state) {
        Serial.println("[MOCK] WiFi.shutdown()");
    }
	
	void forceSleepBegin() const {}
	void forceSleepWake() const {}

	int channel(){return 11;}
	const char* BSSIDstr(){return "SIMULATED WIFI";}
	const char* gatewayIP(){return "192.188.0.1";}
	const char* localIP(){return "192.168.178.15";}
	
};

extern WiFiMock WiFi;

#endif


#ifdef UNIT_TEST

#include <Arduino.h>

   
};

WiFiClass WiFi;

#endif // UNIT_TEST
