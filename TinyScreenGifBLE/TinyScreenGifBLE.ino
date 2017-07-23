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

//Debug output adds extra flash and memory requirements!
#ifndef BLE_DEBUG
#define BLE_DEBUG true
#endif

#if defined (ARDUINO_ARCH_AVR)
#define SerialMonitorInterface Serial
#elif defined(ARDUINO_ARCH_SAMD)
#define SerialMonitorInterface SerialUSB
#endif

TinyScreen display = TinyScreen(TinyScreenPlus);

byte buffer[256];
byte leftButton=(1<<1);
byte rightButton=(1<<0);

// SdFat sd;
File dataFile;
bool canAnimate = true;

// BLE
uint8_t ble_connection_state = false;
uint8_t ble_rx_buffer[21];
uint8_t ble_rx_buffer_len = 0;
#define PIPE_UART_OVER_BTLE_UART_TX_TX 0

void setup(void) {
  SerialMonitorInterface.begin(9600);
  while (!SerialMonitorInterface); //This line will block until a serial monitor is opened with TinyScreen+!

  pinMode(10, OUTPUT);
  
  Wire.begin();
  
  if (!SD.begin(10)) {
    SerialMonitorInterface.print(F("Card failed"));
    while(1);
  }
  
  display.begin();
  display.setFlip(1);
  display.setBrightness(6);
  display.clearWindow(0, 0, 96, 64);
  display.setFont(liberationSans_8ptFontInfo);

  BLEsetup();
}

void loop() {
  aci_loop();  // Process any ACI commands or events from the NRF8001- main BLE handler, must run often. Keep main loop short.
  
  if (ble_rx_buffer_len) {//Check if data is available
    SerialMonitorInterface.print(ble_rx_buffer_len);
    SerialMonitorInterface.print(" : ");
    SerialMonitorInterface.println((char*)ble_rx_buffer);
    ble_rx_buffer_len = 0;//clear afer reading
  }

  if (SerialMonitorInterface.available()) {//Check if serial input is available to send
    delay(10);//should catch input
    uint8_t sendBuffer[21];
    uint8_t sendLength = 0;
    while (SerialMonitorInterface.available() && sendLength < 19) {
      sendBuffer[sendLength] = SerialMonitorInterface.read();
      sendLength++;
    }
    if (SerialMonitorInterface.available()) {
      SerialMonitorInterface.print(F("Input truncated, dropped: "));
      if (SerialMonitorInterface.available()) {
        SerialMonitorInterface.write(SerialMonitorInterface.read());
      }
    }
    sendBuffer[sendLength] = '\0'; //Terminate string
    sendLength++;
    if (!lib_aci_send_data(PIPE_UART_OVER_BTLE_UART_TX_TX, (uint8_t*)sendBuffer, sendLength))
    {
      SerialMonitorInterface.println(F("TX dropped!"));
    }
  }
  
  if (canAnimate) {
    // Start GIF animation.
    playVideo("image_5.tsv",0);
  }

  if (display.getButtons()&leftButton) {
    // Restart GIF animation.
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
        // Stop GIF animation.
        SerialMonitorInterface.println("right button");
        canAnimate = false;
      }
    }
  }
  dataFile.close();
}
