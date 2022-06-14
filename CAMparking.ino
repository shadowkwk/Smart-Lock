#include "esp_camera.h"
#include "Arduino.h"
//SD cards
#include "FS.h"                
#include "SD_MMC.h"            
//Disable brownour problems
#include "soc/soc.h"           
#include "soc/rtc_cntl_reg.h"  
#include "driver/rtc_io.h"
#include <EEPROM.h>            

#define EEPROM_SIZE 1
#define FILE_PHOTO "/photo.jpg"

int pictureNumber = 0;
int Read_from_Serial;

String controller;

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);
  //I/O pins
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
    //return;
  }
}

void loop() {
  // put your main code here, to run repeatedly:

   if (Serial.available() > 0){
    Read_from_Serial = Serial.read();
    if (Read_from_Serial == 48){
      takePicture();
      delay(1000);
    }
    if (Read_from_Serial == 51){
      takePicture();
      delay(1000);
    }
   }
  delay(3000);
  
}

void takePicture() {
  
  if (!SD_MMC.begin()) {
    Serial.println("SD Card Mount Failed");
    return;
  }

  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD Card attached");
    return;
  }

  camera_fb_t * fb = NULL;
  
  // Take Picture actions
  fb = esp_camera_fb_get();

  
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  
  // size++ for every picture caps
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;


  // Path where new picture will be saved in SD Card
  String path = "/picture" + String(pictureNumber) + ".jpg";


  fs::FS &fs = SD_MMC;
  Serial.printf("Picture file name: %s\n", path.c_str());
  File file = fs.open(path.c_str(), FILE_WRITE);

  if (!file) {
    Serial.println("Failed to open file in writing mode");
  }
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
  }
  //close file actions
  file.close();
  esp_camera_fb_return(fb);
}
