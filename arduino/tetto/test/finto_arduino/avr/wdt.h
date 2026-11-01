// Finto <avr/wdt.h> per provare lo sketch sul PC.
#pragma once
#define WDTO_2S 7
inline void wdt_enable(int) {}
inline void wdt_reset() {}
