#ifndef WS2812_EFFECTS_HEADER
#define WS2812_EFFECTS_HEADER

#include "ws2812-RPi.h"

Color_t Wheel(uint8_t WheelPos);
void colorWipe(Color_t c, uint8_t wait);
void rainbow(uint8_t wait);
void rainbowCycle(uint8_t wait);
void theaterChase(Color_t c, uint8_t wait);
void theaterChaseRainbow(uint8_t wait);
void rainbowCycle_r(uint8_t wait);
void rainbowCycle_wipe(uint8_t wait);
void RainFall(Color_t c,uint8_t wait,int sleepafter);
void Twinkle();
void Twinkle_Fade();
void Twinkle_T(Color_t c);

#endif
