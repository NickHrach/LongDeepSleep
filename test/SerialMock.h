#ifndef _SERIAL_MOCK_
#define _SERIAL_MOCK_

#include <iostream>
#include <cstdarg>

class SerialMock {
public:
    void begin(unsigned long) { }
    void println(const char* msg) { std::cout << "\t" << msg << std::endl; }
    void println(int val) { std::cout << "\t" << val << std::endl; }
    void println(unsigned long val) { std::cout << "\t" << val << std::endl; }
    void println() { std::cout << "\t" << std::endl; }
    void print(const char* msg) { std::cout << "\t" << msg; }
    void print(int val) { std::cout<< "\t"  << val; }
    void print(unsigned long val) { std::cout << "\t" << val; }
    void printf(const char* format, ...) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
	
/*
	void println(int64_t val)          { std::cout << "\t" << val << "\n"; }
    void println(uint64_t val)         { std::cout << "\t" << val << "\n"; }
	void print(int64_t val)            { std::cout << "\t" << val; }
    void print(uint64_t val)           { std::cout << "\t" << val; }
*/

void println(unsigned int val)     { std::cout << "\t" << val << "\n"; }
void println(long val)             { std::cout << "\t" << val << "\n"; }
void print(unsigned int val)     { std::cout << "\t" << val ; }
void print(long val)             { std::cout << "\t" << val ; }

// Für „größere“ 64-Bit-Zahlen:
void println(long long val)            { std::cout << "\t" << val << "\n"; }
void println(unsigned long long val)   { std::cout << "\t" << val << "\n"; }
void print(long long val)            { std::cout << "\t" << val; }
void print(unsigned long long val)   { std::cout << "\t" << val; }

	uint32_t baudRate(){return _baudrate;}
	
	void _setTestBaudrate(uint32_t br){_baudrate=br;}
private:
	
	uint32_t _baudrate=0;
};

extern SerialMock Serial;

#endif