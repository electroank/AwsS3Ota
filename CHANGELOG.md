# Changelog

All notable changes to the AwsS3Ota library will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-01-12

### Added
- Initial release of AwsS3Ota library
- Support for all ESP32 variants (ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6, ESP32-H2)
- Secure HTTPS firmware downloads from AWS S3
- JSON manifest-based version checking
- FreeRTOS task for non-blocking boot-time updates
- Comprehensive callback system for application integration:
  - `onCheckStart()` - Pre-check conditions
  - `onSuspendTasks()` - Task suspension before OTA
  - `onResumeTasks()` - Task resumption after failed OTA
  - `onOtaStarted()` - OTA process started notification
  - `onOtaProgress()` - Download progress updates
  - `onOtaFinished()` - Successful OTA completion
  - `onOtaFailed()` - OTA failure notification
  - `onOtaUpToDate()` - Firmware already current notification
- Configurable settings:
  - Max retry attempts
  - Startup delay before boot check
  - Debug logging enable/disable
- Three example sketches:
  - SimpleOta - Minimal setup for quick start
  - BasicOta - Standard setup with task management
  - AdvancedOta - Complete example with LED feedback and manual triggers
- Comprehensive documentation:
  - Detailed README with AWS setup guide
  - API reference with usage examples
  - Troubleshooting section
  - Contributing guidelines
- AWS Root CA certificate validation
- Automatic retry mechanism with exponential backoff
- HTTP timeout handling for reliable downloads
- Progress tracking during firmware download
- Memory-efficient binary download with streaming
- Error handling and recovery
- Version comparison logic

### Documentation
- Complete README.md with:
  - Installation instructions (3 methods)
  - Quick start guide
  - AWS S3 and API Gateway setup tutorial
  - Lambda function example
  - Complete API reference
  - Troubleshooting guide
  - Board compatibility list
- CONTRIBUTING.md with development guidelines
- LICENSE (MIT)
- CHANGELOG.md (this file)
- Inline code documentation with Doxygen-style comments
- Example sketches with detailed comments

### Examples
- SimpleOta.ino - Minimal working example (~150 lines)
- BasicOta.ino - Standard implementation with tasks (~200 lines)
- AdvancedOta.ino - Full-featured with LED indicators (~300 lines)

### Build System
- library.properties with correct metadata
- keywords.txt for Arduino IDE syntax highlighting
- .gitignore for Arduino projects

## [Unreleased]

### Planned Features
- Support for custom HTTP headers in manifest requests
- Optional firmware signature verification
- Download progress percentage in callback
- Support for chunked downloads
- WiFi reconnection handling during download
- Configurable LED pins for visual feedback
- Support for other cloud providers (Azure, Google Cloud)
- OTA rollback mechanism
- Firmware version history tracking
- Delta updates (partial firmware updates)

### Known Issues
- None reported yet

---

## Version History

- **1.0.0** (2025-01-12) - Initial public release

## Upgrade Guide

### Upgrading to 1.0.0
This is the first release, no upgrade needed.

## Support

For issues, questions, or contributions:
- GitHub Issues: https://github.com/electroank/AwsS3Ota/issues
- Email: electroank@gmail.com

---

**Note**: This project follows semantic versioning. For more information, visit [semver.org](https://semver.org/).
