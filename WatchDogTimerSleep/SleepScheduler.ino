#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>

int requestedSleepIterations = 0;
volatile byte watchdogInterrupts = 0;

ISR(WDT_vect)
{
  watchdogInterrupts++;
}

void setupSleepScheduler() {
    // recommended to prevent reset loops
  wdt_disable();
}

void runSleepScheduler(int (*handler)()) {
  if (requestedSleepIterations > 0) {
    sleepUntilInterrupt();
    if (watchdogInterrupts >= requestedSleepIterations) {
      // set watchdog to interrupt and wait 16ms
      // WDE is the flag that needs to be set to indicate that we want to reset, 
      // since we set no other bits the wait time is the minumum of 16ms.
      //configureWatchDog(1<<WDE);
      wdt_disable();
      power_all_enable(); 
      requestedSleepIterations = 0;
      watchdogInterrupts = 0;
    }
  }
  else {
      requestedSleepIterations = handler();
      if (requestedSleepIterations > 0) {

        // disable everything to consume power
        power_all_disable(); 

        // Configure watchdog to wait 8s, and then interrupt.
        // After those 8 seconds, the watchdog will call the ISR interrupt method, 
        // which will increment the watchdogInterrupts counter. As long as this value 
        // is lower than watchdogInterruptsBeforeReset, we will keep putting the arduino to sleep.
        // Once watchdogInterrupts >= watchdogInterruptsBeforeReset, we will reconfigure the watchdog to reset after 16ms.
        // WDIE is the flag that needs to be set to indicate that we want an interrupt, WDP3 and WDP0 indicate the 8 seconds interval
        configureWatchDog((1<<WDIE) | (1<<WDP3) | (1<<WDP0));
      }
  }
}

void configureWatchDog(byte config) {
      /*
        Time-out (ms)
        WDP3/WDP2/WDP1/WDP0
        0000 16
        0001 32
        0010 64
        0011 125
        0100 250
        0101 500
        0110 1000
        0111 2000
        1000 4000
        1001 8000
      */
      wdt_disable();
      WDTCSR |= (1<<WDCE) | (1<<WDE);
      WDTCSR = config;
      wdt_reset();
}

void sleepUntilInterrupt() {
  // configure sleep
  sleep_enable();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  // sleep
  sleep_cpu(); // this call will block, until the ISR method is called.
  sleep_disable(); // not sure if this needs to be called?
}
