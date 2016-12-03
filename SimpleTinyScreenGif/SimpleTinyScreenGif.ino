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

TinyScreen display = TinyScreen(TinyScreenPlus);

//SdFat sd;
File dataFile;
//SdFile fileInfo;

void setup(void) {
  Wire.begin();
  display.begin();
  Serial.begin(9600);
  if (!SD.begin(10)) {
    Serial.println(F("Card failed"));
    while(1);
  }
}

void loop() {
//  sd.vwd()->rewind();
  dataFile.rewindDirectory();
  while(dataFile.openNextFile()){
    Serial.println("hoge");
    char filename[12];
    if(!dataFile.isDirectory()){
      strcpy(filename, dataFile.name());
//      int len=strlen(filename)-4;
      if(strcmp(filename,"image_5.tsv"))
        playVideo(filename,0);
    }
    dataFile.close();
  }
}


void playVideo(char * filename,char skip){
  Serial.println(filename);
  uint8_t buffer[512];
  dataFile = SD.open(filename);
  if(!dataFile) {
    Serial.println(F("Error opening file!"));
    delay(100);
    return;
  }
  dataFile.seek(0);
  unsigned char t=0;
  while(dataFile.available()) {
    display.goTo(0, 0);
    //if(t++&skip)dataFile.seek(dataFile.position()+(96*64));//switch to SdFat
    unsigned long timer=millis();
    for(int i=0; i<12 && dataFile.available(); i++){
      dataFile.read(buffer,512);
      display.startData();
      display.writeBuffer(buffer,512);
      display.endTransfer();
    }
    timer=millis()-timer;
    Serial.println(timer);
  }
  dataFile.close();
}
