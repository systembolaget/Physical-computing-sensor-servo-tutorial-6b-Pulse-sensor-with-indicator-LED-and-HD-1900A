// Tutorial 6b. Pulse sensor with indicator LED and HD-1900A

// Main parts: Adafruit Metro Mini, pulse sensor, LED, servo, resistor,
// 10k trim potentiometer, momentary switch, capacitors

// Libraries required to rotate the servo and to smooth data; the RunningMedian
// library was developed by Rob Tillaart https://github.com/RobTillaart/RunningMedian
#include <RunningMedian.h>
#include <Servo.h>

// Instances a samples object from the library that holds 7 sensor
// readings, refreshed at each reading cycle (= FIFO order). The
// higher the readings window, the smoother the data, but the higher
// the delay
RunningMedian samples = RunningMedian(7);

// Instances a servo object
Servo myServo;

// Variables that remain constant
const byte pinPulseSensor = A0; // Analog pin from pulse sensor
const byte pinPotentiometer = A1; // Analog pin from potentiometer
const byte pinLED = 5; // Digital pin for the LED's anode
const byte pinServo = 6; // PWM pin for the servo
const byte pinSwitch = 8; // Digital pin from momentary switch

// Variables that can change
byte lastSwitchState = HIGH; // Tracks the last switch state, open (= HIGH) at start
byte indicatorState = false; // Tracks if the LED and servo are on or off, off (= false) at start
byte servoAngle = 90; // Default angle of the servo at start-up
int sensorSignalMin = 1023; //
int sensorSignalMax = 0; //
int sensorSignal; // Value fetched from the pulse sensor; can range from 0 to 1023
int signalThreshold = 0; // Threshold which value counts as a real pulse, varies with each person

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
  int signalMedian = samples.getMedian();

  // Map the smoothed data to the minimum and maximum servo angle range,
  // in my case 10° to 170°. The value range 400 to 700 is what I saw
  // in the serial monitor with one of my fingers. This range can change
  // with other fingers, the pressure applied to the sensor, and was
  // different on fingers of my children
  // Servo capable of 120° rotation max.

  // Map the smoothed values to the max. servo angle range the servo is
  // capable of, depending on the minimum and maximum values recorded for
  // the next person after resetting the program
  servoAngle = map(signalMedian, sensorSignalMin, sensorSignalMax, 20, 160);
  
  // Make sure outlier values are within the possible angle range
  servoAngle = constrain(servoAngle, 20, 160);

  // Print various values to the serial monitor/plotter for the next person
  // wearing the sensor
  Serial.print(sensorSignal);
  Serial.print(",");
  Serial.print(signalMedian);
  Serial.print(",");
  Serial.print(signalThreshold);
  Serial.print(",");
  Serial.println(servoAngle);

  // If the indicator LED and servo are enabled
  if (indicatorState)
  {
    // Rotate the servo to the new angle
    myServo.write(servoAngle);

    // And if the signal's value is higher than the threshold set with
    // the potentiometer
    if (signalMedian >= signalThreshold)
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

  // A pause (milliseconds) is necessary for the sensor and servo
  delay(20);
}

void checkPotentiometer()
{
  // Read the voltage from the potentiometer pin
  signalThreshold = analogRead(pinPotentiometer);
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
      // Here, you can do something on releasing the button; we toggle an
      // LED and servo on and off
      indicatorState = !indicatorState;
    }
    // Last, store the current switch state for the next time around
    lastSwitchState = switchState;
  }
}
