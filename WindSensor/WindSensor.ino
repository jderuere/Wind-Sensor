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

#include <YunClient.h>
#include <Mailbox.h>
#include <Console.h>
#include <YunServer.h>
#include <Process.h>
#include <BridgeSSLClient.h>
#include <BridgeUdp.h>
#include <Bridge.h>
#include <BridgeServer.h>
#include <FileIO.h>
#include <BridgeClient.h>
#include <HttpClient.h>
#include <ArduinoJson.h>

#define analogPinForRV    1   // change to pins you the analog pins are using
#define analogPinForTMP   0

#define PN_SERVER       "http://pubsub.pubnub.com"
#define WEBPAGE  "/publish/"

String pubkey =  "pub-c-1c1a123a-7113-4ecf-89e3-146b9d42a3ac";
String subkey = "sub-c-35c09676-2680-11e6-9f24-02ee2ddab7fe";
String channel = "%2Fusers%2Fjderuere%2Fconcordia-tml";

/****************************** Channel ***************************************/

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

HttpClient client;

void setup() {
  Bridge.begin();

  Serial.println(F("Internet done!"));
  Serial.println(F("Resolving PubNub..."));

  SerialUSB.begin(9600);

  while (!SerialUSB); // wait for a serial connection
  Serial.println(F("Resolved!"));

  //   Uncomment the three lines below to reset the analog pins A2 & A3
  //   This is code from the Modern Device temp sensor (not required)
  pinMode(A2, INPUT);        // GND pin
  pinMode(A3, INPUT);        // VCC pin
  digitalWrite(A3, LOW);     // turn off pullups
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

//  Serial.println(zeroWind_volts);

  // This from a regression from data in the form of
  // Vraw = V0 + b * WindSpeed ^ c
  // V0 is zero wind at a particular temperature
  // The constants b and c were determined by some Excel wrangling with the solver.

  WindSpeed_MPH = pow(((RV_Wind_Volts - zeroWind_volts) / .2300) , 2.7265);

  StaticJsonBuffer<200> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();
  root["sensor"] = "wind";
//  root["tmp_volts"] = TMP_Therm_ADunits * 0.0048828125;
  //  root["rv_volts"] = RV_Wind_Volts;
  //  root["tempc_100"] = TempCtimes100;
  //  root["zerowind_volts"] = zeroWind_volts;
  root["windspeed_mph"] = WindSpeed_MPH;

  String message;
  root.printTo(message);

  char messageBuffer[message.length() + 1];
  message.toCharArray(messageBuffer, message.length() + 1);

  //  Serial.println(messageBuffer);

  String message2 = URLEncode(messageBuffer);

  // Serial.println("Message: " + message);

  Serial.println("Publishing message:");
  String url = "http://pubsub.pubnub.com/publish/" + pubkey + "/" + subkey + "/0/" + channel + "/0/" + message2;
  Serial.println(url);
  client.get(url);

  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    char c = client.read();
    SerialUSB.print(c);
  }
  SerialUSB.flush();

  delay(1000);
}

String URLEncode(const char* msg)
{
  const char *hex = "0123456789abcdef";
  String encodedMsg = "";

  while (*msg != '\0') {
    if ('{' == *msg || '}' == *msg || ':' == *msg || ',' == *msg || '"' == *msg) {
      encodedMsg += '%';
      encodedMsg += hex[*msg >> 4];
      encodedMsg += hex[*msg & 15];
    } else {
      encodedMsg += *msg;
    }
    msg++;
  }
  return encodedMsg;
}

