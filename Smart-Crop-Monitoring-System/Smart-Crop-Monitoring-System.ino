// Code By Prashanth G
// 

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// Code By Prashanth G
// WiFi and Blynk Credentials
char auth[] = "ItphuO6WvZeG5o65ir8Rc0ubPqEUjE6g";
char ssid[] = "pro";
char pass[] = "123456789";

// Sensor & Actuator Pins
#define DHTPIN 18
#define SOIL_MOISTURE_PIN 34
#define RAIN_SENSOR_PIN 35
#define PIR_SENSOR_PIN 19
#define RELAY_PIN 23
#define DHTTYPE DHT11

// Initialize DHT and LCD
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Change I2C address if needed

// Blynk Virtual Pins
#define VIRTUAL_TEMP V1
#define VIRTUAL_HUMIDITY V2
#define VIRTUAL_SOIL V3
#define VIRTUAL_RAIN_LED V4
#define VIRTUAL_PIR_LED V5
#define VIRTUAL_PUMP_STATUS V6

// Flags to Avoid Repeated Notifications
bool rainNotified = false;
bool pirNotified = false;
bool pumpStatus = false; // false = OFF, true = ON

void setup() {
    Serial.begin(115200);

    // Initialize WiFi and Blynk
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");

    Blynk.config(auth, "blynk.cloud", 80);
    Blynk.connect();

    // Initialize Sensors
    dht.begin();
    lcd.init();
    lcd.backlight();

    // Pin Modes
    pinMode(SOIL_MOISTURE_PIN, INPUT);
    pinMode(RAIN_SENSOR_PIN, INPUT);
    pinMode(PIR_SENSOR_PIN, INPUT);
    pinMode(RELAY_PIN, OUTPUT);

    // Ensure Pump is OFF initially
    digitalWrite(RELAY_PIN, LOW);
}

void loop() {
    if (Blynk.connected()) {
        Blynk.run();
    }
    // Code By Prashanth G
    // Read Sensors
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int soilMoisture = analogRead(SOIL_MOISTURE_PIN);
    bool isRaining = digitalRead(RAIN_SENSOR_PIN);
    bool isMotionDetected = digitalRead(PIR_SENSOR_PIN);

    // Convert Soil Moisture to Percentage
    int soilMoisturePercent = map(soilMoisture, 0, 4095, 100, 0);

    // Control Relay (Water Pump)
    if (soilMoisturePercent < 35) {
        digitalWrite(RELAY_PIN, HIGH); // Pump ON
        pumpStatus = true;
    } else if (soilMoisturePercent >= 45) {
        digitalWrite(RELAY_PIN, LOW); // Pump OFF
        pumpStatus = false;
    }

    // Display Data on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp:");
    lcd.print(temperature);
    lcd.print("C");

    lcd.setCursor(0, 1);
    lcd.print("Moist:");
    lcd.print(soilMoisturePercent);
    lcd.print("%");

    lcd.setCursor(10, 1);
    lcd.print("Pump:");
    lcd.print(pumpStatus ? "H" : "L");

    // Send Data to Blynk
    Blynk.virtualWrite(VIRTUAL_TEMP, temperature);
    Blynk.virtualWrite(VIRTUAL_HUMIDITY, humidity);
    Blynk.virtualWrite(VIRTUAL_SOIL, soilMoisturePercent);
    Blynk.virtualWrite(VIRTUAL_PUMP_STATUS, pumpStatus ? 1 : 0);

    // LED Indicators in Blynk
    Blynk.virtualWrite(VIRTUAL_RAIN_LED, isRaining ? 0 : 255);
    Blynk.virtualWrite(VIRTUAL_PIR_LED, isMotionDetected ? 255 : 0);

    // Blynk Notifications
    if (isRaining && !rainNotified) {
        Blynk.logEvent("rain_alert", "Rain Detected! 🌧️");
        rainNotified = true;
    } else if (!isRaining) {
        rainNotified = false;
    }

    if (isMotionDetected && !pirNotified) {
        Blynk.logEvent("motion_alert", "Motion Detected! 🚶‍♂️");
        pirNotified = true;
    } else if (!isMotionDetected) {
        pirNotified = false;
    }

    delay(2000); // Wait before next reading
}
