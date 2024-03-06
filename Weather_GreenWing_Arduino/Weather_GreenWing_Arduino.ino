#include <Wire.h>
#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <esp_adc_cal.h>
#include <ArduinoOTA.h>
#include <Preferences.h>
#include <Arduino.h>
#include <freertos/semphr.h>

// Constants
#define DEFAULT_VREF 1100       // Default VREF value for ADC calibration
#define uS_TO_S_FACTOR 1000000  // Conversion from microseconds to seconds
#define TIME_TO_SLEEP 111     // Time to sleep in seconds

SemaphoreHandle_t syncSemaphore;

// Pin Definitions
#define MOS_PIN 26
#define DHT_PIN 25
#define LDR_PIN 33
#define WATER_LEVEL_PIN 35
#define SOIL_MOISTURE_PIN 32
#define WIND_SENSOR_PIN 34
#define CHARGING_PIN 39

// Voltage thresholds
const double MAX_VOLTAGE = 3.800;
const double MIN_VOLTAGE = 2.800;
const float VOLTAGE_THRESHOLD = 0.0008056640625;  // Define a suitable threshold

// OTA Credentials
const char* OTA_HOSTNAME = "GreenWing";
const char* OTA_PASSWORD = "20010123";

// Preferences for Wi-Fi Credentials
Preferences preferences;

// DHT Sensor
DHT dht(DHT_PIN, DHT11);

// Wind Sensor Variables
volatile unsigned long pulseCount = 0;
volatile unsigned long lastPulseTime = 0;
float windSpeed;
const float CONVERSION_FACTOR = 0.4;

// Boot Counter
RTC_DATA_ATTR int bootCount = 0;

// Global variable declarations
int ldrValue = 0;
int waterLevelValue = 0;
int soilMoistureValue = 0;
float rain = 0;
int inBuiltHallSensor = 0;
String PROJECT_API_KEY = "GreenWing";
// const char* SERVER_NAME = "http://localhost/greenwing/sensordata.php";  // Replace with your actual server name
const char* SERVER_NAME = "http://green-wing.scienceontheweb.net/sensordata.php";
esp_adc_cal_characteristics_t* adc_chars;
double batteryVoltage;
float batteryPercentage = 0.0;
String batteryStatus = "Unknown";
bool uploadEnabled = false;
bool loggingEnabled = false;       // Variable to track logging state
const int BUCKET_VOLUME_ML = 8;  // Volume of each bucket in milliliters

void Core0Task(void* pvParameters);
void Core1Task(void* pvParameters);
void setup();
void loop();
void connectToWiFi();
void setupOTA();
void uploadData();
void readDHT();
void readSensor(int pin, const char* sensorName, int& sensorValue, bool inverse);
void pulseCounter();
float calculateWindSpeed(unsigned long pulseCount, unsigned long timeInterval);
void readWindSpeed();
void batteryMonitor();
void goToDeepSleep();
void logSensorData();
void handleCommands();
void resetCalibration(const char* sensorName);
void checkForRecalibrationCommand();
float calculateRain();

