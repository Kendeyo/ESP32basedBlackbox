#ifndef BLACKBOX_H
#define BLACKBOX_H

////////////////////////////////////////////////////////////////////////////////////
///////////Header Files/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include <WiFi.h>
#include <ThingSpeak.h>
#include "DHT.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////
//// MACROS and Definitions////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

// Pin Definitions
#define DHTPIN 4
#define VIBRATION_SENSOR_PIN 35
#define BUZZER_PIN 13
#define rxGPS 25         
#define txGPS 26     

// Thresholds
#define TEMP_THRESHOLD 35
#define HUMIDITY_THRESHOLD 60
#define ACCEL_THRESHOLD 40  // Acceleration threshold in m/s^2 (e.g., a crash)
#define TILT_THRESHOLD1 90   // Tilt threshold in degrees
#define TILT_THRESHOLD2 30   // Tilt threshold in degrees
#define DHTTYPE DHT22


///////////////////////////////////////////////////////////////////////////////////
//// variable declarations////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
extern unsigned long myChannelNumber; 
extern const char * myWriteAPIKey; 
extern float tilt ;
extern float humidity ;
extern float temperature;
extern uint16_t vibrationState;


// Wi-Fi credentials
extern const char* ssid;
extern const char* password;


extern char datebf[13];
extern char timebf[9];

extern char latbf[18];
extern char longtbf[18];
extern char satsbf[8];


unsigned long previousMillis = 0; // Stores the last time the action was performed
const unsigned long interval = 15000; // Interval in milliseconds (15 seconds)

///////////////////////////////////////////////////////////////////////////////////
////Funcntion prototypes  /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void systemInit();
void pinConfig();
void connectToWifi();
void sendToThingspeak();

#endif