#ifndef button_h
#define button_h

#define BUTTON_PIN 10  // Button connected to digital pin 10
#define BUTTON_MIN_REPRESS 100UL // x msecs acceptable between 2 button presses
#define BUTTON_DEBOUNCE_DELAY 10UL // x msecs to guard against button debounce
#define BUTTON_SAMPLE_NUM 4

void ReadButton();

#endif
