#ifndef _SERIAL_MOCK_
#define _SERIAL_MOCK_

#include <iostream>
#include <cstdarg>

class SerialMock {
public:
    void begin(unsigned long) { }
    void println(const char* msg) { std::cout << msg << std::endl; }
    void println(int val) { std::cout << val << std::endl; }
    void println(unsigned long val) { std::cout << val << std::endl; }
    void println() { std::cout << std::endl; }
    void print(const char* msg) { std::cout << msg; }
    void print(int val) { std::cout << val; }
    void print(unsigned long val) { std::cout << val; }
    void printf(const char* format, ...) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
	
/*
	void println(int64_t val)          { std::cout << val << "\n"; }
    void println(uint64_t val)         { std::cout << val << "\n"; }
	void print(int64_t val)            { std::cout << val; }
    void print(uint64_t val)           { std::cout << val; }
*/

void println(unsigned int val)     { std::cout << val << "\n"; }
void println(long val)             { std::cout << val << "\n"; }
void print(unsigned int val)     { std::cout << val ; }
void print(long val)             { std::cout << val ; }

// Für „größere“ 64-Bit-Zahlen:
void println(long long val)            { std::cout << val << "\n"; }
void println(unsigned long long val)   { std::cout << val << "\n"; }
void print(long long val)            { std::cout << val; }
void print(unsigned long long val)   { std::cout << val; }

	uint32_t baudRate(){return _baudrate;}
	
	void _setTestBaudrate(uint32_t br){_baudrate=br;}
private:
	
	uint32_t _baudrate=0;
};

extern SerialMock Serial;

#endif