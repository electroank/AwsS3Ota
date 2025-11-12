# ESP32 OTA via AWS S3

This repository demonstrates how to perform over-the-air (OTA) firmware updates for an ESP32 using a manifest hosted on an AWS S3 bucket.

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

