//-------------------------------------------------------------------------------
//  TinyCircuits TinyScreen Video Player Example Sketch
//  Last Updated 9 May 2015
//  
//  This demo plays all .tsv files in the root directory of an SD card. The file
//  format is just raw 8 bit pixel data- that's the only way we can squeeze a
//  usable framerate from the 8MHz AVR.
//
//  Written by Ben Rose, TinyCircuits http://Tiny-Circuits.com
//
//-------------------------------------------------------------------------------

#include <TinyScreen.h>
#include <SD.h>
#include <Wire.h>
#include <SPI.h>
#include <STBLE.h>

#if defined (ARDUINO_ARCH_AVR)
#define SerialMonitorInterface Serial
#elif defined(ARDUINO_ARCH_SAMD)
#define SerialMonitorInterface SerialUSB
#endif

TinyScreen display = TinyScreen(TinyScreenPlus);

byte buffer[256];
byte leftButton=(1<<1);
byte rightButton=(1<<0);

//SdFat sd;
File dataFile;
bool canAnimate = true;

void setup(void) {
  Wire.begin();
  SerialMonitorInterface.begin(9600);
  
  if (!SD.begin(10)) {
    SerialMonitorInterface.print(F("Card failed"));
    while(1);
  } 
  display.begin();
  display.setFlip(1);
  display.setBrightness(6);
  display.clearWindow(0, 0, 96, 64);
  display.setFont(liberationSans_8ptFontInfo);
}

void loop() {
  if (canAnimate) {
    playVideo("image_5.tsv",0);
  }

  if (display.getButtons()&leftButton) {
    SerialMonitorInterface.println("left button");
    canAnimate = true;   
  }
}

void playVideo(char * filename, char skip) {
  SerialMonitorInterface.print("playVideo : ");
  SerialMonitorInterface.println(filename);
  
  dataFile = SD.open(filename); 
  if (!dataFile) {
    SerialMonitorInterface.println("Error opening file!");
    return;
  }
  
  dataFile.seek(0);
  unsigned char t = 0;
  
  while (dataFile.available()) {
    display.goTo(0, 0);
    if (t++&skip) dataFile.seek(dataFile.position() + (96 * 64));
    for (int i = 0; i < 24 && dataFile.available(); i++) {
      dataFile.read(buffer, 256);
      display.startData();
      display.writeBuffer(buffer, 256);
      display.endTransfer();
      
      if (display.getButtons()&rightButton) {
        SerialMonitorInterface.println("right button");
        canAnimate = false;
      }
    }
  }
  dataFile.close();
}
