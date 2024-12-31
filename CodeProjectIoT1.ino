#define BLYNK_TEMPLATE_ID "TMPL6VVXblR4X"
#define BLYNK_TEMPLATE_NAME "NODEMCU"
#define BLYNK_AUTH_TOKEN "hZCObuZfyLV2udEjN-5-Hcoc45a4ZFNn"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp8266.h>
#include "HX711.h"

// Define the pins
#define DOUT  D6
#define CLK   D7
#define RELAY_PIN D5 // GPIO pin connected to the relay module

HX711 scale;  // HX711 object for load cell

// Blynk Authentication Token
#define BLYNK_PRINT Serial
char auth[] = BLYNK_AUTH_TOKEN;

// WiFi credentials
char ssid[] = "GalaxyA13";
char pass[] = "yomom321";

// Calibration factor (you can adjust this based on your scale)
float calibration_factor = 211000; // Adjust this as needed
float weight;
float threshold = 0.5; // Set a threshold weight for lock activation (in Kg)

// Blynk virtual pin to control the lock manually
int controlSL = 0;  // Variable to store the value from V0 (unlock=1, lock=0)

// BLYNK_WRITE for Virtual Pin V0 to read the switch state
BLYNK_WRITE(V0) {
  controlSL = param.asInt(); // Update controlSL with the state of V0 (0 or 1)
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("HX711 Calibration Sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place a known weight on scale");

  // Initialize the HX711 load cell
  scale.begin(DOUT, CLK);
  scale.set_scale();
  scale.tare();  // Reset the scale to 0

  // Initialize the relay pin
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Initially, make sure relay is in the "locked" state

  // Initialize Blynk
  Blynk.begin(auth, ssid, pass);
  
  // Set up the Blynk virtual pin V0 as input (this is handled by BLYNK_WRITE)
}

void loop() {
  Blynk.run(); // Keep Blynk connected

  measureWeight();
  controlLock();
}

void measureWeight() {
  scale.set_scale(calibration_factor); // Set the calibration factor
  Serial.print("Reading: ");
  
  weight = scale.get_units(5); // Read weight in Kg

  if (weight < 0) {
    weight = 0.00;
  }

  Serial.print("Kilogram: ");
  Serial.print(weight); 
  Serial.print(" Kg");
  Serial.print(" calibration_factor: ");
  Serial.print(calibration_factor);
  Serial.println();

  // Send the weight to virtual pins in the Blynk app
  Blynk.virtualWrite(V1, weight);  // Send weight to Virtual Pin V1 (could be a gauge or graph)
}

void controlLock() {
  // If controlSL is 1, lock the solenoid regardless of weight
  if (controlSL == 1) {
    digitalWrite(RELAY_PIN, HIGH);  // Lock solenoid (relay ON)
    Serial.println("Relay ON (Lock engaged via V0 control)");
  } else {
    // Otherwise, control the lock based on weight
    if (weight >= threshold) {
      // Unlock the solenoid (deactivate relay)
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("Relay OFF (Unlock engaged due to weight)");
    } else {
      // Lock the solenoid (activate relay)
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Relay ON (Lock engaged due to weight)");
    }
  }
  delay(1000); // Delay for 1 second before the next reading
}
