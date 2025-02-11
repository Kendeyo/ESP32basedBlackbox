/*
*  Code base for the ESP32based BlackBox
*  Does monitoring of various parameters, saves the data locally on SD Card, sends the data on cloud
*  platform and gives alaert via SMS through GSM
*/

#include "blackbox.h"

DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial gpsSerial(rxGPS, txGPS);
SoftwareSerial gsmSerial(rxGSM, txGSM);

WiFiClient client;
Adafruit_MPU6050 mpu; // MPU6050 setup
TinyGPSPlus gps;
alerts notification;

String dataMessage;
String gsmMessage;

uint32_t readingID = 0;
extern sensors_event_t a, g, temp;
extern double lat;
extern double longtd;

unsigned long currentMillis;
unsigned long previousMillis = 0; // Stores the last time the action was performed
const unsigned long interval = 15000; // Interval in milliseconds (15 seconds)

bool isAlert = false;

void setup() {
  pinConfig();
  systemInit();
  Serial.begin(9600);
  gpsSerial.begin(9600);  // connect gps sensor
  gsmSerial.begin(9600);
  dht.begin();
  connectToWifi();// Connect to Wi-Fi
  ThingSpeak.begin(client);

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }
  Serial.println("MPU6050 Initialized!");
   sdCardSetup();

}

void loop() {

  currentMillis = millis(); // Get the current time
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
    notification = ACCELERATION;
    isAlert = true;
    if(isAlert){sendMessage();}
    //checkparam();
    Serial.println("Accident detected! Triggering alert...");
    tone(BUZZER_PIN, 1000, 2000);  // Activate buzzer
    }//end of if

  if ((abs(tilt) > TILT_THRESHOLD1) ||  (abs(tilt) < TILT_THRESHOLD2)) {
    notification = TILT;
    isAlert = true;
    if(isAlert){sendMessage();}
    //checkparam();
    Serial.println("Tilt detected! Triggering alert...");
    tone(BUZZER_PIN, 2000, 2000);  // Activate buzzer
    }//end of if

  if (temperature > TEMP_THRESHOLD || humidity > HUMIDITY_THRESHOLD) {
    notification = TEMPERATURE;
    isAlert = true;
    if(isAlert){sendMessage();}
    //checkparam();
    Serial.println("High temp/humidity detected! Triggering alert...");
    //Serial.print("Temperature: ");Serial.println(temperature);  //for debugg
    //Serial.print("Humidity: ");Serial.println(humidity);        //for debugg
    tone(BUZZER_PIN, 1500, 2000);  // Activate buzzer
    }//end of if


  while (gpsSerial.available()){
    if (gps.encode(gpsSerial.read()))   // encode gps data
    { 
      #if 1 //make 0 to disable the prints
        Serial.print("SATS: ");Serial.println(gps.satellites.value());
        Serial.print("LAT: ");Serial.println(gps.location.lat(), 6);
        Serial.print("LONG: ");Serial.println(gps.location.lng(), 6);
        Serial.print("ALT: ");Serial.println(gps.altitude.meters());
        Serial.print("SPEED: ");Serial.println(gps.speed.mps());
      #endif

      uint8_t sats = gps.satellites.value(); 
      lat = gps.location.lat();
      longtd = gps.location.lng();
      double speed = gps.speed.mps();

      sprintf(satsbf, "SATS: %d", sats);
      sprintf(latbf, "LAT : %.6f",lat);
      sprintf(longtbf, "LONG : %.6f", longtd);


      #if 1 //set to 0 not to see in serial
        Serial.println(satsbf);
        Serial.println(latbf);
        Serial.println(longtbf); 
        Serial.print("Date: ");Serial.print(gps.date.day()); Serial.print("/");Serial.print(gps.date.month()); Serial.print("/");Serial.println(gps.date.year());
      #endif

      //-------------------------------------------
      uint8_t day = gps.date.day();  // 
      uint8_t month = gps.date.month();
      uint16_t year = gps.date.year();
      sprintf(datebf, "%d/%d/%d", day, month,year);  
      Serial.println(datebf);
      //--------------------------------------------
      //////////////////////////////////////////////////////////////
      //////////to save CPU cycles//////////////////////////////////
      /////////////////////////////////////////////////////////////
      #if 0 //set to 1 to see in serial
        Serial.print("Hour: ");
        Serial.print(gps.time.hour()); Serial.print(":");
        Serial.print(gps.time.minute()); Serial.print(":");
        Serial.println(gps.time.second()); 
      #endif

      //------------------------------------------------------------
      uint8_t hr = 3 + gps.time.hour();  
      uint8_t mins = gps.time.minute();
      sprintf(timebf,"%d:%2d", hr, mins);
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
    logSDCard();
    readingID++;
  }//end of if

  delay(100);  //for stability

}//end of main loop


