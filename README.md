# AwsS3Ota - ESP32 OTA Updates from AWS S3

[![Arduino Library](https://img.shields.io/badge/Arduino-Library-blue.svg)](https://www.arduino.cc/reference/en/libraries/)
[![ESP32](https://img.shields.io/badge/ESP32-Compatible-green.svg)](https://www.espressif.com/en/products/socs/esp32)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A comprehensive Arduino library for ESP32 boards to perform secure Over-The-Air (OTA) firmware updates from AWS S3.

## ✨ Features

- ✅ **Universal ESP32 Support** - Works with all ESP32 variants (ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6)
- 🔒 **Secure HTTPS Downloads** - Uses AWS Root CA certificate validation
- 🔄 **Version Management** - Automatic version checking via JSON manifest
- ⚡ **Non-Blocking Boot Checks** - FreeRTOS task for automatic updates on startup
- 📊 **Progress Callbacks** - Real-time feedback during download and flashing
- 🎛️ **Flexible Control** - Callback system for complete application integration
- 🔁 **Automatic Retries** - Configurable retry mechanism for network reliability
- 📝 **Debug Logging** - Optional verbose logging for troubleshooting

## 📋 Table of Contents

- [Installation](#-installation)
- [Dependencies](#-dependencies)
- [Quick Start](#-quick-start)
- [AWS Setup](#-aws-setup)
- [API Reference](#-api-reference)
- [Examples](#-examples)
- [Troubleshooting](#-troubleshooting)
- [Contributing](#-contributing)
- [License](#-license)

## 📦 Installation

### Method 1: Arduino Library Manager (Recommended)

1. Open Arduino IDE
2. Go to **Sketch** → **Include Library** → **Manage Libraries...**
3. Search for "AwsS3Ota"
4. Click **Install**

### Method 2: Manual Installation

1. Download this repository as a ZIP file
2. In Arduino IDE, go to **Sketch** → **Include Library** → **Add .ZIP Library...**
3. Select the downloaded ZIP file

### Method 3: Git Clone

```bash
cd ~/Documents/Arduino/libraries/
git clone https://github.com/electroank/AwsS3Ota.git
```

## 📚 Dependencies

This library requires:

- **ArduinoJson** (v6.x or later) - Install from Library Manager
- **WiFi** - Included with ESP32 core
- **HTTPClient** - Included with ESP32 core
- **WiFiClientSecure** - Included with ESP32 core
- **Update** - Included with ESP32 core

## 🚀 Quick Start

### 1. Basic Setup

```cpp
#include <WiFi.h>
#include "AwsS3Ota.h"

#define FIRMWARE_VERSION "1.0.0"

const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASS = "YourWiFiPassword";
const char* OTA_API_BASE_URL = "https://your-api.execute-api.us-east-1.amazonaws.com";
const char* OTA_FIRMWARE_ENDPOINT = "/prod/firmware";

// AWS Root CA Certificate (download from https://www.amazontrust.com/repository/AmazonRootCA1.pem)
const char* AWS_ROOT_CA = 
"-----BEGIN CERTIFICATE-----\n"
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ik3szjEGiTANBgkqhkiG9w0BAQsF\n"
// ... (rest of certificate)
"-----END CERTIFICATE-----\n";

AwsS3Ota otaUpdater;

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Configure OTA
  otaUpdater.begin(OTA_API_BASE_URL, OTA_FIRMWARE_ENDPOINT, FIRMWARE_VERSION, AWS_ROOT_CA);
  otaUpdater.setDebug(true);
  
  // Set required callbacks
  otaUpdater.onCheckStart([]() { return true; });
  otaUpdater.onSuspendTasks([]() { return true; });
  otaUpdater.onResumeTasks([]() { return true; });
  
  // Start automatic boot check
  otaUpdater.startBootCheckTask();
}

void loop() {
  // Your application code
}
```

### 2. Manifest File Format

Your API endpoint should return a JSON file with this structure:

```json
{
  "version": "1.0.1",
  "url": "https://your-bucket.s3.amazonaws.com/firmware.bin?X-Amz-Algorithm=..."
}
```

- **version**: Semantic version string (e.g., "1.0.1")
- **url**: Pre-signed HTTPS URL to the firmware binary in S3

## ☁️ AWS Setup

### Step 1: Create an S3 Bucket

```bash
# Using AWS CLI
aws s3 mb s3://your-firmware-bucket --region us-east-1
```

Or use the AWS Console:
1. Go to S3 console
2. Click "Create bucket"
3. Name it (e.g., `my-esp32-firmware`)
4. Keep default settings

### Step 2: Upload Firmware Binary

```bash
# Upload your .bin file
aws s3 cp firmware.bin s3://your-firmware-bucket/firmware.bin
```

### Step 3: Create Lambda Function

Create a Lambda function to generate pre-signed URLs:

```python
import json
import boto3
from botocore.exceptions import ClientError

s3_client = boto3.client('s3')
BUCKET_NAME = 'your-firmware-bucket'
FIRMWARE_KEY = 'firmware.bin'
CURRENT_VERSION = '1.0.1'

def lambda_handler(event, context):
    try:
        # Generate pre-signed URL (valid for 1 hour)
        url = s3_client.generate_presigned_url(
            'get_object',
            Params={'Bucket': BUCKET_NAME, 'Key': FIRMWARE_KEY},
            ExpiresIn=3600
        )
        
        return {
            'statusCode': 200,
            'headers': {
                'Content-Type': 'application/json',
                'Access-Control-Allow-Origin': '*'
            },
            'body': json.dumps({
                'version': CURRENT_VERSION,
                'url': url
            })
        }
    except ClientError as e:
        print(e)
        return {
            'statusCode': 500,
            'body': json.dumps({'error': 'Failed to generate URL'})
        }
```

### Step 4: Create API Gateway

1. Go to API Gateway console
2. Create a REST API
3. Create a GET method pointing to your Lambda function
4. Deploy to a stage (e.g., `prod`)
5. Note the Invoke URL: `https://abc123.execute-api.us-east-1.amazonaws.com/prod`

### Step 5: Get AWS Root CA

Download the Amazon Root CA certificate:

```bash
curl -o AmazonRootCA1.pem https://www.amazontrust.com/repository/AmazonRootCA1.pem
```

Then convert it to a C string for your sketch.

## 📖 API Reference

### Initialization

#### `begin(apiBaseUrl, firmwareEndpoint, currentVersion, rootCa)`

Initialize the OTA updater with your configuration.

**Parameters:**
- `apiBaseUrl` (const char*) - Base URL of your API Gateway
- `firmwareEndpoint` (const char*) - Endpoint path (e.g., "/prod/firmware")
- `currentVersion` (const char*) - Current firmware version string
- `rootCa` (const char*) - AWS Root CA certificate in PEM format

**Example:**
```cpp
otaUpdater.begin(
  "https://abc123.execute-api.us-east-1.amazonaws.com",
  "/prod/firmware",
  "1.0.0",
  AWS_ROOT_CA
);
```

### Configuration Methods

#### `setDebug(bool enabled)`

Enable or disable debug logging to Serial.

**Default:** `true`

```cpp
otaUpdater.setDebug(true);  // Enable debug logs
```

#### `setMaxRetries(int retries)`

Set the maximum number of retry attempts for fetching the manifest.

**Default:** `3`

```cpp
otaUpdater.setMaxRetries(5);  // Retry up to 5 times
```

#### `setStartupDelay(unsigned long delayMs)`

Set delay before boot-time OTA check starts.

**Default:** `10000` (10 seconds)

```cpp
otaUpdater.setStartupDelay(5000);  // Wait 5 seconds
```

### Callback Methods

#### `onCheckStart(callback)`

**Required callback.** Called before OTA check begins. Return `false` to deny OTA.

```cpp
otaUpdater.onCheckStart([]() {
  if (batteryLevel < 20) return false;  // Don't OTA if battery low
  return true;  // Allow OTA
});
```

#### `onSuspendTasks(callback)`

**Required callback.** Called to suspend application tasks before OTA.

```cpp
TaskHandle_t myTaskHandle;

otaUpdater.onSuspendTasks([]() {
  if (myTaskHandle) vTaskSuspend(myTaskHandle);
  mqttClient.disconnect();
  return true;
});
```

#### `onResumeTasks(callback)`

**Required callback.** Called to resume application tasks if OTA fails.

```cpp
otaUpdater.onResumeTasks([]() {
  if (myTaskHandle) vTaskResume(myTaskHandle);
  mqttClient.connect();
  return true;
});
```

#### `onOtaStarted(callback)`

Optional callback when OTA download starts.

```cpp
otaUpdater.onOtaStarted([]() {
  Serial.println("OTA started!");
  digitalWrite(LED_PIN, HIGH);
});
```

#### `onOtaProgress(callback)`

Optional callback during download (called frequently).

```cpp
otaUpdater.onOtaProgress([]() {
  static bool ledState = false;
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState);  // Blink LED
});
```

#### `onOtaFinished(callback)`

Optional callback when OTA succeeds (before reboot).

```cpp
otaUpdater.onOtaFinished([]() {
  Serial.println("OTA complete! Rebooting...");
});
```

#### `onOtaFailed(callback)`

Optional callback when OTA fails.

```cpp
otaUpdater.onOtaFailed([](const char* reason) {
  Serial.printf("OTA failed: %s\n", reason);
  digitalWrite(LED_ERROR, HIGH);
});
```

#### `onOtaUpToDate(callback)`

Optional callback when firmware is already current.

```cpp
otaUpdater.onOtaUpToDate([]() {
  Serial.println("Firmware is up to date!");
});
```

### Update Methods

#### `checkAndUpdate(bool userInitiated = false)`

Check for updates and perform OTA if available. **This is a blocking call.**

**Parameters:**
- `userInitiated` (bool) - Set to `true` if manually triggered (for logging)

**Returns:** `true` if update was successful (device will reboot), `false` otherwise

```cpp
// Manual check triggered by button press
if (digitalRead(BUTTON_PIN) == LOW) {
  bool updated = otaUpdater.checkAndUpdate(true);
  if (!updated) {
    Serial.println("No update available or failed");
  }
}
```

#### `startBootCheckTask(taskName, stackSize, priority)`

Start a FreeRTOS task to check for updates on boot (non-blocking).

**Parameters:**
- `taskName` (const char*) - Task name (default: "otaCheckTask")
- `stackSize` (uint32_t) - Stack size in bytes (default: 8192)
- `priority` (UBaseType_t) - Task priority (default: 1)

```cpp
otaUpdater.startBootCheckTask();  // Use defaults
// or
otaUpdater.startBootCheckTask("myOtaTask", 10240, 2);  // Custom settings
```

## 💡 Examples

### Example 1: Simple OTA (Minimal Setup)

See `examples/SimpleOta/SimpleOta.ino` for a minimal working example.

### Example 2: Basic OTA with LED Feedback

See `examples/BasicOta/BasicOta.ino` for a complete example with LED indicators.

### Example 3: Advanced OTA with Task Management

See `examples/AdvancedOta/AdvancedOta.ino` for integration with FreeRTOS tasks, MQTT, and sensors.

## 🔧 Troubleshooting

### "Manifest fetch failed"

- Check your WiFi connection
- Verify API Gateway URL is correct
- Ensure API Gateway CORS is configured (if needed)
- Check Lambda function logs in CloudWatch

### "Invalid URL (not https)"

- Ensure your S3 pre-signed URL starts with `https://`
- Check Lambda function is generating the URL correctly

### "Update.begin failed"

- Your ESP32 may not have enough flash space
- Check partition scheme in Arduino IDE (Tools → Partition Scheme)
- Use a scheme with OTA support (e.g., "Minimal SPIFFS (1.9MB APP with OTA)")

### "Incomplete download"

- Check network stability
- Try increasing the HTTP timeout in the library
- Verify the firmware binary is not corrupted

### Task Won't Resume

- Make sure `onResumeTasks` callback is properly implemented
- Check that task handles are valid before suspending/resuming

## 📊 Supported Boards

- ✅ ESP32 (original) - all variants
- ✅ ESP32-S2
- ✅ ESP32-S3
- ✅ ESP32-C3
- ✅ ESP32-C6
- ✅ ESP32-H2

## 🤝 Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

### Development Setup

1. Fork this repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👤 Author

**Ankan Sarkar**
- GitHub: [@electroank](https://github.com/electroank)
- Email: electroank@gmail.com

## 🙏 Acknowledgments

- ESP32 Arduino Core team
- ArduinoJson library by Benoit Blanchon
- AWS SDK documentation
- The ESP32 community

## 📝 Changelog

### Version 1.0.0 (2025-01-12)

- Initial release
- Support for all ESP32 variants
- Secure HTTPS downloads from S3
- FreeRTOS task support
- Comprehensive callback system
- Automatic version checking
- Configurable retry mechanism

---

**Star ⭐ this repo if you find it useful!**
