/**

 * @file BasicOta.ino

 * @brief SUPER SIMPLE ESP32 OTA from AWS S3 * @file BasicOta.ino * @file BasicOta.ino

 * 

 * This is as simple as it gets! Just 3 steps: * @brief Basic example for the AwsS3Ota library with task management * @brief Example sketch for the AwsS3Ota library.

 * 1. Configure WiFi and API

 * 2. Call ota.begin() *  * @note This sketch demonstrates how to set up the library and implement all the required callbacks.

 * 3. Call ota.checkOnBoot()

 *  * This example demonstrates: */

 * That's it! The library handles everything else automatically.

*/

#include <WiFi.h> * - OTA updater configuration#include <WiFi.h>

#include <AwsS3Ota.h>

#include "aws_root_ca.h"  // AWS certificate (in same folder) * - Complete callback implementation#include <WiFiClientSecure.h>



// ===== YOUR CONFIGURATION ===== * - FreeRTOS task suspend/resume#include "AwsS3Ota.h" // Include your new library

#define FIRMWARE_VERSION "1.0.0"

 * - Automatic boot-time update check

const char* ssid = "YourWiFiSSID";

const char* password = "YourWiFiPassword"; * // --- Your Project Settings ---



// Your API Gateway manifest URL (full URL including endpoint) * This is a good starting point for most applications.// !!! REPLACE WITH YOUR VALUES !!!

const char* manifestUrl = "https://your-api-id.execute-api.us-east-1.amazonaws.com/prod/firmware";

 */#define FIRMWARE_VERSION "1.0.0"

// ===== CREATE OTA INSTANCE =====

AwsS3Ota ota;



void setup() {#include <WiFi.h>// WiFi

  Serial.begin(115200);

  delay(1000);#include <AwsS3Ota.h>const char* WIFI_SSID = "YOUR_WIFI_SSID";

  

  Serial.println("\n\nESP32 AWS S3 OTA Example");const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

  Serial.printf("Firmware Version: %s\n\n", FIRMWARE_VERSION);

// ===== CONFIGURATION - EDIT THESE VALUES =====

  // Connect to WiFi

  Serial.printf("Connecting to %s...", ssid);// OTA API

  WiFi.begin(ssid, password);

  #define FIRMWARE_VERSION "1.0.0"// Example: "[https://abcdef123.execute-api.us-east-1.amazonaws.com](https://abcdef123.execute-api.us-east-1.amazonaws.com)"

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);const char* OTA_API_BASE_URL = "[https://your-api-id.execute-api.us-east-1.amazonaws.com](https://your-api-id.execute-api.us-east-1.amazonaws.com)";

    Serial.print(".");

  }// WiFi Credentials// Example: "/prod/firmware"

  

  Serial.println("\nWiFi connected!");const char* WIFI_SSID = "YourWiFiSSID";const char* OTA_FIRMWARE_ENDPOINT = "/prod/firmware";

  Serial.printf("IP: %s\n\n", WiFi.localIP().toString().c_str());

const char* WIFI_PASSWORD = "YourWiFiPassword";

  // ===== SETUP OTA (Just 2 lines!) =====

  ota.begin(manifestUrl, FIRMWARE_VERSION, AWS_ROOT_CA);// AWS Root CA Certificate (for S3)

  ota.checkOnBoot(5);  // Check for updates 5 seconds after boot

  // AWS API Gateway Configuration// You can get this from: [https://www.amazontrust.com/repository/AmazonRootCA1.pem](https://www.amazontrust.com/repository/AmazonRootCA1.pem)

  // Optional: Enable debug messages

  ota.setDebug(true);const char* API_BASE_URL = "https://your-api-id.execute-api.us-east-1.amazonaws.com";const char* AWS_ROOT_CA = \

  

  Serial.println("Setup complete! OTA will check in 5 seconds...\n");const char* API_ENDPOINT = "/prod/firmware";"-----BEGIN CERTIFICATE-----\n" \

}

"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ik3szjEGiTANBgkqhkiG9w0BAQsF\n" \

