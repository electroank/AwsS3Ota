/**
 * @file AwsS3Ota.cpp
 * @brief Simplified OTA implementation - WiFiManager style!
 * @author Ankan Sarkar
 * @version 2.0.0
 * @date 2025
 * @license MIT
 */

#include "AwsS3Ota.h"

// Buffers for manifest parsing
#define MAX_VERSION_LEN 32
#define MAX_FIRMWARE_URL_LEN 512

AwsS3Ota::AwsS3Ota() {
    // Constructor
}

// ========================================
// SIMPLE API IMPLEMENTATION
// ========================================

void AwsS3Ota::begin(const char* manifestUrl, const char* currentVersion, const char* rootCa) {
    strncpy(_manifestUrl, manifestUrl, sizeof(_manifestUrl) - 1);
    strncpy(_currentVersion, currentVersion, sizeof(_currentVersion) - 1);
    _awsRootCa = rootCa;
    
    log("AwsS3Ota initialized");
    log("Version: %s", _currentVersion);
    log("Manifest URL: %s", _manifestUrl);
}

void AwsS3Ota::checkOnBoot(int delaySeconds) {
    log("Setting up boot-time OTA check (delay: %d seconds)", delaySeconds);
    
    // Create a struct to pass parameters to the task
    struct TaskParams {
        AwsS3Ota* instance;
        int delay;
    };
    
    TaskParams* params = new TaskParams{this, delaySeconds};
    
    xTaskCreate(
        bootCheckTask,
        "OTA_Boot",
        8192,
        params,
        1,
        &_bootCheckTaskHandle
    );
}

void AwsS3Ota::checkEvery(unsigned long intervalMs) {
    _checkInterval = intervalMs;
    log("Setting up periodic OTA check (every %lu ms)", intervalMs);
    
    xTaskCreate(
        intervalCheckTask,
        "OTA_Interval",
        8192,
        this,
        1,
        &_intervalCheckTaskHandle
    );
}

bool AwsS3Ota::checkNow() {
    log("Manual OTA check triggered");
    return performOtaUpdate();
}

// ========================================
// CONFIGURATION METHODS
// ========================================

void AwsS3Ota::setAutoTaskSuspend(bool enabled) {
    _autoTaskSuspend = enabled;
    log("Auto task suspend: %s", enabled ? "enabled" : "disabled");
}

void AwsS3Ota::setDebug(bool enabled) {
    _debugMode = enabled;
}

void AwsS3Ota::setMaxRetries(int retries) {
    _maxRetries = retries;
    log("Max retries set to: %d", retries);
}

void AwsS3Ota::setHttpTimeout(int timeoutSeconds) {
    _httpTimeout = timeoutSeconds;
    log("HTTP timeout set to: %d seconds", timeoutSeconds);
}

// ========================================
// CALLBACK SETTERS
// ========================================

void AwsS3Ota::onStart(OtaEventCallback_t cb) { _cbOnStart = cb; }
void AwsS3Ota::onProgress(OtaProgressCallback_t cb) { _cbOnProgress = cb; }
void AwsS3Ota::onComplete(OtaEventCallback_t cb) { _cbOnComplete = cb; }
void AwsS3Ota::onError(OtaErrorCallback_t cb) { _cbOnError = cb; }
void AwsS3Ota::onNoUpdate(OtaEventCallback_t cb) { _cbOnNoUpdate = cb; }

// ========================================
// CORE OTA LOGIC
// ========================================

bool AwsS3Ota::performOtaUpdate() {
    if (_isUpdating) {
        log("OTA already in progress!");
        return false;
    }
    
    _isUpdating = true;
    bool success = false;
    
    log("=== Starting OTA Update ===");
    log("Free heap: %d bytes", ESP.getFreeHeap());
    
    // Check WiFi
    if (WiFi.status() != WL_CONNECTED) {
        log("ERROR: WiFi not connected");
        if (_cbOnError) _cbOnError("WiFi not connected");
        _isUpdating = false;
        return false;
    }
    
    // Auto-suspend tasks if enabled
    if (_autoTaskSuspend) {
        log("Auto-suspending all tasks...");
        autoSuspendTasks();
        vTaskDelay(pdMS_TO_TICKS(500));  // Give tasks time to suspend
    }
    
    // Notify start
    if (_cbOnStart) _cbOnStart();
    
    // Fetch manifest
    char remoteVersion[MAX_VERSION_LEN] = {0};
    char downloadUrl[MAX_FIRMWARE_URL_LEN] = {0};
    
    if (!fetchManifest(remoteVersion, sizeof(remoteVersion), downloadUrl, sizeof(downloadUrl))) {
        log("ERROR: Failed to fetch manifest");
        if (_cbOnError) _cbOnError("Manifest fetch failed");
        goto cleanup;
    }
    
    // Compare versions
    log("Current version: %s", _currentVersion);
    log("Remote version: %s", remoteVersion);
    
    if (strcmp(remoteVersion, _currentVersion) == 0) {
        log("Firmware is already up-to-date");
        if (_cbOnNoUpdate) _cbOnNoUpdate();
        goto cleanup;
    }
    
    log("Update available! %s -> %s", _currentVersion, remoteVersion);
    
    // Download and flash
    if (downloadAndFlash(downloadUrl)) {
        log("=== OTA Update Successful! ===");
        if (_cbOnComplete) _cbOnComplete();
        success = true;
        vTaskDelay(pdMS_TO_TICKS(2000));
        ESP.restart();  // Will not return
    } else {
        log("ERROR: Download/flash failed");
        if (_cbOnError) _cbOnError("Download or flash failed");
    }
    
cleanup:
    // Resume tasks if suspended
    if (_autoTaskSuspend) {
        log("Resuming tasks...");
        autoResumeTasks();
    }
    
    _isUpdating = false;
    log("=== OTA Update Complete ===");
    return success;
}

