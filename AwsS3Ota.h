/**
 * @file AwsS3Ota.h
 * @brief Arduino library for performing Over-The-Air (OTA) updates from an AWS S3 bucket.
 * @author (Your Name Here)
 * @note This library is designed for ESP32 and uses FreeRTOS tasks.
 */

#ifndef AWS_S3_OTA_H
#define AWS_S3_OTA_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Update.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Use std::function for flexible callbacks
#include <functional>

// Define callback function types
typedef std::function<void(void)> OtaEventCallback_t;
typedef std::function<void(const char* message)> OtaErrorCallback_t;
typedef std::function<bool(void)> OtaTaskControlCallback_t; // Return bool indicates success
typedef std::function<bool(void)> OtaCheckCallback_t; // Return bool to allow/deny OTA

class AwsS3Ota {
public:
    // ---- Public Methods ----

    /**
     * @brief Constructor
     */
    AwsS3Ota();

    /**
     * @brief Sets up the core OTA parameters.
     * @param apiBaseUrl The base URL for your manifest API (e.g., "https://api.example.com")
     * @param firmwareEndpoint The endpoint for the firmware manifest (e.g., "/firmware")
     * @param currentVersion The firmware version string of the running sketch.
     * @param rootCa The AWS Root CA certificate for HTTPS validation.
     */
    void begin(const char* apiBaseUrl, const char* firmwareEndpoint, const char* currentVersion, const char* rootCa);

    /**
     * @brief Checks for a new firmware version and performs the update if available.
     * @note This is a blocking function and will suspend/resume tasks using callbacks.
     * @param userInitiated Set to true if triggered manually (e.g., by a button).
     * @return true if an update was successfully performed, false otherwise.
     */
    bool checkAndUpdate(bool userInitiated = false);

    /**
     * @brief Starts a non-blocking FreeRTOS task to check for an update on boot.
     * @param taskName Name for the FreeRTOS task (e.g., "otaCheckTask").
     * @param stackSize Stack size for the task.
     * @param priority Task priority.
     */
    void startBootCheckTask(const char* taskName = "otaCheckTask", uint32_t stackSize = 8192, UBaseType_t priority = 1);

    /**
     * @brief Sets the maximum number of retries for fetching the manifest.
     * @param retries Number of retries (default: 3).
     */
    void setMaxRetries(int retries);

    /**
     * @brief Sets the delay in milliseconds before the boot task runs.
     * @param delayMs Delay in milliseconds (default: 10000).
     */
    void setStartupDelay(unsigned long delayMs);

    /**
     * @brief Enables or disables debug messages to Serial.
     * @param enabled True to enable, false to disable.
     */
    void setDebug(bool enabled);

    // ---- Callback Setters ----

    /**
     * @brief (Required) Set callback to run before OTA check. 
     * Use this to check conditions (e.g., system flags, battery level).
     * @param cb Function that returns true (allow OTA) or false (deny OTA).
     */
    void onCheckStart(OtaCheckCallback_t cb);

    /**
     * @brief (Required) Set callback to suspend application tasks.
     * @param cb Function to suspend tasks. Should return true on success.
     */
    void onSuspendTasks(OtaTaskControlCallback_t cb);

    /**
     * @brief (Required) Set callback to resume application tasks.
     * @param cb Function to resume tasks. Should return true on success.
     */
    void onResumeTasks(OtaTaskControlCallback_t cb);

    /**
     * @brief (Optional) Set callback for when the OTA process starts (after tasks are suspended).
     * @param cb Function to call (e.g., set_cyan_led()).
     */
    void onOtaStarted(OtaEventCallback_t cb);

    /**
     * @brief (Optional) Set callback for when the update is complete (before restart).
     * @param cb Function to call.
     */
    void onOtaFinished(OtaEventCallback_t cb);

    /**
     * @brief (Optional) Set callback for when the OTA process fails or is aborted.
     * @param cb Function to call (e.g., set_red_led()).
     */
    void onOtaFailed(OtaErrorCallback_t cb);

    /**
     * @brief (Optional) Set callback for when the firmware is already up-to-date.
     * @param cb Function to call (e.g., set_green_led()).
     */
    void onOtaUpToDate(OtaEventCallback_t cb);

    /**
     * @brief (Optional) Set callback for download progress.
     * @param cb Function to call during download (e.g., blink_led()).
     */
    void onOtaProgress(OtaEventCallback_t cb);


private:
    // ---- Private Member Variables ----
    char _apiBaseUrl[128];
    char _firmwareEndpoint[64];
    char _currentVersion[32];
    const char* _awsRootCa = NULL;

    int _maxRetries = 3;
    unsigned long _otaStartupDelay = 10000; // 10 seconds
    bool _debugMode = true;
    TaskHandle_t _otaTaskHandle = NULL;

    // ---- Private Callbacks ----
    OtaCheckCallback_t _cbOnCheckStart = nullptr;
    OtaTaskControlCallback_t _cbOnSuspendTasks = nullptr;
    OtaTaskControlCallback_t _cbOnResumeTasks = nullptr;
    OtaEventCallback_t _cbOnOtaStarted = nullptr;
    OtaEventCallback_t _cbOnOtaFinished = nullptr;
    OtaErrorCallback_t _cbOnOtaFailed = nullptr;
    OtaEventCallback_t _cbOnOtaUpToDate = nullptr;
    OtaEventCallback_t _cbOnOtaProgress = nullptr;

    // ---- Private Helper Methods ----

    /**
     * @brief Builds the full manifest URL.
     * @param buffer Buffer to store the URL.
     * @param bufferSize Size of the buffer.
     */
    void buildApiUrl(char* buffer, size_t bufferSize);

    /**
     * @brief Fetches and parses the firmware manifest JSON.
     * @param remoteVersion Buffer to store the fetched version.
     * @param versionSize Size of the version buffer.
     * @param downloadUrl Buffer to store the fetched download URL.
     * @param urlSize Size of the URL buffer.
     * @return true on success, false on failure.
     */
    bool fetchFirmwareManifest(char* remoteVersion, size_t versionSize, char* downloadUrl, size_t urlSize);

    /**
     * @brief Performs the actual HTTP download and flash update.
     * @param downloadUrl The S3 URL to the firmware binary.
     * @return true on success, false on failure.
     */
    bool performHttpOta(const char* downloadUrl);

    /**
     * @brief Static task function that calls the class instance's method.
     * @param pvParameters Pointer to the AwsS3Ota instance.
     */
    static void otaCheckTaskWrapper(void *pvParameters);

    /**
     * @brief The actual task logic for the boot-time check.
     */
    void runBootCheck();

    /**
     * @brief Internal logging function.
     * @param format Printf-style format string.
     * @param ... Arguments.
     */
    void log(const char* format, ...);
};

#endif // AWS_S3_OTA_H