void loop() {

  // Your normal code here - OTA happens automatically!// AWS Root CA Certificate"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \

  

  static unsigned long lastPrint = 0;// Download from: https://www.amazontrust.com/repository/AmazonRootCA1.pem"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \

  if (millis() - lastPrint > 10000) {

    Serial.printf("[%lu s] App running - Free heap: %d bytes\n", const char* AWS_ROOT_CA = \"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \

                  millis() / 1000, ESP.getFreeHeap());

    lastPrint = millis();"-----BEGIN CERTIFICATE-----\n" \"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \

  }

  "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ik3szjEGiTANBgkqhkiG9w0BAQsF\n" \"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvoEXnqmKrvc6\n" \

  delay(100);

}"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \"Dwh6FsQ6+kM2ujzAUeD0HeQueryVatXHGTdp9pQgnL28fNpiUQbpWJNK6ANEYIjz\n" \


"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \"HeM5dE95OlzmS6xJlOyjpUp2gsr0PMMaxM80L11AC9aqNEoVLpc3bBVCGYJTgwCi\n" \

"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \"oGjwO5dyKkYxNANfYm2wYarNf6S8ZF93v2IL5AoR8UOLsHwYJelQodwBaP/GAmB+\n" \

"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \"PJ+jrmSKbqooQQxfopDHcrfEbcQjVr9SQberCaq8beMaOb5BAgBR7Yot3LbSKibw\n" \

"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvoEXnqmKrvc6\n" \"MO+tLIVcGBECAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \

"Dwh6FsQ6+kM2ujzAUeD0HeQueryVatXHGTdp9pQgnL28fNpiUQbpWJNK6ANEYIjz\n" \"AYYwHQYDVR0OBBYEFIQhAEqCUVimxKP39Kxuc65KGSFcMA0GCSqGSIb3DQEBCwUA\n" \

"HeM5dE95OlzmS6xJlOyjpUp2gsr0PMMaxM80L11AC9aqNEoVLpc3bBVCGYJTgwCi\n" \"A4IBAQCSLpCTMiEyQgVBGefYk3IPzNOZ5KjEbmxs9S/o9muLBrhBmcccCjPoTwSj\n" \

"oGjwO5dyKkYxNANfYm2wYarNf6S8ZF93v2IL5AoR8UOLsHwYJelQodwBaP/GAmB+\n" \"OmyDVEP1nGYUoZIc2sbSHcmTYrqY1KsH4S0JgE9OZIPR0xptrarU0i6mR/LbR3pL\n" \

"PJ+jrmSKbqooQQxfopDHcrfEbcQjVr9SQberCaq8beMaOb5BAgBR7Yot3LbSKibw\n" \"tGplwiQNkvAUNjGANVUY5qV8ubRHf1N26CX6sNm8SgmGrRVTgcglC8jOPZ0nkyc1\n" \

"MO+tLIVcGBECAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \"1GTTnstdPfID0eIRmQtyhVbfTuQY2N3mYm/RHMJj+GXO66Xls3TM2q2Iq9mYmKDr\n" \

"AYYwHQYDVR0OBBYEFIQhAEqCUVimxKP39Kxuc65KGSFcMA0GCSqGSIb3DQEBCwUA\n" \"jEa2uKZQEIQBCbF4MwfIom1UvBnvnCVwzYg2C/EGyY+LCPiJzXQfG8jEaMlt0WbN\n" \

"A4IBAQCSLpCTMiEyQgVBGefYk3IPzNOZ5KjEbmxs9S/o9muLBrhBmcccCjPoTwSj\n" \"H/PZtRNin5WoTfiOQtvFv2/E1IY+\n" \

"OmyDVEP1nGYUoZIc2sbSHcmTYrqY1KsH4S0JgE9OZIPR0xptrarU0i6mR/LbR3pL\n" \"-----END CERTIFICATE-----\n";

"tGplwiQNkvAUNjGANVUY5qV8ubRHf1N26CX6sNm8SgmGrRVTgcglC8jOPZ0nkyc1\n" \

"1GTTnstdPfID0eIRmQtyhVbfTuQY2N3mYm/RHMJj+GXO66Xls3TM2q2Iq9mYmKDr\n" \// --- Task Handles (Example) ---

"jEa2uKZQEIQBCbF4MwfIom1UvBnvnCVwzYg2C/EGyY+LCPiJzXQfG8jEaMlt0WbN\n" \// We need these to suspend/resume tasks during the update

"H/PZtRNin5WoTfiOQtvFv2/E1IY+\n" \TaskHandle_t g_mainLoopTaskHandle = NULL;

"-----END CERTIFICATE-----\n";// TaskHandle_t g_anotherTaskHandle = NULL; // If you have other tasks



// ===== END CONFIGURATION =====// --- System Flags (Example) ---

