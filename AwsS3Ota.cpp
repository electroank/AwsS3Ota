/**
 * @file AwsS3Ota.cpp
 * @brief Implementation of the AwsS3Ota library.
 */

#include "AwsS3Ota.h"

// Buffers for manifest parsing
#define MAX_VERSION_LEN 32
#define MAX_FIRMWARE_URL_LEN 256

AwsS3Ota::AwsS3Ota() {
    // Constructor
}

void AwsS3Ota::begin(const char* apiBaseUrl, const char* firmwareEndpoint, const char* currentVersion, const char* rootCa) {
    strncpy(_apiBaseUrl, apiBaseUrl, sizeof(_apiBaseUrl) - 1);
    strncpy(_firmwareEndpoint, firmwareEndpoint, sizeof(_firmwareEndpoint) - 1);
    strncpy(_currentVersion, currentVersion, sizeof(_currentVersion) - 1);
    _awsRootCa = rootCa;
}

void AwsS3Ota::setMaxRetries(int retries) {
    _maxRetries = retries;
}

void AwsS3Ota::setStartupDelay(unsigned long delayMs) {
    _otaStartupDelay = delayMs;
}

void AwsS3Ota::setDebug(bool enabled) {
    _debugMode = enabled;
}

void AwsS3Ota::log(const char* format, ...) {
    if (!_debugMode) return;
    char loc_buf[128];
    char * temp = loc_buf;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    int len = vsnprintf(temp, sizeof(loc_buf), format, copy);
    va_end(copy);
    if (len < 0) {
        va_end(arg);
        return;
    };
    if (len >= (int)sizeof(loc_buf)) {
        temp = (char*) malloc(len + 1);
        if (temp == NULL) {
            va_end(arg);
            return;
        }
        len = vsnprintf(temp, len + 1, format, arg);
    }
    va_end(arg);
    Serial.print("[OTA] ");
    Serial.println(temp);
    if (temp != loc_buf) {
        free(temp);
    }
}

// ---- Callback Setters ----
void AwsS3Ota::onCheckStart(OtaCheckCallback_t cb) { _cbOnCheckStart = cb; }
void AwsS3Ota::onSuspendTasks(OtaTaskControlCallback_t cb) { _cbOnSuspendTasks = cb; }
void AwsS3Ota::onResumeTasks(OtaTaskControlCallback_t cb) { _cbOnResumeTasks = cb; }
void AwsS3Ota::onOtaStarted(OtaEventCallback_t cb) { _cbOnOtaStarted = cb; }
void AwsS3Ota::onOtaFinished(OtaEventCallback_t cb) { _cbOnOtaFinished = cb; }
void AwsS3Ota::onOtaFailed(OtaErrorCallback_t cb) { _cbOnOtaFailed = cb; }
void AwsS3Ota::onOtaUpToDate(OtaEventCallback_t cb) { _cbOnOtaUpToDate = cb; }
void AwsS3Ota::onOtaProgress(OtaEventCallback_t cb) { _cbOnOtaProgress = cb; }

// ---- Private Helper Methods ----

void AwsS3Ota::buildApiUrl(char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%s%s", _apiBaseUrl, _firmwareEndpoint);
}

bool AwsS3Ota::fetchFirmwareManifest(char* remoteVersion, size_t versionSize, char* downloadUrl, size_t urlSize) {
    memset(remoteVersion, 0, versionSize);
    memset(downloadUrl, 0, urlSize);

    if (WiFi.status() != WL_CONNECTED) {
        log("WiFi not connected");
        return false;
    }

    char url[MAX_FIRMWARE_URL_LEN];
    buildApiUrl(url, sizeof(url));
    log("Checking updates from: %s", url);

    for (int attempt = 1; attempt <= _maxRetries; attempt++) {
        if (attempt > 1) {
            log("Retry %d/%d", attempt, _maxRetries);
            vTaskDelay(pdMS_TO_TICKS(2000));
        }

        WiFiClientSecure client;
        client.setCACert(_awsRootCa);

        HTTPClient http;
        http.begin(client, url);
        http.setTimeout(15000);
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        http.setReuse(false);
        http.addHeader("Accept", "application/json");
        http.addHeader("Cache-Control", "no-cache");
        http.addHeader("Connection", "close");
        
        int code = http.GET();

        if (code != HTTP_CODE_OK) {
            log("HTTP GET failed, error %d", code);
            http.end();
            continue;
        }

        String payload = http.getString();
        log("Manifest response code: %d, Length: %d bytes", code, payload.length());
        
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, payload);
        http.end();

        if (err) {
            log("JSON parse failed: %s", err.c_str());
            continue;
        }

        if (!doc["version"].is<const char*>() || !doc["url"].is<const char*>()) {
            log("Manifest missing 'version' or 'url'");
            continue;
        }

        const char* version = doc["version"];
        const char* url_str = doc["url"];

        if (!version || !url_str || strlen(version) == 0 || strlen(url_str) == 0) {
            log("Manifest has empty 'version' or 'url'");
            continue;
        }

        if (strncmp(url_str, "https://", 8) != 0) {
            log("Invalid URL (not https)");
            continue;
        }

        if (strlen(url_str) >= urlSize || strlen(version) >= versionSize) {
            log("Version or URL string too large for buffer");
            continue;
        }

        strncpy(remoteVersion, version, versionSize - 1);
        strncpy(downloadUrl, url_str, urlSize - 1);

        log("Found remote version v%s", remoteVersion);
        return true;
    }

    log("Manifest fetch failed after all retries.");
    return false;
}

