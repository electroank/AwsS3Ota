# AWS Setup Guide for AwsS3Ota

This guide walks you through setting up AWS infrastructure for OTA updates.

## Prerequisites

- AWS Account
- AWS CLI installed (optional, but recommended)
- Basic knowledge of AWS services

## Architecture Overview

```
ESP32 Device
    ↓
API Gateway (HTTPS)
    ↓
Lambda Function
    ↓
S3 Bucket (Firmware Binary)
    ↓
Pre-signed URL
    ↓
ESP32 Downloads Firmware
```

## Step 1: Create an S3 Bucket

### Using AWS Console:

1. Go to [AWS S3 Console](https://console.aws.amazon.com/s3/)
2. Click **"Create bucket"**
3. **Bucket name**: `my-esp32-firmware` (must be globally unique)
4. **Region**: Choose your preferred region (e.g., `us-east-1`)
5. **Block Public Access**: Keep all public access blocked (we'll use pre-signed URLs)
6. Click **"Create bucket"**

### Using AWS CLI:

```bash
aws s3 mb s3://my-esp32-firmware --region us-east-1
```

## Step 2: Upload Firmware Binary

### Build Your Firmware:

1. In Arduino IDE, go to **Sketch** → **Export compiled Binary**
2. The `.bin` file will be saved in your sketch folder
3. Note the file path (e.g., `MyProject.ino.esp32.bin`)

### Upload to S3:

#### Using AWS Console:
1. Open your S3 bucket
2. Click **"Upload"**
3. Select your `.bin` file
4. Click **"Upload"**

#### Using AWS CLI:
```bash
aws s3 cp MyProject.ino.esp32.bin s3://my-esp32-firmware/firmware.bin
```

### Organize by Version (Recommended):
```bash
# Upload with version number
aws s3 cp firmware.bin s3://my-esp32-firmware/v1.0.1/firmware.bin

# Or use versioning
aws s3api put-bucket-versioning \
    --bucket my-esp32-firmware \
    --versioning-configuration Status=Enabled
```

## Step 3: Create IAM Role for Lambda

### Using AWS Console:

1. Go to [IAM Console](https://console.aws.amazon.com/iam/)
2. Click **"Roles"** → **"Create role"**
3. **Trusted entity**: AWS Service → Lambda
4. **Permissions**: Search and add:
   - `AWSLambdaBasicExecutionRole` (for CloudWatch logs)
   - Create a custom inline policy for S3:

```json
{
    "Version": "2012-10-17",
    "Statement": [
        {
            "Effect": "Allow",
            "Action": [
                "s3:GetObject",
                "s3:HeadObject"
            ],
            "Resource": "arn:aws:s3:::my-esp32-firmware/*"
        }
    ]
}
```

5. **Role name**: `esp32-ota-lambda-role`
6. Click **"Create role"**

## Step 4: Create Lambda Function

### Using AWS Console:

1. Go to [Lambda Console](https://console.aws.amazon.com/lambda/)
2. Click **"Create function"**
3. **Function name**: `esp32-firmware-manifest`
4. **Runtime**: Python 3.11 (or latest)
5. **Permissions**: Use existing role → `esp32-ota-lambda-role`
6. Click **"Create function"**

### Add Function Code:

Copy the code from `extras/lambda_function.py` in this library.

### Configure Environment Variables:

1. In Lambda console, go to **"Configuration"** → **"Environment variables"**
2. Add these variables:
   - `BUCKET_NAME` = `my-esp32-firmware`
   - `FIRMWARE_KEY` = `firmware.bin`
   - `FIRMWARE_VERSION` = `1.0.1`

### Test the Function:

1. Click **"Test"** tab
2. Create a new test event (any name)
3. Use default test event template
4. Click **"Test"**
5. Check the response - should return JSON with version and URL

## Step 5: Create API Gateway

### Using AWS Console:

1. Go to [API Gateway Console](https://console.aws.amazon.com/apigateway/)
2. Click **"Create API"** → **"REST API"** (not private)
3. Click **"Build"**
4. **API name**: `esp32-ota-api`
5. Click **"Create API"**

### Create Resource and Method:

1. Click **"Actions"** → **"Create Resource"**
   - **Resource Name**: `firmware`
   - **Resource Path**: `/firmware`
   - Click **"Create Resource"**

2. Select the `/firmware` resource
3. Click **"Actions"** → **"Create Method"** → Select **GET**
4. **Integration type**: Lambda Function
5. **Lambda Function**: `esp32-firmware-manifest`
6. Click **"Save"** → **"OK"** (to give API Gateway permission)

### Enable CORS (Optional but Recommended):

1. Select the `/firmware` resource
2. Click **"Actions"** → **"Enable CORS"**
3. Keep defaults
4. Click **"Enable CORS and replace existing CORS headers"**

### Deploy API:

1. Click **"Actions"** → **"Deploy API"**
2. **Deployment stage**: New Stage
3. **Stage name**: `prod`
4. Click **"Deploy"**

### Note Your Invoke URL:

You'll see something like:
```
https://abc123xyz.execute-api.us-east-1.amazonaws.com/prod
```

**This is your `API_BASE_URL`** for the Arduino sketch!

Your endpoint will be: `/firmware`

## Step 6: Get AWS Root CA Certificate

Download the Amazon Root CA certificate:

```bash
curl -o AmazonRootCA1.pem https://www.amazontrust.com/repository/AmazonRootCA1.pem
```

Convert to C string format for Arduino:

```bash
# On Linux/Mac:
awk 'NF {sub(/\r/, ""); printf "%s\\n\"\\\n\"", $0;}' AmazonRootCA1.pem

# Or manually:
# 1. Open the .pem file
# 2. Add "-----BEGIN CERTIFICATE-----\n" \ at the start
# 3. Add \n after each line
# 4. Add "-----END CERTIFICATE-----\n"; at the end
```

Result should look like:
```cpp
const char* AWS_ROOT_CA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ik3szjEGiTANBgkqhkiG9w0BAQsF\n" \
// ... more lines ...
"-----END CERTIFICATE-----\n";
```

## Step 7: Update Your Arduino Sketch

```cpp
#define FIRMWARE_VERSION "1.0.0"

const char* API_BASE_URL = "https://abc123xyz.execute-api.us-east-1.amazonaws.com";
const char* API_ENDPOINT = "/prod/firmware";
const char* AWS_ROOT_CA = "..."; // Paste the certificate here
```

## Testing Your Setup

### Test API Gateway:

```bash
curl https://abc123xyz.execute-api.us-east-1.amazonaws.com/prod/firmware
```

Expected response:
```json
{
    "version": "1.0.1",
    "url": "https://my-esp32-firmware.s3.amazonaws.com/firmware.bin?AWSAccessKeyId=..."
}
```

### Test Pre-signed URL:

Copy the `url` from above and paste it in a browser. Your firmware binary should download.

### Test with ESP32:

1. Upload sketch with version `1.0.0`
2. The device will check for updates on boot
3. If your Lambda returns `1.0.1`, the device will update
4. If it returns `1.0.0`, device will report "up to date"

## Updating Firmware

### To deploy a new version:

1. **Update your Arduino code** with new version number:
   ```cpp
   #define FIRMWARE_VERSION "1.0.1"
   ```

2. **Build and upload** the binary to S3:
   ```bash
   aws s3 cp new-firmware.bin s3://my-esp32-firmware/firmware.bin
   ```

3. **Update Lambda environment variable**:
   ```bash
   aws lambda update-function-configuration \
       --function-name esp32-firmware-manifest \
       --environment "Variables={BUCKET_NAME=my-esp32-firmware,FIRMWARE_KEY=firmware.bin,FIRMWARE_VERSION=1.0.1}"
   ```

4. Devices running `1.0.0` will now update to `1.0.1` on next check!

## Cost Estimation

For typical IoT usage:

- **S3**: ~$0.023/GB storage, minimal for firmware files
- **Lambda**: Free tier: 1M requests/month, 400,000 GB-seconds compute
- **API Gateway**: Free tier: 1M API calls/month for 12 months
- **Data Transfer**: First 1 GB/month free, then ~$0.09/GB

**Example**: 100 devices checking once per day = ~3,000 requests/month = FREE

## Security Best Practices

1. **Never commit AWS credentials** to GitHub
2. **Use IAM roles** with least privilege
3. **Enable S3 bucket versioning** for rollback capability
4. **Set CloudWatch alarms** for unusual API usage
5. **Use pre-signed URLs** (not public S3 URLs)
6. **Validate firmware** before uploading
7. **Test updates** on a single device first
8. **Keep Lambda logs** for troubleshooting

## Troubleshooting

### Lambda returns 403 Forbidden
- Check IAM role has S3 read permissions
- Verify bucket name and key are correct

### ESP32 can't connect to API
- Check WiFi credentials
- Verify API Gateway URL is correct
- Test API with curl first

### "Invalid certificate"
- Ensure you're using the correct AWS Root CA
- Check certificate is not expired

### OTA downloads but fails to flash
- Check partition scheme in Arduino IDE
- Ensure firmware binary is valid
- Verify enough free flash space

## Advanced Topics

### Multiple Firmware Versions:
```python
# Modify Lambda to support query parameters
version = event.get('queryStringParameters', {}).get('channel', 'stable')
FIRMWARE_KEY = f'{version}/firmware.bin'
```

### Rollback Mechanism:
Enable S3 versioning and modify Lambda to return specific version IDs.

### Firmware Signing:
Generate SHA256 hash and include in manifest for verification.

## Support

For issues with this setup:
- Check [AWS Documentation](https://docs.aws.amazon.com/)
- Open an issue on [GitHub](https://github.com/electroank/AwsS3Ota/issues)
- Email: electroank@gmail.com

---

**Happy OTA updating! 🚀**
