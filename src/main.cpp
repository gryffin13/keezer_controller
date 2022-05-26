#include <Arduino.h>
#include <OneWire.h>
#include <LiquidCrystal.h>
#include <FastLED.h>
int Relay = 12; 
float highTemp = 50;
float lowTemp = 45;
const int selectButtonPin = 2;
const int upButtonPin = 3;
const int downButtonPin = 4; 

#define NUM_LEDS 1
#define DATA_PIN 13
CRGB leds[NUM_LEDS];

volatile int selectButtonState = 0;         // variable for reading the pushbutton status
volatile int downButtonState = 0;  
volatile int upButtonState = 0;  

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 200;    // the debounce time; increase if the output flickers
unsigned long tempChangeTime = 0;
unsigned long tempDelay = 200;
unsigned long lastSampleTime = 0;
unsigned long sampleDelay = 1000;
unsigned long lastLoopTime = 0;
unsigned long loopDelay = 3000;


OneWire  ds(6);  // on pin 6
LiquidCrystal lcd(7, 8, 9, 10, 11, 5);
//LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Function to change the value of the upper temperature limits
void changeHighTemp(){
   while (digitalRead(selectButtonPin) == LOW){ //allows changes until "select button" is pressed - then exits menu
    if (millis()-lastDebounceTime > debounceDelay){  //only allow button presses once every so often (defined by debounce delay)       
        if (digitalRead(upButtonPin)==HIGH){//when "up button" is pressed, increments limit by +1
          highTemp = highTemp+1;
          lcd.clear();
          lcd.print("NEW HIGH");//displays changed limit on LCD screen
          lcd.setCursor(0,1);
          lcd.print(": ");
          lcd.print(highTemp); 
          Serial.println("new high temp:");
             Serial.println(highTemp);
        
        }
        else if (digitalRead(downButtonPin)==HIGH){//when "down button" is pressed, decrements limit by -1
          highTemp = highTemp-1;
          lcd.clear();
          lcd.print("NEW HIGH");//displays changed limit on LCD screen
          lcd.setCursor(0,1);
          lcd.print(": ");
          lcd.print(highTemp); 
          Serial.println("new high temp:");
             Serial.println(highTemp);
        }
        else {
          }
          lastDebounceTime=millis();
      }
   }
   lcd.clear();
  lcd.print("NEW LIMIT");
  lcd.setCursor(0,1);
  lcd.print("SET");
   return;
   }

// Function to change the value of the lower temperature limits
void changeLowTemp(){
   while (digitalRead(selectButtonPin) == LOW){ //allows changes until "select button" is pressed - then exits menu
    if (millis()-lastDebounceTime > debounceDelay){ //only allow button presses once every so often (defined by debounce delay)    
        if (digitalRead(upButtonPin)==HIGH){ //when "up button" is pressed, increments limit by +1
          lowTemp = lowTemp+1;
          lcd.clear();
          lcd.print("NEW LOW:");//displays changed limit on LCD screen
          lcd.setCursor(0,1);
          lcd.print(" ");
          lcd.print(lowTemp); 
          Serial.println("new low temp:");
             Serial.println(lowTemp);
        }
        else if (digitalRead(downButtonPin)==HIGH){//when "down button" is pressed, decrements limit by -1
          lowTemp = lowTemp-1;
          lcd.clear();
          lcd.print("NEW LOW:");//displays changed limit on LCD screen
          lcd.setCursor(0,1);
          lcd.print(" ");
          lcd.print(lowTemp); 
          Serial.println("new low temp:");
             Serial.println(lowTemp);
        }
        else {
          }
          lastDebounceTime=millis();
      }
   }
   lcd.clear();
  lcd.print("NEW LIMIT");
  lcd.setCursor(0,1);
  lcd.print("SET");
   return;
}