// This prevents the boot-time OTA check from running if you're in a config portal

// Global variablesbool g_isConfigPortalActive = false; 

AwsS3Ota otaUpdater;

TaskHandle_t mainTaskHandle = NULL;// --- LED Pins (Example) ---

TaskHandle_t workerTaskHandle = NULL;// Define your LED pins if you have them

bool configPortalActive = false;  // Example flag to block OTA// #define LED_PIN_R 25

// #define LED_PIN_G 26

// ===== APPLICATION TASKS =====// #define LED_PIN_B 27



// Example worker task that gets suspended during OTA// --- Create Library Instance ---

void workerTask(void* parameter) {AwsS3Ota g_otaUpdater;

  Serial.println("[WORKER TASK] Started");

  // ---- 1. Implement Callback Functions ----

  while (true) {// These functions are a bridge between the library and your main sketch.

    // Your application logic here

    Serial.println("[WORKER TASK] Processing...");// Called by the library to check if an OTA is allowed to start.

    vTaskDelay(pdMS_TO_TICKS(3000));bool cbCheckOtaStart() {

  }  if (g_isConfigPortalActive) {

}    Serial.println("[MAIN] Config portal is active, denying OTA check.");

    return false; // Deny OTA check

// ===== OTA CALLBACK FUNCTIONS =====  }

  // Add other checks here (e.g., battery level > 20%)

/**  return true; // Allow OTA check

 * Check if OTA is allowed to start}

 * Return false to deny OTA

 */// Called by the library to suspend your application tasks.

bool cbCheckOtaStart() {bool cbSuspendTasks() {

  Serial.println("[CALLBACK] Checking if OTA is allowed...");  Serial.println("[MAIN] Suspending tasks for OTA...");

    if (g_mainLoopTaskHandle) vTaskSuspend(g_mainLoopTaskHandle);

  // Example: Don't allow OTA if config portal is active  // if (g_anotherTaskHandle) vTaskSuspend(g_anotherTaskHandle);

  if (configPortalActive) {  

    Serial.println("[CALLBACK] OTA denied: Config portal is active");  // Disconnect MQTT, stop sensor polling, etc.

    return false;  // ...

  }  

    return true; // Report success

  // Example: Check free heap}

  if (ESP.getFreeHeap() < 40000) {

    Serial.printf("[CALLBACK] OTA denied: Low memory (%d bytes)\n", ESP.getFreeHeap());// Called by the library to resume your application tasks if OTA fails.

    return false;bool cbResumeTasks() {

  }  Serial.println("[MAIN] Resuming tasks after OTA failure/completion.");

    if (g_mainLoopTaskHandle) vTaskResume(g_mainLoopTaskHandle);

  Serial.println("[CALLBACK] OTA allowed");  // if (g_anotherTaskHandle) vTaskResume(g_anotherTaskHandle);

  return true;

}  // Reconnect MQTT, resume sensor polling, etc.

  // ...

/**

 * Suspend application tasks before OTA  return true; // Report success

 */}

bool cbSuspendTasks() {

  Serial.println("[CALLBACK] Suspending application tasks...");// Called when OTA starts (tasks are suspended)

  void cbOtaStarted() {

  // Suspend worker task  Serial.println("[MAIN] OTA Started. Setting LED to CYAN (example).");

  if (workerTaskHandle != NULL) {  // Set your LED to indicate download

    vTaskSuspend(workerTaskHandle);  // e.g., analogWrite(LED_PIN_B, 255); analogWrite(LED_PIN_G, 255);

    Serial.println("[CALLBACK]   • Worker task suspended");}

  }

  // Called on download progress

  // Suspend main loop taskvoid cbOtaProgress() {

  if (mainTaskHandle != NULL) {  // This is called many times during download.

    vTaskSuspend(mainTaskHandle);  // You can blink an LED here.

    Serial.println("[CALLBACK]   • Main task suspended");  // Example:

  }  // static bool ledState = false;

    // ledState = !ledState;

  // Add other cleanup here (disconnect MQTT, stop sensors, etc.)  // digitalWrite(LED_PIN_B, ledState);

    Serial.print("."); // Just print a dot for progress

  return true;}

}

// Called if the update fails

/**void cbOtaFailed(const char* reason) {

 * Resume application tasks if OTA fails  Serial.printf("[MAIN] OTA Failed. Reason: %s. Setting LED to RED (example).\n", reason);

 */  // e.g., analogWrite(LED_PIN_R, 255);

bool cbResumeTasks() {}

  Serial.println("[CALLBACK] Resuming application tasks...");

  // Called if update succeeds (right before restart)

  // Resume main loop taskvoid cbOtaFinished() {

  if (mainTaskHandle != NULL) {  Serial.println("[MAIN] OTA Success. Setting LED to BLUE (example). Restarting...");

    vTaskResume(mainTaskHandle);  // e.g., analogWrite(LED_PIN_B, 255);

    Serial.println("[CALLBACK]   • Main task resumed");}

  }

  // Called if firmware is already up-to-date

  // Resume worker taskvoid cbOtaUpToDate() {

  if (workerTaskHandle != NULL) {  Serial.println("[MAIN] Firmware is already up to date. Setting LED to GREEN (example).");

    vTaskResume(workerTaskHandle);  // e.g., analogWrite(LED_PIN_G, 255);

    Serial.println("[CALLBACK]   • Worker task resumed");}

  }

  

  // Add other restoration here (reconnect MQTT, restart sensors, etc.)void setup() {

    Serial.begin(115200);

  return true;  while (!Serial); // Wait for serial

}  

  Serial.println("\n\nBooting Sketch...");

