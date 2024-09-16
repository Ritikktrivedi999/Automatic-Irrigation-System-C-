// Blynk template settings
#define BLYNK_TEMPLATE_ID "YOUR_ID_" 
#define BLYNK_TEMPLATE_NAME "YOUR TEMPLATE NAME"

// Include required libraries
#include <DHT.h>           // DHT sensor library for temperature and humidity
#include <ESP32Servo.h>     // Servo motor library
#include <WiFi.h>           // Wi-Fi library
#include <BlynkSimpleEsp32.h> // Blynk ESP32 library for cloud connection

// Blynk virtual pins
#define HUMIDITY_PIN V0
#define TEMPERATURE_PIN V2
#define MOTION_LED_PIN V1
#define WATERING_LED_PIN V3
#define MANUAL_SWITCH_PIN V4

// Sensor and actuator pin definitions
#define DHTPIN 4           // DHT sensor data pin
#define DHTTYPE DHT22      // Type of DHT sensor (DHT22)
#define PIR_PIN 14         // PIR sensor pin
#define SERVO_PIN 32       // Servo motor pin
#define LED_PIN 2          // Onboard LED pin

// Initialize DHT sensor and servo motor objects
DHT dht(DHTPIN, DHTTYPE);
Servo myservo;

// Wi-Fi and Blynk credentials
char auth[] = "QvAeukvraLqSZTCfENp47QBkI7y_-5Fm";  // Blynk app authentication token
char ssid[] = "Wokwi-GUEST";                        // Wi-Fi SSID
char pass[] = "";                                   // Wi-Fi password (empty for open networks)

// Global variables
bool isWatering = false;
bool lightOn = false;
bool systemOn = true;  // System starts in "ON" state

// Blynk write handler to control the system's on/off state via the manual switch
BLYNK_WRITE(MANUAL_SWITCH_PIN) {
  int switchState = param.asInt();
  if (switchState == HIGH && !systemOn) {
    turnOnSystem();  // Turn on the system
  } else if (switchState == LOW && systemOn) {
    turnOffSystem(); // Turn off the system
  }
}

void setup() {
  // Initialize serial communication at 9600 baud
  Serial.begin(9600);

  // Initialize sensors and actuators
  dht.begin();
  myservo.attach(SERVO_PIN);
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Connect to Blynk and Wi-Fi
  Blynk.begin(auth, ssid, pass);

  // Initialize Blynk widgets
  Blynk.virtualWrite(MOTION_LED_PIN, LOW);
  Blynk.virtualWrite(WATERING_LED_PIN, LOW);
  Blynk.virtualWrite(MANUAL_SWITCH_PIN, HIGH);  // System starts as ON

  // Ensure the system is in the correct state at startup
  if (!systemOn) {
    turnOffSystem();
  }
}

void loop() {
  // Process Blynk messages
  Blynk.run();

  // If system is off, skip the loop
  if (!systemOn) {
    delay(1000);  // Wait 1 second before next loop
    return;
  }

  // Read temperature and humidity from DHT sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check if sensor readings are valid
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Send temperature and humidity data to Blynk
  Blynk.virtualWrite(TEMPERATURE_PIN, temperature);
  Blynk.virtualWrite(HUMIDITY_PIN, humidity);

  // Log temperature and humidity to the serial monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C | Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  // Handle watering logic based on humidity and temperature
  handleWatering(humidity, temperature);

  // Handle motion detection logic using PIR sensor
  handleMotionDetection();

  delay(1000);  // Delay for 1 second before next loop iteration
}

// Function to handle watering logic
void handleWatering(float humidity, float temperature) {
  if (humidity < 85.0 || temperature > 30.0) {
    // Watering condition met
    Serial.println("Watering condition met. Data updated on Blynk cloud.");
    myservo.write(180);  // Move servo to 180 degrees (watering)
    Blynk.virtualWrite(WATERING_LED_PIN, HIGH); // Turn on watering LED in Blynk
  } else {
    // No need for watering
    Serial.println("Normal condition.");
    myservo.write(0);  // Reset servo to 0 degrees (default position)
    Blynk.virtualWrite(WATERING_LED_PIN, LOW); // Turn off watering LED in Blynk
  }
}

// Function to handle motion detection logic
void handleMotionDetection() {
  int pirStatus = digitalRead(PIR_PIN);  // Read PIR sensor state

  if (pirStatus == HIGH && !lightOn) {
    // Motion detected and light is off
    digitalWrite(LED_PIN, HIGH);           // Turn on onboard LED
    Blynk.virtualWrite(MOTION_LED_PIN, HIGH); // Turn on motion LED in Blynk
    lightOn = true;
  } else if (pirStatus == LOW && lightOn) {
    // No motion detected and light is on
    digitalWrite(LED_PIN, LOW);            // Turn off onboard LED
    Blynk.virtualWrite(MOTION_LED_PIN, LOW);  // Turn off motion LED in Blynk
    lightOn = false;
  }
}

// Function to turn the system ON
void turnOnSystem() {
  Serial.println("System turned ON");
  systemOn = true;
}

// Function to turn the system OFF
void turnOffSystem() {
  Serial.println("System turned OFF");
  myservo.write(0);  // Reset servo to 0 degrees
  Blynk.virtualWrite(WATERING_LED_PIN, LOW);  // Turn off watering LED in Blynk
  digitalWrite(LED_PIN, LOW);  // Turn off onboard LED
  Blynk.virtualWrite(MOTION_LED_PIN, LOW);  // Turn off motion LED in Blynk
  lightOn = false;
  systemOn = false;
}
