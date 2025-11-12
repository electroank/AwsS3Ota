# AwsS3Ota v2.0 - MAJOR UPDATE! 🎉

## What Changed?

Your feedback was perfect! I've completely redesigned the library to be **WiFiManager-style simple**.

## Before & After Comparison

### ❌ OLD (v1.0) - TOO COMPLEX!

```cpp
#include <WiFi.h>
#include <AwsS3Ota.h>

const char* AWS_ROOT_CA = "-----BEGIN CERTIFICATE-----\n" ... ; // 30 lines!

TaskHandle_t myTaskHandle = NULL;

AwsS3Ota ota;

void setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  myTaskHandle = xTaskGetCurrentTaskHandle();
  
  ota.begin("https://api.com", "/firmware", "1.0.0", AWS_ROOT_CA);
  
  // Required callbacks - scary for beginners!
  ota.onCheckStart([]() { return true; });
  ota.onSuspendTasks([]() {
    if (myTaskHandle) vTaskSuspend(myTaskHandle);
    return true;
  });
  ota.onResumeTasks([]() {
    if (myTaskHandle) vTaskResume(myTaskHandle);
    return true;
  });
  
  ota.startBootCheckTask();
}
```

### ✅ NEW (v2.0) - SUPER SIMPLE!

```cpp
#include <WiFi.h>
#include <AwsS3Ota.h>
#include "aws_root_ca.h"  // Certificate in separate file!

AwsS3Ota ota;

void setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  // Just 2 lines! Done!
  ota.begin("https://api.com/firmware", "1.0.0", AWS_ROOT_CA);
  ota.checkOnBoot(5);
}
```

**That's it! 2 lines vs 20+ lines!**

## 🚀 Key Improvements

### 1. ✅ AWS Root CA in Separate File
- Certificate moved to `aws_root_ca.h`
- Examples stay clean and readable
- Just `#include "aws_root_ca.h"`

### 2. ✅ Simplified API
- **OLD**: `begin(apiBase, endpoint, version, cert)` - 4 parameters
- **NEW**: `begin(fullUrl, version, cert)` - 3 parameters, simpler!

### 3. ✅ Automatic Task Management
- **OLD**: Manual task suspend/resume via callbacks
- **NEW**: Library automatically finds and pauses ALL tasks!
- No more `vTaskSuspend()` / `vTaskResume()`

### 4. ✅ Simple Check Functions
```cpp
ota.checkOnBoot(5);      // Check once, 5 seconds after boot
ota.checkEvery(3600000); // Check every hour, forever
ota.checkNow();          // Check immediately (manual trigger)
```

### 5. ✅ Hard HTTP Timeouts
- Prevents device from hanging
- `ota.setHttpTimeout(60);` - Force abort after 60 seconds
- Safety first!

### 6. ✅ Optional Callbacks (Not Required!)
- **OLD**: Callbacks were REQUIRED
- **NEW**: Callbacks are OPTIONAL (for advanced users)
```cpp
ota.onStart([]() { digitalWrite(LED, HIGH); });    // Optional
ota.onProgress([](int p) { Serial.println(p); });  // Optional
ota.onComplete([]() { Serial.println("Done!"); }); // Optional
```

## 📁 New File Structure

```
examples/
├── BasicOta/
│   ├── BasicOta.ino        ← Super simple! ~40 lines
│   └── aws_root_ca.h       ← Certificate here
├── SimpleOta/
│   ├── SimpleOta.ino       ← Minimal! ~30 lines
│   └── aws_root_ca.h
└── AdvancedOta/
    ├── AdvancedOta.ino     ← With callbacks! ~100 lines
    └── aws_root_ca.h
```

## 🎯 What Each Example Shows

### BasicOta.ino (RECOMMENDED for beginners)
- WiFi connection
- 2-line OTA setup
- Boot-time check
- Automatic task management

**Perfect for**: Quick start, beginners, production code

### SimpleOta.ino
- Minimal code
- Periodic checking (every 1 hour)
- Even simpler than BasicOta!

**Perfect for**: Long-running devices, sensors

### AdvancedOta.ino
- Manual trigger (button)
- Progress callbacks
- LED feedback
- Error handling

**Perfect for**: Interactive devices, debugging

## 🔄 Migration Guide (v1 → v2)

### If you used the old version:

**Old code:**
```cpp
ota.begin(apiBase, endpoint, version, cert);
ota.onCheckStart(callback1);
ota.onSuspendTasks(callback2);
ota.onResumeTasks(callback3);
ota.startBootCheckTask();
```

