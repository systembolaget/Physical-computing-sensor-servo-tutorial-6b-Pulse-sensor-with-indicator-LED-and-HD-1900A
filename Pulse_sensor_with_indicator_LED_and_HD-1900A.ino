// Tutorial 6b. Pulse sensor with indicator LED and HD-1900A

// Main parts: Adafruit Metro Mini, pulse sensor, LED, servo, resistor,
// 10k trim potentiometer, momentary switch, capacitors

// Libraries required to rotate the servo and to smooth data; the RunningMedian
// library was developed by Rob Tillaart https://github.com/RobTillaart/RunningMedian
#include <RunningMedian.h>
#include <Servo.h>

// Instances a samples object from the library that holds 15 sensor
// readings, refreshed at each reading cycle (= FIFO order). The
// higher the readings window, the smoother the data, but the higher
// the delay
RunningMedian samples = RunningMedian(15);

// Instances a servo object
Servo myServo;

// Variables that remain constant
const byte pinPulseSensor = A0; // Analog pin from pulse sensor
const byte pinPotentiometer = A1; // Analog pin from potentiometer
const byte pinLED = 5; // Digital pin for the LED's anode
const byte pinServo = 6; // PWM pin for the servo
const byte pinSwitch = 8; // Digital pin from momentary switch
const byte servoAngleMin = 20; // Depends on servo used
const byte servoAngleMax = 160; // Depends on servo used

// Variables that can change
byte lastSwitchState = HIGH; // Tracks the last switch state, open (= HIGH) at start
byte indicatorState = false; // Tracks if the LED and servo are on or off, off (= false) at start
byte servoAngle = 90; // Default angle of the servo at start-up
int sensorSignalMin = 1023; // Will become the next person's minimum pulse value
int sensorSignalMax = 0; // Will become the next person's maximum pulse value
int sensorSignal = 0; // Value fetched from the pulse sensor; can range from 0 to 1023
int sensorSignalMedian = 0; // Median value of sensorSignal
int sensorSignalThreshold = 0; // Threshold which value counts as a real pulse, varies with each person

void setup()
{
  // Initialise momentary switch pin with an internal pull-up resistor
  // so that the momentary switch is read as open (= HIGH) at start
  pinMode (pinSwitch, INPUT_PULLUP);

  // Initialise the LED pin
  pinMode(pinLED, OUTPUT);

  // Initialises the servo with its pin
  myServo.attach(pinServo);

  // Read the voltage from the pulse sensor's pin for seven seconds
  // after the next person has put on the velcro ring on the ring
  // finger. This will set the minimum and maximum signal values
  while (millis() < 7000)
  {
    sensorSignal = analogRead(pinPulseSensor);

    // Set the maximum sensor value
    if (sensorSignal > sensorSignalMax)
    {
      sensorSignalMax = sensorSignal;
    }

    // Set the minimum sensor value
    if (sensorSignal < sensorSignalMin)
    {
      sensorSignalMin = sensorSignal;
    }
  }

  // Start the serial monitor or plotter to display various values
  Serial.begin(115200);
  Serial.println("sensorSignal:,signalMedian:,signalThreshold:,servoAngle:");
}

void loop()
{
  // Check if the switch was pressed to toggle the LED and servo on/off
  checkSwitch();

  // Check the potentiometer used to adjust the pulse sensor's signal
  // threshold for the next person wearing the sensor
  checkPotentiometer();

  // Read the voltage from the pulse sensor
  sensorSignal = analogRead(pinPulseSensor);

  // And add it to the running median calculation (= FIFO queue)
  samples.add(sensorSignal);

  // Then calculate the running median to smooth the data
  sensorSignalMedian = samples.getMedian();

  // Print various values to the serial monitor/plotter for the next person
  // wearing the sensor
  Serial.print(sensorSignal);
  Serial.print(",");
  Serial.print(sensorSignalMedian);
  Serial.print(",");
  Serial.print(sensorSignalThreshold);
  Serial.print(",");
  Serial.println(servoAngle);

  // If the indicator LED and servo are enabled
  if (indicatorState)
  {
    // Flash the LED
    flashLED();
    
    // Rotate the servo
    rotateServo();
  }
  else
  {
    // Turn the LED off
    digitalWrite(pinLED, LOW);
  }

  // A pause (milliseconds) is necessary for the sensor and servo
  delay(20);
}

void checkPotentiometer()
{
  // Read the voltage from the potentiometer pin
  sensorSignalThreshold = analogRead(pinPotentiometer);
}

void checkSwitch()
{
  // The momentary switch is hardware debounced with a 0.1uF capacitor; no
  // debouncing code is necessary. See http://www.gammon.com.au/switches
  // Read the voltage from the momentary switch pin to see if something
  // has changed (was the button pressed or released?)
  byte switchState = digitalRead (pinSwitch);

  // Has the momentary switch state changed since the last time it was
  // checked?
  if (switchState != lastSwitchState)
  {
    // Then, test if the switch was closed (button pressed)
    if (switchState == LOW)
    {
      // Here, you can do something on pressing the button
    }
    else
    {
      // Here, you can do something on releasing the button; we switch an
      // LED and servo on and off by toggling a flag variable
      indicatorState = !indicatorState;
    }
    // Last, store the current switch state for the next time around
    lastSwitchState = switchState;
  }
}

void flashLED()
{
  // If the signal's value is higher than the threshold set with
  // the potentiometer
  if (sensorSignalMedian >= sensorSignalThreshold)
  {
    // Turn the LED on
    digitalWrite(pinLED, HIGH);
  }
  // Otherwise
  else
  {
    // Turn the LED off
    digitalWrite(pinLED, LOW);
  }
}

void rotateServo()
{
  // Map the smoothed values to the max. servo angle range the servo is
  // capable of, depending on the minimum and maximum values recorded for
  // the next person after resetting the program
  servoAngle = map(sensorSignalMedian, sensorSignalMin, sensorSignalMax, servoAngleMin, servoAngleMax);

  // Make sure outlier values are within the possible angle range
  servoAngle = constrain(servoAngle, servoAngleMin, servoAngleMax);

  // Rotate the servo to the new angle
  myServo.write(servoAngle);
}
