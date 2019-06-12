/*
Sketch for driving Agilent attenuator with an rotary encoder
*/

#include <Wire.h>
#include "SSD1306Ascii.h" //https://github.com/greiman/SSD1306Ascii
#include "SSD1306AsciiWire.h"

#define I2C_ADDRESS 0x3C // 0X3C+SA0 - 0x3C or 0x3D
#define RST_PIN -1 // Define proper RST_PIN if required

#define ENCA 3 // Rotary encoder clockwise on pin 2
#define ENCB 2 // Rotary encoder counterclockwise on pin 3

#define SWI 4 // Define switch: directset or setting attenuator after pressing the setbutton
#define SBN 5 // Define setbutton: set attenuator
#define X10 6 // Define stepsize of the rotary encoder

SSD1306AsciiWire oled;
int encA0, encA1, encB0, encB1,
    att1, att2, attenuator,
    displayvalue0, displayvalue1 = 0;
int counter0, counter1 = 0;
int startup = 0;
byte bitnew = 0;
String attdisplay;

void setup()
{
  Serial.begin(9600);

  Wire.begin();
  
  // For the rotary encoder
  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);

  pinMode(SWI, INPUT_PULLUP);
  pinMode(SBN, INPUT_PULLUP);
  pinMode(X10, INPUT_PULLUP);
  
  // Output pins for the attenuator
  for(int i = 7; i < 14; i++)
  {
    pinMode(i, OUTPUT);
  }

  // Initialize oled-display  
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(fixed_bold10x15);
  oled.clear();
  oled.println(" Verzwakker");
  oled.set2X();

  counter0 = 0;
  counter1 = 81;
}

void setattenuator()
{
  // Write to the Agilent attenuator
  att1 = counter1 / 10;
  att2 = counter1 % 10;
  
  // For 80 and 81 dB attenuation
  if (att1 == 8)
  {
    att1 = 7;
    att2 = att2 + 10;
  }
  
  // The fourth attenuator in the 8494H is 4dB
  if (att2 > 7)
    att2 = att2 + 4;

  attenuator = (att1 << 4) + att2;

  for(int i = 0; i < 7; i++)
  {
  bitnew = bitRead(attenuator, i);
  delay(100);
  digitalWrite(i + 7, bitnew);
  }

  counter0 = counter1;
}

void loop() {

  // For the rotary encoder
  encA1 = digitalRead(ENCA);
  if (encA1 != encA0)
  {
    if (encA1 == 1)
    {
      // Check for the stepsize switch
      if(digitalRead(X10) == LOW)
        counter1 = counter1 + 10;
      else
        counter1++;
    }
    encA0 = encA1;
  }
  
  encB1 = digitalRead(ENCB);
  if (encB1 != encB0)
  {
    if (encB1 == 1)
    {
      // Check for the stepsize switch
      if(digitalRead(X10) == LOW)
        counter1 = counter1 - 10;
      else
        counter1--;
    }
    encB0 = encB1;
  }
  
  // Counter1 has to be between 0 and 81 dB
  if (counter1 < 0)
	  counter1 = 0;
  if (counter1 > 81)
    counter1 = 81;

  // Detect a new value from the encoder
  if (counter1 != counter0)
  {
    // Show value on oled
    displayvalue1 = counter1;
    if (displayvalue1 != displayvalue0)
    {
      displayvalue0 = displayvalue1;
      oled.setCursor(15,3);
      oled.clearToEOL();
      attdisplay = String(counter1);
      oled.print(attdisplay);
      oled.print(" dB ");
    }
    
    // Check if the set-button is pressed or do direct set
    if(digitalRead(SBN) == LOW || digitalRead(SWI) == HIGH || startup == 0)
      setattenuator();

    startup = 1;
  }
}
