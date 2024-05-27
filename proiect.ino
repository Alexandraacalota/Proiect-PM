#include <Wire.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

#define RX_PIN 2
#define TX_PIN 3

// Define constants
const int MPU_ADDR = 0x68;
const int BUTTON_PIN = 4;
int16_t accelX, accelY, accelZ;
int prevStepCount = 0;
int stepCount = 0;
float accMagnitudePrev = 0;

// User-specific constants
float weightKg = 0.0;
const float stepCalories = 0.04;
float caloriesBurned = 0.0;

SoftwareSerial BTSerial(RX_PIN, TX_PIN);

void resetEEPROM() {
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
  stepCount = 0;
  caloriesBurned = 0.0;
  Serial.println("EEPROM reset");
}

void setup() {
  Wire.begin();
  Serial.begin(9600);
  BTSerial.begin(9600);

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  delay(2000);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  EEPROM.get(0, stepCount);

  Serial.println("Enter your weight in kg:");
  while (Serial.available() == 0) {
    delay(100);
  }
  weightKg = Serial.parseFloat();
  Serial.print("Weight set to: ");
  Serial.print(weightKg);
  Serial.println(" kg");
  stepCount = 0;
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    resetEEPROM();
    delay(1000);
  }
  readAccelerometerData();
  detectStep();
  calculateCalories();
  displayData();
  delay(100);
}

void readAccelerometerData() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  accelX = Wire.read() << 8 | Wire.read();
  accelY = Wire.read() << 8 | Wire.read();
  accelZ = Wire.read() << 8 | Wire.read();
}

void saveStepCount() {
  // Save stepCount to EEPROM
  EEPROM.put(0, stepCount);
}

void detectStep() {
  float accX = accelX / 16384.0;
  float accY = accelY / 16384.0;
  float accZ = accelZ / 16384.0;

  // Calculate the magnitude of acceleration
  float accMagnitude = sqrt(accX * accX + accY * accY + accZ * accZ);

  // Peak detection
  if (accMagnitudePrev > accMagnitude + 0.1 && accMagnitudePrev > 1.5) {
    stepCount++;
    saveStepCount();
  }
  accMagnitudePrev = accMagnitude;
}

void calculateCalories() {
  // Calories burned based on steps
  caloriesBurned = 0.015 * stepCount * stepCalories * weightKg;
}

void displayData() {
  if (stepCount > prevStepCount) {
    Serial.print("Steps: ");
    Serial.print(stepCount);
    Serial.print(" Calories burned: ");
    Serial.print(caloriesBurned);
    Serial.println(" kcal");
    prevStepCount = stepCount;
  }
}
