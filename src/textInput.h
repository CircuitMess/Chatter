#ifndef textInput_h
#define textInput_h


#define map_width 7
#define RESET_MTP '~'

#include <Arduino.h>
#include <Buttons.h>

extern uint32_t textPointer;
extern boolean btnHeld;
extern boolean btnHeldField[10];
extern bool textLimitFlag;
extern unsigned long prevKeyMillis;
extern Buttons buttons;

String textInput(String buffer, int16_t length);
int multi_tap(byte key);

#endif