/**  Serial.printf("Current Firmware Version: %s\n", FIRMWARE_VERSION);

 * Called when OTA download starts

 */  // Get handle for this task (the loop) so it can be suspended

void cbOtaStarted() {  g_mainLoopTaskHandle = xTaskGetCurrentTaskHandle();

  Serial.println("\n========================================");  

  Serial.println("🚀 OTA UPDATE STARTED");  // TODO: Setup your LEDs

  Serial.println("========================================");  // pinMode(LED_PIN_R, OUTPUT);

  // You could set an LED to cyan/blue here  // pinMode(LED_PIN_G, OUTPUT);

}  // pinMode(LED_PIN_B, OUTPUT);

  // Serial.println("LEDs initialized.");

/**

 * Called during download (frequently)  // Connect to WiFi

 */  Serial.printf("Connecting to %s ", WIFI_SSID);

void cbOtaProgress() {  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Just print dots for progress  while (WiFi.status() != WL_CONNECTED) {

  static unsigned long lastPrint = 0;    delay(500);

  if (millis() - lastPrint > 2000) {  // Every 2 seconds    Serial.print(".");

    Serial.print(".");  }

    lastPrint = millis();  Serial.println("\nWiFi Connected.");

  }  Serial.print("IP Address: ");

}  Serial.println(WiFi.localIP());



/**  // ---- 2. Configure the OTA Library ----

 * Called when OTA succeeds (before reboot)  g_otaUpdater.setDebug(true);       // Enable serial logging

 */  g_otaUpdater.setStartupDelay(5000); // 5-second delay after boot before checking

void cbOtaFinished() {  g_otaUpdater.setMaxRetries(3);

  Serial.println("\n========================================");

  Serial.println("✅ OTA UPDATE COMPLETE");  // Register all your callback functions

  Serial.println("Device will reboot now...");  g_otaUpdater.onCheckStart(cbCheckOtaStart);

  Serial.println("========================================");  g_otaUpdater.onSuspendTasks(cbSuspendTasks);

  // You could set an LED to green here  g_otaUpdater.onResumeTasks(cbResumeTasks);

}  g_otaUpdater.onOtaStarted(cbOtaStarted);

  g_otaUpdater.onOtaProgress(cbOtaProgress);

/**  g_otaUpdater.onOtaFailed(cbOtaFailed);

 * Called when OTA fails  g_otaUpdater.onOtaFinished(cbOtaFinished);

 */  g_otaUpdater.onOtaUpToDate(cbOtaUpToDate);

void cbOtaFailed(const char* reason) {

  Serial.println("\n========================================");  // Initialize the library with your specific endpoints and version

  Serial.printf("❌ OTA UPDATE FAILED: %s\n", reason);  g_otaUpdater.begin(OTA_API_BASE_URL, OTA_FIRMWARE_ENDPOINT, FIRMWARE_VERSION, AWS_ROOT_CA);

  Serial.println("========================================");

  // You could set an LED to red here  // ---- 3. Start the boot-time check ----

}  // This will run on its own core and check for updates after the startup delay.

  // It will use the callbacks you provided.

/**  g_otaUpdater.startBootCheckTask();

 * Called when firmware is already up to date

 */  Serial.println("Setup complete. Main loop is running.");

