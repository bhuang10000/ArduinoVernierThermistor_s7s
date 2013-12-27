/*
VernierThermistor (v 2013.11)
 Reads the temperature from a Vernier Stainless Steel Temperature Probe (TMP-BTA)
 or Surface Temperature Sensor (STS-BTA) connected to the BTA connector. 
 As written, the readings will be displayed every half second. Change the variable 
 TimeBetweenReadings to change the rate.
 
 We use the Steinhart-Hart equation (in the function Thermistor) to determine temperature 
 from the raw A/D converter reading. Because of the use of log functions, in the Steinhart-Hart 
 equation, this sketch requires the math.h library. 
 
 See www.vernier.com/engineering/stem/sensors/temperature-sensor/
 for more information on how thermistors are read.
 
 This is a great resource for building your own temperature sensor and understanding
 a little about the theory of operation of these sensors.
 
 http://www2.vernier.com/sample_labs/EPV-01-build_temperature_sensor.pdf
 
 See www.vernier.com/arduino for more information.
 
 Modified by: B. Huang, SparkFun Electronics
 Date: December 4, 2013
 
 Included code to interface to the Serial 7 Segment or Open Segment Display
 
 */

#include <math.h>
#include <SoftwareSerial.h>
#define DELIMITTER '\t'

const int s7sRxPin = 13;  // connect Rx pin of serial 7 segment display to Digital 13
int ThermistorPIN = A0 ;  // Analog Pin 0
float Time;
int Count; //reading from the A/D converter (10-bit)
float Temp;
char tempString[10];

unsigned long timeRef = 0;
unsigned int ndx = 0;
int refreshRate = 50;  // refresh rate of display in millis
int displayVal = 0;

SoftwareSerial s7s1(0, s7sRxPin);  

void setup() 
{

  s7s1.begin(9600);  // start SoftwareSerial interface for Serial Seven Segment Display on pin S7sRxPin
  Serial.begin(9600);  // start HardwareSerial interface for reporting data back to computer terminal
  Serial.println("Vernier Format 2");
  Serial.println("Temperature Readings taken using Ardunio");
  Serial.println("Data Set");
  Serial.print("Time");
  Serial.print("\t"); //tab character
  Serial.println ("Temperature"); 
  Serial.print("seconds");
  Serial.print("\t"); // tab character
  Serial.println ("degrees C"); 
  timeRef = millis();
  ndx = 0;
  clearDisplay();
  setDecimals(0b000000);

  Serial.println();
  Serial.println("*****************************************************");
  Serial.print("Data Collection Rate is set to: ");
  Serial.print(1000 / refreshRate);
  Serial.println(" samples/sec.");
  Serial.println("Type any number in the Serial Monitor to change this.");
  Serial.println("*****************************************************");
  delay(1000);
}
void loop() 
{
  if (Serial.available() > 0)    // check for change in data rate
  {
    refreshRate = 1000 / Serial.parseInt();
    ndx = 0;
    timeRef = millis(); 
    Serial.println();
    Serial.println("*****************************************************");
    Serial.print("Data Collection Rate is set to: ");
    Serial.print(1000 / refreshRate);
    Serial.println(" samples/sec.");
    Serial.println("Type any number in the Serial Monitor to change this.");
    Serial.println("*****************************************************");
    delay(1000);
  }

  if (millis() >= ndx*refreshRate + timeRef)
  {
    ndx++;
    Serial.print((millis() - timeRef)/1000.0, 2); 
    Serial.print(DELIMITTER); 
    Count = analogRead(ThermistorPIN);       // read count from the A/D converter 
    Temp = Thermistor(Count);       // and  convert it to Celsius
    Serial.print(Temp,1);   // display temperature to one digit                                

    int numDigits = log10(abs(Temp));  // determine the number of digits (above the decimal)
    displayVal = abs(Temp) * pow(10, 3 - numDigits);

    if (Temp < 0)
    {
      s7s1.print("-"); 
      if (Temp > -1) setDecimals(0b1 << numDigits);   
      else  setDecimals(0b1 << numDigits + 1);   
           
      sprintf(tempString, "%3d", displayVal);
      tempString[3] = '\0';
      s7sprint(tempString);
    }
    else
    {
      setDecimals(0b1 << numDigits);    
      sprintf(tempString, "%4d", displayVal);
      s7sprint(tempString);
    }  
  }
}

float Thermistor(int Raw) //This function calculates temperature from ADC count
{
  /* Inputs ADC count from Thermistor and outputs Temperature in Celsius
   *  requires: include <math.h>
   * There is a huge amount of information on the web about using thermistors with the Arduino.
   * Here we are concerned about using the Vernier Stainless Steel Temperature Probe TMP-BTA and the 
   * Vernier Surface Temperature Probe STS-BTA, but the general principles are easy to extend to other
   * thermistors.
   * This version utilizes the Steinhart-Hart Thermistor Equation:
   *    Temperature in Kelvin = 1 / {A + B[ln(R)] + C[ln(R)]3}
   *   for the themistor in the Vernier TMP-BTA probe:
   *    A =0.00102119 , B = 0.000222468 and C = 1.33342E-7
   *    Using these values should get agreement within 1 degree C to the same probe used with one
   *    of the Vernier interfaces
   * 
   * Schematic:
   *   [Ground] -- [thermistor] -------- | -- [15,000 ohm bridge resistor] --[Vcc (5v)]
   *                                     |
   *                                Analog Pin 0
   *
   For the circuit above:
   * Resistance = ( Count*RawADC /(1024-Count))
   */
  long Resistance; 
  float Resistor = 15000; //brige resistor
  // the measured resistance of your particular bridge resistor in
  //the Vernier BTA-ELV this is a precision 15K resisitor 
  float Temp;  // Dual-Purpose variable to save space.
  Resistance=( Resistor*Raw /(1024-Raw)); 
  Temp = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
  Temp = 1 / (0.00102119 + (0.000222468 * Temp) + (0.000000133342 * Temp * Temp * Temp));
  Temp = Temp - 273.15;  // Convert Kelvin to Celsius                      
  return Temp;                                      // Return the Temperature
}

void clearDisplay()
{
  s7s1.write(0x76); 
}

void setDecimals(byte decimals)
{
  s7s1.write(0x77);
  s7s1.write(decimals);
}

void s7sprint(String toSend)
{
  Serial.println(toSend);
  s7s1.print(toSend.substring(0, 4));
}








