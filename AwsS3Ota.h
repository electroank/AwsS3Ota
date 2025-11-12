/**
 * @file AwsS3Ota.h
 * @brief Arduino library for performing Over-The-Air (OTA) updates from AWS S3
 * @author Ankan Sarkar
 * @version 1.0.0
 * @date 2025
 * @license MIT
 * 
 * @note This library is designed for all ESP32 variants including:
 *       - ESP32 (original)
 *       - ESP32-S2
 *       - ESP32-S3
 *       - ESP32-C3
 *       - ESP32-C6
 * 
 * @note Uses FreeRTOS tasks for non-blocking boot-time OTA checks
 */

#ifndef AWS_S3_OTA_H
#define AWS_S3_OTA_H

#include <Arduino.h>

// ESP32-specific includes
#if defined(ESP32)
  #include <WiFi.h>
  #include <HTTPClient.h>
  #include <WiFiClientSecure.h>
  #include <Update.h>
  #include <freertos/FreeRTOS.h>
  #include <freertos/task.h>
#else
  #error "This library only supports ESP32 boards"
#endif

#include <ArduinoJson.h>
#include <functional>
#include <vector>

// Define callback function types (optional - for advanced users)
typedef std::function<void(void)> OtaEventCallback_t;
typedef std::function<void(const char* message)> OtaErrorCallback_t;
typedef std::function<void(int progress)> OtaProgressCallback_t;

class AwsS3Ota {
public:
    // ========================================
    // SIMPLE API (Beginner-Friendly)
    // ========================================

    /**
     * @brief Constructor - Creates OTA updater instance
     */
    AwsS3Ota();

    /**
     * @brief Initialize OTA with manifest URL and current version
     * @param manifestUrl Full URL to your manifest JSON (e.g., "https://xxx.execute-api.us-east-1.amazonaws.com/prod/firmware")
     * @param currentVersion Your firmware version (e.g., "1.0.0")
     * @param rootCa AWS Root CA certificate (use AWS_ROOT_CA from aws_root_ca.h)
     * 
     * @example
     * #include "aws_root_ca.h"
     * AwsS3Ota ota;
     * ota.begin("https://api.example.com/firmware", "1.0.0", AWS_ROOT_CA);
     */
    void begin(const char* manifestUrl, const char* currentVersion, const char* rootCa);

    /**
     * @brief Check for updates ONE TIME at boot (after delay)
     * @param delaySeconds Wait this many seconds before checking (default: 10)
     * 
     * This creates a background task that checks once and exits.
     * Perfect for automatic updates on boot!
     * 
     * @example
     * void setup() {
     *   WiFi.begin(ssid, password);
     *   while (WiFi.status() != WL_CONNECTED) delay(500);
     *   ota.checkOnBoot(5);  // Check after 5 seconds
     * }
     */
    void checkOnBoot(int delaySeconds = 10);

    /**
     * @brief Check for updates EVERY X milliseconds (repeating)
     * @param intervalMs Check interval in milliseconds (e.g., 3600000 = 1 hour)
     * 
     * This creates a background task that checks repeatedly.
     * Perfect for long-running devices!
     * 
     * @example
     * ota.checkEvery(3600000);  // Check every 1 hour
     * ota.checkEvery(86400000); // Check every 24 hours
     */
    void checkEvery(unsigned long intervalMs);

    /**
     * @brief Check for updates RIGHT NOW (blocking call)
     * @return true if update was successful, false otherwise
     * 
     * Use this for manual triggers (button press, MQTT command, etc.)
     * This will block until complete!
     * 
     * @example
     * if (buttonPressed) {
     *   if (ota.checkNow()) {
     *     Serial.println("Updated! Rebooting...");
     *   }
     * }
     */
    bool checkNow();

    // ========================================
    // CONFIGURATION (Optional)
    // ========================================

    /**
     * @brief Enable/disable automatic task suspension
     * @param enabled true = automatically pause all tasks during OTA (default)
     * 
     * When enabled, library will automatically find and suspend all
     * FreeRTOS tasks during update (except OTA task itself).
     * 
     * @example
     * ota.setAutoTaskSuspend(true);  // Enable auto-suspend (default)
     */
    void setAutoTaskSuspend(bool enabled);

