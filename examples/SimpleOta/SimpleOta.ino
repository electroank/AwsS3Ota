/**
 * @file SimpleOta.ino
 * @brief Minimal ESP32 OTA Example - Check every 1 hour
 * 
 * This example shows periodic OTA checking.
 * Device will check for updates every hour automatically!
 */

#include <WiFi.h>
#include <AwsS3Ota.h>
#include "aws_root_ca.h"

// ===== CONFIGURATION =====
#define FIRMWARE_VERSION "1.0.0"

const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";
const char* manifestUrl = "https://your-api-id.execute-api.us-east-1.amazonaws.com/prod/firmware";

// ===== CREATE OTA =====
AwsS3Ota ota;

void setup() {
  Serial.begin(115200);
  
  // Connect WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  Serial.println("\nWiFi connected!");
  
  // Setup OTA - check every 1 hour
  ota.begin(manifestUrl, FIRMWARE_VERSION, AWS_ROOT_CA);
  ota.checkEvery(3600000);  // 3600000 ms = 1 hour
  
  Serial.println("OTA will check every 1 hour\n");
}

void loop() {
  // Your code here
  delay(1000);
}
