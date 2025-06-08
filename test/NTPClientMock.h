#ifndef _NTP_CLIENT_MOCK_
#define _NTP_CLIENT_MOCK_

#include <cstdio>
#include <cstring>
#include <ctime>

class MockNTPClient {
private:
    uint64_t currentEpoch = 0;  // interne Zeit im Test

public:
    // Setzt die Zeit für Tests
    void setMockTime(uint64_t epoch) {
        currentEpoch = epoch;
    }

    // Gibt die aktuell gesetzte Zeit zurück
    unsigned long getEpochTime() {
        return (unsigned long)currentEpoch;
    }

    // Formatierte Zeit als String (z.B. "HH:MM:SS" oder "YYYY-MM-DD HH:MM:SS")
    // Für simplicity hier rudimentär umgesetzt
    const char* time2string(uint64_t epoch, char* buffer, bool withDate) {
        time_t t = (time_t)epoch;
        struct tm* tm_info = gmtime(&t);
        if (withDate) {
            strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", tm_info);
        } else {
            strftime(buffer, 30, "%H:%M:%S", tm_info);
        }
        return buffer;
    }

    // Startet die Zeit-Synchronisierung (Mock: macht nichts)
    void begin() {}

    // Erzwingt ein Update der Zeit (Mock: immer erfolgreich)
    bool forceUpdate() {
        // Simuliere, dass Zeit immer verfügbar ist
        return true;
    }

    // Formatiert aktuelle Zeit (ähnlich wie time2string, für Debug)
    void getFormattedDateTime(char* buffer, bool withDateTime) {
        time2string(currentEpoch, buffer, withDateTime);
    }
};

typedef MockNTPClient NTPClient;
#endif