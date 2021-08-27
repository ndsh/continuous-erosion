/**
 * Author Teemu Mäntykallio
 * Initializes the library and turns the motor in alternating directions.
*/
/*
 * nodemcu pins für continuous erosion

D0 = mechanical stop
D1 = mechanical stop
D2 = dir
D3 = step
D4 = en
D5 = CS
 */

#define RESET 0
#define NORMAL 1

int state = 0;
 
#define EN_PIN    D4  // Nano v3:   16 Mega:    38  //enable (CFG6)
#define DIR_PIN   D2  //            19          55  //direction
#define STEP_PIN  D3  //            18          54  //step
#define CS_PIN    D5  //            17          40  //chip select

constexpr uint32_t steps_per_mm = 80;
bool dir = true;

#include <TMC2130Stepper.h>
TMC2130Stepper driver = TMC2130Stepper(EN_PIN, DIR_PIN, STEP_PIN, CS_PIN);

#include <AccelStepper.h>
AccelStepper stepper = AccelStepper(stepper.DRIVER, STEP_PIN, DIR_PIN);

void setup() {
    SPI.begin();
    Serial.begin(9600);
    while(!Serial);
    Serial.println("Start...");
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    driver.begin();             // Initiate pins and registeries
    driver.rms_current(600);    // Set stepper current to 600mA. The command is the same as command TMC2130.setCurrent(600, 0.11, 0.5);
    driver.stealthChop(1);      // Enable extremely quiet stepping
    driver.stealth_autoscale(1);
    driver.microsteps(16);

    stepper.setMaxSpeed(50*steps_per_mm); // 100mm/s @ 80 steps/mm
    stepper.setAcceleration(1000*steps_per_mm); // 2000mm/s^2
    stepper.setEnablePin(EN_PIN);
    stepper.setPinsInverted(false, false, true);
    stepper.enableOutputs();
    stepper.disableOutputs();
}

void loop() {
  switch (state) {
    
   case RESET:
    // run backwards until hit one step. then count forwards until other step
    stepper.enableOutputs();
    state = NORMAL;
    
   break; 

   case NORMAL:
    
        //stepper.disableOutputs();
        
        int steps = 100*steps_per_mm;
        uint32_t ms = millis();
        static uint32_t last_time = 0;
        if ((ms - last_time) > 2000) {
          
          if (!dir) {
            Serial.println("Dir -> 0");
            steps = -5000;
            Serial.println(steps);
            stepper.setPinsInverted(true, false, true);
            
           //steps *= -1; 
          } else {
            stepper.setPinsInverted(false, false, true);
            Serial.println("Dir -> 1");
            Serial.println(steps);
          }
          dir = !dir;
          last_time = ms;
        }
        stepper.move(steps); // Move 100mm
        //stepper.enableOutputs();
    
    stepper.run();
   break;
   
  }
  /*
    if (stepper.distanceToGo() == 0) {
        stepper.disableOutputs();
        delay(100);
        stepper.move(100*steps_per_mm); // Move 100mm
        stepper.enableOutputs();
    }
    stepper.run();
  */
}
