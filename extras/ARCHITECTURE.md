# AwsS3Ota - System Architecture & Flow

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         ESP32 Device                            │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                  Your Application                         │  │
│  │  ┌────────────┐  ┌────────────┐  ┌────────────┐         │  │
│  │  │  Sensors   │  │    MQTT    │  │   Tasks    │         │  │
│  │  └────────────┘  └────────────┘  └────────────┘         │  │
│  └──────────────────────────────────────────────────────────┘  │
│                            ↓ ↑                                  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │              AwsS3Ota Library (This Library)             │  │
│  │  • Version Checking                                      │  │
│  │  • HTTPS Download                                        │  │
│  │  • Task Management                                       │  │
│  │  • Callback System                                       │  │
│  └──────────────────────────────────────────────────────────┘  │
│                            ↓                                    │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                    ESP32 Arduino Core                     │  │
│  │  WiFi │ HTTPClient │ Update │ FreeRTOS                   │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                              ↓ WiFi/HTTPS
┌─────────────────────────────────────────────────────────────────┐
│                        AWS Cloud Infrastructure                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                      API Gateway                          │  │
│  │  https://xyz.execute-api.us-east-1.amazonaws.com/prod    │  │
│  └──────────────────────────────────────────────────────────┘  │
│                            ↓                                    │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                    Lambda Function                        │  │
│  │  • Generate Manifest                                      │  │
│  │  • Create Pre-signed URL                                  │  │
│  └──────────────────────────────────────────────────────────┘  │
│                            ↓                                    │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                      S3 Bucket                            │  │
│  │  firmware.bin (your compiled ESP32 firmware)             │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

## OTA Update Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                    Boot-Time OTA Check Flow                     │
└─────────────────────────────────────────────────────────────────┘

[1] Device Boots
      ↓
[2] Connect to WiFi
      ↓
[3] Create OTA Task (FreeRTOS)
      ↓
[4] Wait Startup Delay (configurable, default 10s)
      ↓
[5] onCheckStart() callback
      ├─ false → Exit (OTA denied)
      └─ true → Continue
      ↓
[6] Suspend Application Tasks
      ↓ onSuspendTasks() callback
      ↓
[7] Fetch Manifest from API Gateway
      ├─ GET https://your-api.com/prod/firmware
      └─ Response: {"version": "1.0.1", "url": "https://..."}
      ↓
[8] Compare Versions
      ├─ Same → onOtaUpToDate() → Resume Tasks → Exit
      └─ Different → Continue
      ↓
[9] onOtaStarted() callback
      ↓
[10] Download Firmware Binary
      ├─ Stream from S3 pre-signed URL
      ├─ Write to flash (Update API)
      └─ onOtaProgress() called repeatedly
      ↓
[11] Verify Flash
      ├─ Success → [12]
      └─ Failure → onOtaFailed() → Resume Tasks → Exit
      ↓
[12] onOtaFinished() callback
      ↓
[13] ESP.restart()
      ↓
[14] Device Boots with New Firmware
```

## Callback Execution Order

```
Normal OTA Update:
  1. onCheckStart()       → Return true/false
  2. onSuspendTasks()     → Return true/false
  3. onOtaStarted()       → Notification
  4. onOtaProgress()      → Called many times
  5. onOtaFinished()      → Notification
  6. [Device Reboots]

Failed OTA Update:
  1. onCheckStart()       → Return true/false
  2. onSuspendTasks()     → Return true/false
  3. onOtaStarted()       → Notification
  4. onOtaProgress()      → Called many times
  5. onOtaFailed(reason)  → With error message
  6. onResumeTasks()      → Resume your tasks
  7. [Continue Running]

Already Up-to-Date:
  1. onCheckStart()       → Return true/false
  2. onSuspendTasks()     → Return true/false
  3. onOtaUpToDate()      → Notification
  4. onResumeTasks()      → Resume your tasks
  5. [Continue Running]
```

## Task State Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                       Task State Changes                        │
└─────────────────────────────────────────────────────────────────┘

Before OTA:
  [Main Task: Running] ──┐
  [Worker Task: Running] ┼─→ Application Active
  [OTA Task: Not Created]┘

During OTA:
  [Main Task: SUSPENDED] ──┐
  [Worker Task: SUSPENDED] ┼─→ OTA In Progress
  [OTA Task: Running]──────┘

After Successful OTA:
  [Device Reboots]
  [All tasks restart with new firmware]

After Failed OTA:
  [Main Task: RESUMED] ──┐
  [Worker Task: RESUMED] ┼─→ Application Active Again
  [OTA Task: Deleted]────┘
```

