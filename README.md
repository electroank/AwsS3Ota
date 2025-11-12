# ESP32 OTA via AWS S3

This repository demonstrates how to perform over-the-air (OTA) firmware updates for an ESP32 using a manifest hosted on an AWS S3 bucket.

## How It Works

The OTA update process follows these steps:

1. **Device checks for updates** - Your ESP32 connects to a manifest URL (JSON file hosted on S3)
2. **Version comparison** - The device compares the version in the manifest with its current firmware version
3. **Download decision** - If a newer version is available, the device downloads the firmware binary from the URL specified in the manifest
4. **Secure download** - The device downloads the firmware over HTTPS using SSL/TLS encryption
5. **Flash and reboot** - The new firmware is written to the OTA partition, and the device reboots into the new version

```
┌─────────┐         ┌──────────────┐         ┌─────────────┐
│  ESP32  │ ───1───>│ manifest.json│ ───2───>│ firmware.bin│
│ Device  │ <──4────│   (S3 URL)   │ <──3────│  (S3 URL)   │
└─────────┘         └──────────────┘         └─────────────┘
     │                                               │
     └───────────────5. Flash & Reboot──────────────┘
```

## Why AWS Root CA Certificate?

When your ESP32 downloads files from AWS S3, it uses **HTTPS** (not HTTP) to ensure:
- **Encryption** - Data cannot be intercepted or read by attackers
- **Authentication** - You're actually talking to AWS, not an imposter server
- **Integrity** - The firmware hasn't been tampered with during download

The **AWS Root CA certificate** is used to verify that the SSL/TLS certificate presented by S3 is legitimate. Without it:
- ❌ Your device could download malicious firmware from a fake server
- ❌ Attackers could perform man-in-the-middle attacks
- ❌ Your OTA updates would be insecure

With the certificate:
- ✅ Your device verifies it's connecting to real AWS servers
- ✅ All downloads are encrypted end-to-end
- ✅ Your firmware updates are secure and trustworthy

**The certificate is included in this library** - you just need to include it in your sketch!

## Prerequisites
- ESP32 partition scheme that supports OTA (e.g., factory + ota_0 + ota_1). Do not use a single huge app partition.
- An AWS S3 bucket.
- A compiled firmware binary (.bin) from your Arduino/PlatformIO build.
- The S3 objects (manifest and binary) must be accessible to the device — either public or via pre-signed URLs.

## Steps

1. Build your firmware
    - Compile your sketch and locate the generated .bin file (Arduino IDE typically produces a file like `yourproject.ino.bin`).

2. Upload the binary to S3
    - Use the AWS Console or AWS CLI. Example (AWS CLI):
      aws s3 cp ./yourcompiledbinfile.ino.bin s3://your-bucket/yourcompiledbinfile.ino.bin --acl public-read

3. Create a manifest file
    - The manifest is a JSON file containing at least `version` and `url`. You may add metadata (date, uploader, notes).
    - Example `manifest.json`:
      {"version":"1.2.0","url":"https://yours3bucket.s3.eu-north-1.amazonaws.com/yourcompiledbinfile.ino.bin"}

4. Host the manifest
    - Upload `manifest.json` to the same (or another) S3 bucket and ensure it is downloadable by the ESP32. Alternatively, use a pre-signed URL for the manifest.

5. Point your device to the manifest
    - In your device firmware (see example code in this library), set the manifest URL. The device will fetch the manifest, compare versions, download the `url` binary, and perform OTA if needed.

## Tips and notes
- Make sure the manifest `version` increases when you upload a new firmware; the device uses this to decide whether to update.
- If you cannot make the bucket public, use AWS pre-signed URLs (valid for a limited time) for the binary and/or manifest.
- During the OTA download and install, other tasks on the ESP32 will be paused. The update time depends on internet speed. The ESP32 will automatically restart after a successful update.
- Verify correct Content-Type (e.g., `application/octet-stream`) if you run into download issues.

That's it — follow the example code in this library and your ESP32 should be able to update from S3-hosted manifests and binaries.

