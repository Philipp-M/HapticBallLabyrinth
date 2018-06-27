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
double xh = 0;         // position of the handle [m]
double xh_prev = 0;
double vh = 0;         //velocity of the handle
double v[SIZE];
double lastVh = 0;     //last velocity of the handle
double lastLastVh = 0; //last last velocity of the handle
double rp = 0.004191;   //[m]
double rs = 0.073152;   //[m]
double rh = 0.065659;   //[m]
// Force output variables
double force = 0;           // force at the handle
double Tp = 0;              // torque of the motor pulley
double duty = 0;            // duty cylce (between 0 and 255)
unsigned int output = 0;    // output command to the motor

bool inSurface = false;
bool commandCompleted = false;
double timeOfImpact = 0;


// Communication
char buffer [32];
double ringbuffer[RINGBUFFER_SIZE];
int curIdxRing = 0;
String buffer2;
char curCnt = 0;

/*
      Setup function - this function run once when reset button is pressed.
*/
double sgn(double x) {
  return x > 0 ? 1 : x < 0 ? -1 : 0;
}

static void handleRxChar( uint8_t c )
{
  if (!commandCompleted) {
    if (curCnt < sizeof(buffer))
      buffer[curCnt++] = c;
    if (c == '\n') {
      buffer[sizeof(buffer) - 2] = '\n';
      buffer[sizeof(buffer) - 1] = '\0';
      commandCompleted = true;
    }
  }
}
double removeOutlier(double* xs) {
  return 0.7;
  double sum = 0;
  int outlierIdx1 = 0;
  int outlierIdx2 = 0;
  double maxDif = 0;
  for(int i = 0; i < RINGBUFFER_SIZE; i++) {
    sum += xs[i];
  }
  sum /= RINGBUFFER_SIZE;
  for (int i = 0; i < RINGBUFFER_SIZE; i++) {
    if (maxDif < abs(sum - xs[i])) {
      maxDif = abs(sum - xs[i]);
      outlierIdx2 = outlierIdx1;
      outlierIdx1 = i;
    }
  }
  for (int i = 0; i < RINGBUFFER_SIZE; i++) {
    if (i != outlierIdx1 && i != outlierIdx2)
      return xs[i];
  }
  return xs[0];
}
void setup() {
  // Set up Serial communication
  //Serial.attachInterrupt( handleRxChar );
  Serial.begin(500000);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Set PWM frequency
  setPwmFrequency(pwmPin, 1);
  // Input pins
  pinMode(sensorPosPin, INPUT); // set MR sensor pin to be an input

  // Output pins
  pinMode(pwmPin, OUTPUT);  // PWM pin for motor
  pinMode(dirPin, OUTPUT);  // dir pin for motor

  // Initialize motor
  analogWrite(pwmPin, 0);     // set to not be spinning (0/255)
  digitalWrite(dirPin, LOW);  // set direction

  ringbuffer[0] = 0.0;
  ringbuffer[1] = 0.0;
  ringbuffer[2] = 0.0;
  // Initialize position valiables
  lastLastRawPos = analogRead(sensorPosPin);
  lastRawPos = analogRead(sensorPosPin);
}

bool readLine()
{
  
  int cnt = 0;
  int bytesAvailable = 0;
  if (Serial.available() == 0)
    return false;
  // short wait to let the Serial transfer complete (should take max 0.5 ms)
  delay(5);
  while (Serial.available() > 0)
  {
    char c = Serial.read();
    if (cnt < sizeof(buffer)) {
      buffer[cnt++] = c;
      if ((c == '\n') || (cnt == sizeof(buffer)-1))
        buffer[cnt] = '\0';
    }
  }
  return true;
}

bool readLineNew()
{
  if (Serial.available() == 0)
    return false;
  // short wait to let the Serial transfer complete (should take max 0.5 ms)
  delay(5);
  buffer2 = Serial.readString();
  return true;
}
bool readForce()
{
  uint32_t forcei = 0;
  if (Serial.available() < 4)
    return false;
  // short wait to let the Serial transfer complete (should take max 0.5 ms)
  //delay(5);
  Serial.readBytes((char*)&forcei, 4);
//  ((char*)&forcei)[0] = Serial.read();
//  ((char*)&forcei)[1] = Serial.read();
//  ((char*)&forcei)[2] = Serial.read();
//  ((char*)&forcei)[3] = Serial.read();
  //while (Serial.available() > 0) Serial.read();
  //force = max(-1.0, min(forcei / 1000000.0, 1.0));
  double f = forcei / 1000000.0;
  ringbuffer[curIdxRing++] = forcei / 1000000.0;
  if (curIdxRing >= RINGBUFFER_SIZE)
    curIdxRing = 0;
  force = removeOutlier(ringbuffer);
  return true;
}

