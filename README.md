    ESP32 AWS S3 OTA Update LibraryAn Arduino library for ESP32 to handle Over-The-Air (OTA) firmware updates from a secure AWS S3 URL.This library fetches a JSON manifest file from an API endpoint (e.g., AWS API Gateway) to check for new firmware versions. If a new version is found, it downloads the binary from the S3 URL provided in the manifest and performs the update.FeaturesChecks a JSON manifest for new versions.Downloads firmware from a secure https:// S3 URL.Uses AWS Root CA for server validation.Provides a non-blocking FreeRTOS task to check for updates on boot.Uses callbacks to decouple the library from application-specific logic (like controlling LEDs or suspending other tasks).Configurable retries, timeouts, and debug logging.Manifest File FormatThe library expects your API endpoint to return a JSON file in the following format:{
  "version": "1.0.1",
  "url": "[https://your-s3-bucket.s3.amazonaws.com/firmware.bin?AWSAccessKeyId=](https://your-s3-bucket.s3.amazonaws.com/firmware.bin?AWSAccessKeyId=)..."
}
version: The latest firmware version string.url: The full, pre-signed HTTPS URL to the firmware binary in S3.InstallationDownload this library as a ZIP file.In the Arduino IDE, go to Sketch -> Include Library -> Add .ZIP Library...Select the ZIP file you downloaded.Alternatively, create a folder named AwsS3Ota in your Arduino/libraries directory and copy the AwsS3Ota.h, AwsS3Ota.cpp, library.properties, and keywords.txt files into it.DependenciesThis library depends on:WiFiHTTPClientWiFiClientSecureArduinoJsonYou will need to install ArduinoJson from the Arduino Library Manager.Basic UsageSee the examples/BasicOta/BasicOta.ino file for a complete example.#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "AwsS3Ota.h"

// --- Your Project Settings ---
#define FIRMWARE_VERSION "1.0.0"

// WiFi
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// OTA API
const char* OTA_API_BASE_URL = "[https://your-api-id.execute-api.us-east-1.amazonaws.com](https://your-api-id.execute-api.us-east-1.amazonaws.com)";
const char* OTA_FIRMWARE_ENDPOINT = "/prod/firmware"; // e.g., /prod/firmware

// AWS Root CA Certificate (for S3)
const char* AWS_ROOT_CA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ik3szjEGiTANBgkqhkiG9w0BAQsF\n" \
// ... (rest of your certificate)
"-----END CERTIFICATE-----\n";

// --- Task Handles (Example) ---
TaskHandle_t g_mainLoopTaskHandle = NULL;
TaskHandle_t g_anotherTaskHandle = NULL;

// --- System Flags (Example) ---
bool g_isConfigPortalActive = false;

// --- LED Pins (Example) ---
#define LED_PIN_R 25
#define LED_PIN_G 26
#define LED_PIN_B 27

// --- Create Library Instance ---
AwsS3Ota g_otaUpdater;

// ---- 1. Implement Callback Functions ----

// Called by the library to check if an OTA is allowed to start.
bool cbCheckOtaStart() {
  if (g_isConfigPortalActive) {
    Serial.println("[MAIN] Config portal is active, denying OTA check.");
    return false; // Deny
  }
  // Add other checks here (e.g., battery level > 20%)
  return true; // Allow
}

// Called by the library to suspend your application tasks.
bool cbSuspendTasks() {
  Serial.println("[MAIN] Suspending tasks for OTA...");
  if (g_mainLoopTaskHandle) vTaskSuspend(g_mainLoopTaskHandle);
  if (g_anotherTaskHandle) vTaskSuspend(g_anotherTaskHandle);
  // Disconnect MQTT, etc.
  return true; // Report success
}

// Called by the library to resume your application tasks if OTA fails.
bool cbResumeTasks() {
  Serial.println("[MAIN] Resuming tasks after OTA failure.");
  if (g_mainLoopTaskHandle) vTaskResume(g_mainLoopTaskHandle);
  if (g_anotherTaskHandle) vTaskResume(g_anotherTaskHandle);
  // Reconnect MQTT, etc.
  return true; // Report success
}

// Called when OTA starts (tasks are suspended)
void cbOtaStarted() {
  Serial.println("[MAIN] OTA Started. Setting LED to CYAN.");
  // Set your LED to indicate download
  // e.g., analogWrite(LED_PIN_B, 255);
}

// Called on download progress
void cbOtaProgress() {
  // This is called many times. Blink an LED.
  // e.g., digitalWrite(LED_PIN_B, !digitalRead(LED_PIN_B));
}

// Called if the update fails
void cbOtaFailed(const char* reason) {
  Serial.printf("[MAIN] OTA Failed. Reason: %s. Setting LED to RED.\n", reason);
  // e.g., analogWrite(LED_PIN_R, 255);
}

// Called if update succeeds (before restart)
void cbOtaFinished() {
  Serial.println("[MAIN] OTA Success. Setting LED to BLUE.");
  // e.g., analogWrite(LED_PIN_B, 255);
}

// Called if firmware is already up-to-date
void cbOtaUpToDate() {
  Serial.println("[MAIN] Firmware is up to date. Setting LED to GREEN.");
  // e.g., analogWrite(LED_PIN_G, 255);
}


void setup() {
  Serial.begin(115200);
  Serial.println("\n\nBooting...");

  // Get handle for this task (the loop)
  g_mainLoopTaskHandle = xTaskGetCurrentTaskHandle();
  
  // TODO: Setup your LEDs
  // pinMode(LED_PIN_R, OUTPUT);
  // ...

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected.");

  // ---- 2. Configure the OTA Library ----
  g_otaUpdater.setDebug(true);
  g_otaUpdater.setStartupDelay(5000); // 5-second delay after boot

  // Register all your callback functions
  g_otaUpdater.onCheckStart(cbCheckOtaStart);
  g_otaUpdater.onSuspendTasks(cbSuspendTasks);
  g_otaUpdater.onResumeTasks(cbResumeTasks);
  g_otaUpdater.onOtaStarted(cbOtaStarted);
  g_otaUpdater.onOtaProgress(cbOtaProgress);
  g_otaUpdater.onOtaFailed(cbOtaFailed);
  g_otaUpdater.onOtaFinished(cbOtaFinished);
  g_otaUpdater.onOtaUpToDate(cbOtaUpToDate);

  // Initialize the library
  g_otaUpdater.begin(OTA_API_BASE_URL, OTA_FIRMWARE_ENDPOINT, FIRMWARE_VERSION, AWS_ROOT_CA);

  // ---- 3. Start the boot-time check ----
  // This will run on its own core and check for updates after the startup delay.
  g_otaUpdater.startBootCheckTask();

  Serial.println("Setup complete. Main loop is running.");
}

void loop() {
  // Your main code runs here
  Serial.println("Main loop...");
  vTaskDelay(pdMS_TO_TICKS(5000));

  // Example: Trigger a manual check
  // if (digitalRead(BUTTON_PIN) == LOW) {
  //   Serial.println("Manual OTA check triggered!");
  //   g_otaUpdater.checkAndUpdate(true); // true = user initiated
  // }
}
