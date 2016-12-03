#include <TinyScreen.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

TinyScreen display = TinyScreen(TinyScreenPlus);

byte buffer[256];
char currentLocation[20];
byte directoryLevel=0;
File root;
File file;
byte filesDisplayed=0;
byte selectionLine=0;
byte maxLines=6;
byte firstFile=0;
byte lastFirstFile=-1;
byte pixelsPerLine=10;
byte amtFiles=0;

byte upButton=(1<<3);
byte downButton=(1<<2);
byte leftButton=(1<<1);
byte rightButton=(1<<0);

void setup(void) {
  Wire.begin();
  Serial.begin(9600);
  pinMode(10, OUTPUT);
  if (!SD.begin(10)) {
    Serial.println(F("Card failed, or not present"));
    while(1);
  }
  display.begin();
  display.setFlip(1);
  display.setBrightness(6);
  display.clearWindow(0, 0, 96, 64);
  display.setFont(liberationSans_8ptFontInfo);
  
  currentLocation[0] = '/';
  currentLocation[1] = '\0';
  setDirectory(currentLocation);
}

void loop(){
  int y = 0;
  if (lastFirstFile != firstFile) {
    //Serial.print("calling displayFiles at file ");
    //Serial.println(firstFile);
    displayFiles(firstFile);
    lastFirstFile=firstFile;
  }
  display.clearWindow(0, 0, 6, 64);
  drawRightArrow(0, 2 + (pixelsPerLine * selectionLine));
  delay(200);
  if (display.getButtons()&upButton) {
    Serial.println("up button");
    if (selectionLine > 0)
      selectionLine--;
    else if (firstFile > 0)
      firstFile--;
    return;
  }
  if (display.getButtons()&downButton) {
    Serial.println("down button");
    if (selectionLine < maxLines - 1 && selectionLine < amtFiles - 1)
      selectionLine++;
    else if (firstFile < amtFiles - maxLines)
      firstFile++;
    return;
  }
  if (display.getButtons()&rightButton) {
    Serial.println("right button");
    root.rewindDirectory();
    for (int i = 0; i < firstFile + selectionLine; i++) {
      file = root.openNextFile();
      file.close();
    }
    file = root.openNextFile();
    if (file.isDirectory()) {
      directoryLevel++;
      int i = 0, j = 0;
      while (i < 30 && j < directoryLevel)
        if (currentLocation[i++]=='/') j++;
      strcpy(currentLocation + i, file.name());
      file.close();
      setDirectory(currentLocation);
    } else {
      char fullName[40];
      strcpy(fullName, currentLocation);
      if (directoryLevel) {
        fullName[strlen(currentLocation)] = '/';
        fullName[strlen(currentLocation) + 1] = '\0';
      }
      strcpy(fullName+strlen(fullName), file.name());
      file.close();
      Serial.println(fullName);
      while (display.getButtons()&leftButton);
      while (!(display.getButtons()&leftButton)){
        playVideo(fullName, 0);
      }
      display.clearWindow(0, 0, 96, 64);
      setDirectory(currentLocation);
    }
    return;
  }
  if (display.getButtons()&leftButton) {
    Serial.println("left button");
    if (directoryLevel < 1) return;
    int i = 0, j = 0;
    while (i < 30 && j < directoryLevel)
      if (currentLocation[i++] == '/') j++;
    currentLocation[i] = '\0';
    directoryLevel--;
    setDirectory(currentLocation);
    }
}

void setDirectory(char * location){
  root.close();
  root = SD.open(location);
  root.rewindDirectory();
  file = root.openNextFile();
  amtFiles = 0;
  while (file) {
    amtFiles++;
    file.close();
    file = root.openNextFile();
  }
  file.close();
  //Serial.print(amtFiles);
  //Serial.print(" files at ");
  //Serial.println(location);
  filesDisplayed = 0;
  selectionLine = 0;
  firstFile = 0;
  lastFirstFile = -1;
}

void displayFiles(int startFile){
  root.rewindDirectory();
  for (int i = 0; i < startFile; i++) {
    file = root.openNextFile();
    file.close();
  }
  filesDisplayed = 0;
  file = root.openNextFile();
  int y = 0;
  //display.clearWindow(6,0,96,64);
  while (file && filesDisplayed < maxLines) {
    y = filesDisplayed * pixelsPerLine;
    display.setCursor(6, y);
    display.print(file.name());
    if (file.isDirectory())
      display.print('/');
    filesDisplayed++;
    file.close();
    file = root.openNextFile();
  }
  if (startFile) drawUpArrow(86, 0);
  if (file) drawDownArrow(86, 56);
  file.close();
}

void drawRightArrow(int x, int y) {
  display.drawLine(x,y,x+5,y+3,0xFF);
  display.drawLine(x+5,y+3,x,y+6,0xFF);
}

void drawDownArrow(int x, int y) {
  display.drawLine(x,y,x+3,y+5,0xFF);
  display.drawLine(x+3,y+5,x+6,y,0xFF);
}

void drawUpArrow(int x, int y) {
  display.drawLine(x,y+5,x+3,y,0xFF);
  display.drawLine(x+3,y,x+6,y+5,0xFF);
}

void playVideo(char * filename, char skip) {
  file = SD.open(filename); 
  if (!file) {
    Serial.println("Error opening file!");
    return;
  }
  file.seek(0);
  unsigned char t = 0;
  while (file.available() && !(display.getButtons()&leftButton)) {
    display.goTo(0, 0);
    if (t++&skip) file.seek(file.position() + (96 * 64));
    for (int i = 0; i < 24 && file.available(); i++) {
      file.read(buffer, 256);
      display.startData();
      display.writeBuffer(buffer, 256);
      display.endTransfer();
    }
  }
  file.close();
}
