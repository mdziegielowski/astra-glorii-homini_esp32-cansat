#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>
#include <SD.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_BNO08x_RVC.h>
#include <TinyGPSPlus.h>

// defining ESP32 board pins used for I2C communication with BMP280 module 
#define BMP280_SCL 22
#define BMP280_SDA 21
// defining the address of BMP280 module used for I2C communication protocol 
#define BMP280_I2C_ADDRESS 0x76
// defining ESP32 board pins used for VSPI communication with Ra-02 SX1278 radio communication module 
#define LoRa_MISO 19
#define LoRa_MOSI 23
#define LoRa_SCK 18
#define LoRa_NSS 5
#define LoRa_RST -1
#define LoRa_DIO0 2
// defining ESP32 board pins used for HSPI communication with SD card readers
#define SD_MISO   27
#define SD_MOSI   13
#define SD_SCLK   14
#define SD_SS     15

Adafruit_BMP280 bmp280; // initialization of the Adafruit_BMP280 class object 
Adafruit_BNO08x_RVC bno085rcv = Adafruit_BNO08x_RVC(); // initialization of the Adafruit_BNO08x_RVC class object 

File DataFile; // initialization of the File class object

SPIClass SPI_LoRa(VSPI); // initialization of the SPI class object for VSPI (used by radio comunication module Ra-02 SX1278)
SPIClass SPI_SDreader(HSPI); // initialization of the SPI class object for HSPI (used by SD card reader)

TinyGPSPlus gps; // initialization of the TinyGPSPlus class object

const int MAX_BUFFER_SIZE = 1024; // maximum size of buffer
char SDcardBuffer[MAX_BUFFER_SIZE]; //initialization of char array used by buffer
int bufferIndex = 0; // buffer index

int counter = 0; // counter of packages send by radio communication

void addDataToBuffer(const String& newData) { // function which adds data into buffer array until filled out
  newData.toCharArray(SDcardBuffer + bufferIndex, newData.length()+1); // converting String object into char array and specifying the place from which the data are supposed to be copied into buffer  
  bufferIndex += newData.length(); // updating the index of buffer after adding new data
  SDcardBuffer[bufferIndex++] = '\n';  // adding new line after every data package in buffer 
}

void writeBufferToSD() { // function which saves buffer content on SD card
  DataFile = SD.open("/example.txt", FILE_APPEND); // opening specified file on SD card
  if (DataFile) {
    DataFile.write((const uint8_t*)SDcardBuffer, bufferIndex); // writing buffer content 
    DataFile.close(); // closing file
  }

  // buffer cleaning and resetting  
  bufferIndex = 0;
  memset(SDcardBuffer, 0, MAX_BUFFER_SIZE);
}

String getCoordinates(){ // function which collects Latitude and Longitude data from GPS 
  String GPSdata;
  if (gps.location.isValid()){
    GPSdata += gps.location.lat();
    GPSdata += " ";
    GPSdata += gps.location.lng();
  }
  else{
    GPSdata += "gps not found";
  }
  return GPSdata;
}

void setup() { // setup function

  Serial.begin(9600);
  Serial1.begin(115200, SERIAL_8N1, 4); // UART1 initialization for communication with BNO085 (via UART RVC)
  Serial2.begin(9600); //  UART2 initialization for communication with GPS 
  Wire.begin(BMP280_SDA, BMP280_SCL); // I2C initialization for BMP280
  SPI_SDreader.begin(SD_SCLK,SD_MISO,SD_MOSI); // specifying pins used for SPI communication with SD card reader 

  if (!bno085rcv.begin(&Serial1)) { // connect to the BNO085 sensor over hardware serial
    while (1)
    delay(10);
  }
  
  if(!bmp280.begin(BMP280_I2C_ADDRESS)){ // setting I2C address for BMP280, module starts working
    delay(10);
    while (1);
  }

  if (!SD.begin(SD_SS, SPI_SDreader)) { // SD card reader starts working
    return;
  }

  LoRa.setSPI(SPI_LoRa);
  LoRa.setPins(LoRa_NSS,LoRa_RST, LoRa_DIO0); // specifying pins used for SPI communication with Ra-02 SX1278 module

  if (!LoRa.begin(433E6)) { // setting frequency
    delay(10);
    while (1);
  }

  // radio communication configuration 
  LoRa.setSignalBandwidth(125000);
  LoRa.setSpreadingFactor(8);
  LoRa.setCodingRate4(5);

}

void loop() {

  // collecting data from BMP280 - temperature, pressure, altitude
  String BMPmeasurements;
  BMPmeasurements += bmp280.readTemperature();
  BMPmeasurements += " ";
  BMPmeasurements += bmp280.readPressure();
  BMPmeasurements += " ";
  BMPmeasurements += bmp280.readAltitude();

  // collecting data from BNO085 - RPY angles and accelerometer
  BNO08x_RVC_Data heading;
  if (!bno085rcv.read(&heading)) {
    return;
  }

  String BNOmeasurements;
  BNOmeasurements += heading.yaw;
  BNOmeasurements += " ";
  BNOmeasurements += heading.pitch;
  BNOmeasurements += " ";
  BNOmeasurements += heading.roll;
  BNOmeasurements += " ";
  BNOmeasurements += heading.x_accel;
  BNOmeasurements += " ";
  BNOmeasurements += heading.y_accel;
  BNOmeasurements += " ";
  BNOmeasurements += heading.z_accel;

  // collecting data from GPS - Latitude and Longitude
  String GPSdata;
  while (Serial2.available() > 0){
    if (gps.encode(Serial2.read())){
      GPSdata = getCoordinates();
    }
  }

  if (millis() > 5000 && gps.charsProcessed() < 10){
    Serial.println(F("No GPS data available."));
    while (true);
  }

  // collecting all measurements together 
  String SensorsData;
  SensorsData += BMPmeasurements;
  SensorsData += " ";
  SensorsData += BNOmeasurements;
  SensorsData += " ";
  SensorsData += GPSdata;


  // writing data on SD card using buffer
  addDataToBuffer(SensorsData);
  if(bufferIndex >= MAX_BUFFER_SIZE){
    writeBufferToSD();
  }

  //sending data via radio communications
  LoRa.beginPacket();
  LoRa.print("no.");
  LoRa.print(counter);
  LoRa.print(" ");
  LoRa.print(SensorsData);
  LoRa.endPacket();
  counter++;
}

