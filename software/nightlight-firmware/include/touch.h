#ifndef TOUCH_H
#define TOUCH_H

#include <Arduino.h>
#include <avr/io.h>  // Needed for register definitions

#define TOUCH_PIN_INDEX 3  // Adjust to your touch pin

void touch_init();
uint16_t touch_read();

#endif // TOUCH_H
