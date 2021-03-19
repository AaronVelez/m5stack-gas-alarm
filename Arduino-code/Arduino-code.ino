/*
 Name:		Arduino_code.ino
 Created:	2/18/2021 12:12:37 PM
 Authors:	Daniela Cornejo y Aarón Vélez
*/




//////////////////////////////////////////////////////////////////
////// Libraries and its associated constants and variables //////
//////////////////////////////////////////////////////////////////

////// Board library
#include <M5Stack.h>


////// Comunication libraries
#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
const char* ssid = "<SSID>";
const char* password = "<PASSWORD>";
WiFiUDP ntpUDP;


// Time libraries
#include <TimeLib.h>
#include <NTPClient.h>
NTPClient timeClient(ntpUDP, "north-america.pool.ntp.org", 0, 1000); // For details, see https://github.com/arduino-libraries/NTPClient

#include <Timezone.h>
//  Central Time Zone (Mexico City)
TimeChangeRule mxCDT = { "CDT", First, Sun, Apr, 2, -300 };
TimeChangeRule mxCST = { "CST", Last, Sun, Oct, 2, -360 };
Timezone mxCT(mxCDT, mxCST);


////// Library for Oxygen Sensor
#include <DFRobot_OxygenSensor.h>
#define COLLECT_NUMBER    10             // collect number, the collection range is 1-100.
#define Oxygen_IICAddress ADDRESS_3
/*   iic slave Address, The default is ADDRESS_3.
       ADDRESS_0               0x70      // iic device address.
       ADDRESS_1               0x71
       ADDRESS_2               0x72
       ADDRESS_3               0x73
*/
DFRobot_OxygenSensor Oxygen; // Decalre the class for the Oxygen sensor 


////// Library for SHT31 Temperature and Humidity Sensor
//#include <DFRobot_SHT3x.h>




//////////////////////////////////////////
////// User Constants and Variables //////
//////////////////////////////////////////

////// Time variables
unsigned long time_ms = 0;
time_t unix_t;	// UNIX time stamp
int s = -1;		//Seconds
int m = -1;		//Minutes
int h = -1;		//Hours
int dy = -1;	//Day
int mo = -1;	//Month
int yr = -1;	//Year


////// State machine Shift Registers
int LastSum = -1;			// Last minute that variables were added to the sum for later averaging
int SumNum = 0;				// Number of times a variable value has beed added to the sum for later averaging
int LastLog = -1;			// Last minute that variables were loged to the SD card
bool PayloadRdy = false;	// Payload ready to send to LoRa


////// Measured instantaneous variables
float O2Value = -1;		// Oxygene value read each second
float CO2Value = -1;    // Carbon dioxide value read each second
float Temp = -1;        // Air temperature read each minute
float RH = -1;          // Air RH value read each minute


////// Variables to store sum for eventual averaging
float O2ValueSum = 0;
float CO2ValueSum = 0;
float TempSum = 0;
float RHSum = 0;


////// Values to be logged. They will be the average over the last 5 minutes
float O2ValueAvg = 0;
float CO2ValueAvg = 0;
float TempAvg = 0;
float RHAvg = 0;


//////////////////////////////////////////////////////////////////////////
// the setup function runs once when you press reset or power the board //
//////////////////////////////////////////////////////////////////////////
void setup() {
    // Initialize M5Stack and setup power
    M5.begin();
    M5.Power.begin();
    M5.Lcd.println("M5 started");
    Serial.println("M5 started");
    WiFi.begin(ssid, password);
    WiFi.setAutoReconnect(true);
    M5.Lcd.print("Connecting to internet");
    while ( WiFi.status() != WL_CONNECTED ) {
        M5.Lcd.println(WiFi.status());
        delay(1000);
        //M5.Lcd.print(".");
    }
    M5.Lcd.println();

    // Start NTP client engine
    timeClient.begin();
    
    //Serial.begin(115200); // initalize serial comunication with computer
    //Serial.println(F("Serial communication started"));

    Oxygen.begin(Oxygen_IICAddress);
    //Serial.println(F("Oxygen sensor started"));
    M5.Lcd.print("Oxygen started");

}
 


//////////////////////////////////////////////////////////////////////////
// The loop function runs over and over again until power down or reset //
//////////////////////////////////////////////////////////////////////////
void loop() {
    M5.Lcd.print("Loop start");

    ////// State 2. Get current time
    if (WiFi.isConnected()) { // get UTC unix timestamp from internet time via NTC
        timeClient.update();    // It does not force-update NTC time (see NTPClient declaration for actual udpate interval)
        unix_t = timeClient.getEpochTime();
        unix_t = mxCT.toLocal(unix_t); // Conver to local time
        setTime(unix_t);   // set system time to given unix timestamp
        time_ms = millis();
        M5.Lcd.println(time_ms);
    }
    else { // If no internet connection, estimate unixtime from last update and enlapsed millis
        M5.Lcd.println("No internet");
        if (millis() - time_ms > 1000) {
            unix_t = unix_t + round(((millis() - time_ms) / 1000));
            setTime(unix_t);   // set system time to given unix timestamp
            time_ms = millis();
        }
    }
    s = second(unix_t);
    m = minute(unix_t);
    h = hour(unix_t);
    dy = day(unix_t);
    mo = month(unix_t);
    yr = year(unix_t);
    M5.Lcd.println((String) "Time: " + h + ":" + m + ":" + s);



    ////// State 3. Test if it is time to read gas sensor values (each second?)
    if (true) { // logical test still need to be added
        O2Value = Oxygen.ReadOxygenData(COLLECT_NUMBER); //DFRobot_OxygenSensor Oxygen code
        M5.Lcd.print((String)"Oxygene: " + O2Value + " %vol");
        ////// Test if values are within safe limits
        if (true) { // logical test still need to be added
            // Set alarm global variable to TRUE
        }
    }

    
    ////// State 4. Test if alarm must be triggered
    if (true) { // logical test still need to be added
        // Triger alarm (sound, emails, flashes, etc)
    }
    

    ////// State 5. Test if it is time to read Temp and RH values AND record sensor values for 5-minute averages (each minute)
    if (m != LastSum) {
        // Read Temp and RH sensor values
        // add sensor value to sum variable
        // Update Shift registers
        LastSum = m;
        SumNum += 1;
    }


    ////// State 6. Test if it is time to compute  averages and record in SD card (each 5 minutes)
    if (((m % 5) == 0) && (m != LastLog)) {
        // calculate averages
        // record averages to sd card
        // Reset Shift Registers
        LastLog = m;

        O2ValueSum = 0;
        CO2ValueSum = 0;
        TempSum = 0;
        RHSum = 0;

        SumNum = 0;
    }


    ////// State 7. Test if there is data available to be sent to cloud
    if (true) { // logical test still need to be added
        // add code here
    }


  /*
 //Read voltage for the CO2 sensor
  int sensorValue = analogRead(sensorIn);


  // The analog signal is converted to a voltage
  float voltage = sensorValue*(5000/1024.0);
  if(voltage == 0)
  {
    Serial.println("Fault");
  }
  else if(voltage < 400)
  {
    Serial.println("preheating");
  }
  else
  {
    int voltage_diference=voltage-400;
    float concentration=voltage_diference*50.0/16.0;
    // Print Voltage
    Serial.print("voltage:");
    Serial.print(voltage);
    Serial.println("mv");
    //Print CO2 concentration
    Serial.print(concentration);
    Serial.println("ppm");
  }
  delay(100); // This will show the ppm on the air of CO2
  */

}


