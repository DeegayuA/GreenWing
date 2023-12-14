#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP  10

#define DHT_PIN 25
#define LDR_PIN 34
#define WATER_LEVEL_PIN 35
#define SOIL_MOISTURE_PIN 32
#define WIND_SENSOR_PIN 33

int ldrValue = 0;
int waterLevelValue = 0;
int soilMoistureValue = 0;
volatile unsigned long pulseCount;
volatile unsigned long lastPulseTime;
float windSpeed;

const char* ssid = "SLT FIBER";
const char* password = "Deegayu2001";
const char* SERVER_NAME = "http://localhost/greenwing/sensordata.php";
// const char* SERVER_NAME = "http://greenwing.scienceontheweb.net/sensordata.php";
String PROJECT_API_KEY = "GreenWing";

RTC_DATA_ATTR int bootCount = 0;
DHT dht(DHT_PIN, DHT11);

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

  // Initialize DHT sensor and attach interrupt for wind speed sensor
  dht.begin();
  attachInterrupt(digitalPinToInterrupt(WIND_SENSOR_PIN), pulseCounter, FALLING);

  // Upload data if WiFi is connected
  if (WiFi.status() == WL_CONNECTED) {
    uploadData();
  } else {
    Serial.println("WiFi Disconnected");
  }

  Serial.println("------------------------------------------------------------------------------");
  delay(1000);

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

void connectToWiFi() {
  Serial.println("Connecting to WiFi");

  // Begin WiFi connection
  WiFi.begin(ssid, password);
  int attempts = 0;

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  Serial.println("");
}

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

void readSensor(int pin, const char* sensorName, int& sensorValue) {
  // Read sensor value
  pinMode(pin, INPUT);
  sensorValue = analogRead(pin);

  // Print sensor information
  Serial.println(sensorName + String(" Value: ") + String(sensorValue));
  Serial.println("------------------------------------------------------------------------------");
}

void pulseCounter() {
  // Increment pulse count on each interrupt
  pulseCount++;
}

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

void uploadData() {
  // Read sensor data
  readDHT();
  readSensor(LDR_PIN, "LDR", ldrValue);
  readSensor(WATER_LEVEL_PIN, "Water Level", waterLevelValue);
  readSensor(SOIL_MOISTURE_PIN, "Soil Moisture", soilMoistureValue);
  readWindSpeed();

  // Construct data for HTTP POST request
  String sensorData = "api_key=" + PROJECT_API_KEY +
                      "&light_intensity=" + String(ldrValue) +
                      "&temperature=" + String(dht.readTemperature()) +
                      "&humidity=" + String(dht.readHumidity()) +
                      "&rain=" + String(waterLevelValue) +
                      "&wind=" + String(windSpeed) +
                      "&soil_moisture=" + String(soilMoistureValue) +
                      "&battery_percentage=" + String(101) +
                      "&battery_status=" + String("Charging");

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
  Serial.println(httpResponseCode);

  // Free resources
  http.end();
}
