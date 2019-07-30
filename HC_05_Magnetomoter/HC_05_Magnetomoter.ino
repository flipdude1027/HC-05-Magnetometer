/* This program is modified from my HC_05_Basic program which allows input/output 
 * to/from an HC-05 Bluetooth module. Here's the conceptual idea for this program:
 * 
 * Magnetometer data -> Serial -> Serial1 -> Bluetooth -> Android
 * ***(*NOTE: Serial not necessary! I left it in there to show you my mistake.)***                       
 *                       
 * From here, the data can be processed on an Android app. Ultimately, the data 
 * will be in a String format with the x,y, and z values of the magnetometer. I'll
 * probably use the format "x: <number>, y: <number>, z: <number>;" for each set
 * of data. From then on the Android program will extract only the numbers into an
 * array that has the xyz coordinates of each point. Not sure how I'll structure that.
 * 
 *********************************************************************************  
 * I have to write this again because I forgot to save like an idiot. DO NOT USE
 * write() to send data to Serial1. This sends data as "bytes". Use print() to send
 * data to Serial1. This sends data as readable text (I assume string). write() and 
 * print() do the same thing: they put data into Serial/Serial1. Understand, when
 * data is written to either Serial or Serial1, they AUTOMATICALLY go to their 
 * respective destination, no need to write code to tell the program to do that. So
 * using Serial.print() will automatically display to the Serial Monitor. Using 
 * Serial1.print() will automatically display, via Bluetooth, to whatever app you're
 * using (I used the Bluetooth terminal app). 
 *********************************************************************************
 * ----------------------------- FROM HC_05_Basic --------------------------------
 * Most of the code from Martyn Curry. This program allows Serial Monitor 
 * inputs/outputs from a connected Bluetooth device (a smartphone or something).
 * I'm using the Bluetooth Terminal App for a visual display of data being
 * sent and received. Also, I am using a Mega 2560 Arduino with the Rx/Tx pins
 * of HC-05 connected to the Rx1 and Tx1 slots of the Arduino. Because I am using 
 * the Rx1/Tx1 slots, you will need to use Serial1 in addition to Serial. 
 * 
 * Conceptualize Serial as the computer and Serial1 as the Bluetooth. Also,
 * conceptualize write() as putting data into either Serial or Serial1 and
 * read() as getting data from Serial or Serial1. Think of Serial and Serial1
 * as boxes to put stuff in. We can take stuff from the box (read()) or put 
 * stuff in the box (write()). Sending words through the Serial Monitor is what 
 * we put into the Serial box. Sending words from a terminal on a Bluetooth
 * connected device is what we put into the Serial1 box. Look at the figure 
 * below to further understand this idea. 
 *                        
 *            (BT)               (BT)      (Comp)       (Comp)
 * Words on Connected Device -> Serial1 -> Serial -> Serial Monitor
 *          (Comp)              (Comp)      (BT)          (BT)
 * Words from Serial Monitor -> Serial -> Serial1 -> Connected Device
 * 
 */

// Basic Bluetooth test sketch 5a for the Arduino Mega.
// AT mode using button switch
// HC-05 with EN pin and button switch
//
// Uses serial with the host computer and serial1 for communication with the Bluetooth module
//
//  Pins
//  BT VCC to Arduino 5V out. Disconnect before running the sketch
//  BT GND to Arduino GND
//  BT RX (through a voltage divider) to Arduino TX1 (pin 18)
//  BT TX  to Arduino RX1 (no need voltage divider)   (pin 19)
//
// When a command is entered in to the serial monitor on the computer
// the Arduino will relay it to the Bluetooth module and display the result.
//
#include "MPU9250.h"

// an MPU9250 object with the MPU-9250 sensor on I2C bus 0 with address 0x68
MPU9250 IMU(Wire,0x68);
int status;
char input;

void setup()
{

  /* So we need to do two things to initialize the program: initialize
   * Serial and the Serial1 with baud rates. Make sure the baud rates 
   * are the same or else you'll get weird characters. 
   */
   
  // Communication with main Serial
  Serial.begin(9600);

  // Communication with HC-05 Bluetooth and Serial1
  Serial1.begin(38400); // Sometimes gets garbage characters at same baud rate

  // Something to give visual confirmation on Serial Monitor
  Serial.println(" Start ");

  // Communication with IMU(magnetometer)
  status = IMU.begin();
  if (status < 0) {
    Serial.println("IMU initialization unsuccessful");
    Serial.println("Check IMU wiring or try cycling power");
    Serial.print("Status: ");
    Serial.println(status);
    while(1) {}
  }
}


void loop()
{
  // Gets values from the magnetometer
  IMU.readSensor();

  // Get the x,y,z values of the magnetometer
  /* IMY.getMagxyz_uT() is a floating point decimal which cannot be directly sent via
   * Bluetooth. These numbers will look like ? symbols in the Bluetooth Terminal App. 
   * Instead, I'm going to take each digit and send them as single characters. On the
   * Android application I will make I will put back together the characters to create
   * the number. 
   * 
   * Decimal Number -> Individual Characters -> Reform Decimal
   * ---------------------------------------------------------------------------------
   * 
   * All the crap above isn't necessary. I figured it out. Use Serial1.println() 
   * instead of Serial1.write(). write() sends bytes while println() sends 
   * readable text. write() and println() do the same thing: they put stuff into
   * Serial or Serial1. Again, as noted in the paragraph at the top of this program,
   * Anything that is written (and printed) to either Serial or Serial1 is AUTOMATICALLY
   * sent to either the Serial Monitor (Serial) or the visual display of the connected device
   * (I used the Bluetooth Terminal App on my phone). 
   */
  //Serial.println((int)trunc(IMU.getMagX_uT())); // Whole number
  Serial1.print((int)trunc(IMU.getMagX_uT()));
  Serial1.print("/"); // Delimiter to separate data 
  Serial1.print((int)trunc(IMU.getMagY_uT()));
  Serial1.print("/");
  Serial1.print((int)trunc(IMU.getMagZ_uT()));
  Serial1.println(";"); // Delimiter to show end of data point

  
  // Listens for communication from the BT module, then write it to the serial monitor
  /* -------------------------------From HC_05_Basic----------------------------------
   * Data that is sent from another device to the HC-05 automatically enters the 
   * Serial1 object. Data is sent byte by byte, so a word/string will be split up and 
   * written onto the Arduino Serial Monitor letter by letter. 
   * 
   *            (BT)               (BT)      (Comp)       (Comp)
   * Words on Connected Device -> Serial1 -> Serial -> Serial Monitor
   * ---------------------------------------------------------------------------------
   * Probably not going to use this in the grand scheme of things. But will keep here JIC.
   * ---------------------------------------------------------------------------------
   * *Yeah I don't need anything below this.*
   */
  if (Serial1.available())   { 
    Serial.write( Serial1.read() ); // Serial.write() automatically prints to Serial Monitor
  }

  // Listens for magnetometer data and sends it to HC-05
  /* User input is through the Serial Monitor. 
   *  
   *          (Comp)              (Comp)      (BT)          (BT)
   * Words from Serial Monitor -> Serial -> Serial1 -> Connected Device
   */
  if ( Serial.available() )   {
    input = Serial.read();
    Serial1.write( input );
    //Serial.println(input); // Uncomment this if you want to see what you wrote from Serial Monitor on the Serial Monitor
  }
  delay(2000); // Slows down visual display of data (I used Bluetooth Terminal App on my phone). Delete as needed.
}
