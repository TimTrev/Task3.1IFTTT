// Including the neccessary libraries
#include <Wire.h>
#include <BH1750FVI.h>
#include <WiFiNINA.h>
#include <WiFiClient.h>
#include "secrets.h"

// Constants for sunlight detection and sensor delay
const float SUNLIGHT_THRESHOLD = 300.0;  // Threshold for sunlight detection
const unsigned long SENSOR_DELAY = 5000; // Delay between sensor readings (5 seconds)

// I2C and sensor initialization
#define GY30_ADDRESS 0x23
BH1750FVI lightMeter(BH1750FVI::k_DevModeContLowRes);

WiFiClient client; // Creating a WiFi client to handle HTTP requests

bool sunlightDetected = false;

void setup() {
  Serial.begin(9600);
  
  // Initialize I2C communication and light sensor
  Wire.begin();
  lightMeter.begin();
  
  // Connect to Wi-Fi by calling the function
  connectToWiFi();

  Serial.println("Setup complete. Monitoring sunlight...");
}

void loop() { // Loop used to read the light intensity and print details
  float lux = lightMeter.GetLightIntensity();
  Serial.print("Light level: ");
  Serial.print(lux);
  Serial.println(" lx");

  if (lux > SUNLIGHT_THRESHOLD && !sunlightDetected) { 
    // Sunlight detected, trigger 'begin' notification
    sunlightDetected = true;
    triggerIFTTT("sunlight_begin");
  } 
  else if (lux <= SUNLIGHT_THRESHOLD && sunlightDetected) {
    // Sunlight stopped, trigger 'end' notification
    sunlightDetected = false;
    triggerIFTTT("sunlight_end");
  }

  delay(SENSOR_DELAY);
}

// Wi-Fi connection function
void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi...");
  while (WiFi.begin(SECRET_SSID, SECRET_PASS) != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
}

// Function to trigger IFTTT event
void triggerIFTTT(const String &event) {
  Serial.print("Triggering IFTTT event: ");
  Serial.println(event);

// Formulate the IFTTT URL to look like [  /trigger/{event}/json/with/key/{IFTTT KEY}  ]
  String url = "/trigger/" + event + "/with/key/" + IFTTT_KEY; 
  Serial.print("IFTTT URL: ");
  Serial.println(url);

  if (client.connect("maker.ifttt.com", 80)) { // Connects to IFTTT server on port 80 (HTTP)
    // Send GET request to IFTTT
    client.println("GET " + url + " HTTP/1.1");
    client.println("Host: maker.ifttt.com"); // Specifies the host
    client.println("Connection: close"); // Closes the connection after sengin the request
    client.println();

    // Wait for response from IFTTT
    while (client.connected()) { // Loop continues as long as the client is connected to WiFi
      String line = client.readStringUntil('\n'); // Read each individual line of the response
      if (line == "\r") break; // Stop reading after the end of the headers
    }

    client.stop(); // Closes the connection
    Serial.println("IFTTT event triggered."); // User notification to show the event was triggered
  } else {
    Serial.println("Failed to connect to IFTTT."); // Notify if connection failed
  }
}