**New code:**
```cpp
#include "aws_root_ca.h"
ota.begin(apiBase + endpoint, version, AWS_ROOT_CA);
ota.checkOnBoot();
// Remove all task management callbacks - automatic now!
```

### Breaking Changes:

1. **`begin()` signature changed**
   - Old: `begin(apiBase, endpoint, version, cert)`
   - New: `begin(fullUrl, version, cert)`
   - Fix: Concatenate apiBase + endpoint

2. **Callbacks renamed (optional now)**
   - `onCheckStart()` → Removed (automatic)
   - `onSuspendTasks()` → Removed (automatic)
   - `onResumeTasks()` → Removed (automatic)
   - `onOtaStarted()` → `onStart()`
   - `onOtaFinished()` → `onComplete()`
   - `onOtaFailed()` → `onError()`
   - `onOtaUpToDate()` → `onNoUpdate()`

3. **Task methods changed**
   - `startBootCheckTask()` → `checkOnBoot(seconds)`
   - `checkAndUpdate()` → `checkNow()`

## 🌟 New Features

1. **Automatic Task Suspension**
   - Library finds all tasks automatically
   - Suspends during OTA
   - Resumes after (if update fails)
   - No manual code needed!

2. **Hard Timeouts**
   - Prevents hanging
   - Configurable: `setHttpTimeout(seconds)`
   - Safety feature!

3. **Simpler Callbacks**
   - Progress now gives percentage: `onProgress([](int p) {...})`
   - Error gives message: `onError([](const char* err) {...})`

4. **Better Logging**
   - `[OTA]` prefix on all messages
   - Clearer error messages
   - Easy to debug

## 📊 Comparison Table

| Feature | v1.0 | v2.0 |
|---------|------|------|
| Setup lines | ~20 | ~2 |
| Required callbacks | 3 | 0 |
| AWS Root CA | Inline (~30 lines) | Separate file |
| Task management | Manual | Automatic |
| Timeout handling | Basic | Hard timeout |
| Beginner-friendly | ❌ | ✅ |
| WiFiManager-style | ❌ | ✅ |

## 🎓 Learning Path

1. **Start with**: `examples/BasicOta/BasicOta.ino`
   - Copy/paste
   - Change WiFi and API URL
   - Done!

2. **Next try**: `examples/SimpleOta/SimpleOta.ino`
   - See periodic checking
   - Understand `checkEvery()`

3. **Advanced**: `examples/AdvancedOta/AdvancedOta.ino`
   - Manual triggers
   - Callbacks for feedback
   - LED indicators

## 💬 User Feedback Implemented

Your suggestions:
1. ✅ "AWS Root CA in separate file" - DONE!
2. ✅ "WiFiManager-style simplicity" - DONE!
3. ✅ "Automatic task pause" - DONE!
4. ✅ "Simple functions like checkOnBoot()" - DONE!
5. ✅ "Hard timeouts and failsafes" - DONE!

## 🚦 Quick Start (30 seconds!)

```bash
# 1. Install library (Arduino Library Manager)
Search: "AwsS3Ota"

# 2. Open example
File → Examples → AwsS3Ota → BasicOta

# 3. Edit 3 lines:
const char* ssid = "YourWiFi";
const char* password = "YourPassword";
const char* manifestUrl = "https://your-api.com/firmware";

# 4. Upload!
```

## 📝 TODO for Users

To use this library, you just need:

1. ✅ AWS S3 bucket (store firmware.bin)
2. ✅ AWS Lambda (generate manifest JSON)
3. ✅ API Gateway (expose HTTP endpoint)
4. ✅ 2 lines of code in your sketch!

See `extras/AWS_SETUP_GUIDE.md` for AWS setup.

## 🆘 Support

- **GitHub Issues**: https://github.com/electroank/AwsS3Ota/issues
- **Examples**: Check `examples/` folder
- **Docs**: See `QUICK_REFERENCE.md`
- **AWS Setup**: See `extras/AWS_SETUP_GUIDE.md`

## 🎉 Conclusion

**v2.0 is a COMPLETE REWRITE based on your feedback!**

- ✅ Beginner-friendly
- ✅ WiFiManager-style API
- ✅ Automatic everything
- ✅ Clean examples
- ✅ Production-ready

**Try it now and let me know what you think!**

---

*Happy OTA updating! 🚀*  
*- Ankan*
