"""
AWS Lambda Function for AwsS3Ota Library
=========================================

This Lambda function generates a JSON manifest with firmware version
and a pre-signed S3 URL for downloading the firmware binary.

Setup Instructions:
1. Create an S3 bucket for your firmware files
2. Create this Lambda function in AWS Console
3. Grant the Lambda function S3 read permissions (IAM role)
4. Create an API Gateway REST API pointing to this function
5. Deploy the API and note the invoke URL

Environment Variables:
- BUCKET_NAME: Your S3 bucket name (e.g., 'my-esp32-firmware')
- FIRMWARE_KEY: S3 object key (e.g., 'firmware.bin' or 'v1.0.1/firmware.bin')
- FIRMWARE_VERSION: Current version string (e.g., '1.0.1')
"""

import json
import boto3
import os
from botocore.exceptions import ClientError

# Initialize S3 client
s3_client = boto3.client('s3')

# Configuration from environment variables
BUCKET_NAME = os.environ.get('BUCKET_NAME', 'your-firmware-bucket')
FIRMWARE_KEY = os.environ.get('FIRMWARE_KEY', 'firmware.bin')
FIRMWARE_VERSION = os.environ.get('FIRMWARE_VERSION', '1.0.0')

def lambda_handler(event, context):
    """
    Lambda handler function
    
    Returns a JSON manifest with firmware version and pre-signed S3 URL
    """
    
    print(f"Manifest request received")
    print(f"Bucket: {BUCKET_NAME}, Key: {FIRMWARE_KEY}, Version: {FIRMWARE_VERSION}")
    
    try:
        # Check if firmware file exists
        try:
            s3_client.head_object(Bucket=BUCKET_NAME, Key=FIRMWARE_KEY)
        except ClientError as e:
            if e.response['Error']['Code'] == '404':
                print(f"Firmware file not found: {FIRMWARE_KEY}")
                return {
                    'statusCode': 404,
                    'headers': {
                        'Content-Type': 'application/json',
                        'Access-Control-Allow-Origin': '*'
                    },
                    'body': json.dumps({
                        'error': 'Firmware file not found'
                    })
                }
            raise
        
        # Generate pre-signed URL (valid for 1 hour)
        presigned_url = s3_client.generate_presigned_url(
            'get_object',
            Params={
                'Bucket': BUCKET_NAME,
                'Key': FIRMWARE_KEY
            },
            ExpiresIn=3600  # URL expires in 1 hour
        )
        
        # Get firmware file size (optional, for logging)
        response = s3_client.head_object(Bucket=BUCKET_NAME, Key=FIRMWARE_KEY)
        file_size = response['ContentLength']
        
        print(f"Generated pre-signed URL for {FIRMWARE_KEY} ({file_size} bytes)")
        
        # Return manifest
        manifest = {
            'version': FIRMWARE_VERSION,
            'url': presigned_url
        }
        
        return {
            'statusCode': 200,
            'headers': {
                'Content-Type': 'application/json',
                'Access-Control-Allow-Origin': '*',  # Enable CORS if needed
                'Cache-Control': 'no-cache'
            },
            'body': json.dumps(manifest)
        }
        
    except ClientError as e:
        print(f"AWS Error: {e}")
        return {
            'statusCode': 500,
            'headers': {
                'Content-Type': 'application/json',
                'Access-Control-Allow-Origin': '*'
            },
            'body': json.dumps({
                'error': 'Failed to generate firmware URL',
                'details': str(e)
            })
        }
    except Exception as e:
        print(f"Unexpected error: {e}")
        return {
            'statusCode': 500,
            'headers': {
                'Content-Type': 'application/json',
                'Access-Control-Allow-Origin': '*'
            },
            'body': json.dumps({
                'error': 'Internal server error',
                'details': str(e)
            })
        }


# Example response:
# {
#     "version": "1.0.1",
#     "url": "https://your-bucket.s3.amazonaws.com/firmware.bin?AWSAccessKeyId=..."
# }
