
#include <Arduino.h>
#include "blackbox.h"

 unsigned long myChannelNumber = 2833649; 
 const char * myWriteAPIKey = "QE46CI8UUR09UALY"; 

 float tilt = 0;
 float humidity =0;
 float temperature =0;
 uint16_t vibrationState;


// Wi-Fi credentials
 const char* ssid = "Bytelux";
 const char* password = "Kennedy7730";

 sensors_event_t a, g, temp;


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

void pinConfig(){
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIBRATION_SENSOR_PIN, INPUT);
  Serial.println("Vehicle Monitoring System Initialized!");
}

void connectToWifi(){
 WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    delay(5000);
  }
  Serial.println("Connected to WiFi!");
}

void sendToThingspeak(){
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, a.acceleration.x);  // Acceleration X
  ThingSpeak.setField(4, a.acceleration.y);  // Acceleration Y
  ThingSpeak.setField(5, a.acceleration.z);  // Acceleration Z
  ThingSpeak.setField(6, tilt);              // Tilt angle
  ThingSpeak.setField(7, vibrationState);    // Vibration sensor state

  
  int responseCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (responseCode == 200) {
    Serial.println("Channel update successful.");
  } else {
    Serial.println("Failed to update channel. HTTP error code " + String(responseCode));
  }
}