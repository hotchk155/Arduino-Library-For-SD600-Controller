#include "sd600.h"

#define NUM_LEDS 50
sd600 strip(NUM_LEDS);
void setup() {
  strip.begin();
}
void loop() {  
  for(int pos = 0; pos < NUM_LEDS; ++pos)
  {
    strip.set(pos,RGB(254,0,0));
    strip.refresh();
    delay(10);
  }
  for(int pos = 0; pos < NUM_LEDS; ++pos)
  {
    strip.set(pos,RGB(0,0,0));
    strip.refresh();
    delay(10);
  }
}