bool AwsS3Ota::fetchManifest(char* remoteVersion, size_t versionSize, char* downloadUrl, size_t urlSize) {
    memset(remoteVersion, 0, versionSize);
    memset(downloadUrl, 0, urlSize);
    
    log("Fetching manifest from: %s", _manifestUrl);
    
    for (int attempt = 1; attempt <= _maxRetries; attempt++) {
        if (attempt > 1) {
            log("Retry %d/%d", attempt, _maxRetries);
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
        
        WiFiClientSecure client;
        client.setCACert(_awsRootCa);
        client.setTimeout(_httpTimeout);  // Hard timeout
        
        HTTPClient http;
        http.begin(client, _manifestUrl);
        http.setTimeout(_httpTimeout * 1000);  // Convert to milliseconds
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        http.setReuse(false);
        http.addHeader("Accept", "application/json");
        http.addHeader("Cache-Control", "no-cache");
        http.addHeader("Connection", "close");
        
        log("Sending HTTP GET request...");
        int code = http.GET();
        
        if (code != HTTP_CODE_OK) {
            log("HTTP error: %d", code);
            http.end();
            continue;
        }
        
        String payload = http.getString();
        log("Response: %d bytes", payload.length());
        http.end();
        
        // Parse JSON
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, payload);
        
        if (err) {
            log("JSON parse error: %s", err.c_str());
            continue;
        }
        
        // Extract version and URL
        const char* version = doc["version"];
        const char* url = doc["url"];
        
        if (!version || !url || strlen(version) == 0 || strlen(url) == 0) {
            log("Invalid manifest: missing version or url");
            continue;
        }
        
        if (strncmp(url, "https://", 8) != 0) {
            log("Invalid URL: must be HTTPS");
            continue;
        }
        
        strncpy(remoteVersion, version, versionSize - 1);
        strncpy(downloadUrl, url, urlSize - 1);
        
        log("Manifest OK - Version: %s", remoteVersion);
        return true;
    }
    
    log("Manifest fetch failed after %d attempts", _maxRetries);
    return false;
}

bool AwsS3Ota::downloadAndFlash(const char* downloadUrl) {
    log("Downloading firmware from S3...");
    
    WiFiClientSecure* client = new WiFiClientSecure;
    if (!client) {
        log("ERROR: Failed to create HTTPS client");
        return false;
    }
    
    client->setCACert(_awsRootCa);
    client->setTimeout(_httpTimeout);  // Hard timeout
    
    HTTPClient http;
    http.begin(*client, downloadUrl);
    http.setTimeout(_httpTimeout * 1000);  // Convert to milliseconds
    
    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        log("HTTP error: %d", code);
        http.end();
        delete client;
        return false;
    }
    
    int contentLength = http.getSize();
    log("Firmware size: %d KB", contentLength / 1024);
    
    if (contentLength <= 0) {
        log("ERROR: Invalid content length");
        http.end();
        delete client;
        return false;
    }
    
    // Begin update
    if (!Update.begin(contentLength)) {
        log("ERROR: Update.begin() failed: %d", Update.getError());
        http.end();
        delete client;
        return false;
    }
    
    // Download and write
    WiFiClient* stream = http.getStreamPtr();
    size_t written = 0;
    uint8_t buff[512];
    int lastProgress = -1;
    
    log("Downloading and flashing...");
    
    unsigned long startTime = millis();
    unsigned long timeoutMs = _httpTimeout * 1000;
    
    while (http.connected() && written < contentLength) {
        // Hard timeout check
        if (millis() - startTime > timeoutMs) {
            log("ERROR: Hard timeout reached!");
            Update.abort();
            http.end();
            delete client;
            return false;
        }
        
        size_t available = stream->available();
        if (available) {
            int bytesRead = stream->readBytes(buff, min(available, sizeof(buff)));
            if (bytesRead > 0) {
                if (Update.write(buff, bytesRead) != bytesRead) {
                    log("ERROR: Update.write() failed");
                    Update.abort();
                    http.end();
                    delete client;
                    return false;
                }
                written += bytesRead;
                
                // Progress callback
                int progress = (written * 100) / contentLength;
                if (progress != lastProgress && progress % 10 == 0) {
                    log("Progress: %d%%", progress);
                    if (_cbOnProgress) _cbOnProgress(progress);
                    lastProgress = progress;
                }
            }
            startTime = millis();  // Reset timeout on activity
        }
        vTaskDelay(pdMS_TO_TICKS(1));  // Yield
    }
    
    http.end();
    delete client;
    
    // Verify
    if (written != contentLength) {
        log("ERROR: Incomplete download (%d/%d bytes)", written, contentLength);
        Update.abort();
        return false;
    }
    
    // Finalize
    if (!Update.end(true)) {
        log("ERROR: Update.end() failed: %d", Update.getError());
        return false;
    }
    
    log("Flash successful! (%d bytes written)", written);
    return true;
}