    /**
     * @brief Enable or disable debug messages
     * @param enabled true = show debug messages on Serial
     * 
     * @example
     * ota.setDebug(true);  // Show debug logs
     */
    void setDebug(bool enabled);

    /**
     * @brief Set maximum retry attempts for network requests
     * @param retries Number of retry attempts (default: 3)
     * 
     * @example
     * ota.setMaxRetries(5);  // Retry up to 5 times
     */
    void setMaxRetries(int retries);

    /**
     * @brief Set hard timeout for HTTP requests (safety feature)
     * @param timeoutSeconds Timeout in seconds (default: 120)
     * 
     * Forces OTA to abort if stuck. Prevents device hanging!
     * 
     * @example
     * ota.setHttpTimeout(60);  // 60 second timeout
     */
    void setHttpTimeout(int timeoutSeconds);

    // ========================================
    // ADVANCED API (Optional Callbacks)
    // ========================================

    /**
     * @brief Set callback for when OTA starts
     * @example ota.onStart([]() { digitalWrite(LED, HIGH); });
     */
    void onStart(OtaEventCallback_t cb);

    /**
     * @brief Set callback for download progress
     * @param cb Function receiving progress percentage (0-100)
     * @example ota.onProgress([](int pct) { Serial.printf("Progress: %d%%\n", pct); });
     */
    void onProgress(OtaProgressCallback_t cb);

    /**
     * @brief Set callback for successful update (before reboot)
     * @example ota.onComplete([]() { Serial.println("Success!"); });
     */
    void onComplete(OtaEventCallback_t cb);

    /**
     * @brief Set callback for failed update
     * @param cb Function receiving error message
     * @example ota.onError([](const char* err) { Serial.println(err); });
     */
    void onError(OtaErrorCallback_t cb);

    /**
     * @brief Set callback when firmware is already up-to-date
     * @example ota.onNoUpdate([]() { Serial.println("Already current"); });
     */
    void onNoUpdate(OtaEventCallback_t cb);


private:
    // ---- Private Member Variables ----
    char _manifestUrl[256];
    char _currentVersion[32];
    const char* _awsRootCa = NULL;

    int _maxRetries = 3;
    int _httpTimeout = 120;  // Hard timeout in seconds
    bool _debugMode = true;
    bool _autoTaskSuspend = true;
    
    TaskHandle_t _bootCheckTaskHandle = NULL;
    TaskHandle_t _intervalCheckTaskHandle = NULL;
    unsigned long _checkInterval = 0;
    bool _isUpdating = false;

    // ---- Private Callbacks (Optional) ----
    OtaEventCallback_t _cbOnStart = nullptr;
    OtaEventCallback_t _cbOnComplete = nullptr;
    OtaErrorCallback_t _cbOnError = nullptr;
    OtaEventCallback_t _cbOnNoUpdate = nullptr;
    OtaProgressCallback_t _cbOnProgress = nullptr;

    // ---- Private Helper Methods ----

    /**
     * @brief Core OTA logic - fetches manifest, downloads, flashes
     */
    bool performOtaUpdate();

    /**
     * @brief Fetch manifest JSON from API
     */
    bool fetchManifest(char* remoteVersion, size_t versionSize, char* downloadUrl, size_t urlSize);

    /**
     * @brief Download and flash firmware from URL
     */
    bool downloadAndFlash(const char* downloadUrl);

    /**
     * @brief Automatically suspend all FreeRTOS tasks (except current)
     */
    void autoSuspendTasks();

    /**
     * @brief Resume all previously suspended tasks
     */
    void autoResumeTasks();

    /**
     * @brief Boot check task wrapper
     */
    static void bootCheckTask(void* parameter);

    /**
     * @brief Interval check task wrapper
     */
    static void intervalCheckTask(void* parameter);

    /**
     * @brief Internal logging
     */
    void log(const char* format, ...);

    // Store suspended task handles
    std::vector<TaskHandle_t> _suspendedTasks;
};

#endif // AWS_S3_OTA_H