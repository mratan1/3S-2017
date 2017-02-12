/*
* 
* Objective:  stabilize number of lanturns at maximum power, vary power and restabilize
*
* Last edit: 2/11/17 by Kevin Keene 
*
* File: MPPTTester.ino
* --------------------------
*
* 
*/

#include "SD.h"
#include <Wire.h>
#include  "RTClib.h"

//turns on debug mode
//#define DEBUG

/****************************  EDITABLE CONSTANTS  ****************************/

const int MAX_LANTERNS = 32;
//digital output pins in the order that they will be turned on and off
const int OUTPUT_PINS[] = {
  22, 23, 24, 25, 26, 27, 28, 29,
  30, 31, 32, 33, 34, 35, 36, 37,
  38, 39, 40, 41, 42, 43, 44, 45,
  46, 47, 48, 49, 2, 4, 7, 8
};
const int VOLTAGE_INPUT_PIN = A0;
const int CURRENT_INPUT_PIN = A1;
const int chipSelect = 53;

// Currently Unused
  const float MAX_PANEL_VOLTAGE = 48;
  const float MAX_PANEL_CURRENT = 6;


/****************************  STRUCTURES / CLASSES  ****************************/

struct PanelMeasurements {
  float voltage;
  float current;
  float power; 
};

/****************************  GLOBAL VARIABLES   ****************************/

//RTC_DS1307 RTC; // Real Time Clock

float illumination = 5.0; //initial value
int numActivePins = 1;
PanelMeasurements prevMeasurements;

//File logFile;

/**********************************  SETUP  ***********************************/

void setup() {
  Serial.begin(9600);
  //setUpSD();
  //logHeader(); 
  //setUpRTC();
  setUpPins(); 

//set initial prevMeasurements values, initiate movement up curve
  prevMeasurements.current = 0; 
  prevMeasurements.power = 0; 
  
  
}

/********************************  LOOP  *********************************/

void loop() {
  perturbAndObserve();
  //logData();
  delay(5000); 
}

/********************************  FUNCTIONS  *********************************/

/*
 * Function: setUpRTC
 * -------------------
 * Sets up the Real Time Clock (RTC) of the arduino 
 * To reset the time: uncomment the line. This will reset to time of upload
 */

/*void setUpRTC(){
  Wire.begin();  

  if(!RTC.begin()){
    Serial.println("RTC failed");  
  }
  
  if (!RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}*/

/*
 * Function: setUpPins
 * -------------------
 * Sets up the pins to output / input 
 */
void setUpPins(){
  for (int i = 0; i < MAX_LANTERNS; i++) {
    pinMode(OUTPUT_PINS[i], OUTPUT);
  }
}


/*
 * Function: measureInputs
 * -------------------
 * Uses eclispe equation to model IV curve 
 * Data from SolarWorld 250 mono panel, 600W/m^2 illumination, max V = 40V, max current = 5A
 */
PanelMeasurements measureInputs() {
  PanelMeasurements measurements;
  
  if (Serial.available() > 0) {
    illumination = Serial.parseFloat();
  }
  
  Serial.print("Illumination ");
  Serial.print(illumination);
  Serial.print("\n");

  float measureCurrent = numActivePins * 0.5; //lanturn draws 0.5A
  float measureVoltage = 40.0 * sqrt(1.0 - measureCurrent*measureCurrent / (illumination * illumination)); //eclipse equation
  
  measurements.voltage = measureVoltage;
  measurements.current = measureCurrent;
  measurements.power = measurements.voltage * measurements.current;

  Serial.print("Voltage ");
  Serial.print(measurements.voltage);
  Serial.print("\n");
  //Serial.println(measureVoltage);
  Serial.print("Current ");
  Serial.print(measurements.current);
  Serial.print("\n");
 // Serial.println(measureCurrent);
  Serial.print("Power ");
  Serial.print(measurements.power);
  Serial.print("\n\n");

  return measurements;
}

/*
 * Function: perturbAndObserve
 * -------------------
 *  Compares panel current and power to current and power in previous time to decide to add/remove lantern
 *   MPPT Algorithm:
 *     Change in power and change in current determines if the current operation is to the right or left of the maximum power point
 *     on an IV curve, and moves in the direction of the MPP.
 *     If panel power increases, and panel current increases: charge 1 additional lantern
 *     If panel power increases, and panel current stays the same or decreases: charge 1 fewer lantern
 *     If panel power stays the same or decreases, and panel current increases: charge 1 fewer lantern
 *     If panel power stays the same or decreases, and panel current stays the same or decreases: charge 1 additional lantern
 */