void Core0Task(void* pvParameters) {
  connectToWiFi();
  setupOTA();

  for (;;) {
    // Handle OTA updates
    ArduinoOTA.handle();
    // Other tasks for Core 0
    xSemaphoreGive(syncSemaphore);  // Signal completion
    vTaskDelete(NULL);              // End task
    delay(100);
  }
}
void Core1Task(void* pvParameters) {
  for (;;) {
    readDHT();
    readSensor(LDR_PIN, "LDR", ldrValue, true);
    readSensor(WATER_LEVEL_PIN, "Water Level", waterLevelValue, true);
    readSensor(SOIL_MOISTURE_PIN, "Soil Moisture", soilMoistureValue, false);
    readWindSpeed();
    batteryMonitor();
    handleCommands();  // Check for user commands
    if (uploadEnabled) {
      uploadData();  // Upload sensor data if enabled
    }
    xSemaphoreGive(syncSemaphore);  // Signal completion
    vTaskDelete(NULL);              // End task
    delay(100);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 serial initialized");
  // Dual core selection
  syncSemaphore = xSemaphoreCreateBinary();
  // Check if semaphore creation was successful
  if (syncSemaphore == NULL) {
    Serial.println("Failed to create semaphore");
  }
  xTaskCreatePinnedToCore(Core0Task, "Core0Task", 10000, NULL, 1, NULL, 0);  // Task for Core 0
  xTaskCreatePinnedToCore(Core1Task, "Core1Task", 10000, NULL, 1, NULL, 1);  // Task for Core 1

  adc_chars = (esp_adc_cal_characteristics_t*)calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

  // Initialize Wi-Fi
  connectToWiFi();
  // Initialize OTA
  setupOTA();

  while (!Serial);
  Serial.println("Type 'recalibrate' for sensor recalibration, 'log' for logging data");
  checkForRecalibrationCommand();
  handleCommands();
  delay(1000);

  // Initialize a NTP client for Sri Lanka (UTC+5:30)
  // The offset is in seconds, so 5 hours and 30 minutes is 5*3600 + 30*60 = 19800 seconds
  configTime(19800, 0, "pool.ntp.org", "time.nist.gov");

  // Increment boot count and print
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  // Initialize other components...
  pinMode(MOS_PIN, OUTPUT);
  pinMode(DHT_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(WATER_LEVEL_PIN, INPUT);
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(WIND_SENSOR_PIN, INPUT);
  pinMode(CHARGING_PIN, INPUT);
  dht.begin();
  attachInterrupt(digitalPinToInterrupt(WIND_SENSOR_PIN), pulseCounter, FALLING);
  delay(1000);  // Wait for 1 second

  // Allow a short time window for OTA updates
  unsigned long startMillis = millis();
  while (millis() - startMillis < 1000) {  // 2-second window for OTA
    ArduinoOTA.handle();
    delay(100);  // Short delay to avoid blocking
  }

  // Load logging state from flash memory
  Serial.println("loggingEnabled: " + String(loggingEnabled));
  // Proceed based on logging state
  while (loggingEnabled) {
    logSensorData();
    return;
  }
  // Regular operation
  digitalWrite(MOS_PIN, HIGH);  // Turn MOSFET ON
  delay(500);                  // Wait for 1 second
  uploadData();
  Serial.println("___________________________________");
  digitalWrite(MOS_PIN, LOW);  // Turn MOSFET OFF
  delay(500);                 // Wait for 1 second
  preferences.end();
  goToDeepSleep();
}

void loop() {
    if (loggingEnabled) {
    logSensorData();
  }
}

void connectToWiFi() {
  preferences.begin("wifi", false);
  String ssid = preferences.getString("ssid", "Greenwing");
  String password = preferences.getString("password", "12345678");

  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid.c_str(), password.c_str());
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println("\nWiFi Connected.");

  preferences.end();
}

void setupOTA() {
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    Serial.println("Start updating...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd updating");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
  });

  ArduinoOTA.begin();
}
void uploadData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return;
  }

  readDHT();
  readSensor(LDR_PIN, "LDR", ldrValue, true);  // Inverse mapping for LDR
  readSensor(WATER_LEVEL_PIN, "Water Level", waterLevelValue, true);
  readSensor(SOIL_MOISTURE_PIN, "Soil Moisture", soilMoistureValue, false);
  readWindSpeed();
  batteryMonitor();
  rain = calculateRain();  // Pass the time taken for the bucket to fill
  // Construct data for HTTP POST request
  String sensorData = "api_key=" + PROJECT_API_KEY + "&light_intensity=" + String(ldrValue) + "&temperature=" + String(dht.readTemperature()) + "&humidity=" + String(dht.readHumidity()) + "&rain=" + String(rain) + "&wind=" + String(windSpeed) + "&soil_moisture=" + String(soilMoistureValue) + "&battery_percentage=" + String(batteryPercentage) + "&battery_status=" + batteryStatus;

  WiFiClient client;
  HTTPClient http;
  http.begin(client, SERVER_NAME);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(sensorData);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  if (httpResponseCode > 0) {
    // If you want to print the response content
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.println("Error on sending POST");
  }

  http.end();
}

void readDHT() {
  delay(1000);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    // Serial.println("Temperature: " + String(temperature) + "°C");
    // Serial.println("Humidity: " + String(humidity) + "%");
  }
}

