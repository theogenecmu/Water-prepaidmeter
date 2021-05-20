/*****************Libraries************/
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
#include <SPI.h>
#include <Wire.h>
#include <TinyGPS++.h>
/******************end****************/
/****************wifi connections********************/
char auth[] = "mgsr2wMR7o4vChAMAgKzvfFvADdcDGLi";              //Authentication key
char ssid[] = "theogene";                                       // network name
char pass[] = "1234567890";                                      // network Password
/******************end****************************/
/********************gps pin declaration***************/
//GPS RX to D1 & GPS TX to D2 and Serial Connection
const int RXPin = 4, TXPin = 5;
const uint32_t GPSBaud = 9600;
SoftwareSerial gps_module(RXPin, TXPin);

TinyGPSPlus gps;
WidgetMap myMap(V0); //V0 - virtual pin for Map

BlynkTimer timer; //starting time that controls the dispaly intervals of latitude and longitude 

/**************************end***************/
/*****************variable declaration*********/
#define SENSOR  2    //waterflow sensor on pin D4
#define red 15
#define green 16
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
//boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned long flowMilliLitres;
unsigned int totalMilliLitres;
float flowLitres;
float totalLitres;
int payment;
/************************************end*************/

/*************** counting pulses from waterflow******/
void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}
/***********************end***************/
//WiFiClient client;

/* LCD in blynk*/
WidgetLCD lcd(V4);
/********end***********/
//unsigned int move_index;         
unsigned int move_index = 1; 
void setup()
{
  Serial.begin(9600);
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(SENSOR, INPUT_PULLUP);
  gps_module.begin(GPSBaud); //receiving GPS baudrate
  timer.setInterval(5000L, checkGPS); //setting interval
  Blynk.begin(auth, ssid, pass); // Blynk
  /***************** other variable that for water flow sensor******/
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;
  /********************** end ****************/

  int metters[3] = {1234, 567, 890}; // an array that store meter number
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
}
/*********************checking if Gps is available and reading***/
void checkGPS(){
  if (gps.charsProcessed() < 10)
  {
        Blynk.virtualWrite(V4, "GPS ERROR"); // print sms on Lcd under Bylnk 
  }
}
/***************************end***************/

/****************************checking if meter number exist in the system********/
BLYNK_WRITE(V10) // V10 is the number of Virtual Pin  for receiving payment
{
  int metters[3] = {1234, 567, 890};
  lcd.clear();
  int pinValue1 = param.asInt(); //checking meter number receiving payment from terminal as a number(int)
  for (int i = 0; i < sizeof(metters); i++)
  {
    if (pinValue1 == metters[i])
    {
      lcd.clear();
      lcd.print(1, 0 , "Meter is valid ");
      delay(5000);
      lcd.clear();
      //goto paidValue;
      break;
    }
    else
    {
      lcd.clear();
      lcd.print(0, 0, "meter is wrong");
      lcd.print(0, 1, "retype it");
    }
  }
}
/**********************************end***********************/
/***************payment***********/
BLYNK_WRITE(V11) // V11 is the number of Virtual Pin  for receiving payment
{
  payment = param.asInt(); // receiving payment from terminal as a number(int)
  lcd.clear();
  lcd.print(0, 0, "Meter is loaded:");
  lcd.print(0, 1, payment);
  delay(2000);
  lcd.clear();
}
/************************end *******************/
void loop()
{
  /************reading water flow sensor and do calibrations*************/
  currentMillis = millis();
  if (currentMillis - previousMillis > interval)
  {

    pulse1Sec = pulseCount;
    pulseCount = 0;

    // the loop may not complete is exactly 1 sec interval,so we need to calcuate the interval by ourselves by using below formula
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    flowLitres = (flowRate / 60);

    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
    totalLitres += flowLitres;

    // Print the cumulative total of litres flowed from the starting starting on Blynk

    if ((payment - totalLitres) > 0)
    {
      lcd.print(0, 0, "consu:");
      lcd.print(6, 0, totalLitres );
      lcd.print(13, 0, "L" );
      lcd.print(0, 1, "balance:" );
      lcd.print(8, 1, (payment - totalLitres));
      lcd.print(14, 1, "L");
      digitalWrite(green, HIGH);
      digitalWrite(red, LOW);
    }
    else
    {
      lcd.clear();
      lcd.print(0, 1, "balance:" );
      lcd.print(8, 1, 0);
      //lcd.print(0, 1, "No payment made " );
      digitalWrite(red, HIGH); // turn the LED off
      digitalWrite(green, LOW);
      delay(3000);
    }

  }
  /**************************end ********************/
  /*************************************Gps***************/
  while (gps_module.available() > 0) 
  {
    //displays information every time a new sentence is correctly encoded.
    if (gps.encode(gps_module.read()))
    displayInfo();
  }
  Blynk.run(); // running Blynk
  timer.run(); //running time
}
/***************************dispalyinf latitude and longutide**********************/
void displayInfo()
{
  //WidgetMap myMap(V0);
  if (gps.location.isValid()) 
  {
    //Storing the Latitude. and Longitude
    float latitude = (gps.location.lat());
    float longitude = (gps.location.lng()); 
        
    Blynk.virtualWrite(V1, String(latitude, 6));   
    Blynk.virtualWrite(V2, String(longitude, 6));  
    myMap.location(move_index,latitude, longitude, "GPS_Location");
    
  }
  
  
}

/**************************end*******************************************************/
