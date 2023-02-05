#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>

// work can be executed in the workLoop() method, as long as jobExecuted = false
bool jobExecutionDone = false;
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

  // disable anything we dont need for the job ...
  // we are only interfacing components using I2C and SPI, 
  // so i guess this can be disabled all the time
  power_adc_disable(); 

  Serial.begin(9600);
  Serial.println("setup done");
  Serial.end();
}

void loop() {
  if (jobExecutionDone) {
    sleepUntilInterrupt();
    if (watchdogInterrupts >= watchdogInterruptsBeforeReset) {
      // set watchdog to interrupt and wait 16ms
      // WDE is the flag that needs to be set to indicate that we want to reset, 
      // since we set no other bits the wait time is the minumum of 16ms.
      configureWatchDog(1<<WDE);
    }
  }
  else {
      jobExecutionDone = workLoop();
      if (jobExecutionDone) {
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

bool workLoop() {
  if (millis() > 5000) {
      Serial.begin(9600);
      Serial.println("job done");
      Serial.end();
    // wait for 5 seconds, then return true to start sleep
    return true;
  }
  return false;
}

void configureWatchDog(byte config) {
      wdt_disable();
      WDTCSR |= (1<<WDCE) | (1<<WDE);
      WDTCSR = config;
      wdt_reset();
}

void sleepUntilInterrupt() {
  // configure sleep
  sleep_enable();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  // disable everything to consume power
  power_all_disable(); 

  // sleep
  sleep_cpu(); // this call will block, until the ISR method is called.
  sleep_disable(); // not sure if this needs to be called?
}

