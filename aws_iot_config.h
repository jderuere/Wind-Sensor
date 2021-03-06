/*
 * Copyright 2010-2016 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef config_usr_h
#define config_usr_h

// Copy and paste your configuration into this file
//===============================================================
#define AWS_IOT_MQTT_HOST "abp2c8hh6llep.iot.us-east-1.amazonaws.com"
// your endpoint
#define AWS_IOT_MQTT_PORT 8883
// your port
#define AWS_IOT_CLIENT_ID      "windsensorduino"
// your client ID
#define AWS_IOT_MY_THING_NAME "windsensorduino"
// your thing name
#define AWS_IOT_ROOT_CA_FILENAME "VeriSign-Class 3-Public-Primary-Certification-Authority-G5.pem"
// your root-CA filename
#define AWS_IOT_CERTIFICATE_FILENAME "04bcd8f99d-certificate.pem.crt"
// your certificate filename
#define AWS_IOT_PRIVATE_KEY_FILENAME "04bcd8f99d-private.pem.key"
// your private key filename
//===============================================================
// SDK config, DO NOT modify it
#define AWS_IOT_PATH_PREFIX "../certs/"
#define AWS_IOT_ROOT_CA_PATH AWS_IOT_PATH_PREFIX AWS_IOT_ROOT_CA_FILENAME			// use this in config call
#define AWS_IOT_CERTIFICATE_PATH AWS_IOT_PATH_PREFIX AWS_IOT_CERTIFICATE_FILENAME	// use this in config call
#define AWS_IOT_PRIVATE_KEY_PATH AWS_IOT_PATH_PREFIX AWS_IOT_PRIVATE_KEY_FILENAME	// use this in config call

#endif
