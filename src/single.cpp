#include <Arduino.h>
// Include the AccelStepper Library
#include <AccelStepper.h>

// define step constants
#define FULLSTEP 4
#define STEP_PER_REVOLUTION 2048 // this value is from datasheet
#define RUNSPEED 70
#define INITSPEED 150
#define ACCELL 500

#define OFFSET1 -25

#define SENSOR_HIGH 400
#define SENSOR_MID 350
#define SENSOR_LOW 275

// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
AccelStepper stepper0(FULLSTEP, 5, 3, 4, 2);

// put function declarations here:
void doRotation(AccelStepper &stepper, float rotations); 
void initStepper(AccelStepper &stepper);
void alignBySensor(AccelStepper &stepper, int sensorPin);
void alignBySwitch();
void testSensor();
int getAverage (int (&lastTen)[10], int sensorValue);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  initStepper(stepper0);
  alignBySensor(stepper0, A0);
  delay(1000);
  stepper0.setMaxSpeed(RUNSPEED);
  doRotation(stepper0, 1);
}


void loop() {
  // put your main code here, to run repeatedly:
  stepper0.run();   
  // Serial.write(digitalRead(A0));
  // int sensorValue = analogRead(A0);
  // Serial.println(sensorValue);
}

// put function definitions here:

void doRotation(AccelStepper &stepper, float rotations){
  // Rotate
  stepper.moveTo(stepper.currentPosition() + STEP_PER_REVOLUTION * rotations); 
}

void initStepper(AccelStepper &stepper) {
  stepper.setMaxSpeed(INITSPEED);   // set the maximum speed
  stepper.setAcceleration(ACCELL); // set acceleration
  stepper.setSpeed(INITSPEED);         // set initial speed
  stepper.setCurrentPosition(0); // set position
}

void alignBySwitch() {
  
  pinMode(A0, INPUT);

  Serial.write(digitalRead(A0));
  if (digitalRead(A0) == LOW) {
    Serial.write("LOW");
  } 
  if (digitalRead(A0) == HIGH) {
    Serial.write("HIGH");
  }
  
  initStepper(stepper0);
  doRotation(stepper0, 3);

  while (digitalRead(A0) == HIGH) {
    stepper0.run();
  }
  while (digitalRead(A0) == LOW) {
    stepper0.run();
  }
  stepper0.move(OFFSET1);
  stepper0.setMaxSpeed(RUNSPEED);
  stepper0.runToPosition();
  stepper0.setCurrentPosition(0);
  stepper0.moveTo(0);
  
  
}

void alignBySensor(AccelStepper &stepper, int sensorPin) {
  int alignMode = 0;  
  int lowStart = 0;
  int lowEnd = 0;
  int midpoint = 0;
  int sensorValue = 0;
  int sensorAverage = 0;
  int sensorMax = 0;
  int maxPosition = 0;
  int lastTen[10] = {};

  stepper.moveTo(STEP_PER_REVOLUTION * 2);
  // wait until low
  while (alignMode == 0) {
    sensorValue = analogRead(sensorPin);
    Serial.println(alignMode);
    Serial.println(sensorValue);
    if (sensorValue < SENSOR_LOW) {
      alignMode = 1;  
    }
    stepper.run();
  }

  // // find max point
  // while (alignMode == 1) {
  //   sensorValue = analogRead(sensorPin);
  //   Serial.println(sensorValue);
  //   if (sensorValue == 0) {  // skip
  //     return;
  //   }
  //   if (sensorValue > sensorMax) {
  //     sensorMax = sensorValue;
  //     maxPosition = stepper.currentPosition();
  //   } else if (sensorMax > SENSOR_MID && sensorValue < SENSOR_LOW) {
  //     alignMode = -1; //end
  //   }
  //   stepper.run();
  
  // }


  // find start of high
  while (alignMode == 1) {
    sensorValue = analogRead(sensorPin);
    if (sensorValue == 0) {  // skip
      return;
    }
    sensorAverage = getAverage(lastTen, sensorValue);
    Serial.println(sensorAverage);
    
    if (sensorAverage > SENSOR_HIGH) {
      lowStart = stepper.currentPosition() - 10;
      alignMode = 2;  
    }
    stepper.run();
  
  }

  // find end of high
  while (alignMode == 2) {
    sensorValue = analogRead(sensorPin);
    sensorAverage = getAverage(lastTen, sensorValue);
    Serial.println(sensorAverage);
    if (sensorAverage < SENSOR_HIGH) {
      lowEnd = stepper.currentPosition() - 10;
      alignMode = -1;  // end
    }
    stepper.run();
  
  }

  // wraps around 0
  if (lowEnd - lowStart < 0) {  
    midpoint = (lowStart + lowEnd + STEP_PER_REVOLUTION)/2 % STEP_PER_REVOLUTION;
  } else {
    midpoint = (lowStart + lowEnd)/2;
  }

  stepper0.setSpeed(RUNSPEED);
  stepper0.setMaxSpeed(RUNSPEED);
  // stepper.moveTo(midpoint + OFFSET1);
  stepper.moveTo(midpoint);
  // stepper.moveTo(maxPosition);
  stepper.runToPosition();
  stepper.setCurrentPosition(0);
  
  Serial.println(sensorMax);
  Serial.println("Alignment complete");
  
}

void testSensor() {

}

int getAverage (int (&lastTen)[10], int sensorValue) {
  int total = sensorValue;
  for (int i = 0; i < 9; i++) {
    total += lastTen[i];
    lastTen[i] = lastTen[i+1];
  }
  lastTen[9] = sensorValue;
  int average = total / 10;
  return average;

}