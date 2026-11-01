// Finto "Arduino.h": quel tanto che basta per eseguire tetto.ino sul PC.
#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <deque>
#include <cstdio>

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define A0 14
#define A1 15
#define A2 16
#define A3 17
#define A4 18
#define A5 19
#define PROGMEM
#define PGM_P const char*
#define pgm_read_word(indirizzo) (*(indirizzo))
#define strncpy_P strncpy
class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper*>(s))

namespace finto {
  extern unsigned long ora_ms;
  extern int livello[20];          // livello logico letto sui pin di ingresso
  extern int uscita[20];           // livello scritto sui pin di uscita
  void quando_scritto(uint8_t pin, int valore);   // implementata dal simulatore
  void durante_attesa(unsigned long ms);          // implementata dal simulatore
}

inline unsigned long millis() { return finto::ora_ms; }
inline void delay(unsigned long ms) { finto::durante_attesa(ms); }
inline void pinMode(uint8_t, uint8_t) {}
inline int digitalRead(uint8_t pin) { return finto::livello[pin]; }
inline void digitalWrite(uint8_t pin, int v) { finto::uscita[pin] = v; finto::quando_scritto(pin, v); }

class FintaSeriale {
 public:
  std::deque<char> in;
  std::string out;
  void begin(long) {}
  int available() { return (int)in.size(); }
  int read() { if (in.empty()) return -1; char c = in.front(); in.pop_front(); return (unsigned char)c; }
  void print(const char* s) { out += s; }
  void print(char c) { out += c; }
  void print(const __FlashStringHelper* s) { out += reinterpret_cast<const char*>(s); }
  void println(char c) { out += c; out += "\r\n"; }
  void println(const char* s) { out += s; out += "\r\n"; }
  operator bool() { return true; }
};
extern FintaSeriale Serial;
