#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// Constants
#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP  59

// Pin Definitions
#define MOS_PIN 26
#define DHT_PIN 25
#define LDR_PIN 34
#define WATER_LEVEL_PIN 35
#define SOIL_MOISTURE_PIN 32
#define WIND_SENSOR_PIN 33
#define ANALOG_INPUT_PIN 39
#define CHARGING_PIN 27

// Voltage thresholds
const float MAX_VOLTAGE = 4.2;  // Maximum battery voltage
const float MIN_VOLTAGE = 2.8;  // Minimum battery voltage
const float FULL_SCALE = MAX_VOLTAGE - MIN_VOLTAGE;

// Battery monitoring constants
const int CHARGE_TIME = 60000;  // Time to check for charging (in milliseconds)
const float VOLTAGE_THRESHOLD = 0.1;  // Minimum voltage change considered as charging (adjust as needed)
float percentage = 0.0;
String batteryStatus = "Unknown";

// WiFi and Server Constants
const char* SSID = "Greenwing";
const char* PASSWORD = "20010123";
const char* SERVER_NAME = "http://localhost/greenwing/sensordata.php";
//const char* SERVER_NAME = "http://greenwing.scienceontheweb.net/sensordata.php
String PROJECT_API_KEY = "GreenWing";

// DHT Sensor
DHT dht(DHT_PIN, DHT11);

// Wind Sensor
volatile unsigned long pulseCount;
volatile unsigned long lastPulseTime;
float windSpeed;

// Boot Counter
RTC_DATA_ATTR int bootCount = 0;

unsigned long previousMillis = 0;

// Sensor Values
int ldrValue = 0;
int waterLevelValue = 0;
int soilMoistureValue = 0;

// Function Declarations
void connectToWiFi();
void readDHT();
void readSensor(int pin, const char* sensorName, int& sensorValue);
void pulseCounter();
void readWindSpeed();
void uploadData();
void batteryMonitor();

void setup() {
  Serial.begin(115200);
  Serial.println("------------------------------------------------------------------------------");
  Serial.println("ESP32 serial initialized");

  // Increment boot count
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  // Connect to WiFi
  connectToWiFi();

  Serial.println("Connected to WiFi network with IP Address: " + WiFi.localIP());
  Serial.println("------------------------------------------------------------------------------");

  // Initialize sensors and attach interrupt for wind speed sensor
  pinMode(MOS_PIN, OUTPUT);
  dht.begin();
  attachInterrupt(digitalPinToInterrupt(WIND_SENSOR_PIN), pulseCounter, FALLING);

  digitalWrite(MOS_PIN, HIGH);  // Turn MOSFET ON
  delay(1000);  // Wait for 0.5 seconds

  // Upload data if WiFi is connected
  if (WiFi.status() == WL_CONNECTED) {
    uploadData();
  } else {
    Serial.println("WiFi Disconnected");
  }

  Serial.println("------------------------------------------------------------------------------");
  delay(500);

  digitalWrite(MOS_PIN, HIGH);  // Turn MOSFET ON
  delay(500);  // Wait for 0.5 seconds

  // Set up deep sleep
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  Serial.println("Going to sleep now");
  delay(500);
  Serial.flush();
  esp_deep_sleep_start();
}

void loop() {
  // Nothing to do here
}

// Function Definitions

// Connect to WiFi
void connectToWiFi() {
  Serial.println("Connecting to WiFi");

  // Begin WiFi connection
  WiFi.begin(SSID, PASSWORD);
  int attempts = 0;

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  Serial.println("");
}

// Read DHT sensor values
void readDHT() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  delay(1000);

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
  } else {
    Serial.println("Temperature: " + String(temperature) + " °C");
    Serial.println("Humidity: " + String(humidity) + " %");
    Serial.println("------------------------------------------------------------------------------");
  }
}

// Read generic analog sensor
void readSensor(int pin, const char* sensorName, int& sensorValue) {
  // Read sensor value
  pinMode(pin, INPUT);
  sensorValue = analogRead(pin);

  // Print sensor information
  Serial.println(sensorName + String(" Value: ") + String(sensorValue));
  Serial.println("------------------------------------------------------------------------------");
}

// Wind speed sensor interrupt handler
void pulseCounter() {
  // Increment pulse count on each interrupt
  pulseCount++;
}

// Read wind speed from sensor
void readWindSpeed() {
  unsigned long currentTime = millis();
  unsigned long timeDifference = currentTime - lastPulseTime;

  // Check if a full revolution is completed
  if (timeDifference > 1000) {
    // Calculate wind speed in km/h
    windSpeed = (float)pulseCount / 2.0 / timeDifference * 1000.0 * 3.6;

    // Reset pulse count and time
    pulseCount = 0;
    lastPulseTime = currentTime;

    Serial.println("Wind Speed: " + String(windSpeed) + " km/h");
    Serial.println("------------------------------------------------------------------------------");
  }
}

// Upload sensor data to the server
void uploadData() {
  // Read sensor data
  readDHT();
  readSensor(LDR_PIN, "LDR", ldrValue);
  readSensor(WATER_LEVEL_PIN, "Water Level", waterLevelValue);
  readSensor(SOIL_MOISTURE_PIN, "Soil Moisture", soilMoistureValue);
  readWindSpeed();

  // Battery monitoring
  batteryMonitor();

  // Construct data for HTTP POST request
  String sensorData = "api_key=" + PROJECT_API_KEY +
                      "&light_intensity=" + String(ldrValue) +
                      "&temperature=" + String(dht.readTemperature()) +
                      "&humidity=" + String(dht.readHumidity()) +
                      "&rain=" + String(waterLevelValue) +
                      "&wind=" + String(windSpeed) +
                      "&soil_moisture=" + String(soilMoistureValue) +
                      "&battery_percentage=" + String(percentage) +
                      "&battery_status=" + String(batteryStatus);

  // Print sensor data and free memory
  Serial.print("sensorData: ");
  Serial.println(sensorData);
  Serial.print("Free memory: ");
  Serial.println(ESP.getFreeHeap());
  Serial.println("------------------------------------------------------------------------------");

  // Perform HTTP POST request
  WiFiClient client;
  HTTPClient http;

  http.begin(client, SERVER_NAME);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(sensorData);

  // Print HTTP response code
  Serial.print("HTTP Response code: ");
}
