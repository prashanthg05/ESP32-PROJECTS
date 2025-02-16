#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Blynk credentials (Replace with yours)
#define BLYNK_TEMPLATE_ID "TMPL3f2HVAbLv"
#define BLYNK_TEMPLATE_NAME "Smart Parking"
#define BLYNK_AUTH_TOKEN "flc8Az-EHqyCp0ZBavLXFfNupmpr0Z37"

// WiFi Credentials
char ssid[] = "pro";
char pass[] = "123456789";

// Blynk Timer for sending updates
BlynkTimer timer;

// LCD setup (I2C address 0x27, 16x2 LCD)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Servo objects
Servo entryServo, exitServo;

// IR sensor pins
const int entryIR = 32;   // Entry IR sensor
const int exitIR = 33;    // Exit IR sensor
const int slotIR[] = {13, 12, 14, 27, 26, 25};  // Parking slots IR sensors

// Servo pins
const int entryServoPin = 19;
const int exitServoPin = 18;

// Parking slots status
bool slotStatus[6] = {false, false, false, false, false, false};

// Parking counter
int availableSlots = 6;

void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi and Blynk
Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);


  // Initialize IR sensor pins
  pinMode(entryIR, INPUT);
  pinMode(exitIR, INPUT);
  for (int i = 0; i < 6; i++) {
    pinMode(slotIR[i], INPUT);
  }

  // Attach servos
  entryServo.attach(entryServoPin);
  exitServo.attach(exitServoPin);

  // Close gates initially
  entryServo.write(0);
  exitServo.write(0);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smart Parking");

  // Timer for Blynk updates
  timer.setInterval(1000L, updateParkingStatus);
}

void loop() {
  Blynk.run();
  timer.run();
  controlEntryGate();
  controlExitGate();
  updateLCD();
}

// Update parking slots status and send to Blynk
void updateParkingStatus() {
  int occupiedCount = 0;

  for (int i = 0; i < 6; i++) {
    slotStatus[i] = digitalRead(slotIR[i]) == LOW; // LOW means occupied
    Blynk.virtualWrite(V1 + i, slotStatus[i]); // Update Blynk LEDs

    if (slotStatus[i]) occupiedCount++;
  }

  availableSlots = 6 - occupiedCount;
  Serial.print("Available Slots: ");
  Serial.println(availableSlots);
}

// Entry gate control
void controlEntryGate() {
  if (digitalRead(entryIR) == LOW) { // Car detected at entry
    if (availableSlots > 0) {
      Serial.println("Car detected at entry, opening gate...");
      entryServo.write(90); // Open entry gate
      delay(3000); // Wait for car to enter
      entryServo.write(0);  // Close gate
    } else {
      Serial.println("Parking full! Entry gate closed.");
    }
  }
}

// Exit gate control
void controlExitGate() {
  if (digitalRead(exitIR) == LOW) { // Car detected at exit
    Serial.println("Car detected at exit, opening gate...");
    exitServo.write(90); // Open exit gate
    delay(3000); // Wait for car to exit
    exitServo.write(0);  // Close gate
  }
}

// Update LCD display with slot statuses
void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Slots: ");

  for (int i = 0; i < 6; i++) {
    lcd.print(slotStatus[i] ? "F" : "E"); // F = Full, E = Empty
  }

  lcd.setCursor(0, 1);
  lcd.print("Free: ");
  lcd.print(availableSlots);
}
