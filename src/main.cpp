#include <Arduino.h>
// Include the AccelStepper Library
#include <AccelStepper.h>

// define step constants
#define FULLSTEP 4
#define STEP_PER_REVOLUTION 2048 // this value is from datasheet
#define RUNSPEED 50
#define INITSPEED 150
#define ACCELL 500

#define OFFSET1 -25

// #define SENSOR_HIGH 300
#define SENSOR_HIGH 350
// #define SENSOR_MID 300
#define SENSOR_LOW 240

#define LOOPING true

// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
AccelStepper stepper0(FULLSTEP, 14, 23, 15, 22);
AccelStepper stepper1(FULLSTEP, 9, 11, 10, 12);
AccelStepper stepper2(FULLSTEP, 8, 6, 7, 5);
AccelStepper stepper3(FULLSTEP, 4, 2, 3, 17);

// put function declarations here:
void doRotation(AccelStepper &stepper, float rotations); 
void initStepper(AccelStepper &stepper);
void alignBySensor(AccelStepper &stepper, int sensorPin);
void alignBySwitch();
void testSensor();
int getAverage (int (&lastTen)[10], int sensorValue);
bool stageComplete();
void initStage(int stageNo);
int currentStage;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // analogReference(EXTERNAL);

  initStepper(stepper0);
  initStepper(stepper1);
  initStepper(stepper2);
  initStepper(stepper3);

  alignBySensor(stepper0, A3);
  alignBySensor(stepper1, A0);
  alignBySensor(stepper2, A1);
  alignBySensor(stepper3, A2);
 
  delay(1000);
  // stepper0.setMaxSpeed(RUNSPEED);
 
  initStage(0);
}


void loop() {
  // put your main code here, to run repeatedly:
  if (stageComplete() && currentStage != -1) {
    initStage(currentStage + 1);
  }

  stepper0.run();   
  stepper1.run();   
  stepper2.run();   
  stepper3.run();   
  // Serial.write(digitalRead(A0));
  // int sensorValue = analogRead(A0);
  // Serial.println(sensorValue);
}

// put function definitions here:

void doRotation(AccelStepper &stepper, float rotations){
  // Rotate
  stepper.moveTo(stepper.currentPosition() + static_cast<int>(STEP_PER_REVOLUTION * rotations)); 
}

void initStepper(AccelStepper &stepper) {
  stepper.setSpeed(RUNSPEED);
  stepper.setMaxSpeed(RUNSPEED);
  stepper.setAcceleration(ACCELL); // set acceleration
  stepper.setCurrentPosition(0); // set position
}

void alignBySensor(AccelStepper &stepper, int sensorPin) {
  int alignMode = 0;  
  int lowStart = 0;
  int lowEnd = 0;
  int midpoint = 0;
  int sensorValue = 0;
  int sensorAverage = 0;
  int lastTen[10] = {};

  // stepper.moveTo(STEP_PER_REVOLUTION * 2);
  stepper.setSpeed(INITSPEED);
  stepper.setMaxSpeed(INITSPEED);
  doRotation(stepper, 2);
  
  // wait until low
  while (alignMode == 0) {
    sensorValue = analogRead(sensorPin);
    // Serial.println(alignMode);
    Serial.println(sensorValue);
    if (sensorValue < SENSOR_LOW) {
      alignMode = 1;  
    }
    stepper.run();
    sensorAverage = getAverage(lastTen, sensorValue);
  }


  // find start of high
  while (alignMode == 1) {
    sensorValue = analogRead(sensorPin);
    if (sensorValue == 0) {  // skip
      Serial.println("No sensor value");
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

  Serial.print("lowStart = ");
  Serial.print(lowStart);
  Serial.println("");
  Serial.print("lowEnd = ");
  Serial.print(lowEnd);
  Serial.println("");
  Serial.print("midpoint = ");
  Serial.print(midpoint);
  Serial.println("");

  stepper.setSpeed(RUNSPEED);
  stepper.setMaxSpeed(RUNSPEED);
  // stepper.moveTo(midpoint + OFFSET1);
  stepper.moveTo(midpoint);
  // stepper.moveTo(maxPosition);
  stepper.runToPosition();
  stepper.setCurrentPosition(0);
  
  
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

bool stageComplete() {

  return (
    stepper0.distanceToGo() == 0 &&
    stepper1.distanceToGo() == 0 &&
    stepper2.distanceToGo() == 0 &&
    stepper3.distanceToGo() == 0
  );
}

void initStage(int stageNo) {

  currentStage = stageNo;
  Serial.print("Stage ");
  Serial.print(stageNo);
  Serial.println("");

  switch(stageNo) {
    case 0:
      doRotation(stepper0, 1);
      doRotation(stepper1, 1);
      doRotation(stepper2, 1);
      doRotation(stepper3, 1);
      break;

    case 1:
      delay(1000);
      doRotation(stepper1, -5.0/12.0);
      doRotation(stepper2, 5.0/12.0);
      break;

    case 2:
      delay(1000);
      doRotation(stepper0, -5.0/12.0);
      break;
    
    case 3:
      delay(1000);
      doRotation(stepper0, 2);
      doRotation(stepper1, 2);
      doRotation(stepper2, 2);
      doRotation(stepper3, 2);
      break;

     case 4:
      delay(1000);
      doRotation(stepper3, 2.0/12);
      break;

    case 5:
      doRotation(stepper0, -1.0/12);
      doRotation(stepper1, -1.0/12);
      doRotation(stepper2, 1.0/12);
      doRotation(stepper3, 4.0/12);
      // stop to measure:
      // currentStage = -1;
      break;

    case 6:
      delay(1000);
      doRotation(stepper0, 1.0/12);
      doRotation(stepper2, 1.0/12);
      break;

    case 7:
      delay(1000);
      doRotation(stepper0, (1.0 + 11.0/12) );
      doRotation(stepper1, -2);
      doRotation(stepper2, (1.0 + 11.0/12));
      doRotation(stepper3, -2);
      break;
    
    case 8:
      delay(1000);
      doRotation(stepper0, 0.5 );
      doRotation(stepper2, 0.5);
      break;

    case 9:
      delay(1000);
      doRotation(stepper1, 0.5 );
      doRotation(stepper3, 0.5);
      break;

    case 10:
      if (LOOPING) {
        delay(10000);
        initStage(0);
      } else {
        currentStage = -1;
      }
      break;

    default:
      currentStage = -1;


  }
}