## Data Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                     Manifest Request                            │
└─────────────────────────────────────────────────────────────────┘

ESP32 ─────GET────→ API Gateway ─────→ Lambda
                                         │
                                         ↓
                                    S3.generate_presigned_url()
                                         │
ESP32 ←────JSON──── API Gateway ←───────┘

JSON Response:
{
  "version": "1.0.1",
  "url": "https://bucket.s3.amazonaws.com/firmware.bin?AWSAccessKeyId=..."
}

┌─────────────────────────────────────────────────────────────────┐
│                   Firmware Download                             │
└─────────────────────────────────────────────────────────────────┘

ESP32 ─────GET────→ S3 (using pre-signed URL)
                     │
ESP32 ←───Binary────┘ (firmware.bin streamed in chunks)
  │
  └→ Update.write(chunk)
  └→ Flash Memory
```

## Security Model

```
┌─────────────────────────────────────────────────────────────────┐
│                      Security Layers                            │
└─────────────────────────────────────────────────────────────────┘

[1] WiFi Encryption
      └─ WPA2/WPA3
[2] TLS/HTTPS
      └─ ESP32 ←→ AWS (Port 443)
[3] Certificate Validation
      └─ AWS Root CA verification
[4] Pre-signed URLs
      └─ Time-limited (1 hour default)
[5] IAM Permissions
      └─ Lambda has read-only S3 access
[6] Private S3 Bucket
      └─ No public access
```

## Memory Layout During OTA

```
┌─────────────────────────────────────────────────────────────────┐
│                  ESP32 Flash Partition Layout                   │
└─────────────────────────────────────────────────────────────────┘

0x0000   ┌─────────────────────┐
         │   Bootloader        │
0x8000   ├─────────────────────┤
         │   Partition Table   │
0x9000   ├─────────────────────┤
         │   NVS (Settings)    │
0xC000   ├─────────────────────┤
         │   OTA Data          │
0xD000   ├─────────────────────┤
         │   PHY Init Data     │
0xE000   ├─────────────────────┤
         │                     │
         │   App0 (Current)    │ ← Currently Running
         │                     │
         ├─────────────────────┤
         │                     │
         │   App1 (OTA)        │ ← New Firmware Written Here
         │                     │
         ├─────────────────────┤
         │   SPIFFS/FATFS      │
         │   (File System)     │
         └─────────────────────┘

After Successful OTA:
  • Bootloader switches to App1
  • Next OTA writes to App0
  • Alternates between App0 and App1
```

## Error Handling Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                      Error Recovery                             │
└─────────────────────────────────────────────────────────────────┘

                   Error Occurs
                        ↓
         ┌──────────────┴──────────────┐
         │                             │
    Network Error              Flash Error
         │                             │
    Retry (3x)                  Abort
         │                             │
    ┌────┴────┐                        │
    │         │                        │
 Success   Failure ←───────────────────┘
    │         │
    │    onOtaFailed()
    │         │
    │    onResumeTasks()
    │         │
    └─────────┴──────→ Continue Running

Note: Device never bricks!
  • Current firmware keeps running
  • Bootloader validates new firmware
  • Rollback to previous if boot fails
```

## Typical Timeline

```
Time (s) │ Event
─────────┼─────────────────────────────────────────────
0        │ Device boots, WiFi connects
5        │ Application tasks start
10       │ OTA check begins (default startup delay)
11       │ API request sent
12       │ Manifest received, version compared
13       │ Download starts (if update available)
13-30    │ Firmware downloading (depends on size/speed)
30       │ Flash write complete
31       │ Verification complete
33       │ Device reboots
35       │ New firmware running
```

## Code Integration Pattern

```cpp
// Minimal Integration
void setup() {
  WiFi.begin(SSID, PASS);
  
  otaUpdater.begin(API_URL, ENDPOINT, VERSION, CA);
  otaUpdater.onCheckStart([]() { return true; });
  otaUpdater.onSuspendTasks([]() { return true; });
  otaUpdater.onResumeTasks([]() { return true; });
  otaUpdater.startBootCheckTask();  // Non-blocking
}

void loop() {
  // Your code runs here
  // OTA happens automatically in background
}
```

---

For more details, see the full documentation in README.md
