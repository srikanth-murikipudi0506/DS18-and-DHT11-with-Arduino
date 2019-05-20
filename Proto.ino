#include <SD.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Time.h>
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS1307.h>
RtcDS1307<TwoWire> Rtc(Wire);

#include "DHT.h"

#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

#define ONE_WIRE_BUS 5

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);
float Celcius=0;
float Fahrenheit=0;   
int timeInterval = 0;
int minutes = 1000;
int getSensorRate;  
unsigned long sl = 1;

const int chipSelect = 53;
const int statusled = 6;
const int writefail = 7;
const int writeok = 8;
const int buzzer = 9;

#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0); 


void setup()
{
  
  Serial.begin(9600);
  sensors.begin();

  pinMode(statusled,OUTPUT);
  pinMode(writefail,OUTPUT);
  pinMode(writeok,OUTPUT);
  pinMode(buzzer,OUTPUT);
  
  Initialize_SDcard();
  //Initialize_PlxDaq();
  Serial.println("Enter time interval in Minutes");
  Serial.println("");
  Serial.println("            Office Name");
  Serial.println("             Address");
  Serial.println("");
    
  Serial.println("Instrument. No               "+String(__DATE__));
  //Serial.println(__DATE__);
  Serial.println("");
  Serial.println("");
  Serial.println("Sl.No   Date          Time        Temperature      Humidity");


  Rtc.Begin();
  dht.begin();
  u8g2.begin();
  u8g2.setDisplayRotation(U8G2_R2);


  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Rtc.SetDateTime(compiled);
  
   u8g2.clearBuffer();         // clear the internal memory
   u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
     //setCursor(h,v)
   
   u8g2.drawStr(32,28,"DLCS");  // write something to the internal memory
   u8g2.sendBuffer();         // transfer internal memory to the display
   delay(2000);
}


void Initialize_SDcard()
{
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
      digitalWrite(writeok, LOW);
      digitalWrite(writefail, HIGH);
    // don't do anything more:
    return;
  }
   // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("DLCS.txt", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    
    digitalWrite(writeok, HIGH);
    digitalWrite(writefail, LOW);
    
    dataFile.println("Date,Time,Temperature,Humidity"); //Write the first row of the excel file
    dataFile.close();
  }
}

void Initialize_PlxDaq()
{
Serial.println("CLEARDATA"); //clears up any data left from previous projects
Serial.println("LABEL,Time,Temperature,Serial No"); //always write LABEL, to indicate it as first line
}


#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u:%02u:%02u"),
           //dt.Month(),
           //dt.Day(),
           //dt.Year(), 
            dt.Hour(), 
            dt.Minute(),
            dt.Second() );
      Serial.println(datestring);

}

void getSensorVal()
{ 
  RtcDateTime now = Rtc.GetDateTime();
  printDateTime(now);

  sensors.setResolution(12);
  sensors.requestTemperatures(); 
  Celcius=sensors.getTempCByIndex(0);
  float h = dht.readHumidity();

  Serial.println(String(sl)+"    "+String(__DATE__)+"    "+String(__TIME__)+"       "+(Celcius)+" ºC        "+(h)+"%");

  u8g2.clearBuffer();         // clear the internal memory
  u8g2.setFont(u8g2_font_helvB12_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
  //setCursor(h,v)
  u8g2.setCursor(22,14);
  
  u8g2.print("Temp :"+String(Celcius,1));  // write something to the internal memory
  u8g2.setCursor(22,31);
  
  u8g2.print("Humd :"+String(h,1));
  u8g2.sendBuffer();         // transfer internal memory to the display
   //delay(3000);
   if(Celcius>45){
    digitalWrite(buzzer,HIGH);
  }
  else
    digitalWrite(buzzer,LOW);
    
//Write_SDcard();
  
}

/*
void getFileName(){
sprintf(filename, "%02d%02d%02d.txt", year, month, day);
}
*/

void Write_SDcard()
{
  // getFileName();
  // Serial.println(filename);
    // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("DLCS_new.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {

    dataFile.print(__TIME__); //Store date on SD card
    dataFile.print(","); //Move to next column using a ","

    dataFile.print(Celcius); //Store date on SD card
    dataFile.print(","); //Move to next column using a ","

    dataFile.print(sl); //Store date on SD card
    dataFile.print(","); //Move to next column using a ","

    dataFile.println(); //End of Row move to next row
    dataFile.close(); //Close the file
  }
  else
  Serial.println("OOPS!! SD card writing failed");

}


void loop() {
  
    //If there is anything on the serial port for the Maker Board to read...
    
    if (Serial.available() > 0)
    {
        getSensorRate = Serial.parseInt();
        Serial.println("");
        Serial.println(getSensorRate);
        Serial.println("New Time Interval Recieved.");
        Serial.println("Temperature will be measured for every "+String(getSensorRate)+" Minutes");

        //Converting millis to Minutes
        minutes = 1000*getSensorRate;
      
        u8g2.setFont(u8g2_font_helvB12_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
        //setCursor(h,v)
        u8g2.setCursor(22,20);
        u8g2.print(String(minutes));
        
    }
    
  getSensorVal();
  //Write_SDcard();
  //Write_PlxDaq();
  
  digitalWrite(statusled,LOW);
  delay(1000); 
  digitalWrite(statusled,HIGH);
  sl++;  
  
}

