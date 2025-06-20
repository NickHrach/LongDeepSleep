/* USAGE:
#define _DEBUG_ 1    // SET TO 0 OUT TO REMOVE TRACES
#include <SwitchabelSerial.h>
*/

#if _DEBUG_
#define D_init(...) 	   do {if (Serial.baudRate()==0) Serial.begin(__VA_ARGS__);} while (0)
#define D_print(...)       Serial.print(__VA_ARGS__)
#define D_printf(...)      Serial.printf(__VA_ARGS__)
#define D_write(...)       Serial.write(__VA_ARGS__)
#define D_println(...)     Serial.println(__VA_ARGS__)
#define D_flush()          Serial.flush()
#else
#define D_SerialBegin(...)
#define D_print(...)
#define D_printf(...)
#define D_write(...)
#define D_println(...)
#define D_flush()
#endif