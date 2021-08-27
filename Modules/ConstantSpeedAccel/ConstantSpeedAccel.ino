// ConstantSpeed.pde
// -*- mode: C++ -*-
//
// Shows how to run AccelStepper in the simplest,
// fixed speed mode with no accelerations
/// \author  Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2009 Mike McCauley
// $Id: ConstantSpeed.pde,v 1.1 2011/01/05 01:51:01 mikem Exp mikem $

#include <AccelStepper.h>

AccelStepper stepper(1, D3, D2); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5

void setup()
{ 
  Serial.begin(9600);
  Serial.println("hej");
  stepper.setEnablePin(D4);
   stepper.setMaxSpeed(1000);
   stepper.setSpeed(100);	
}

void loop()
{  
  Serial.println("loop");
   stepper.runSpeed();
}
