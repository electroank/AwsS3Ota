/**
 * @file aws_root_ca.h
 * @brief Amazon Root CA Certificate for AWS S3 HTTPS connections
 * 
 * This is the Amazon Root CA 1 certificate used for validating
 * HTTPS connections to AWS services (S3, API Gateway).
 * 
 * Downloaded from: https://www.amazontrust.com/repository/AmazonRootCA1.pem
 * Valid until: January 17, 2038
 * 
 * You typically don't need to modify this file.
 */

#ifndef AWS_ROOT_CA_H
#define AWS_ROOT_CA_H

const char* AWS_ROOT_CA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ik3szjEGiTANBgkqhkiG9w0BAQsF\n" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvoEXnqmKrvc6\n" \
"Dwh6FsQ6+kM2ujzAUeD0HeQueryVatXHGTdp9pQgnL28fNpiUQbpWJNK6ANEYIjz\n" \
"HeM5dE95OlzmS6xJlOyjpUp2gsr0PMMaxM80L11AC9aqNEoVLpc3bBVCGYJTgwCi\n" \
"oGjwO5dyKkYxNANfYm2wYarNf6S8ZF93v2IL5AoR8UOLsHwYJelQodwBaP/GAmB+\n" \
"PJ+jrmSKbqooQQxfopDHcrfEbcQjVr9SQberCaq8beMaOb5BAgBR7Yot3LbSKibw\n" \
"MO+tLIVcGBECAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0OBBYEFIQhAEqCUVimxKP39Kxuc65KGSFcMA0GCSqGSIb3DQEBCwUA\n" \
"A4IBAQCSLpCTMiEyQgVBGefYk3IPzNOZ5KjEbmxs9S/o9muLBrhBmcccCjPoTwSj\n" \
"OmyDVEP1nGYUoZIc2sbSHcmTYrqY1KsH4S0JgE9OZIPR0xptrarU0i6mR/LbR3pL\n" \
"tGplwiQNkvAUNjGANVUY5qV8ubRHf1N26CX6sNm8SgmGrRVTgcglC8jOPZ0nkyc1\n" \
"1GTTnstdPfID0eIRmQtyhVbfTuQY2N3mYm/RHMJj+GXO66Xls3TM2q2Iq9mYmKDr\n" \
"jEa2uKZQEIQBCbF4MwfIom1UvBnvnCVwzYg2C/EGyY+LCPiJzXQfG8jEaMlt0WbN\n" \
"H/PZtRNin5WoTfiOQtvFv2/E1IY+\n" \
"-----END CERTIFICATE-----\n";

#endif // AWS_ROOT_CA_H