// ========================================
// AUTOMATIC TASK MANAGEMENT
// ========================================

void AwsS3Ota::autoSuspendTasks() {
    _suspendedTasks.clear();
    
    TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();
    
    // Get all task handles
    UBaseType_t taskCount = uxTaskGetNumberOfTasks();
    TaskStatus_t* taskStatusArray = (TaskStatus_t*)pvPortMalloc(taskCount * sizeof(TaskStatus_t));
    
    if (taskStatusArray != NULL) {
        taskCount = uxTaskGetSystemState(taskStatusArray, taskCount, NULL);
        
        log("Found %d tasks, suspending...", taskCount);
        
        for (UBaseType_t i = 0; i < taskCount; i++) {
            TaskHandle_t taskHandle = taskStatusArray[i].xHandle;
            
            // Don't suspend ourselves or IDLE tasks
            if (taskHandle != currentTask && 
                strncmp(taskStatusArray[i].pcTaskName, "IDLE", 4) != 0 &&
                strncmp(taskStatusArray[i].pcTaskName, "OTA_", 4) != 0 &&
                strncmp(taskStatusArray[i].pcTaskName, "Tmr", 3) != 0) {
                
                log("  Suspending: %s", taskStatusArray[i].pcTaskName);
                vTaskSuspend(taskHandle);
                _suspendedTasks.push_back(taskHandle);
            }
        }
        
        vPortFree(taskStatusArray);
    }
    
    log("Suspended %d tasks", _suspendedTasks.size());
}

void AwsS3Ota::autoResumeTasks() {
    log("Resuming %d tasks", _suspendedTasks.size());
    
    for (TaskHandle_t task : _suspendedTasks) {
        vTaskResume(task);
    }
    
    _suspendedTasks.clear();
}

// ========================================
// FREERTOS TASK WRAPPERS
// ========================================

void AwsS3Ota::bootCheckTask(void* parameter) {
    struct TaskParams {
        AwsS3Ota* instance;
        int delay;
    };
    
    TaskParams* params = (TaskParams*)parameter;
    AwsS3Ota* ota = params->instance;
    int delaySec = params->delay;
    
    delete params;  // Clean up
    
    ota->log("Boot check task started (delay: %d sec)", delaySec);
    
    // Wait for WiFi
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    // Wait delay
    vTaskDelay(pdMS_TO_TICKS(delaySec * 1000));
    
    ota->log("Running boot-time OTA check...");
    ota->performOtaUpdate();
    
    ota->log("Boot check task complete, deleting task");
    ota->_bootCheckTaskHandle = NULL;
    vTaskDelete(NULL);
}

void AwsS3Ota::intervalCheckTask(void* parameter) {
    AwsS3Ota* ota = (AwsS3Ota*)parameter;
    
    ota->log("Interval check task started (interval: %lu ms)", ota->_checkInterval);
    
    while (true) {
        // Wait for WiFi
        while (WiFi.status() != WL_CONNECTED) {
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
        
        // Wait interval
        vTaskDelay(pdMS_TO_TICKS(ota->_checkInterval));
        
        ota->log("Running scheduled OTA check...");
        ota->performOtaUpdate();
    }
    
    // Never reaches here
}

// ========================================
// UTILITY FUNCTIONS
// ========================================

void AwsS3Ota::log(const char* format, ...) {
    if (!_debugMode) return;
    
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    Serial.print("[OTA] ");
    Serial.println(buffer);
}
