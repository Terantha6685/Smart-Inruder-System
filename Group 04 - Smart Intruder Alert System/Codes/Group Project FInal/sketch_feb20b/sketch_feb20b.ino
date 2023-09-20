/*********
  Rui Santos
  Complete instructions at: https://RandomNerdTutorials.com/esp32-cam-save-picture-firebase-storage/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  Based on the example provided by the ESP Firebase Client Library
*********/


/*
const firebaseConfig = {

  apiKey: "AIzaSyDK6QFMHiLaWD86bTBdc_emddsYBc0G1dQ",

  authDomain: "test-b6614.firebaseapp.com",

  databaseURL: "https://test-b6614-default-rtdb.firebaseio.com",

  projectId: "test-b6614",

  storageBucket: "test-b6614.appspot.com",

  messagingSenderId: "279277966733",

  appId: "1:279277966733:web:b077994c8b8990d1ff19c4"

};*/


#include "WiFi.h"
#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include <SPIFFS.h>
#include <FS.h>
#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include <addons/TokenHelper.h>
#include "addons/RTDBHelper.h"
//#include <FirebaseESP8266.h>

//Replace with your network credentials
//const char* ssid = "REPLACE_WITH_YOUR_SSID";
//const char* password = "REPLACE_WITH_YOUR_PASSWORD";

//const char* ssid = "Dialog 4G";
//const char* password = "1A78YT18R61";

//const char* ssid = "Dialog 4G 414";
//const char* password="d6c630F1";

//const char* ssid = "UOV_Wi-Fi_2.4GHz";
//const char* password = "";

const char* ssid = "Sada";
const char* password = "12345678";

//const char* ssid="STUDENTS";
//const char* password="";

//const char* ssid="STAFF GENERAL";
//const char* password="";

//const char* ssid="Redmi Note 8";
//const char* password="";

//const char* ssid = "Hex";
//const char* password = "54321hex";

//const char* ssid="Tharani 4G";
//const char* password="YBG8MHDAF45";


#define DATABASE_URL "smartintrudersystem-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app


// Insert Firebase project API Key
//const char* API_KEY="AIzaSyDK6QFMHiLaWD86bTBdc_emddsYBc0G1dQ";
const char* API_KEY="AIzaSyAB1ndzvcM83yea382JtjpwneGSDrc1U_A";

// Insert Authorized Email and Corresponding Password
const char* USER_EMAIL="smartintrudersystem@gmail.com";
const char* USER_PASSWORD = "Smart@1234";

// Insert Firebase storage bucket ID e.g bucket-name.appspot.com
const char* STORAGE_BUCKET_ID ="smartintrudersystem.appspot.com";

// Photo File Name to save in SPIFFS
const char* FILE_PHOTO = "/data/photo.jpg";

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

static boolean takeNewPhoto = true;

//Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;


//static boolean taskCompleted = false;

// Check if photo capture was successful
bool checkPhoto( fs::FS &fs ) {
  File f_pic = fs.open( FILE_PHOTO );
  unsigned int pic_sz = f_pic.size();
  return ( pic_sz > 100 );
}

// Capture Photo and Save it to SPIFFS
void capturePhotoSaveSpiffs( void ) {
  camera_fb_t * fb = NULL; // pointer
  bool ok = 0; // Boolean indicating if the picture has been taken correctly
  do {
    // Take a photo with the camera

    listAllFiles();
    Serial.println("Taking a photo...");

    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }
    // Photo file name
    Serial.printf("Picture file name: %s\n", FILE_PHOTO);
    File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);
    // Insert the data in the photo file
    if (!file) {
      Serial.println("Failed to open file in writing mode");
    }
    else {
      file.write(fb->buf, fb->len); // payload (image), payload length
      Serial.print("The picture has been saved in ");
      Serial.print(FILE_PHOTO);
      Serial.print(" - Size: ");
      Serial.print(file.size());
      Serial.println(" bytes");
    }
    // Close the file
    file.close();
    esp_camera_fb_return(fb);
    listAllFiles();
    // check if file has been correctly saved in SPIFFS
    ok = checkPhoto(SPIFFS);
  } while ( !ok );
}

void initWiFi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("Connected");
  }
}

void initSPIFFS(){
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }
}

void initCamera(){
 // OV2640 camera module
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 9;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  } 
}


void listAllFiles(){
 
  File root = SPIFFS.open("/");
 
  File file = root.openNextFile();
 
  while(file){
 
      Serial.print("FILE: ");
      Serial.println(file.name());
 
      file = root.openNextFile();
  }
 
}

void uploadFile(){
  if (Firebase.ready()){
    //taskCompleted = true;
    Serial.print("Uploading picture... ");

    //MIME type should be valid to avoid the download problem.
    //The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
    if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, FILE_PHOTO /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, FILE_PHOTO /* path of remote file stored in the bucket */, "image/jpeg" /* mime type */)){
      Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());
      //String link = fbdo.downloadURL().c_str();
      //Firebase.RTDB.setString(&fbdo,"/image",link);
    }
    else{
      Serial.println(fbdo.errorReason());
    }
  }
}

void captureImage(){
  if (takeNewPhoto) {
      capturePhotoSaveSpiffs();
      takeNewPhoto = false;
    }
}

int val = 0;
#define pirPin 13
int calibrationTime = 30;
long unsigned int PirlowIn;
long unsigned int Pirpause = 5000;
boolean lockLow = true;
boolean takeLowTime;
int PIRValue = 0;

void PIRSensor() {
   if(digitalRead(pirPin) == HIGH) {
      if(lockLow) {
         PIRValue = 1;
         lockLow = false;
         Serial.println("Motion detected.");
         delay(50);
         captureImage();
          delay(5000);
          takeNewPhoto = true;   
          delay(1);
          uploadFile();
          ESP.restart();
      }
      takeLowTime = true;
   }
   if(digitalRead(pirPin) == LOW) {
      if(takeLowTime){
         PirlowIn = millis();takeLowTime = false;
      }
      if(!lockLow && millis() - PirlowIn > Pirpause) {
         PIRValue = 0;
         lockLow = true;
         Serial.println("Motion ended.");
         delay(50);
      }
   }
   int val = digitalRead(pirPin);
   Serial.println(val);
   delay(5000);
}

void setup() {

  // Serial port for debugging purposes
  Serial.begin(115200);
  initWiFi();
  initSPIFFS();

  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  initCamera();

  //Firebase
  // Assign the api key
  configF.api_key = API_KEY;
  //Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  //Assign the callback function for the long running token generation task
  configF.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);

  configF.database_url = DATABASE_URL;

   pinMode(pirPin, INPUT);
  
}
void loop() {
 
  PIRSensor();

}