void readSensor(int pin, const char* sensorName, int& sensorValue, bool inverse) {
  preferences.begin("calibration", false);  // Open NVS in read-write mode

  static int lastValue = -1;
  static int sameValueCount = 0;

  // Construct the key strings
  String minKey = String(sensorName) + "_min";
  String maxKey = String(sensorName) + "_max";

  // Load calibration data from NVS
  int minRawValue = preferences.getInt(minKey.c_str(), 4095);
  int maxRawValue = preferences.getInt(maxKey.c_str(), 0);

  pinMode(pin, INPUT);
  int rawValue = analogRead(pin);

  // Update calibration data if needed
  if (rawValue < minRawValue || rawValue > maxRawValue) {
    minRawValue = min(minRawValue, rawValue);
    maxRawValue = max(maxRawValue, rawValue);

    // Save updated calibration data to NVS
    preferences.putInt(minKey.c_str(), minRawValue);
    preferences.putInt(maxKey.c_str(), maxRawValue);
  }

  // Map the raw value to a 0-100 range based on the calibrated min-max range
  int mappedValue = 0;
  if (maxRawValue != minRawValue) {  // Prevent division by zero
    mappedValue = (rawValue - minRawValue) * 100 / (maxRawValue - minRawValue);
  }

  // Inverse the percentage if needed (for LDR)
  if (inverse) {
    mappedValue = 100 - mappedValue;
  }

  // Constrain the value to ensure it's within 0-100
  sensorValue = constrain(mappedValue, 0, 100);

  // Serial.println(String(sensorName) + " Value: " + String(sensorValue) + "%");

  if (sensorValue == lastValue) {
    sameValueCount++;
    if (sameValueCount >= 5) {
      resetCalibration(sensorName);
      sameValueCount = 0;
      Serial.println(String(sensorName) + " calibration reset.");
    }
  } else {
    sameValueCount = 0;
  }

  lastValue = sensorValue;
}
void resetCalibration(const char* sensorName) {
  preferences.begin("calibration", false);  // Open NVS in read-write mode

  // Construct the key strings for min and max values
  String minKey = String(sensorName) + "_min";
  String maxKey = String(sensorName) + "_max";

  // Reset calibration values
  preferences.putInt(minKey.c_str(), 4095);  // Set to default max ADC value
  preferences.putInt(maxKey.c_str(), 0);     // Set to default min ADC value

  preferences.end();

  Serial.println(String(sensorName) + " calibration reset.");
}
void checkForRecalibrationCommand() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');


    if (input.equalsIgnoreCase("recalibrate")) {
      resetCalibration("LDR");
      resetCalibration("Water Level");
      resetCalibration("Soil Moisture");
      // Add other sensors if necessary

      Serial.println("All sensors recalibrated.");
    }
  }
  delay(500);  // Wait for 3 seconds
}
void handleCommands() {
  if (Serial.available() > 0) {
    inBuiltHallSensor = hallRead();
    String input = Serial.readStringUntil('\n');
    if (input.equalsIgnoreCase("log")) {
          // if (input.equalsIgnoreCase("log") || inBuiltHallSensor < 0 || inBuiltHallSensor > 100) {

      loggingEnabled = true;   // Enable logginglog
    } else {
      digitalWrite(MOS_PIN, LOW);  // Turn MOSFET ON
      loggingEnabled = false;      // Disable logging
    }
  }
}

void logSensorData() {
  inBuiltHallSensor = hallRead();

  if (inBuiltHallSensor < 0 || inBuiltHallSensor > 100) {
    digitalWrite(MOS_PIN, LOW);  // Turn MOSFET OFF
    loggingEnabled = false;      // Disable logging   
    return;                      // Exit the function
  }

  // If inBuiltHallSensor is greater than or equal to zero, continue logging sensor data
  digitalWrite(MOS_PIN, HIGH);  // Turn MOSFET ON
  readDHT();
  readSensor(LDR_PIN, "LDR", ldrValue, true);  // Inverse mapping for LDR
  readSensor(SOIL_MOISTURE_PIN, "Soil Moisture", soilMoistureValue, false);
  readWindSpeed();
  batteryMonitor();
  rain = calculateRain();

  // Update inBuiltHallSensor after logging data
  inBuiltHallSensor = hallRead();

  // Print debug information
  Serial.println("\nDebug Mode: ");
  Serial.print(" | Temp: " + String(dht.readTemperature()) + "°C");
  Serial.print(" | Hum: " + String(dht.readHumidity()) + "%");
  Serial.print(" | Light: " + String(ldrValue) + "%");
  Serial.print(" | Water: " + String(rain) + "mm/h");
  Serial.print(" | Soil: " + String(soilMoistureValue) + "%");
  Serial.print(" | Wind: " + String(windSpeed) + " km/h");
  Serial.print(" | Battery: " + batteryStatus + " " + String(batteryPercentage) + "%");
  Serial.print(" | inBuiltHallEccect: " + String(inBuiltHallSensor));
  return;
}


