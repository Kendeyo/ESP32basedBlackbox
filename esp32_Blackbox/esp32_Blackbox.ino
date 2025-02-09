#include "blackbox.h"

DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial gpsSerial(rxGPS, txGPS);

WiFiClient client;
Adafruit_MPU6050 mpu; // MPU6050 setup
TinyGPSPlus gps;

extern sensors_event_t a, g, temp;

void setup() {
  pinConfig();
  systemInit();
  Serial.begin(9600);
  gpsSerial.begin(9600);  // connect gps sensor
  dht.begin();
  connectToWifi();// Connect to Wi-Fi
 

  ThingSpeak.begin(client);

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }
  Serial.println("MPU6050 Initialized!");
    // Read MPU6050 data
}

void loop() {

  unsigned long currentMillis = millis(); // Get the current time
  // Read DHT22 data
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  
  
// Read the state of the SW420 vibration sensor
  vibrationState = digitalRead(VIBRATION_SENSOR_PIN);

  // Check for valid readings
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
    }//end of if

  // Read MPU6050 data
  //sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

//////////////////////////////////////////////////////////////
//////////to save CPU cycles//////////////////////////////////
/////////////////////////////////////////////////////////////

  #if 0  //make 1 to print on serial

    // Print MPU6050 readings
    Serial.print("Acceleration X: "); Serial.println(a.acceleration.x);
    Serial.print("Acceleration Y: "); Serial.println(a.acceleration.y);
    Serial.print("Acceleration Z: "); Serial.println(a.acceleration.z);

    Serial.print("Gyro X: "); Serial.println(g.gyro.x);
    Serial.print("Gyro Y: "); Serial.println(g.gyro.y);
    Serial.print("Gyro Z: "); Serial.println(g.gyro.z);

  #endif


 tilt = atan2(a.acceleration.y, a.acceleration.x) * (180 / PI);  // Calculate tilt angle in degrees

  // Logic for buzzer - only activate when thresholds are exceeded
  if (abs(a.acceleration.x) > ACCEL_THRESHOLD || abs(a.acceleration.y) > ACCEL_THRESHOLD || abs(a.acceleration.z) > ACCEL_THRESHOLD) {
    Serial.println("Accident detected! Triggering alert...");
    tone(BUZZER_PIN, 1000, 3000);  // Activate buzzer
    }//end of if

  if ((abs(tilt) > TILT_THRESHOLD1) ||  (abs(tilt) < TILT_THRESHOLD2)) {
    Serial.println("Tilt detected! Triggering alert...");
    tone(BUZZER_PIN, 2000, 3000);  // Activate buzzer
    }//end of if

  if (temperature > TEMP_THRESHOLD || humidity > HUMIDITY_THRESHOLD) {
    Serial.println("High temp/humidity detected! Triggering alert...");
    //Serial.print("Temperature: ");Serial.println(temperature);  //for debugg
    //Serial.print("Humidity: ");Serial.println(humidity);        //for debugg
    tone(BUZZER_PIN, 1500, 3000);  // Activate buzzer
    }//end of if


  while (gpsSerial.available()){
    if (gps.encode(gpsSerial.read()))   // encode gps data
    {
      
      Serial.print("SATS: ");Serial.println(gps.satellites.value());
      Serial.print("LAT: ");Serial.println(gps.location.lat(), 6);
      Serial.print("LONG: ");Serial.println(gps.location.lng(), 6);
      Serial.print("ALT: ");Serial.println(gps.altitude.meters());
      Serial.print("SPEED: ");Serial.println(gps.speed.mps());

      uint8_t sats = gps.satellites.value(); 
      double lat = gps.location.lat();
      double longtd = gps.location.lng();
      double speed = gps.speed.mps();

      sprintf(satsbf, "SATS: %d", sats);
      sprintf(latbf, "LAT : %.6f",lat);
      sprintf(longtbf, "LONG : %.6f", longtd);

      Serial.println(satsbf);
      Serial.println(latbf);
      Serial.println(longtbf);
 
      
      Serial.print("Date: ");Serial.print(gps.date.day()); Serial.print("/");Serial.print(gps.date.month()); Serial.print("/");Serial.println(gps.date.year());

      //-------------------------------------------
      uint8_t day = gps.date.day();  // 
      uint8_t month = gps.date.month();
      uint16_t year = gps.date.year();
      sprintf(datebf, "%d/%d/%d", day, month,year);  
      Serial.println(datebf);
      //--------------------------------------------

      
      Serial.print("Hour: ");
      Serial.print(gps.time.hour()); Serial.print(":");
      Serial.print(gps.time.minute()); Serial.print(":");
      Serial.println(gps.time.second()); 

      //------------------------------------------------------------
      uint8_t hr = 3 + gps.time.hour();  
      uint8_t mins = gps.time.minute();
      sprintf(timebf,"%d : %2d", hr, mins);
      Serial.println(timebf);
     //--------------------------------------------------------------

      Serial.println("---------------------------");
      delay(1000);
    }//end of if
  }//end of GPS while

 
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // Save the last time the action was performed
    // Send data to ThingSpeak regardless of alerts
   sendToThingspeak();
  }//end of if

  delay(100);  //for stability

}//end of main loop

