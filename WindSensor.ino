/* Modern Device Wind Sensor Sketch for Rev C Wind Sensor
  This sketch is only valid if the wind sensor if powered from
  a regulated 5 volt supply. An Arduino or Modern Device BBB, RBBB
  powered from an external power supply should work fine. Powering from
  USB will also work but will be slightly less accurate in our experience.

  When using an Arduino to power the sensor, an external power supply is better. Most Arduinos have a
  polyfuse which protects the USB line. This fuse has enough resistance to reduce the voltage
  available to around 4.7 to 4.85 volts, depending on the current draw.

  The sketch uses the on-chip temperature sensing thermistor to compensate the sensor
  for changes in ambient temperature. Because the thermistor is just configured as a
  voltage divider, the voltage will change with supply voltage. This is why the
  sketch depends upon a regulated five volt supply.

  Other calibrations could be developed for different sensor supply voltages, but would require
  gathering data for those alternate voltages, or compensating the ratio.

  Hardware Setup:
  Wind Sensor Signals    Arduino
  GND                    GND
  +V                     5V
  RV                     A1    // modify the definitions below to use other pins
  TMP                    A0    // modify the definitions below to use other pins


  Paul Badger 2014

  Hardware setup:
  Wind Sensor is powered from a regulated five volt source.
  RV pin and TMP pin are connected to analog inputs.

*/

#include <aws_iot_mqtt.h>
#include <aws_iot_version.h>
#include "aws_iot_config.h"

#define analogPinForRV    1   // change to pins you the analog pins are using
#define analogPinForTMP   0

aws_iot_mqtt_client myClient;

char msg[32]; // read-write buffer
int rc = -100; // return value placeholder
bool success_connect = false; // whether it is connected

// to calibrate your sensor, put a glass over it, but the sensor should not be
// touching the desktop surface however.
// adjust the zeroWindAdjustment until your sensor reads about zero with the glass over it.

const float zeroWindAdjustment =  .2; // negative numbers yield smaller wind speeds and vice versa.

int TMP_Therm_ADunits;  //temp termistor value from wind sensor
float RV_Wind_ADunits;    //RV output from wind sensor
float RV_Wind_Volts;
unsigned long lastMillis;
int TempCtimes100;
float zeroWind_ADunits;
float zeroWind_volts;
float WindSpeed_MPH;

void setup() {
  Serial.begin(115200);

  while (!Serial); // wait for a serial connection

  char curr_version[80];
  snprintf_P(curr_version, 80, PSTR("AWS IoT SDK Version(dev) %d.%d.%d-%s\n"), VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);
  Serial.println(curr_version);

  // Set up the client
  if((rc = myClient.setup(AWS_IOT_CLIENT_ID)) == 0) {
    // Load user configuration
    if((rc = myClient.config(AWS_IOT_MQTT_HOST, AWS_IOT_MQTT_PORT, AWS_IOT_ROOT_CA_PATH, AWS_IOT_PRIVATE_KEY_PATH, AWS_IOT_CERTIFICATE_PATH)) == 0) {
      // Use default connect: 60 sec for keepalive
      if((rc = myClient.connect()) == 0) {
        success_connect = true;
      } else {
        Serial.println(F("Config failed!"));
        Serial.println(rc);
      }
    } else {
      Serial.println(F("Setup failed!"));
      Serial.println(rc);
    }
  }

  //   Uncomment the three lines below to reset the analog pins A2 & A3
  //   This is code from the Modern Device temp sensor (not required)
  pinMode(A2, INPUT);        // GND pin
  pinMode(A3, INPUT);        // VCC pin
  digitalWrite(A3, LOW);     // turn off pullups

  // Delay to make sure SUBACK is received, delay time could vary according to the server
  delay(1000);
}

void loop() {
  TMP_Therm_ADunits = analogRead(analogPinForTMP);
  RV_Wind_ADunits = analogRead(analogPinForRV);
  RV_Wind_Volts = (RV_Wind_ADunits * 0.0048828125);

  // these are all derived from regressions from raw data as such they depend on a lot of experimental factors
  // such as accuracy of temp sensors, and voltage at the actual wind sensor, (wire losses) which were unaccouted for.
  TempCtimes100 = (0.005 * ((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits)) - (16.862 * (float)TMP_Therm_ADunits) + 9075.4;

  zeroWind_ADunits = -0.0006 * ((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits) + 1.0727 * (float)TMP_Therm_ADunits + 47.172; //  13.0C  553  482.39

  zeroWind_volts = (zeroWind_ADunits * 0.0048828125) - zeroWindAdjustment;

  // This from a regression from data in the form of
  // Vraw = V0 + b * WindSpeed ^ c
  // V0 is zero wind at a particular temperature
  // The constants b and c were determined by some Excel wrangling with the solver.

  WindSpeed_MPH = pow(((RV_Wind_Volts - zeroWind_volts) / .2300) , 2.7265);
  if(!isnan(WindSpeed_MPH)) {
    Serial.println(WindSpeed_MPH);
      
   if (success_connect) {
      char buffer[10];
      dtostrf(WindSpeed_MPH, 2, 2, buffer);
  
      // Generate a new message in each loop and publish to a topic
      if((rc = myClient.publish("windspeed_mph", buffer, 4, 1, false)) != 0) {
        Serial.println(F("Publish failed!"));
        Serial.println(rc);
      }
    
      // Get a chance to run a callback
      if((rc = myClient.yield()) != 0) {
        Serial.println("Yield failed!");
        Serial.println(rc);
      }
   }
 }
 
 delay(10); 
}
