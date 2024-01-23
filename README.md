# On-board C++ software for Astra Glorii Homini's CanSat based on ESP32

The repository is included as a part of Design Review reports for CanSat made by Astra Glorii Homini team. 
## Critical Design Review [CDR]

### -  Update 0.1
The code is developed for ESP32 to control modules used for collecting data during the mission. However, the solutions used are still being checked to provide maximum measurements efficiency so that can be changed at further stages of the project.

#### Futures ensured:
* sensors initialization using different communication protocols such as UART, SPI or I2C in setup function
``` {.cpp}
// UART1, UART2 and I2C inicialization for further use 
Serial1.begin(115200, SERIAL_8N1, 4);
Serial2.begin(9600);
Wire.begin(BMP280_SDA, BMP280_SCL);
```
* radio LoRa support with Bandwidth, Spring Factor and CodingRate4 configuration
``` {.cpp}
// Example of settings for LoRa configuration
LoRa.setSignalBandwidth(125000); 
LoRa.setSpreadingFactor(9);
LoRa.setCodingRate4(8);
```
* double SPI connection that allows SD card reader and radio LoRa module working simultaneously
``` {.cpp}
// Use of VSPI and HSPI buses  
LoRa.setSPI(SPI_LoRa);
LoRa.setPins(LoRa_NSS,LoRa_RST, LoRa_DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    delay(100);
    while (1);
  }

SPI_SDreader.begin(SD_SCLK,SD_MISO,SD_MOSI);

  if (!SD.begin(SD_SS, SPI_SDreader)) {
    Serial.println("Error, SD Initialization Failed");
    return;
  }
```
* SD card saving system using buffer allowing to collect more data and relieve the card from overload
* LoRa package sending 

#### Planned future modifications:
* multithreading as needed to provide best performance
* GPS calibration function which will set $GPRMC as the only protocol used in communication must be included in the code. 
