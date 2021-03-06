//#include <NeoHWSerial.h>
#include <math.h>

#define SIZE 5
#define RINGBUFFER_SIZE 3
/*
    DECLARATION
*/

// Pin
int pwmPin = 5; // PWM output pin for motor
int dirPin = 8; // direction output pin for motor
int sensorPosPin = A2; // input pin for MR sensor

// position tracking

int updatedPos = 0;     // keeps track of the latest updated value of the MR sensor reading
int rawPos = 0;         // current raw reading from MR sensor
int lastRawPos = 0;     // last raw reading from MR sensor
int lastLastRawPos = 0; // last last raw reading from MR sensor
int flipNumber = 0;     // keeps track of the number of flips over the 180deg mark
int tempOffset = 0;
int rawDiff = 0;
int lastRawDiff = 0;
int rawOffset = 0;
int lastRawOffset = 0;
const int flipThresh = 700;  // threshold to determine whether or not a flip over the 180 degree mark occurred
boolean flipped = false;

// Kinematics
float xh = 0;         // position of the handle [m]
float xh_prev = 0;
float vh = 0;         //velocity of the handle
float v[SIZE];
float lastVh = 0;     //last velocity of the handle
float lastLastVh = 0; //last last velocity of the handle
float rp = 0.004191;   //[m]
float rs = 0.073152;   //[m]
float rh = 0.065659;   //[m]
// Force output variables
float force = 0;           // force at the handle
float Tp = 0;              // torque of the motor pulley
float duty = 0;            // duty cylce (between 0 and 255)
unsigned int output = 0;    // output command to the motor

void setup() {
  // Set up Serial communication
  Serial.begin(500000);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  setPwmFrequency(pwmPin, 1);
  // Input pins
  pinMode(sensorPosPin, INPUT); // set MR sensor pin to be an input

  // Output pins
  pinMode(pwmPin, OUTPUT);  // PWM pin for motor
  pinMode(dirPin, OUTPUT);  // dir pin for motor

  // Initialize motor
  analogWrite(pwmPin, 0);     // set to not be spinning (0/255)
  digitalWrite(dirPin, LOW);  // set direction

  // Initialize position valiables
  lastLastRawPos = analogRead(sensorPosPin);
  lastRawPos = analogRead(sensorPosPin);
}

bool readForce()
{
  int32_t forcei = 0;
  if (Serial.available() < 4)
    return false;
  //char buffer[4];
  //Serial.readBytes(buffer, 4);
  // ((char*)&forcei)[3] = buffer[3];
  // ((char*)&forcei)[2] = buffer[2];
  // ((char*)&forcei)[1] = buffer[1];
  // ((char*)&forcei)[0] = buffer[0];
  ((char*)&forcei)[3] = Serial.read();
  ((char*)&forcei)[2] = Serial.read();
  ((char*)&forcei)[1] = Serial.read();
  ((char*)&forcei)[0] = Serial.read();
  force = (float)forcei / 1000000.0;
  while (Serial.available() > 0)
    Serial.read();
  return true;
}

void communication() {
  if (readForce()) {
    //int32_t forcei = force;// * 1000000;
    int32_t xhi = xh * 1000000;
    Serial.write((const char*)&xhi, 4);
    Serial.flush();
  }
}

void readPosCount() {
  // Get voltage output by MR sensor
  rawPos = analogRead(sensorPosPin);  //current raw position from MR sensor
  // Calculate differences between subsequent MR sensor readings
  rawDiff = rawPos - lastRawPos;          //difference btwn current raw position and last raw position
  lastRawDiff = rawPos - lastLastRawPos;  //difference btwn current raw position and last last raw position
  rawOffset = abs(rawDiff);
  lastRawOffset = abs(lastRawDiff);
  
  // Update position record-keeping vairables
  lastLastRawPos = lastRawPos;
  lastRawPos = rawPos;

  // Keep track of flips over 180 degrees
  if ((lastRawOffset > flipThresh) && (!flipped)) { // enter this anytime the last offset is greater than the flip threshold AND it has not just flipped
    if (lastRawDiff > 0) {       // check to see which direction the drive wheel was turning
      flipNumber--;              // cw rotation
    } else {                     // if(rawDiff < 0)
      flipNumber++;              // ccw rotation
    }
    if (rawOffset > flipThresh) { // check to see if the data was good and the most current offset is above the threshold
      updatedPos = rawPos + flipNumber * rawOffset; // update the pos value to account for flips over 180deg using the most current offset
      tempOffset = rawOffset;
    } else {                     // in this case there was a blip in the data and we want to use lastactualOffset instead
      updatedPos = rawPos + flipNumber * lastRawOffset; // update the pos value to account for any flips over 180deg using the LAST offset
      tempOffset = lastRawOffset;
    }
    flipped = true;            // set boolean so that the next time through the loop won't trigger a flip
  } else {                        // anytime no flip has occurred
    updatedPos = rawPos + flipNumber * tempOffset; // need to update pos based on what most recent offset is
    flipped = false;
  }
}

void calPosMeter()
{
 // Add the function displacement & speed calculation
  float rh = 60;   //[mm]
  //float ts = 0.01176 * updatedPos - 5.01933; // Device 1
  float ts = 0.01183 * updatedPos - 8.29819; // Device 4
  xh_prev = xh;
  xh = rh * (ts * 3.14159 / 180); // Compute the position of the handle based on ts
  vh = (xh - xh_prev) / .0001;
}

void motorControl()
{
  Tp = rp / rs * rh * force;  // Compute the require motor pulley torque (Tp) to generate that force
  // Determine correct direction for motor torque
  if (force < 0) {
    digitalWrite(dirPin, HIGH);
  } else {
    digitalWrite(dirPin, LOW);
  }

  // Compute the duty cycle required to generate Tp (torque at the motor pulley)
  duty = sqrt(abs(Tp) / 0.03);

  // Make sure the duty cycle is between 0 and 100%
  if (duty > 1) {
    duty = 1;
  } else if (duty < 0) {
    duty = 0;
  }
  output = (int)(duty * 255);  // convert duty cycle to output signal
  analogWrite(pwmPin, output); // output the signal
}

void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if (pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch (divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if (pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if (pin == 3 || pin == 11) {
    switch (divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}

void loop() {
  readPosCount();
  calPosMeter();
  communication();
  motorControl();
  // delay before next reading:
  delay(1);
}
