

void stateMachine() {
  switch(state) {
    
    case CALIBRATE:
      //Serial.println();
      //Serial.println("· · · CALIBRATE");
      // run backward until hit one step. then count forwards until other step
      if(!leftHit) {
        buttonState1 = digitalRead(BUTTON1);
        changeDirection(false);
        //Serial.println(buttonState1);
        if(buttonState1 == 0) {
          stepper.move(steps);
          stepper.run();        
          
        } else {
          leftHit = true;
          changeDirection(true);
        }
      } else if(!rightHit) {
        buttonState2 = digitalRead(BUTTON2);
        if(buttonState2 == 0) {
          stepper.move(steps);
          stepper.run();
          
          stepCounter++;
        } else {
          rightHit = true;
          dir = false;
          changeDirection(false);
        }
      }
      if(leftHit && rightHit) {
        state = PROMISC;
        setupDone = true;
      }
      
    break;

    case PROMISC:
    
      setupDone = true;
      state = SCAN;
    break;

    case SCAN:
      //Serial.println();
      //Serial.println("· · · SCAN");
      
      wifi_set_channel(channel);
      
        //while (true) {
        nothing_new++;                          // Array is not finite, check bounds and adjust if required
        if(nothing_new > 50) {
          nothing_new = 0;
          channel++;
          //Serial.print("Channel up =>");
          //Serial.println(channel);
          if(channel == 15) break;             // Only scan channels 1 to 14
          wifi_set_channel(channel);
        }
        delay(1);  // critical processing timeslice for NONOS SDK! No delay(0) yield()
        // Press keyboard ENTER in console with NL active to repaint the screen
        if ((Serial.available() > 0) && (Serial.read() == '\n')) {
          Serial.println("\n-------------------------------------------------------------------------------------\n");
          for (int u = 0; u < clients_known_count; u++) print_client(clients_known[u]);
          for (int u = 0; u < aps_known_count; u++) print_beacon(aps_known[u]);
          Serial.println("\n-------------------------------------------------------------------------------------\n");
        }
      //}
      // hoher wert, langsamere bewegung
      buttonState1 = digitalRead(BUTTON1);
      buttonState2 = digitalRead(BUTTON2);
      if(buttonState1 == 1) {
        changeDirection(true);
      } else if(buttonState2 == 1) {
        changeDirection(false);
      }

      //stepper.disableOutputs();
      //stepper.moveTo(steps);
      //stepper.enableOutputs();

      stepper.run();

      
      if(millis() - timestamp > interval) {
        Serial.print("Known clients:");
        Serial.print("\t");
        Serial.print(clients_known_count);
        Serial.print("\t");
        Serial.print("Known APs:");
        Serial.print("\t");
        Serial.print(aps_known_count);
        Serial.print("\t");
        Serial.print(averageSignalStrength);
        Serial.print("\t");
        calcRSSI = averageSignalStrength/(clients_known_count+aps_known_count);
        Serial.println(calcRSSI);
        steps_per_mm = map(calcRSSI, 0, 80, 10000, 1);
        steps = 500*steps_per_mm;
        timestamp = millis();
        state = CENTER;
       // stepper.setSpeed(500*steps_per_mm); // 100mm/s @ 80 steps/mm
        //stepper.setAcceleration(1000*steps_per_mm); // 2000mm/s^2
        //dir = !dir;
        //changeDirection(dir);
        channel = 1;
        stepper.move(steps);
        //devices 
  ä    }
      
    break;

    case CENTER:
      //Serial.println("CENTER");
      state = SCAN;
    break;
   
      
  }
}
