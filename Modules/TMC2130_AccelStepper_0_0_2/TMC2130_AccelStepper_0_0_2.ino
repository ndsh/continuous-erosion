/*
 * nodemcu pins für continuous erosion
 * 
 * Continuous Erosion

  D0 = mechanical stop
  D1 = mechanical stop
  D2 = dir
  D3 = step
  D4 = en
  D5 = CS
 */

#include <TMC2130Stepper.h>

#define RESET 0
#define CALIBRATION 1
#define GOCENTER 2
#define GRIND 3
#define DIRECTION 4
#define PAUSE 5
 
#define EN_PIN    D4  // Nano v3:   16 Mega:    38  //enable (CFG6)
#define DIR_PIN   D2  //            19          55  //direction
#define STEP_PIN  D3  //            18          54  //step
#define CS_PIN    D5  //            17          40  //chip select
#define BUTTON1   D1  // linker stopp
#define BUTTON2   D0  // rechter stopp

unsigned long lastMillis;
unsigned int state;
unsigned long interval;

unsigned long grindTime = 120000;
unsigned long pauseTime = 30000;

constexpr uint32_t steps_per_mm = 80;
bool dir = true;
int stepCounter = 0;
int steps = 0;


TMC2130Stepper driver = TMC2130Stepper(EN_PIN, DIR_PIN, STEP_PIN, CS_PIN);

#include <AccelStepper.h>
AccelStepper stepper = AccelStepper(stepper.DRIVER, STEP_PIN, DIR_PIN);

int buttonState1 = 0;
int buttonState2 = 0; 
bool activeButton = false;

void setup() {
    pinMode(BUTTON1, INPUT);
    pinMode(BUTTON2, INPUT);
    
    SPI.begin();
    Serial.begin(9600);
    while(!Serial);
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

    lastMillis = 0;
    state = RESET;
    interval = 1000;

    steps = 100*steps_per_mm;

    // Show welcome
    Serial.println(":: Continuous Erosion 0.0.2 ::");
    Serial.println(""); Serial.println(""); Serial.println("");
    Serial.println(":: RESETTING DEVICE");
    changeDirection(false);
}

void loop() {
  
  switch (state) {  
    case RESET:
      // run backward until hit one step. then count forwards until other step
      stepper.enableOutputs();
      buttonState1 = digitalRead(BUTTON1);
      if(buttonState1 == 0) {
        stepper.move(steps); // Move 100mm 
        stepper.run();
      } else {
        Serial.println(":: CALIBRATING");
        changeDirection(true);
        state = CALIBRATION;
      }
    break; 
  
    case CALIBRATION:
      // run forward until hit 2nd stop
      // count "steps"
      buttonState2 = digitalRead(BUTTON2);
      if(buttonState2 == 0 && !activeButton) {
        stepper.move(steps); // Move 100mm 
        stepper.run();
        stepCounter++;
      } else {
        activeButton = true;
        Serial.println(":: LETS GRIND");
        state = GRIND;
        dir = false;
        changeDirection(false);

        
      }
    break;

    case DIRECTION:
      if(buttonState1 == 1) {
        dir = true;
        changeDirection(true);
        /*
        for(int i = 0; i < 50; i++) {
          stepper.move(steps); // Move 100mm 
          stepper.run();
        }
        */
        
      } else if(buttonState2 == 1) {
        dir = false;
        changeDirection(false);
        /*
        for(int i = 0; i < 50; i++) {
          stepper.move(steps); // Move 100mm 
          stepper.run();
        }
        */
      }
      delay(100);
      Serial.println(":: LETS GRIND");
      state = GRIND;
    break;
    
    case GRIND:
      // go backwards until hit stop
      // go forwards until hit stop
      
      //stepper.disableOutputs();

      buttonState1 = digitalRead(BUTTON1); // links
      buttonState2 = digitalRead(BUTTON2); // rechts
      
      // buttons debouncen?
      if(buttonState1 == 0 && buttonState2 == 0) {
        stepper.move(steps); // Move 100mm      
        stepper.run();
      } else {
        Serial.println(":: CHANGE DIRECTION");
        state = DIRECTION;
      }

      if(millis() - lastMillis < grindTime) return;
      lastMillis = millis();
      state = PAUSE;
      /*
      int steps = 100*steps_per_mm;
      uint32_t ms = millis();
      static uint32_t last_time = 0;
      if ((ms - last_time) > 2000) {
        
        if (!dir) {
          Serial.println("Dir -> 0");
          steps = -5000;
          Serial.println(steps);
          changeDirection(false);
          //stepper.setPinsInverted(true, false, true);
          
         //steps *= -1; 
        } else {
          changeDirection(true);
          //stepper.setPinsInverted(false, false, true);
          Serial.println("Dir -> 1");
          Serial.println(steps);
        }
        dir = !dir;
        last_time = ms;
      }
      */
      

      //stepper.enableOutputs();
     break;

     case PAUSE:
      if(millis() - lastMillis < pauseTime) return;
      lastMillis = millis();
      state = GRIND;
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

void changeDirection(bool b) {
  if(b) stepper.setPinsInverted(false, false, true);
  else stepper.setPinsInverted(true, false, true);
}
