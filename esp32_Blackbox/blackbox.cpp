////////////////////////////////////////////////////////////////////////////////////
///////////Header Files/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
#include <Arduino.h>
#include "blackbox.h"

///////////////////////////////////////////////////////////////////////////////////
//// variable definitions  ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

 unsigned long myChannelNumber = 123455;         // your ThingsSpeakchannel ID  //2833649
 const char * myWriteAPIKey = "xxxxxx"; // your ThinsSpeak Write API Key  //QE46CI8UUR09UALY
 float tilt = 0;
 float humidity =0;
 float temperature =0;
 uint16_t vibrationState;
 sensors_event_t a, g, temp;
 double lat =0;
 double longtd = 0;

// Wi-Fi credentials
 const char* ssid = "open";                  //your SSID name
 const char* password = "12345678";          //your password

 char datebf[13];
 char timebf[9];

 char latbf[18];
 char longtbf[18];
 char satsbf[8];
////////////////////////////////////////////////////////////////////////
// Name: systemInit()
// Function: This function  initializes the system using beeps
// @param1: None
// @return: void
////////////////////////////////////////////////////////////////////////

void systemInit(){
  digitalWrite(BUZZER_PIN, HIGH);
  delay(50);
  digitalWrite(BUZZER_PIN, LOW);
  delay(50);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(50);
  digitalWrite(BUZZER_PIN, LOW);
  delay(50);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(50);
  digitalWrite(BUZZER_PIN, LOW);
  delay(50);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(50);
  digitalWrite(BUZZER_PIN, LOW);
  delay(500);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(800);
  digitalWrite(BUZZER_PIN, LOW);
}

////////////////////////////////////////////////////////////////////////
// Name: pinConfig()
// Function: This function  configures the pins for the system
// @param1: None
// @return: void
////////////////////////////////////////////////////////////////////////
void pinConfig(){
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIBRATION_SENSOR_PIN, INPUT);
  Serial.println("Vehicle Monitoring System Initialized!");
}


////////////////////////////////////////////////////////////////////////
// Name: connectToWifi()
// Function: This function  connects the system to WiFi
// @param1: None
// @return: void
////////////////////////////////////////////////////////////////////////
void connectToWifi(){
 WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    delay(5000);
  }
  Serial.println("Connected to WiFi!");
}


////////////////////////////////////////////////////////////////////////
// Name: connectToWifi()
// Function: This function sends collected data to ThingSpeak
// @param1: None
// @return: void
////////////////////////////////////////////////////////////////////////
void sendToThingspeak(){
  ThingSpeak.setField(1, temperature);      // temperature
  ThingSpeak.setField(2, a.acceleration.x); // Acceleration X
  ThingSpeak.setField(3, a.acceleration.y); // Acceleration Y 
  ThingSpeak.setField(4, a.acceleration.z); // Acceleration Z
  ThingSpeak.setField(5, tilt);             // Tilt angle
  ThingSpeak.setField(6, vibrationState);   // Vibration sensor state          
  // ThingSpeak.setField(7, (float)lat);       // latitude
  // ThingSpeak.setField(8, (float)longtd);    // longitude
  ThingSpeak.setField(7, (float)-1.3823);       // latitude
  ThingSpeak.setField(8, (float)36.7685);    // longitude
  


  int responseCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (responseCode == 200) {
    Serial.println("Channel update successful.");
  } else {
    Serial.println("Failed to update channel. HTTP error code " + String(responseCode));
  }
}

