/*
MIT License

Copyright (c) 2025 Marco Ratto

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
  #define DEBUG_PORT Serial
#endif

#ifdef ENABLE_DEBUG
  #if defined (ARDUINO_RASPBERRY_PI_PICO) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
    // Su Pico non c'è printf, usiamo vsnprintf
    #include <stdarg.h>
    inline void debug_printf(const char* format, ...) {
      char buffer[128];
      va_list args;
      va_start(args, format);
      vsnprintf(buffer, sizeof(buffer), format, args);
      va_end(args);
      DEBUG_PORT.print(buffer);
    }
    #define DEBUG_PRINTF(...) debug_printf(__VA_ARGS__)
  #else
    #define DEBUG_PRINTF(...) DEBUG_PORT.printf( __VA_ARGS__ )
  #endif

  #define DEBUG_PRINTLN(...) DEBUG_PORT.println( __VA_ARGS__ )
  #define DEBUG_PRINT(...) DEBUG_PORT.print( __VA_ARGS__ )

#else
  #define DEBUG_PRINTF(...)
  #define DEBUG_PRINTLN(...)
  #define DEBUG_PRINT(...)
#endif