bool AwsS3Ota::performHttpOta(const char* downloadUrl) {
    if (!downloadUrl || downloadUrl[0] == '\0' || strlen(downloadUrl) < 10) {
        log("ERROR: Invalid download URL");
        return false;
    }
  
    WiFiClientSecure *client = new WiFiClientSecure;
    if (!client) {
        log("HTTPS client creation failed");
        return false;
    }
  
    client->setCACert(_awsRootCa);
  
    HTTPClient http;
    http.begin(*client, downloadUrl);
    http.setTimeout(60000);
  
    log("Starting firmware download from: %s", downloadUrl);
    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        log("HTTP GET failed, error %d", code);
        http.end();
        delete client;
        return false;
    }
  
    int contentLength = http.getSize();
    log("Firmware size: %d KB", contentLength / 1024);
  
    if (!Update.begin(contentLength > 0 ? contentLength : UPDATE_SIZE_UNKNOWN)) {
        log("Update.begin failed: %d", Update.getError());
        http.end();
        delete client;
        return false;
    }
  
    WiFiClient* stream = http.getStreamPtr();
    size_t written = 0;
    uint8_t buff[512];
    int lastProgress = 0;
  
    log("Download started...");
  
    while (http.connected() && (contentLength > 0 ? written < contentLength : true)) {
        // Call progress callback
        if (_cbOnOtaProgress) {
            _cbOnOtaProgress();
        }

        size_t available = stream->available();
        if (available) {
            int bytesRead = stream->readBytes(buff, min(available, sizeof(buff)));
            if (bytesRead > 0) {
                if (Update.write(buff, bytesRead) != bytesRead) {
                    log("Update.write failed!");
                    Update.abort();
                    http.end();
                    delete client;
                    return false;
                }
                written += bytesRead;
                
                if (contentLength > 0) {
                    int progress = (written * 100) / contentLength;
                    if (progress >= lastProgress + 10 && progress <= 100) {
                        log("Progress: %d%% (%d/%d KB)", progress, written / 1024, contentLength / 1024);
                        lastProgress = progress;
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1)); // Yield
    }
  
    log("Download complete, flashing...");
  
    if (contentLength > 0 && written != contentLength) {
        log("Incomplete download: %d/%d bytes", written, contentLength);
        Update.abort();
        http.end();
        delete client;
        return false;
    }
  
    if (!Update.end() || !Update.isFinished()) {
        log("Update.end failed: %d", Update.getError());
        http.end();
        delete client;
        return false;
    }
  
    http.end();
    delete client;
    log("Flash update successful.");
    return true;
}

bool AwsS3Ota::checkAndUpdate(bool userInitiated) {
    log("Preparing for update check...");

    // 1. Check preconditions using callback
    if (_cbOnCheckStart) {
        if (!_cbOnCheckStart()) {
            log("Pre-check callback denied OTA start.");
            return false;
        }
    } else {
        log("ERROR: onCheckStart callback is not set. Aborting.");
        return false;
    }

    if (WiFi.status() != WL_CONNECTED) {
        log("WiFi not connected. Aborting.");
        return false;
    }

    // 2. Suspend other tasks via callback
    if (_cbOnSuspendTasks) {
        if (!_cbOnSuspendTasks()) {
            log("Failed to suspend tasks. Aborting.");
            return false;
        }
    } else {
        log("ERROR: onSuspendTasks callback is not set. Aborting.");
        return false;
    }

    // 3. Signal OTA start
    if (_cbOnOtaStarted) {
        _cbOnOtaStarted();
    }

    vTaskDelay(pdMS_TO_TICKS(1000)); // Give time for tasks to suspend, LEDs to change
    log("Free heap: %lu KB", (unsigned long)(ESP.getFreeHeap() / 1024));

    char remoteVersion[MAX_VERSION_LEN];
    char downloadUrl[MAX_FIRMWARE_URL_LEN];
    bool otaSuccess = false;
    const char* failureReason = "Unknown error";

    // 4. Fetch Manifest
    bool manifestSuccess = fetchFirmwareManifest(remoteVersion, sizeof(remoteVersion),
                                                   downloadUrl, sizeof(downloadUrl));

    if (!manifestSuccess) {
        failureReason = "Manifest fetch failed";
    } else if (remoteVersion[0] == '\0' || downloadUrl[0] == '\0') {
        failureReason = "Empty manifest response";
    } else if (strcmp(remoteVersion, _currentVersion) == 0) {
        log("Firmware is already up to date (v%s)", _currentVersion);
        if (_cbOnOtaUpToDate) {
            _cbOnOtaUpToDate();
        }
        // This is not a failure, just no update
        otaSuccess = false; // But we don't treat it as a hard error
        failureReason = nullptr; // Clear failure reason
    } else {
        log("Updating v%s -> v%s", _currentVersion, remoteVersion);
        if (strlen(downloadUrl) < 10 || strncmp(downloadUrl, "https://", 8) != 0) {
            failureReason = "Invalid download URL in manifest";
        } else {
            // 5. Perform Download & Flash
            if (performHttpOta(downloadUrl)) {
                otaSuccess = true;
                failureReason = nullptr; // Success
            } else {
                failureReason = "Firmware download or flash failed";
            }
        }
    }

    // 6. Handle outcome
    if (otaSuccess) {
        log("✓ Success! Restarting...");
        if (_cbOnOtaFinished) {
            _cbOnOtaFinished();
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
        ESP.restart();
    } else {
        log("✗ Update failed or not needed.");
        if (failureReason && _cbOnOtaFailed) {
            _cbOnOtaFailed(failureReason);
        }
        // 7. Resume tasks
        if (_cbOnResumeTasks) {
            if (!_cbOnResumeTasks()) {
                log("ERROR: Failed to resume tasks! System may be unstable.");
            }
        } else {
            log("ERROR: onResumeTasks callback is not set. Tasks will not be resumed.");
        }
    }
  
    return otaSuccess;
}

// ---- FreeRTOS Task Methods ----

void AwsS3Ota::startBootCheckTask(const char* taskName, uint32_t stackSize, UBaseType_t priority) {
    log("Starting boot-time OTA check task...");
    xTaskCreate(
        otaCheckTaskWrapper,
        taskName,
        stackSize,
        this, // Pass the class instance as the parameter
        priority,
        &_otaTaskHandle
    );
}

void AwsS3Ota::otaCheckTaskWrapper(void *pvParameters) {
    // Call the non-static member function
    ((AwsS3Ota*)pvParameters)->runBootCheck();
}

void AwsS3Ota::runBootCheck() {
    log("✓ OTA task started");
  
    // Wait for WiFi connection
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
  
    // Wait startup delay before checking
    log("Waiting %lu ms before boot check...", _otaStartupDelay);
    vTaskDelay(pdMS_TO_TICKS(_otaStartupDelay));
    
    // Check if OTA is allowed by the main app
    if (_cbOnCheckStart) {
        if (!_cbOnCheckStart()) {
            log("Boot check aborted by onCheckStart callback (e.g., config portal active).");
            vTaskDelete(NULL); // Delete this task
            return;
        }
    } else {
        log("ERROR: onCheckStart callback not set. Aborting boot check.");
        vTaskDelete(NULL);
        return;
    }

    log("Performing boot-time update check...");
    
    // This will block the task, but not the main loop
    (void)checkAndUpdate(false);
  
    log("Boot check complete. Task exiting.");
    _otaTaskHandle = NULL;
    vTaskDelete(NULL); // Delete this task
}