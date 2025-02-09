#include "blackbox.h"

DHT dht(DHTPIN, DHTTYPE);

WiFiClient client;
Adafruit_MPU6050 mpu; // MPU6050 setup



void setup() {
  pinConfig();
  systemInit();
  Serial.begin(115200);
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
  // Read DHT22 data
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  
  
// Read the state of the SW420 vibration sensor
  vibrationState = digitalRead(VIBRATION_SENSOR_PIN);

  // Check for valid readings
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Read MPU6050 data
  //sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

/////////////////////////////////////////////////////////
//////////to save CPU cycles////////////////////////////
////////////////////////////////////////////////////////

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
  }
  if (abs(tilt) > TILT_THRESHOLD) {
    Serial.println("Tilt detected! Triggering alert...");
    tone(BUZZER_PIN, 2000, 3000);  // Activate buzzer
  }

  if (temperature > TEMP_THRESHOLD || humidity > HUMIDITY_THRESHOLD) {
    Serial.println("High temp/humidity detected! Triggering alert...");
    tone(BUZZER_PIN, 1500, 3000);  // Activate buzzer
  }

  // Send data to ThingSpeak regardless of alerts
  sendToThingspeak();

  delay(20000);  // Wait 20 seconds between updates (this can be adjusted based on your needs)
}

