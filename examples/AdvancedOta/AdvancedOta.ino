/**
 * @file AdvancedOta.ino
 * @brief Advanced ESP32 OTA Example with Manual Trigger & Callbacks
 * 
 * This example shows:
 * - Manual OTA check via button
 * - Progress callbacks for LED indication
 * - Custom task management (optional)
 */

#include <WiFi.h>
#include <AwsS3Ota.h>
#include "aws_root_ca.h"

// ===== CONFIGURATION =====
#define FIRMWARE_VERSION "1.0.0"
#define BUTTON_PIN 0  // Boot button on most ESP32 boards

const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";
const char* manifestUrl = "https://your-api-id.execute-api.us-east-1.amazonaws.com/prod/firmware";

// Optional: LED pins for visual feedback
#define LED_PIN 2  // Built-in LED on most ESP32 boards

// ===== CREATE OTA =====
AwsOta ota;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\nAdvanced OTA Example");
  Serial.printf("Firmware Version: %s\n", FIRMWARE_VERSION);
  
  // Setup hardware
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Connect WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected!");
  Serial.printf("IP: %s\n\n", WiFi.localIP().toString().c_str());
  
  // ===== SETUP OTA WITH CALLBACKS =====
  ota.begin(manifestUrl, FIRMWARE_VERSION, AWS_ROOT_CA);
  ota.setDebug(true);
  
  // Optional: Configure behavior
  ota.setAutoTaskSuspend(true);   // Automatically pause all tasks during OTA
  ota.setHttpTimeout(60);          // 60 second timeout
  ota.setMaxRetries(3);            // Retry up to 3 times
  
  // Optional: Register callbacks for visual feedback
  ota.onStart([]() {
    Serial.println(">>> OTA Started! <<<");
    digitalWrite(LED_PIN, HIGH);
  });
  
  ota.onProgress([](int percent) {
    Serial.printf(">>> OTA Progress: %d%% <<<\n", percent);
    // Blink LED during download
    digitalWrite(LED_PIN, (percent % 20) < 10);
  });
  
  ota.onComplete([]() {
    Serial.println(">>> OTA Complete! Rebooting... <<<");
    digitalWrite(LED_PIN, HIGH);
  });
  
  ota.onError([](const char* error) {
    Serial.printf(">>> OTA Error: %s <<<\n", error);
    digitalWrite(LED_PIN, LOW);
  });
  
  ota.onNoUpdate([]() {
    Serial.println(">>> Firmware is up-to-date <<<");
    digitalWrite(LED_PIN, LOW);
  });
  
  // Check on boot (after 5 seconds)
  ota.checkOnBoot(5);
  
  Serial.println("==================================");
  Serial.println("Setup complete!");
  Serial.println("Press BOOT button for manual OTA check");
  Serial.println("==================================\n");
}

void loop() {
  // Your application code
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat > 15000) {
    Serial.printf("[%lu s] Running... Heap: %d bytes\n", 
                  millis() / 1000, ESP.getFreeHeap());
    lastHeartbeat = millis();
  }
  
  // Manual OTA trigger via button
  static bool buttonPressed = false;
  
  if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
    buttonPressed = true;
    delay(50);  // Debounce
    
    if (digitalRead(BUTTON_PIN) == LOW) {
      Serial.println("\n>>> BUTTON PRESSED - Manual OTA Check <<<\n");
      
      bool updated = ota.checkNow();  // This blocks until complete
      
      if (!updated) {
        Serial.println("\n>>> Manual check complete - no update <<<\n");
      }
    }
  }
  
  if (digitalRead(BUTTON_PIN) == HIGH) {
    buttonPressed = false;
  }
  
  delay(100);
}
