#include <avr/sleep.h>

void RTC_init(void)
{
  /* Initialize RTC: */
  while (RTC.STATUS > 0)
  {
    ;                                   /* Wait for all register to be synchronized */
  }
  RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;    /* 32.768kHz Internal Ultra-Low-Power Oscillator (OSCULP32K) */

  RTC.PITINTCTRL = RTC_PI_bm;           /* PIT Interrupt: enabled */

  RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc /* RTC Clock Cycles 16384, resulting in 32.768kHz/16384 = 2Hz */
  | RTC_PITEN_bm;                       /* Enable PIT counter: enabled */
}

ISR(RTC_PIT_vect)
{
  RTC.PITINTFLAGS = RTC_PI_bm;          /* Clear interrupt flag by writing '1' (required) */
}

void setup() {
  RTC_init();                           /* Initialize the RTC timer */
  pinMode(2, OUTPUT);                   /* Configure pin#7 as an output */
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  /* Set sleep mode to POWER DOWN mode */
  sleep_enable();                       /* Enable sleep mode, but not going to sleep yet */
}

void loop() {
  sleep_cpu();                          /* Sleep the device and wait for an interrupt to continue */
  digitalWrite(2, CHANGE);              /* Device woke up and toggle LED on pin#7 */
}