void setup(){ 
pinMode(Relay, OUTPUT);     //Set Pin12 as output 
lcd.begin(8, 2);
lcd.clear();
//lcd.print("  Current Temp:");
Serial.begin(9600);
Serial.println("new function start");
FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

}
//----------------------------
//temperature sampling and Farenheit return function for use in main loop 
  float TempReturn(){
        byte i;
          byte present = 0;
          byte type_s;
          byte data[12];
          byte addr[8];
          float celsius, fahrenheit;
          if ( !ds.search(addr)) {
            ds.reset_search();
            delay(250);
            return(-99.9);
          }
          for( i = 0; i < 8; i++) {
          }
         
          if (OneWire::crc8(addr, 7) != addr[7]) {
        
              return(-99.9);
          }
          // the first ROM byte indicates which chip
          switch (addr[0]) {
            case 0x10:
              //Serial.println("  Chip = DS18S20");  // or old DS1820
              type_s = 1;
              break;
            case 0x28:
              //Serial.println("  Chip = DS18B20");
              type_s = 0;
              break;
            case 0x22:
              //Serial.println("  Chip = DS1822");
              type_s = 0;
              break;
            default:
              //Serial.println("Device is not a DS18x20 family device.");
              return(-99.9);
          }
          ds.reset();
          ds.select(addr);
          ds.write(0x44,1);         // start conversion, with parasite power on at the end
          delay(1000);     // maybe 750ms is enough, maybe not
          // we might do a ds.depower() here, but the reset will take care of it.
          present = ds.reset();
          ds.select(addr);   
          ds.write(0xBE);         // Read Scratchpad
         
          //Serial.print("  Data = ");
          //Serial.print(present,HEX);
          //Serial.print(" ");
          for ( i = 0; i < 9; i++) {           // we need 9 bytes
            data[i] = ds.read();
            //Serial.print(data[i], HEX);
            //Serial.print(" ");
          }
          //Serial.print(" CRC=");
          //Serial.print(OneWire::crc8(data, 8), HEX);
          //Serial.println();
         
          // convert the data to actual temperature
         
          unsigned int raw = (data[1] << 8) | data[0];
          if (type_s) {
            raw = raw << 3; // 9 bit resolution default
            if (data[7] == 0x10) {
              // count remain gives full 12 bit resolution
              raw = (raw & 0xFFF0) + 12 - data[6];
            }
          } else {
            byte cfg = (data[4] & 0x60);
            if (cfg == 0x00) raw = raw << 3;  // 9 bit resolution, 93.75 ms
            else if (cfg == 0x20) raw = raw << 2; // 10 bit res, 187.5 ms
            else if (cfg == 0x40) raw = raw << 1; // 11 bit res, 375 ms
            // default is 12 bit resolution, 750 ms conversion time
          }
          celsius = (float)raw / 16.0;
          fahrenheit = celsius * 1.8 + 32.0;
          return fahrenheit; //returns value of temperature in degrees farenheit as "TempF"
    }


//---------------------------------------
// temperature change function - entered when button 1 (select button) is pressed
void tempChange() {
  Serial.println("entering temp change function");
  lastLoopTime=millis(); //sets record of time loop was entered
  lcd.clear(); //clear CLD to display menu text
  lcd.print("CHNG HI ");
  lcd.setCursor(0,1);
  lcd.print("OR LOW?");
    Serial.println("millis:");
  Serial.println(millis());
Serial.println("last loop time:");
  Serial.println(lastLoopTime);
  while ((millis()-lastLoopTime) < loopDelay){ //waits for button press only as long as menu delay (loopDelay) is set to wait - exits if nothing pressed in time
    if (digitalRead(upButtonPin)==HIGH){ //if "up button" is pressed, enters function for changing high temp limit
        lcd.clear();
        lcd.print("CHANGE U");//displays text to show which choice was registered
        lcd.setCursor(0,1);
        lcd.print("P LIMIT"); 
        delay(500);
        Serial.println("change high temp function:");
             
      changeHighTemp();    
      
    }
    else if (digitalRead(downButtonPin)==HIGH){//if "down button" is pressed, enters function for changing low temp limit
      lcd.clear();
        lcd.print("CHANGE L");//displays text to show which choice was registered
        lcd.setCursor(0,1);
        lcd.print("OW LIMIT"); 
        delay(500);
        Serial.println("change low temp function:");
             
      changeLowTemp();
    }
    else {
      }
  }
  lcd.clear();
  lcd.print("EXITING ");
  lcd.setCursor(0,1);
  lcd.print("TO MAIN");
  
  return;
  
}



void loop() { 
  //Serial.println("main loop");
    
    if (digitalRead(selectButtonPin)==LOW){
      if((millis()-lastSampleTime)>sampleDelay){
          float TempF;
            TempF = TempReturn();
            Serial.println("tempF:");
  Serial.println(TempF);
            if (TempF < 500) {             
            }
            if (TempF > highTemp) 
            {         
             digitalWrite(Relay, LOW);   //Turn off relay 
             lcd.clear();
             lcd.print("Temp:");
             lcd.setCursor(0, 1);
             lcd.print(TempF); //write current temp to LCD screen
             lcd.print((char)223); //degree symbol
             lcd.print("F");
             Serial.println("current temp:");
             Serial.println(TempF);
            }
            else if (TempF < lowTemp)
            { 
              if (TempF >-70) // don't let -99.9 return influence logic
            {             
             digitalWrite(Relay, HIGH);    //Turn on relay 
             lcd.clear();
             lcd.print("Temp:");             
             lcd.setCursor(0, 1);
             lcd.print(TempF);
             lcd.print((char)223);
             lcd.print("F");
             Serial.println("current temp:");
             Serial.println(TempF);
            }
            }
    
            else //neutral zone - still want temp to display
            {
             lcd.clear();
             lcd.print("Temp:");
             lcd.setCursor(0, 1);
             lcd.print(TempF);
             lcd.print((char)223);
             lcd.print("F");  
             Serial.println("current temp:");
             Serial.println(TempF);
            }
          lastSampleTime=millis();
      }
    }
    else if (digitalRead(selectButtonPin)==HIGH){
    tempChange();
    delay(250);
    }        
}
