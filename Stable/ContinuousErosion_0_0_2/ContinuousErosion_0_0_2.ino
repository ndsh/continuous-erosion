/*
 * Continuous Erosion
 */

// todo
// [ ] RSSI Sniffing
// [ ] Two mechanical end-stops
// [ ] TMC2130
// [ ] State-machine

#include "Helper.h"
#include "StateMachine.h"

void setup() { 
  setupEnv();
}

void loop() {
  stateMachine();
}
