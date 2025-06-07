#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <DHT.h>

// Pin Definitions (Adjust according to your ESP32 pinout)
const int LM35_PIN = 25;         // Must be an analog-capable pin
const int LDR_PIN = 26;          // Must be an analog-capable pin
const int DHT_PIN = 7;          // Any digital GPIO
const int WATER_LEVEL_PIN = 5;  // Digital input
const int RELAY_PIN = 18;        // Output pin
const int LED_PIN = 19;           // Built-in LED on some ESP32 boards

#define DHT_TYPE DHT11

// Sensor and Display Initialization
DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(WATER_LEVEL_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();
  dht.begin();
  Serial.begin(115200); // Use a higher baud rate for ESP32

  lcd.setCursor(0, 0);
  lcd.print("System Initializing");
  delay(2000);
  lcd.clear();
}

void loop() {
  float temperature = readLM35Temperature();
  float humidity = readHumidity();
  int ldrValue = readLDRValue();
  bool isWaterPresent = digitalRead(WATER_LEVEL_PIN);

  displayTemperatureHumidity(temperature, humidity);
  controlLightBasedOnLDR(ldrValue);
  controlRelayBasedOnWaterLevel(isWaterPresent);

  logToSerial(ldrValue, isWaterPresent);

  delay(1000);
}

float readLM35Temperature() {
  int total = 0;
  for (int i = 0; i < 10; i++) {
    total += analogRead(LM35_PIN);
    delay(10);
  }
  float average = total / 10.0;
  float voltage = average * (3.3 / 4095.0);  // ESP32 ADC range
  return voltage * 100.0; // LM35 gives 10mV/°C
}

float readHumidity() {
  float h = dht.readHumidity();
  return isnan(h) ? 0 : h;
}

int readLDRValue() {
  int total = 0;
  for (int i = 0; i < 10; i++) {
    total += analogRead(LDR_PIN);
    delay(5);
  }
  return total / 10;
}

void controlLightBasedOnLDR(int ldrValue) {
  if (ldrValue < 2000) { // Adjust threshold based on actual light level
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

void controlRelayBasedOnWaterLevel(bool waterDetected) {
  digitalWrite(RELAY_PIN, waterDetected ? HIGH : LOW);
}

void displayTemperatureHumidity(float temp, float humidity) {
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp, 1);
  lcd.print((char)223);
  lcd.print("C   ");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity, 0);
  lcd.print("%   ");
}

void logToSerial(int ldrValue, bool waterDetected) {
  Serial.print("LDR: ");
  Serial.print(ldrValue);
  Serial.print(" | Water Level: ");
  Serial.println(waterDetected ? "HIGH" : "LOW");
}
