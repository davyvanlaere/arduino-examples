#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>

volatile byte watchdogInterrupts = 0;

// 5 minutes = approx. 38 periods of 8 seconds
const byte watchdogInterruptsBeforeReset = 38;

ISR(WDT_vect)
{
  watchdogInterrupts++;
}

void setup() {
  // recommended to prevent reset loops
  wdt_disable();

  // we are only interfacing components using I2C and SPI, 
  // so i guess this can be disabled all the time
  power_adc_disable(); 

  // do work
  executeJob();

  // set watchdog to interrupt and wait 8s
  configureWatchDog((1<<WDIE) | (1<<WDP3) | (1<<WDP0));

  Serial.begin(9600);
  Serial.println("setup done");
  Serial.end();
}

void loop() {
  if (watchdogInterrupts >= watchdogInterruptsBeforeReset) {
      Serial.begin(9600);
      Serial.println("resetting");
      Serial.end();
      
      // set watchdog to interrupt and wait 16ms
      configureWatchDog(1<<WDE);
  }

  // go to sleep
  sleep_enable();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_cpu();
  sleep_disable();

  Serial.begin(9600);
  Serial.print("loop ");
  Serial.println(watchdogInterrupts);
  Serial.end();
}

void configureWatchDog(byte config) {
      wdt_disable();
      WDTCSR |= (1<<WDCE) | (1<<WDE);
      WDTCSR = config;
      wdt_reset();
}

void executeJob() {
  Serial.begin(9600);
  Serial.println("perform scheduled job");
  Serial.end();
}
