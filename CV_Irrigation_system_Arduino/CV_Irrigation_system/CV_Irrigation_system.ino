#include <EEPROM.h>  // Include EEPROM library
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Flow Sensor Variables
unsigned int flowPulseCount = 0;
float flowRate = 0.0;                    // Flow rate in mL/s
float totalMilliliters = 0.0;            // Total volume in milliliters
float tankCapacityMilliliters = 1000.0;  // Tank capacity in milliliters
int reading_delay = 500;

// Corrected Pulses per milliliter for YF-S201 5V
const float pulsesPerMilliliter = 0.25;

unsigned long currentTime;
unsigned long previousTime = 0;
unsigned long startTime = 0;
unsigned long elapsedTime;
boolean flowSensorState = LOW;
boolean lastFlowSensorState = LOW;
unsigned long debounceTime = 0;  // Debounce time
int initialPulsesToIgnore = 10;

// Motor Driver Variables
int ledPin = 13;  // Built-in LED pin number
int enablePin = 6;
int in1 = 8;
int in2 = 7;
const int flowSensorPin = 9;
const int mosfetGatePin = 10;  // Connect to the gate of the MOSFET

// Motor Control Variables
bool motorEnabled = false;
bool emptyingTank = false;  // Flag to track empty tank operation
int motorSpeed;

// EEPROM address for storing motor speed
const int speedAddress = 0;
// Set the LCD I2C address, 0x3F
LiquidCrystal_I2C lcd(0x3F, 16, 2);

