#include <Adafruit_NeoPixel.h>
TaskHandle_t TaskMusicLED; 

const int micPin = 34;
const int pixelPin = 5;
const int MaxPixelCount = 300;
int pixelCount = 300;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(pixelCount, pixelPin, NEO_GRB + NEO_KHZ800);
#include "Audio.h"
void MusicFunc(void * pvParameters){
 Serial.print("MUSIC running on core ");
 Serial.println(xPortGetCoreID());

   for(;;){
    //Serial.print("INLOOP ");
    int t = millis();
    static int time = 0;
    int dt = t - time;
    if (time == 0) dt = 0;
    time = t;
    analyze();
   static int oldTime = 0;
    if(time - oldTime >= 30){
        oldTime = time;
        display();
      }
     vTaskDelay(1);
   }
  }
  
void LoadMusicLED(){
  AutoConnectInput& noOfLEDs = settingsAux.getElement<AutoConnectInput>("no_of_leds");
  if(noOfLEDs.value.toInt() > 0){
    pixelCount = noOfLEDs.value.toInt();
  }
  strip.begin();
  strip.show();
xTaskCreatePinnedToCore(
                    MusicFunc,   /* Task function. */
                    "MusicLED",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &TaskMusicLED,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */   
}
