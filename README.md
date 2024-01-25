# On-board C++ software for Astra Glorii Homini's CanSat based on ESP32 :rocket: 🛰

The repository is included as a part of Design Review reports for CanSat made by Astra Glorii Homini team. 
## Critical Design Review [CDR]

### -  Update 0.1 🙉
The code is developed for ESP32 to control modules used for collecting data during the mission. However, the solutions used are still being checked to provide maximum measurements efficiency so that can be changed at further stages of the project.

#### Ensured features:
* sensors initialization using different communication protocols such as UART, SPI or I2C in setup function
``` {.cpp}
Serial1.begin(115200, SERIAL_8N1, 4); // UART1 inicialization for comunication with BNO085 (via UART RVC)
Serial2.begin(9600); //  UART2 inicialization for comunication with GPS 
Wire.begin(BMP280_SDA, BMP280_SCL); // I2C inicialization for BMP280
```
* radio LoRa support with Bandwidth, Spring Factor and CodingRate4 configuration
``` {.cpp}
// Example of settings for radio comunication config
LoRa.setSignalBandwidth(125000);
LoRa.setSpreadingFactor(8);
LoRa.setCodingRate4(5);
```
* double SPI connection that allows SD card reader and radio LoRa module working simultaneously
``` {.cpp}
// Use of VSPI and HSPI buses  
SPI_SDreader.begin(SD_SCLK,SD_MISO,SD_MOSI); // specifying pins used for SPI comunication with SD card reader 
if (!SD.begin(SD_SS, SPI_SDreader)) { // SD card reader starts working
    return;
  }

LoRa.setSPI(SPI_LoRa);
LoRa.setPins(LoRa_NSS,LoRa_RST, LoRa_DIO0); // specifying pins used for SPI comunication with Ra-02 SX1278 module

if (!LoRa.begin(433E6)) { // setting frequency
    delay(10);
    while (1);
}
```
* SD card writing system using buffer to relieve the card from damage due to over-writing 
``` {.cpp}
const int MAX_BUFFER_SIZE = 1024; // maximum size of buffer
char SDcardBuffer[MAX_BUFFER_SIZE]; //inicialization of char array used by buffer
int bufferIndex = 0; // buffer index

void addDataToBuffer(const String& newData) { // function which adds data into buffer array until filled out
  newData.toCharArray(SDcardBuffer + bufferIndex, newData.length()+1); // converting String object into char array and specifying the place from which the data are supposed to be coppied into buffer  
  bufferIndex += newData.length(); // updating the index of buffer after adding new data
  SDcardBuffer[bufferIndex++] = '\n';  // adding new line after every data package in buffer 
}

void writeBufferToSD() { // function which saves buffer content on SD card
  DataFile = SD.open("/example.txt", FILE_APPEND); // opening specified file on SD card
  if (DataFile) {
    DataFile.write((const uint8_t*)SDcardBuffer, bufferIndex); // writing buffer content 
    DataFile.close(); // closing file
}
```
* Ra-02 SX1278 radio communication sending system
``` {.c++}
LoRa.beginPacket();
  LoRa.print("no.");
  LoRa.print(counter);
  LoRa.print(" ");
  LoRa.print(SensorsData);
  LoRa.endPacket();
  counter++;
```
#### Planned future modifications:
* multithreading, which is relevant to provide the best performance of sending data via radio communication each second and writing as many measurements as possible on SD card
* GPS calibration function which will set $GPRMC as the only protocol used in communication 
* adding support for the RGB LED indicating the CanSat launch status