void setup() {
  pinMode(flowSensorPin, INPUT);
  Serial.begin(115200);
  // Initialize the LCD
  lcd.init();

  // Turn on the backlight
  lcd.backlight();

  // Print a message to the LCD
  pinMode(mosfetGatePin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(ledPin, OUTPUT);
  lcd.setCursor(3, 0);
  lcd.print("GreenWing!");
  lcd.setCursor(2, 1);
  lcd.print("- Starting -");
  delay(reading_delay * 3);  // Reading delay
  lcd.clear();
  Serial.print("Enter commands: ON, OFF, EMPTY, RESET, SPEED <value> | ");
  lcd.setCursor(0, 0);
  lcd.print("Manual Commands,");
  lcd.setCursor(0, 1);
  lcd.print("ON,OFF,E,R,SPEED");
  delay(reading_delay * 3);  // Reading delay
  lcd.clear();
  startTime = millis();
  EEPROM.get(speedAddress, motorSpeed);
  if (motorSpeed == 255 || motorSpeed == 0) {  // EEPROM is empty or invalid value
    motorSpeed = 255;                          // Default speed
  }
  Serial.print("Current Motor Speed: ");
  Serial.println(motorSpeed);
  lcd.setCursor(2, 0);
  lcd.print("Motor Speed");
  lcd.setCursor(6, 1);
  lcd.print(motorSpeed);
  lcd.print(" ");
  delay(reading_delay * 2);  // Reading delay
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("GreenWing!");
  lcd.setCursor(0, 1);
  lcd.print("- System Ready -");
}

void resetFlowSensor() {
  lcd.clear();
  flowPulseCount = 0;
  totalMilliliters = 0.0;
  startTime = millis();
  Serial.println("Flow sensor reset.");
  lcd.setCursor(0, 0);
  lcd.print("Data Resetting...");
  delay(reading_delay / 2);  // Reading delay
  lcd.setCursor(6, 1);
  lcd.print("Done");
}

void emptyTank() {
  emptyingTank = true;
  motorEnabled = true;
  analogWrite(enablePin, 255);
  Serial.println("Tank emptying. Motor turned on. Press OFF to stop");
  resetFlowSensor();
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("TANK DRAINING");
  delay(reading_delay / 2);  // Reading delay
  lcd.setCursor(1, 1);
  lcd.print("Type 0 to STOP");
}

void loop() {
  float remainingMilliliters;  // Declare it at the start of loop
  flowSensorState = digitalRead(flowSensorPin);
  if (flowSensorState != lastFlowSensorState) {
    if (flowSensorState == HIGH) {
      if (initialPulsesToIgnore > 0) {
        initialPulsesToIgnore--;
      } else {
        flowPulseCount++;
      }
    }
    delay(debounceTime);
    lastFlowSensorState = flowSensorState;
  }

  currentTime = millis();
  elapsedTime = currentTime - previousTime;
  if (elapsedTime >= 500) {  // Adjusted for more frequent output
    previousTime = currentTime;

    if (motorEnabled && !emptyingTank) {  // Only calculate flow when not emptying tank
      flowRate = ((float)flowPulseCount / pulsesPerMilliliter) / (elapsedTime / 1000.0);
      totalMilliliters += flowRate * (elapsedTime / 1000.0);  // Accumulate total volume in mL
      flowPulseCount = 0;

      // Calculate total time and average flow rate continuously
      float totalTimeInSeconds = (currentTime - startTime) / 1000.0;
      float averageFlowRate = (totalMilliliters > 0 && totalTimeInSeconds > 0) ? (totalMilliliters / totalTimeInSeconds) : 0.0;
      lcd.clear();
      Serial.print("Flow Rate: ");
      Serial.print(flowRate);
      Serial.print(" mL/s \t");
      Serial.print("Total: ");
      Serial.print(totalMilliliters, 4);
      Serial.print(" mL \t");
      remainingMilliliters = tankCapacityMilliliters - totalMilliliters;
      Serial.print("Remaining Water in tank: ");
      Serial.print(remainingMilliliters, 4);
      Serial.println(" mL");

      lcd.setCursor(0, 0);
      lcd.print("Flow Rate: ");
      lcd.print(flowRate);
      lcd.print(" mL/s ");
      lcd.setCursor(0, 1);
      lcd.print("Left: ");
      lcd.print(remainingMilliliters, 2);
      lcd.print(" mL");

      if (remainingMilliliters <= 0) {
        lcd.clear();
        motorEnabled = false;
        analogWrite(enablePin, 0);
        Serial.println("Tank empty. Motor turned OFF.");
        // Print average flow rate and total time
        Serial.print("Average Flow Rate: ");
        Serial.print(averageFlowRate);
        Serial.println(" ml/s");
        Serial.print("Total Time Taken: ");
        Serial.print(totalTimeInSeconds);
        Serial.println(" seconds");
        lcd.setCursor(0, 0);
        lcd.print(" Avg Rate: ");
        lcd.print(averageFlowRate);
        lcd.print(" mL/s ");
        lcd.setCursor(2, 1);
        lcd.print("Time: ");
        lcd.print(totalTimeInSeconds, 2);
        lcd.print(" S");
        delay(reading_delay * 5);  // Reading delay
        resetFlowSensor();
      }
    }
  }

  // Check for Serial Input for Motor Control
  while (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    // command.trim();  // Remove any whitespace

    // Check if the command is an integer
    if (command.toInt() != 0 && command.toInt() != 1) {
      // If not an integer, handle it as a string command
      if (command.equalsIgnoreCase("EMPTY") || command.equalsIgnoreCase("E")) {
        emptyTank();
      } else if (command.equalsIgnoreCase("ON") || command.equalsIgnoreCase("y")) {
        digitalWrite(ledPin, HIGH);
        emptyingTank = false;  // Reset the emptyingTank flag
        motorEnabled = true;
        analogWrite(enablePin, motorSpeed);
        Serial.println("Motor turned ON");
      } else if (command.equalsIgnoreCase("OFF") || command.equalsIgnoreCase("n")) {
        digitalWrite(ledPin, LOW);
        motorEnabled = false;
        emptyingTank = false;  // Reset the emptyingTank flag
        analogWrite(enablePin, 0);
        Serial.println("Motor turned OFF");
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("GreenWing!");
        lcd.setCursor(0, 1);
        lcd.print("- System Wait  -");
      } else if (command.equalsIgnoreCase("RESET") || command.equalsIgnoreCase("R")) {
        resetFlowSensor();
      } else {
        int inputSpeed = command.toInt();
        inputSpeed = constrain(inputSpeed, 2, 9);      // Ensure speed is within 2-9 range
        motorSpeed = map(inputSpeed, 2, 9, 126, 255);  // Scale speed to 126-255
        EEPROM.put(speedAddress, motorSpeed);            // Save the new speed to EEPROM using put
        if (motorEnabled) {
          analogWrite(enablePin, motorSpeed);
        }
        Serial.print("Motor speed set to: ");
        Serial.println(motorSpeed);
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("Motor Speed");
        lcd.setCursor(6, 1);
        int display_speed = map(motorSpeed, 126, 255, 2, 9);
        lcd.print(motorSpeed);
        lcd.print(" ");
        delay(reading_delay * 3);  // Reading delay
      }
    } else {
      // If the command is an integer, handle it accordingly
      for (int i = 0; i < command.length(); i += 2) {
        int bitValue = command.charAt(i) - '0';  // Convert char to int (0 or 1)

        if (bitValue == 1) {
          digitalWrite(ledPin, HIGH);
          emptyingTank = false;  // Reset the emptyingTank flag
          motorEnabled = true;
          analogWrite(enablePin, motorSpeed);
          Serial.println("Motor turned ON");
        } else if (bitValue == 0) {  // Check for character one
          digitalWrite(ledPin, LOW);
          motorEnabled = false;
          emptyingTank = false;  // Reset the emptyingTank flag
          analogWrite(enablePin, 0);
          Serial.println("Motor turned OFF");
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("GreenWing!");
          lcd.setCursor(0, 1);
          lcd.print("Wait for Plant");  // Print the current bit value
        } else {
          Serial.println("Invalid command");
        }
      }
    }
  }


  // Motor Driver Logic
  if (motorEnabled) {
    analogWrite(enablePin, motorSpeed);  // Set motor speed
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(mosfetGatePin, HIGH);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(mosfetGatePin, LOW);
  }
}