void cbOtaUpToDate() {}

  Serial.println("\n========================================");

  Serial.println("✅ FIRMWARE UP TO DATE");void loop() {

  Serial.println("========================================");  // Your main application code runs here

  // You could blink an LED green here  Serial.println("Main loop is alive...");

}  vTaskDelay(pdMS_TO_TICKS(5000));



// ===== SETUP =====  // Example: Trigger a manual check on-demand

  // (You could tie this to a button press or MQTT command)

void setup() {  /*

  // Initialize Serial  static bool manualCheckDone = false;

  Serial.begin(115200);  if (millis() > 60000 && !manualCheckDone) { // After 1 minute, do a manual check

  delay(1000);     manualCheckDone = true;

       Serial.println("Triggering manual OTA check...");

  Serial.println("\n\n====================================");     

  Serial.println("  ESP32 AWS S3 OTA - Basic Example");     // This is a blocking call, but it uses your callbacks

  Serial.println("====================================");     // to suspend/resume this loop task.

  Serial.printf("Firmware Version: %s\n", FIRMWARE_VERSION);     bool updated = g_otaUpdater.checkAndUpdate(true); // true = user initiated

  Serial.printf("ESP32 Chip: %s\n", ESP.getChipModel());     if (updated) {

  Serial.printf("Free Heap: %d bytes\n\n", ESP.getFreeHeap());       // This code will likely not be reached, as the device will restart.

     } else {

  // Get handle to main loop task (this task)       Serial.println("Manual check finished, no update applied.");

  mainTaskHandle = xTaskGetCurrentTaskHandle();     }

  }

  // Connect to WiFi  */

  Serial.printf("Connecting to WiFi: %s ", WIFI_SSID);}
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n✗ WiFi connection failed!");
    Serial.println("Please check credentials and try again.");
    while (1) { delay(1000); }  // Halt
  }
  
  Serial.println("\n✓ WiFi Connected");
  Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Signal Strength: %d dBm\n\n", WiFi.RSSI());

  // Configure OTA updater
  Serial.println("Configuring OTA updater...");
  
  otaUpdater.begin(API_BASE_URL, API_ENDPOINT, FIRMWARE_VERSION, AWS_ROOT_CA);
  otaUpdater.setDebug(true);           // Enable verbose logging
  otaUpdater.setStartupDelay(8000);    // Wait 8 seconds before boot check
  otaUpdater.setMaxRetries(3);         // Retry up to 3 times

  // Register required callbacks
  otaUpdater.onCheckStart(cbCheckOtaStart);
  otaUpdater.onSuspendTasks(cbSuspendTasks);
  otaUpdater.onResumeTasks(cbResumeTasks);

  // Register optional callbacks
  otaUpdater.onOtaStarted(cbOtaStarted);
  otaUpdater.onOtaProgress(cbOtaProgress);
  otaUpdater.onOtaFinished(cbOtaFinished);
  otaUpdater.onOtaFailed(cbOtaFailed);
  otaUpdater.onOtaUpToDate(cbOtaUpToDate);

  Serial.println("✓ OTA configured\n");

  // Start worker task (example application task)
  xTaskCreate(
    workerTask,
    "workerTask",
    4096,
    NULL,
    1,
    &workerTaskHandle
  );
  Serial.println("✓ Worker task started\n");

  // Start automatic boot-time OTA check (non-blocking)
  Serial.println("Starting boot-time OTA check (in 8 seconds)...");
  otaUpdater.startBootCheckTask();

  Serial.println("\n====================================");
  Serial.println("Setup complete! Main loop running.");
  Serial.println("====================================\n");
}

// ===== MAIN LOOP =====

void loop() {
  // Your main application code goes here
  
  static unsigned long lastHeartbeat = 0;
  
  if (millis() - lastHeartbeat > 10000) {  // Every 10 seconds
    Serial.printf("[MAIN] Heartbeat - Uptime: %lu s, Free heap: %d bytes\n", 
                  millis() / 1000, 
                  ESP.getFreeHeap());
    lastHeartbeat = millis();
  }
  
  // Example: You could trigger manual OTA check here
  // if (someCondition) {
  //   Serial.println("\n[MAIN] Triggering manual OTA check...");
  //   bool updated = otaUpdater.checkAndUpdate(true);  // Blocking call
  //   if (!updated) {
  //     Serial.println("[MAIN] No update available or failed");
  //   }
  // }
  
  delay(100);
}
