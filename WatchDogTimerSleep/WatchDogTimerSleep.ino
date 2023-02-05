#include <avr/wdt.h>

void setup() {
  setupSleepScheduler();

  Serial.begin(9600);
  Serial.println("setup done");
  Serial.end();
}

void loop() {
  runSleepScheduler(workLoop);
}

int workLoop() {
  Serial.begin(9600);
  Serial.println("start job");
  delay(2000);
  Serial.println("end job");
  Serial.end();
  return 1;
}