////////////////////////////////////////////////////////////////////////
// Name: sdCardSetup()
// Function: This function sets up the SD card
// @param1: None
// @return: void
////////////////////////////////////////////////////////////////////////

void  sdCardSetup(){
   // Initialize SD card
  SD.begin(SD_CS);  
  if(!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;    // init failed
  }

  // If the data.txt file doesn't exist
  // Create a file on the SD card and write the data labels
  File file = SD.open("/data.csv");
  if(!file) {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/data.csv", "Reading ID, Time, Temperature, Tilt, VibrationState, Latitude, Longitude \r\n");
  }
  else {
    Serial.println("File already exists");  
  }
  file.close();

}


////////////////////////////////////////////////////////////////////////
// Name: logSDCard()
// Function: This function writes to the SD card (DON'T MODIFY THIS FUNCTION)
// @param1: None
// @return: void
////////////////////////////////////////////////////////////////////////
void logSDCard() {
  dataMessage = String(readingID) + "," + String(timebf) + "," +String(temperature) + "," + String(tilt) + "," + String(vibrationState) + "," + String(lat) + "," + 
                String(longtd) + "\r\n";
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  appendFile(SD, "/data.csv", dataMessage.c_str());
}


////////////////////////////////////////////////////////////////////////
// Name: writeFile()
// Function: This function writes to the SD card (DON'T MODIFY THIS FUNCTION)
// @param1: message path
// @param2: the message 
// @return: void
////////////////////////////////////////////////////////////////////////
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}


////////////////////////////////////////////////////////////////////////
// Name: appendFile()
// Function: This function appends data to the SD card (DON'T MODIFY THIS FUNCTION)
// @param1: message path
// @param2: the message 
// @return: void
////////////////////////////////////////////////////////////////////////
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

////////////////////////////////////////////////////////////////////////
// Name: sendMessage()
// Function: This function sends txt message to the relevant operator
// @param1: none
// @return: void
////////////////////////////////////////////////////////////////////////

void sendMessage(){
  gsmSerial.println("AT+CMGF=1");
  updateSerial();
  gsmSerial.println("AT+CMGS=\"+254713352766\"");
  updateSerial();
  gsmSerial.print("Hello from BlackBox \n");  // enter your message here
  gsmSerial.print("Something Occured. Check location \n");  
  gsmMessage = "Location: https://maps.google.com/?q=" + String(lat) + "," + String(longtd) + "\r\n" ;
  gsmSerial.print(gsmMessage);
  updateSerial();
  gsmSerial.write(26);
  Serial.println("done");
  isAlert = false;
}

////////////////////////////////////////////////////////////////////////
// Name: checkparam()
// Function: This function checks the kind of notification and sends the message
// @param1: none
// @return: void
////////////////////////////////////////////////////////////////////////

void checkparam(){
  switch(notification){
    case ACCELERATION:
      sendMessage();
      break;

    case TILT:
      sendMessage();
      break;

    case TEMPERATURE:
      sendMessage();
      break;

    default:
     //do nothing
     break;
  }//end of switch
}

////////////////////////////////////////////////////////////////////////
// Name: updateSerial() 
// Function: This function udates both Software and Hardware serial
// @param1: none
// @return: void
////////////////////////////////////////////////////////////////////////

void updateSerial() {
  delay(500);
  while (Serial.available()) {
    gsmSerial.write(Serial.read());  //Forward what Serial received to Software Serial Port
  }
  while (gsmSerial.available()) {
    Serial.write(gsmSerial.read());  //Forward what Software Serial received to Serial Port
  }
}