# AwsS3Ota - Quick Reference

## 🚀 Super Simple Setup (2 Lines!)

```cpp
#include <WiFi.h>
#include <AwsS3Ota.h>
#include "aws_root_ca.h"

AwsS3Ota ota;

void setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  // That's it! Just 2 lines:
  ota.begin("https://your-api.com/firmware", "1.0.0", AWS_ROOT_CA);
  ota.checkOnBoot(5);  // Check after 5 seconds
}
```

## 📚 API Reference

### Basic Methods (Beginners)

#### `begin(manifestUrl, version, rootCA)`
Initialize OTA with your API endpoint.
```cpp
ota.begin("https://api.execute-api.us-east-1.amazonaws.com/prod/firmware", 
          "1.0.0", 
          AWS_ROOT_CA);
```

#### `checkOnBoot(delaySeconds)`
Check for updates ONCE after boot (after delay).
```cpp
ota.checkOnBoot(10);  // Check 10 seconds after boot
```

#### `checkEvery(intervalMs)`
Check for updates REPEATEDLY at intervals.
```cpp
ota.checkEvery(3600000);   // Every 1 hour
ota.checkEvery(86400000);  // Every 24 hours
```

#### `checkNow()`
Check for updates RIGHT NOW (blocking).
```cpp
if (buttonPressed) {
  ota.checkNow();  // Blocks until complete
}
```

### Configuration Methods (Optional)

#### `setAutoTaskSuspend(bool)`
Enable/disable automatic task suspension (default: `true`).
```cpp
ota.setAutoTaskSuspend(true);  // Auto-pause all tasks during OTA
```

#### `setDebug(bool)`
Show/hide debug messages.
```cpp
ota.setDebug(true);  // Show debug logs
```

#### `setMaxRetries(int)`
Number of retry attempts for network requests.
```cpp
ota.setMaxRetries(5);  // Retry up to 5 times
```

#### `setHttpTimeout(int)`
Hard timeout in seconds (safety feature).
```cpp
ota.setHttpTimeout(60);  // 60 second timeout
```

### Callback Methods (Advanced)

#### `onStart(callback)`
Called when OTA update starts.
```cpp
ota.onStart([]() {
  Serial.println("OTA started!");
  digitalWrite(LED, HIGH);
});
```

#### `onProgress(callback)`
Called with progress percentage (0-100).
```cpp
ota.onProgress([](int percent) {
  Serial.printf("Progress: %d%%\n", percent);
});
```

#### `onComplete(callback)`
Called when update succeeds (before reboot).
```cpp
ota.onComplete([]() {
  Serial.println("Success! Rebooting...");
});
```

#### `onError(callback)`
Called when update fails.
```cpp
ota.onError([](const char* error) {
  Serial.printf("Error: %s\n", error);
});
```

#### `onNoUpdate(callback)`
Called when firmware is already up-to-date.
```cpp
ota.onNoUpdate([]() {
  Serial.println("Already current version");
});
```

## 💡 Usage Examples

### Example 1: Boot-time Check
```cpp
void setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  ota.begin(manifestUrl, "1.0.0", AWS_ROOT_CA);
  ota.checkOnBoot(5);  // Check once, 5 seconds after boot
}
```

### Example 2: Periodic Check
```cpp
void setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  ota.begin(manifestUrl, "1.0.0", AWS_ROOT_CA);
  ota.checkEvery(3600000);  // Check every hour, forever
}
```

### Example 3: Manual Trigger
```cpp
void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Checking for updates...");
    ota.checkNow();  // Check immediately
  }
}
```

### Example 4: With Callbacks
```cpp
void setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  ota.begin(manifestUrl, "1.0.0", AWS_ROOT_CA);
  
  // Optional: Add visual feedback
  ota.onStart([]() { digitalWrite(LED, HIGH); });
  ota.onProgress([](int p) { Serial.printf("%d%%\n", p); });
  ota.onComplete([]() { Serial.println("Done!"); });
  
  ota.checkOnBoot(5);
}
```

## 🔧 Manifest JSON Format

Your API endpoint must return JSON in this format:

```json
{
  "version": "1.0.1",
  "url": "https://your-bucket.s3.amazonaws.com/firmware.bin?AWSAccessKeyId=..."
}
```

- `version`: Firmware version string
- `url`: HTTPS pre-signed S3 URL to the .bin file

## ✨ Key Features

✅ **Automatic Task Management** - Library automatically pauses/resumes ALL tasks  
✅ **Hard Timeouts** - Won't hang! Aborts if stuck  
✅ **Auto-Retry** - Retries on network failure  
✅ **Non-Blocking** - Uses FreeRTOS tasks  
✅ **Progress Callbacks** - Visual feedback  
✅ **Beginner-Friendly** - WiFiManager-style simple API  

## 🛡️ Safety Features

- ✅ Hard HTTP timeout (prevents hanging)
- ✅ Automatic task suspend/resume
- ✅ Certificate validation (AWS Root CA)
- ✅ Version comparison (won't flash same version)
- ✅ Flash verification before reboot
- ✅ Auto-retry on network failure

## 📦 What You Need

1. **AWS S3 Bucket** - Store your firmware.bin
2. **AWS Lambda** - Generate manifest JSON
3. **API Gateway** - HTTP endpoint for manifest
4. **This Library** - Handles everything else!

## 🆘 Troubleshooting

**"WiFi not connected"**
- Make sure WiFi is connected before calling OTA methods

**"Manifest fetch failed"**
- Check your API Gateway URL
- Test URL in browser first
- Verify JSON format

**"Download failed"**
- Check S3 pre-signed URL is valid
- Increase timeout: `ota.setHttpTimeout(120);`
- Check network stability

**"Update.begin failed"**
- Verify partition scheme supports OTA
- Arduino IDE: Tools → Partition Scheme → Choose one with OTA

## 📖 Full Documentation

For complete AWS setup guide, see:
- `extras/AWS_SETUP_GUIDE.md`
- `extras/ARCHITECTURE.md`
- `README.md`

## 🎯 Comparison with v1.0

### Old Way (v1.0) - Complex! ❌
```cpp
ota.begin(apiBase, endpoint, version, cert);
ota.onCheckStart([]() { return true; });
ota.onSuspendTasks([]() { 
  vTaskSuspend(task1);
  vTaskSuspend(task2);
  return true;
});
ota.onResumeTasks([]() {
  vTaskResume(task1);
  vTaskResume(task2);
  return true;
});
ota.startBootCheckTask();
```

### New Way (v2.0) - Simple! ✅
```cpp
ota.begin(fullUrl, version, cert);
ota.checkOnBoot();
// Done! Automatic task management!
```

---

**Need help? Open an issue on GitHub!**  
https://github.com/electroank/AwsS3Ota/issues
