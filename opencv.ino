#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"


// Pin definition for CAMERA_MODEL_AI_THINKER
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

int contador=0;
unsigned char newImage[30][40][3];

bool frameAvailable=true;


void getPixel(int x, int y, unsigned char *image, unsigned char *pixel)
{
  int p = image[(y * 160 + x) * 2 + 1] | (image[(y * 160 + x) * 2] << 8);
  pixel[0] = (p >> 11) << 3;
  pixel[1] = ((p >> 5) & 0b111111) << 2;
  pixel[2] = (p & 0b11111) << 3;
}



void downSample(unsigned char *frame)
{
  for(int y = 0; y < 30; y++)
  {
    for(int x = 0; x < 40; x++)
    {
      int r = 0;
      int g = 0;
      int b = 0;
      for(int j = 0; j < 4; j++)
        for(int i = 0; i < 4; i++)
        {
          unsigned char p[3];          
          getPixel(x * 4 + i, y * 4 + j, frame, p);
          r += p[0];
          g += p[1];
          b += p[2];
        }
      newImage[y][x][0] = r >> 4;
      newImage[y][x][1] = g >> 4;
      newImage[y][x][2] = b >> 4;
    }
  }
  /*
  for(int y = 0; y < 30; y++)
    for(int x = 0; x < 40; x++) 
      getPixel(x * 4 + 2, y * 4 + 2, frame, newImage[y][x]);*/
}




void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
 
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  //Serial.println();
  
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
  config.pixel_format = PIXFORMAT_RGB565; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_QQVGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_XGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;
  }
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
 
  
    
}

void loop() {
  
  camera_fb_t * fb = esp_camera_fb_get();
  if (fb){
    downSample(fb->buf);  //fb->width, fb->height, fb->format, fb->buf, fb->len
    frameAvailable = true;
    esp_camera_fb_return(fb);
  };   
  if(frameAvailable) 
  {
     for(int y = 0; y < 30; y++){
        for(int x = 0; x < 40; x++){

          if(newImage[y][x][0]>200) {
            Serial.print("1");
          }else{
            Serial.print("0");            
          }
          /*
          Serial.print("(");
          Serial.print(newImage[y][x][0]);
          Serial.print(newImage[y][x][1]);
          Serial.print(newImage[y][x][2]);
          Serial.print(")");          
          */
        }
        Serial.println("");          
     }
     Serial.println("");        Serial.println("");        
    frameAvailable = false;
  }

  delay(1);
  
}