void perturbAndObserve() {
  PanelMeasurements newMeasurements = measureInputs();
  
    if (newMeasurements.power > prevMeasurements.power) {
      if (newMeasurements.current > prevMeasurements.current) {
       addOutput(); 
      } else {
       removeOutput();
      }
    } else {
      if (newMeasurements.current > prevMeasurements.current) {
        removeOutput();
      } else {
        addOutput();
      }

  }
  activatePins();
  prevMeasurements = newMeasurements;
}

/*
 * Function: addOutput
 * -------------------
 * Adds a lantern
 */
 void addOutput() {
  if (numActivePins < MAX_LANTERNS) {
    numActivePins++;
  }
}

/*
 * Function: evaluateState
 * -------------------
 * Removes a lantern
 */
 void removeOutput() {
  if (numActivePins > 0) {
    numActivePins--;
  }
}

/*
 * Function: activatePins
 * -------------------
 * Turns on all the pins with active lanterns and turns off all the non-active ones. 
 */
void activatePins() {
  for (int i = 0; i < numActivePins; i++) {
    //Serial.print("Turning on pin ");
    //Serial.println(i);
    digitalWrite(OUTPUT_PINS[i], HIGH);
  }

  for (int i = numActivePins; i < MAX_LANTERNS; i++) {
    //Serial.print("Turning off pin ");
    //Serial.println(i);
    digitalWrite(OUTPUT_PINS[i], LOW);

  }
}

/*
 * Function: setUpSD
 * -------------------
 * Sets up the SD Card
 */
/*void setUpSD() {
  Serial.print("Initializing SD card...");
  if (!SD.begin(10,11,12,13)) {
    Serial.println("Card failed, or not present");
  }
  logFile = SD.open("logfile.txt", FILE_WRITE);
  if (!logFile) {
    Serial.println("Failed to open log file");
  }else{
    Serial.println("card initialized.");  
  }
  logFile.close(); 
}
*/

/*
 * Function: logHeader
 * -------------------
 * Prints the header for the SD card
 */
/*void logHeader() {
  logFile = SD.open("logfile.txt", FILE_WRITE);
  if(logFile){
    logFile.print("Time, ");
    logFile.print("Voltage, ");
    logFile.print("Current, ");
    logFile.print("Power, ");
    logFile.println("Active Lanterns");
    logFile.flush(); 
    logFile.close(); 
  }
}*/
/*
 * Function: logData
 * -------------------
 * Prints the data in CSV format
 */
/*void logData() {
  logFile = SD.open("logfile.txt", FILE_WRITE);
  if(logFile){
    DateTime now = RTC.now(); 
    logFile.print(now.year(), DEC);
    logFile.print('/');
    logFile.print(now.month(), DEC);
    logFile.print('/');
    logFile.print(now.day(), DEC);
    logFile.print(" ");
    logFile.print(now.hour(), DEC);
    logFile.print(':');
    logFile.print(now.minute(), DEC);
    logFile.print(':');
    logFile.print(now.second(), DEC);
    logFile.print(",");
    
    logFile.print(String(prevMeasurements.voltage));
    logFile.print(",");
    logFile.print(String(prevMeasurements.current));
    logFile.print(",");
    logFile.print(String(prevMeasurements.power));
    logFile.print(",");
    logFile.println(String(numActivePins));
    logFile.flush(); 
    logFile.close();
  }
}*/

/*
 * Function: initSweeep
 * -------------------
 * Another possible method of performing a linear sweep by running the perturbAndObserve until it no longer
 * changes. 
 * Danger: Oscillation. 
 */
/*void initSweep() {
    int activeLanterns = numActivePins; 
    while(activeLanterns != numActivePins){
      activeLanterns = numActivePins; 
      perturbAndObserve(); 
    }
}*/

/*
 * Function: linearSweep
 * -------------------
 * Compares the power for all the possible number of lanterns and activates the optimum number. 
 */
/*void linearSweep() {
    int pinsForMaxPow = 0; 
    float maxPow = 0; 

    Serial.println("Begining linear sweep");

    for(int i = 0; i < MAX_LANTERNS; i++) {
        numActivePins = i; 
        activatePins(); 
        PanelMeasurements newMeasurements = measureInputs();
        if(newMeasurements.power > maxPow) {
          maxPow = newMeasurements.power; 
          pinsForMaxPow = i; 
        }
    }
    numActivePins = pinsForMaxPow; 
    activatePins(); 

    Serial.print("Linear sweep finished. Maximum power found: ");
    Serial.print(maxPow);
    Serial.print(" Number of lanterns: ");
    Serial.println(pinsForMaxPow);
}*/