void IRAM_ATTR pulseCounter() {
  pulseCount++;
  lastPulseTime = millis();
}

float calculateWindSpeed(unsigned long pulseCount, unsigned long timeInterval) {
  float rotations = pulseCount;
  float timeSeconds = timeInterval / 1000.0;
  return rotations / timeSeconds * CONVERSION_FACTOR;
}

void readWindSpeed() {
  unsigned long currentTime = millis();
  if (currentTime - lastPulseTime > 1000) {
    noInterrupts();
    float localPulseCount = pulseCount;
    pulseCount = 0;
    interrupts();

    windSpeed = (float)localPulseCount / 2.0 / (currentTime - lastPulseTime) * 1000.0 * 3.6;
    lastPulseTime = currentTime;

    if (!isnan(windSpeed)) {
      // Serial.println("Wind Speed: " + String(windSpeed) + " km/h");
    } else {
      Serial.println("Failed to read Wind Speed sensor!");
    }
  }
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void batteryMonitor() {
  pinMode(CHARGING_PIN, INPUT);

  // Get calibrated ADC reading
  uint32_t adc_reading = analogRead(CHARGING_PIN);
  uint32_t millivolts = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);  // Calibrated reading
  batteryVoltage = (float)millivolts / 1000.0 * 2;                           // Convert to volts -  *2 (voltage divider)

  // Calculate battery percentage
  batteryPercentage = (batteryVoltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE) * 100.000;
  batteryPercentage = constrain(batteryPercentage, 0.000, 100.000);

  // Determine battery status
  static float lastVoltage = batteryVoltage;  // Initialize with the current reading
  // Serial.print("\nCurrent Voltage: ");
  // Serial.print(batteryVoltage, 10);
  // Serial.print("V, Last Voltage: ");
  // Serial.print(lastVoltage, 10);
  // Serial.println("V");

  // Check for significant change
  if (abs(batteryVoltage - lastVoltage) > VOLTAGE_THRESHOLD) {
    batteryStatus = (batteryVoltage > lastVoltage) ? "Charging" : "Discharging";
  } else {
    batteryStatus = "Stable";
  }

  // Print battery information
  // Serial.print("\nBattery Percentage: ");
  // Serial.print(batteryPercentage);
  // Serial.println("%");
  // Serial.print("Battery Status: ");
  // Serial.println(batteryStatus);

  // Update lastVoltage for the next loop
  lastVoltage = batteryVoltage;
}

void goToDeepSleep() {
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Going to deep sleep now");
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  Serial.flush();  // Ensure all Serial data is sent before sleeping
  esp_deep_sleep_start();
}

float calculateRain() {
  unsigned long startTime = millis();  // Initialize startTime
  unsigned long endTime = 0;
  unsigned long timeTaken = 0;
  bool bucketFilled = false;
  bool previousRainData = false;
  unsigned long timeout = 1000;  // 2 seconds timeout

  // Loop until the bucket is filled (two consecutive changes in the water level)
  while (!bucketFilled && (millis() - startTime < timeout)) {
    // Read the water level sensor
    bool rainData = digitalRead(WATER_LEVEL_PIN);  // Corrected variable name to lowercase

    // Check if the water level sensor has changed
    if (rainData != previousRainData) {  // If there's a change in rain data
      if (startTime == 0) {
        startTime = millis();  // Record the start time
      } else {
        endTime = millis();               // Record the end time
        timeTaken = endTime - startTime;  // Calculate the time taken
        bucketFilled = true;              // Set the flag to exit the loop
      }
    }

    previousRainData = rainData;  // Update the previous rain data
  }

  if (timeTaken > 0.0) {  // Avoid division by zero
    // Calculate the rain in ml/s
    float rainRate = 15.0 / timeTaken;  // Corrected to use floating point division
    // Convert to mm/h (1 ml = 1 mm for water)
    float rain = rainRate * 3600.0;  // Corrected to use floating point multiplication
    return rain;
  } else {
    return 0.0;  // Return 0 if no time has elapsed (avoid division by zero)
  }
}