//void communication() {
//  if (commandCompleted) {
//    switch(buffer[0]) {
//      case 'p':
//        Serial.println(xh);
//        Serial.flush();
//        break;
//      case 'f':
//        force = atof(buffer + 1);
//        Serial.println(force); // "synchronous" communication...
//        Serial.flush();
//        break;
//      case 'c':
//        force = atof(buffer + 1);
//        Serial.println(xh); // "synchronous" communication...
//        Serial.flush();
//        break;
//      default:
//        buffer[sizeof(buffer) - 2] = '\n';
//        Serial.println(buffer); // "synchronous" communication...
//        Serial.flush();
//        break;
//    }
//  }
//  commandCompleted = false;
//}
//void communication()
//{
//  if (readLine()) { // got a command from host
//    switch(buffer[0]) {
//      case 'p':
//        Serial.println(xh);
//        //sprintf(buffer, "%lf\n", xh);
//        //Serial.write(buffer, strlen(buffer));
//        //Serial.flush();
//        break;
//      case 'f':
//        force = atof(buffer + 1);
//        //sprintf(buffer, "%lf\n", force);
//        //Serial.write(buffer, strlen(buffer));
//        Serial.println(force); // "synchronous" communication...
//        //Serial.flush();
//        break;
//      case 'c':
//        force = atof(buffer + 1);
//        //sprintf(buffer, "%lf\n", force);
//        //Serial.write(buffer, strlen(buffer));
//        Serial.println(xh); // "synchronous" communication...
//        //Serial.flush();
//        break;
//      default:
//        buffer[sizeof(buffer) - 2] = '\n';
//        Serial.println(buffer); // "synchronous" communication...
//        //Serial.flush();
//        break;
//    }
//  } else {
//    //Serial.println("nonono");
//    buffer[0] = ' '; // clear the buffer command if no incoming command
//  }
//}
//
//void communication()
//{
//  int32_t xhi = xh * 1000000;
//  int32_t forcei = force * 1000000;
//  if (readLineNew()) {
//    while(Serial.availableForWrite() < 32);
//    switch(buffer2[0]) {
//      case 'p':
//        //sprintf(buffer, "%lf\n", xh);
//        //Serial.write(buffer, strlen(buffer));
//        Serial.write((const char*)&xhi, 4);
//        //Serial.println(xh);
//        Serial.flush();
//        break;
//      case 'c':
//        force = atof(buffer2.c_str() + 1);
//        forcei = force * 1000000;
//        //sprintf(buffer, "%lf\n", xh);
//        //Serial.write(buffer);
//        //Serial.write(buffer, strlen(buffer));
//        Serial.write((const char*)&forcei, 4);
//        //Serial.println(xh); // "synchronous" communication...
//        Serial.flush();
//        break;
//      default:
//        //buffer[sizeof(buffer) - 2] = '\n';
//        //Serial.println(buffer); // "synchronous" communication...
//        //Serial.flush();
//        break;
//    }
//  }
//}
void communication() {
  int32_t forcei = force * 1000000;
  if (readForce()) {
    int32_t xhi = xh * 1000000;
    Serial.write((const char*)&forcei, 4);
    Serial.flush();
  }
}
/*
    readPosCount() function
*/
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

/*
    calPosMeter()
*/
void calPosMeter()
{
 // Add the function displacement & speed calculation
  double rh = 60;   //[mm]
  //double ts = 0.01231 * updatedPos - 2.37275; // Compute the angle of the sector pulley (ts) in degrees based on updatedPos
  double ts = 0.01207 * updatedPos - 6.49203; // dev 1
  //double ts = 0.01181 * updatedPos - 2.11106; // dev 4
  xh_prev = xh;
  xh = rh * (ts * 3.14159 / 180); // Compute the position of the handle based on ts
  vh = (xh - xh_prev) / .0001;
}

//void forceRendering()
//{
//  double x_wall = 10.0;
//  double k_wall = -0.008;
//
//  // parameters of decaying sinusoid
//  double amplitude = 0.6 * vh;
//  double decayingRate = 0.5;
//  double frequency = 80;
//  double springForceFactor = 300;
//
//  if (xh < x_wall) {
//    force = 0;
//    inSurface = false;
//  } else {
//    if(!inSurface) {
//      timeOfImpact = millis();
//      inSurface = true;
//    }
//    double t = (millis() - timeOfImpact)/1000.0;
//    double sinForce = -k_wall * amplitude * exp(-decayingRate*t) * sin(2 * M_PI * frequency * t);
//    double springForce = -k_wall * springForceFactor * abs(x_wall - xh);
//    force = sinForce + springForce;
//    if(force < 0) {
//      force = 0;
//    }
//  }
//}

/*
      Output to motor
*/
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
/*
   setPwmFrequency
*/
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
/*
    Loop function
*/
void loop() {
  // put your main code here, to run repeatedly
  // read the position in count
  readPosCount();
  // convert position to meters
  calPosMeter();

  communication();
  // calculate rendering force
  //forceRendering();
  // output to motor
  motorControl();
  // delay before next reading:
  delay(3);
}
