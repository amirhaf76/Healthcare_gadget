#include <SPI.h>
#include <SD.h>
#include "address_and_keys.h"

#define SD_MODULE_DEBUG 1
#define PIN_CS 53

bool make_file_ready() {
  #if SD_MODULE_DEBUG
  // Open serial communications and wait for port to open:
  Serial.print("Initializing SD card...");
  #endif

  pinMode(PIN_CS, OUTPUT);

  if (!SD.begin(PIN_CS)) {
    #if SD_MODULE_DEBUG
    Serial.println("initialization failed!");
    #endif
    
    return false;
  }

  #if SD_MODULE_DEBUG
  Serial.println("initialization done.");
  #endif

  return true;
}

bool print_in_file(int val, char * fileName) {
  // making file ready
  if (!make_file_ready()) 
    return false; // file is not ready and return

  File myFile = SD.open(fileName, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    #if SD_MODULE_DEBUG
    Serial.print("Writing to test.txt...");
    #endif
    
    myFile.print(val);

    // close the file:
    myFile.close();

    #if SD_MODULE_DEBUG
    Serial.println("done.");
    #endif

    return true;
  } else {
    // if the file didn't open, print an error:
    #if SD_MODULE_DEBUG
    Serial.print("error opening ");
    Serial.print(fileName);
    Serial.println(" file");
    #endif

    return false;
  }
}


bool print_in_file(String val, char * fileName) {
  // making file ready
  if (!make_file_ready()) 
    return false; // file is not ready and return

  File myFile = SD.open(fileName, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    #if SD_MODULE_DEBUG
    Serial.print("Writing to test.txt...");
    #endif
    
    myFile.print(val);

    // close the file:
    myFile.close();

    #if SD_MODULE_DEBUG
    Serial.println("done.");
    #endif

    return true;
  } else {
    // if the file didn't open, print an error:
    #if SD_MODULE_DEBUG
    Serial.print("error opening ");
    Serial.print(fileName);
    Serial.println(" file");
    #endif

    return false;
  }
}


bool print_in_file(double val, char * fileName) {
  // making file ready
  if (!make_file_ready()) 
    return false; // file is not ready and return

  File myFile = SD.open(fileName, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    #if SD_MODULE_DEBUG
    Serial.print("Writing to test.txt...");
    #endif
    
    myFile.print(val);

    // close the file:
    myFile.close();

    #if SD_MODULE_DEBUG
    Serial.println("done.");
    #endif

    return true;
  } else {
    // if the file didn't open, print an error:
    #if SD_MODULE_DEBUG
    Serial.print("error opening ");
    Serial.print(fileName);
    Serial.println(" file");
    #endif

    return false;
  }
}


bool create_csv_file(const int buff[], size_t siz, char * fileName) {
  // making file ready
  if (!make_file_ready()) 
    return false; // file is not ready and return

  File myFile = SD.open("myFile.csv", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    #if SD_MODULE_DEBUG
    Serial.print("Writing to test.txt...");
    #endif
    
    for (size_t i = 0; i < siz; ++i) {
      myFile.print(i);
      myFile.print(",");
      myFile.println(buff[i]);
    }
      

    // close the file:
    myFile.close();

    #if SD_MODULE_DEBUG
    Serial.println("done.");
    #endif

    return true;
  } else {
    // if the file didn't open, print an error:
    #if SD_MODULE_DEBUG
    Serial.print("error opening ");
    Serial.print(fileName);
    Serial.println(" file");
    #endif

    return false;
  }
}

bool read_from_file(char * fileName) {
  // making file ready
  if (!make_file_ready()) 
    return false; // file is not ready and return

  // re-open the file for reading:

  File myFile = SD.open(fileName, FILE_READ);

  if (myFile) {
    #if SD_MODULE_DEBUG
    Serial.println(fileName);
    #endif

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }

    // close the file:
    myFile.close();

    return true;
  } else {
    // if the file didn't open, print an error:
    #if SD_MODULE_DEBUG
    Serial.println("error opening test.txt");
    #endif

    return false;
  }

}