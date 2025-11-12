/**
 * @file BasicOta.ino
 * @brief Example sketch for the AwsS3Ota library.
 * @note This sketch demonstrates how to set up the library and implement all the required callbacks.
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "AwsS3Ota.h" // Include your new library

// --- Your Project Settings ---
// !!! REPLACE WITH YOUR VALUES !!!
#define FIRMWARE_VERSION "1.0.0"

// WiFi
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// OTA API
// Example: "[https://abcdef123.execute-api.us-east-1.amazonaws.com](https://abcdef123.execute-api.us-east-1.amazonaws.com)"
const char* OTA_API_BASE_URL = "[https://your-api-id.execute-api.us-east-1.amazonaws.com](https://your-api-id.execute-api.us-east-1.amazonaws.com)";
// Example: "/prod/firmware"
const char* OTA_FIRMWARE_ENDPOINT = "/prod/firmware";

// AWS Root CA Certificate (for S3)
// You can get this from: [https://www.amazontrust.com/repository/AmazonRootCA1.pem](https://www.amazontrust.com/repository/AmazonRootCA1.pem)
const char* AWS_ROOT_CA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ik3szjEGiTANBgkqhkiG9w0BAQsF\n" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvoEXnqmKrvc6\n" \
"Dwh6FsQ6+kM2ujzAUeD0HeQueryVatXHGTdp9pQgnL28fNpiUQbpWJNK6ANEYIjz\n" \
"HeM5dE95OlzmS6xJlOyjpUp2gsr0PMMaxM80L11AC9aqNEoVLpc3bBVCGYJTgwCi\n" \
"oGjwO5dyKkYxNANfYm2wYarNf6S8ZF93v2IL5AoR8UOLsHwYJelQodwBaP/GAmB+\n" \
"PJ+jrmSKbqooQQxfopDHcrfEbcQjVr9SQberCaq8beMaOb5BAgBR7Yot3LbSKibw\n" \
"MO+tLIVcGBECAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0OBBYEFIQhAEqCUVimxKP39Kxuc65KGSFcMA0GCSqGSIb3DQEBCwUA\n" \
"A4IBAQCSLpCTMiEyQgVBGefYk3IPzNOZ5KjEbmxs9S/o9muLBrhBmcccCjPoTwSj\n" \
"OmyDVEP1nGYUoZIc2sbSHcmTYrqY1KsH4S0JgE9OZIPR0xptrarU0i6mR/LbR3pL\n" \
"tGplwiQNkvAUNjGANVUY5qV8ubRHf1N26CX6sNm8SgmGrRVTgcglC8jOPZ0nkyc1\n" \
"1GTTnstdPfID0eIRmQtyhVbfTuQY2N3mYm/RHMJj+GXO66Xls3TM2q2Iq9mYmKDr\n" \
"jEa2uKZQEIQBCbF4MwfIom1UvBnvnCVwzYg2C/EGyY+LCPiJzXQfG8jEaMlt0WbN\n" \
"H/PZtRNin5WoTfiOQtvFv2/E1IY+\n" \
"-----END CERTIFICATE-----\n";

// --- Task Handles (Example) ---
// We need these to suspend/resume tasks during the update
TaskHandle_t g_mainLoopTaskHandle = NULL;
// TaskHandle_t g_anotherTaskHandle = NULL; // If you have other tasks

// --- System Flags (Example) ---
// This prevents the boot-time OTA check from running if you're in a config portal
bool g_isConfigPortalActive = false; 

// --- LED Pins (Example) ---
// Define your LED pins if you have them
// #define LED_PIN_R 25
// #define LED_PIN_G 26
// #define LED_PIN_B 27

// --- Create Library Instance ---
AwsS3Ota g_otaUpdater;

// ---- 1. Implement Callback Functions ----
// These functions are a bridge between the library and your main sketch.

// Called by the library to check if an OTA is allowed to start.
bool cbCheckOtaStart() {
  if (g_isConfigPortalActive) {
    Serial.println("[MAIN] Config portal is active, denying OTA check.");
    return false; // Deny OTA check
  }
  // Add other checks here (e.g., battery level > 20%)
  return true; // Allow OTA check
}

// Called by the library to suspend your application tasks.
bool cbSuspendTasks() {
  Serial.println("[MAIN] Suspending tasks for OTA...");
  if (g_mainLoopTaskHandle) vTaskSuspend(g_mainLoopTaskHandle);
  // if (g_anotherTaskHandle) vTaskSuspend(g_anotherTaskHandle);
  
  // Disconnect MQTT, stop sensor polling, etc.
  // ...
  
  return true; // Report success
}

// Called by the library to resume your application tasks if OTA fails.
bool cbResumeTasks() {
  Serial.println("[MAIN] Resuming tasks after OTA failure/completion.");
  if (g_mainLoopTaskHandle) vTaskResume(g_mainLoopTaskHandle);
  // if (g_anotherTaskHandle) vTaskResume(g_anotherTaskHandle);

  // Reconnect MQTT, resume sensor polling, etc.
  // ...

  return true; // Report success
}

// Called when OTA starts (tasks are suspended)
void cbOtaStarted() {
  Serial.println("[MAIN] OTA Started. Setting LED to CYAN (example).");
  // Set your LED to indicate download
  // e.g., analogWrite(LED_PIN_B, 255); analogWrite(LED_PIN_G, 255);
}

// Called on download progress
void cbOtaProgress() {
  // This is called many times during download.
  // You can blink an LED here.
  // Example:
  // static bool ledState = false;
  // ledState = !ledState;
  // digitalWrite(LED_PIN_B, ledState);
  Serial.print("."); // Just print a dot for progress
}

// Called if the update fails
void cbOtaFailed(const char* reason) {
  Serial.printf("[MAIN] OTA Failed. Reason: %s. Setting LED to RED (example).\n", reason);
  // e.g., analogWrite(LED_PIN_R, 255);
}

// Called if update succeeds (right before restart)
void cbOtaFinished() {
  Serial.println("[MAIN] OTA Success. Setting LED to BLUE (example). Restarting...");
  // e.g., analogWrite(LED_PIN_B, 255);
}

// Called if firmware is already up-to-date
void cbOtaUpToDate() {
  Serial.println("[MAIN] Firmware is already up to date. Setting LED to GREEN (example).");
  // e.g., analogWrite(LED_PIN_G, 255);
}


void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial
  
  Serial.println("\n\nBooting Sketch...");
  Serial.printf("Current Firmware Version: %s\n", FIRMWARE_VERSION);

  // Get handle for this task (the loop) so it can be suspended
  g_mainLoopTaskHandle = xTaskGetCurrentTaskHandle();
  
  // TODO: Setup your LEDs
  // pinMode(LED_PIN_R, OUTPUT);
  // pinMode(LED_PIN_G, OUTPUT);
  // pinMode(LED_PIN_B, OUTPUT);
  // Serial.println("LEDs initialized.");

  // Connect to WiFi
  Serial.printf("Connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // ---- 2. Configure the OTA Library ----
  g_otaUpdater.setDebug(true);       // Enable serial logging
  g_otaUpdater.setStartupDelay(5000); // 5-second delay after boot before checking
  g_otaUpdater.setMaxRetries(3);

  // Register all your callback functions
  g_otaUpdater.onCheckStart(cbCheckOtaStart);
  g_otaUpdater.onSuspendTasks(cbSuspendTasks);
  g_otaUpdater.onResumeTasks(cbResumeTasks);
  g_otaUpdater.onOtaStarted(cbOtaStarted);
  g_otaUpdater.onOtaProgress(cbOtaProgress);
  g_otaUpdater.onOtaFailed(cbOtaFailed);
  g_otaUpdater.onOtaFinished(cbOtaFinished);
  g_otaUpdater.onOtaUpToDate(cbOtaUpToDate);

  // Initialize the library with your specific endpoints and version
  g_otaUpdater.begin(OTA_API_BASE_URL, OTA_FIRMWARE_ENDPOINT, FIRMWARE_VERSION, AWS_ROOT_CA);

  // ---- 3. Start the boot-time check ----
  // This will run on its own core and check for updates after the startup delay.
  // It will use the callbacks you provided.
  g_otaUpdater.startBootCheckTask();

  Serial.println("Setup complete. Main loop is running.");
}

void loop() {
  // Your main application code runs here
  Serial.println("Main loop is alive...");
  vTaskDelay(pdMS_TO_TICKS(5000));

  // Example: Trigger a manual check on-demand
  // (You could tie this to a button press or MQTT command)
  /*
  static bool manualCheckDone = false;
  if (millis() > 60000 && !manualCheckDone) { // After 1 minute, do a manual check
     manualCheckDone = true;
     Serial.println("Triggering manual OTA check...");
     
     // This is a blocking call, but it uses your callbacks
     // to suspend/resume this loop task.
     bool updated = g_otaUpdater.checkAndUpdate(true); // true = user initiated
     if (updated) {
       // This code will likely not be reached, as the device will restart.
     } else {
       Serial.println("Manual check finished, no update applied.");
     }
  }
  */
}