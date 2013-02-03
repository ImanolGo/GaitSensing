 
#include <SD.h>
#include <avr/io.h> 
#include <avr/wdt.h>

#include <Bounce.h>

// On the kathrin board, CS is pin 2. Note that even if it's not
 // used as the CS pin, the hardware CS pin (10 on most Arduino boards,
 // 53 on the Mega) must be left as an output or the SD library
 // functions will not work
const int chipSelect = 2;

const int r1 = A0; //right foot heel
const int r2 = A1;
const int r3 = A2;
const int l1 = A3; //left foot heel
const int l2 = A4;
const int l3 = A5;

const int tonePin =  9;      // the number of the Tone pin
const int ledPin =  8;      // the number of the LED pin

const int s2 = 7;     // Up
const int s3 = 5;     // Down
const int s4 = 3;     // Left / Right foot
const int s5 = 6;     // Record / Calibrate
const int s6 = 4;     // On / Off

// Instantiate a Bounce object with a 5 millisecond debounce time
int durationTime = 1000;
Bounce bouncer_s5 = Bounce(s5,50); 

int lastState_s3 = 0;     // previous state of the button
int lastState_s2 = 0;     // previous state of the button
int lastState_s4 = 0;     // previous state of the button
int lastState_s5 = 0;     // previous state of the button
int lastState_s6 = 0;     // previous state of the button

float l1Note = 440;
float r1Note = 659.26;

int l1MIDI = 69;
int r1MIDI = 76;

long start_l1,start_r1;         //the starting time of each step

int buttonState = 0;         // variable for reading the pushbutton status
int noteOffset = 0;
float freqOffset = 0;

// Variables will change:
int ledState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated

// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 500;           // interval at which to blink (milliseconds)

boolean pl1, pl2,pl3,pr1,pr2,pr3; //play booleans
int foot = 2; // 0 = left, 1 = right, 2 = both

boolean recMode; //defines if it's running in recording mode or in calibration mode
long startRecTime;  //defines the time when the recording starts

const int minThreshold =  400;   // the minimum threshold to be reached
const int maxThreshold =  950;  // the maximum threshold to be reached
const int thresholdStep =  25;   // the threslhold step
int hysteresisFactor = thresholdStep*2;// hysteresis factor


//e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
int noteDuration = 1000/16;
// thresholds
int thl1, thl2,thl3,thr1,thr2,thr3;


char dataFileName[13] = "data.txt"; 
char settingsFileName[13] = "settings.txt"; 
char textBuffer[81]; 
byte res;

int sensorValue = 0;  // variable to store the value coming from the sensor
File dataFile; //data file
File settingsFile; //settings file

void readSettings()
{
      settingsFile = SD.open(settingsFileName, FILE_WRITE);        
    // if the file opened okay, write to it:
    if (settingsFile) {
      Serial.println("Settings file open");
      // read from the file until there's nothing else in it:
      int threshold_foot = 0; //if threshold_foot== 0 -> left, if threshold_foot== 1 -> Right
      char number [256] = {"100"};
      int n = 0;
      while (settingsFile.available()) 
      {
          char c = settingsFile.read();
          Serial.print(c);
          if(c=='\n')
          {
              int thr = atoi(number);
              n = 0;
              if(threshold_foot==0)
              {
                  thl1 = thr;
              }
              else
              {
                  thr1 = thr;
              }
              threshold_foot = (threshold_foot+1)%2;
          }
          
    	  else
          {
              number[n] = c;
              n++;
          }
      }
      
      Serial.print("Threshold l1 -> ");
      Serial.print(thl1);
      Serial.print('\n');
      Serial.print("Threshold r1 -> ");
      Serial.print(thr1);
      Serial.print('\n');
    } else
    {
      // if the file didn't open, print an error:
      Serial.println("error opening settings.txt");
    }
}

void setup() {
  
  pl1 =pl2=pl3=pr1=pr2=pr3 = true;
  thl1=thl2=thl3=thr1=thr2=thr3=750;
  
  pinMode(A5, INPUT); // sets the analog pins 0..5 as inputs
  pinMode(A4, INPUT); // use A5..A0 LSB..MSB
  pinMode(A3, INPUT); 
  pinMode(A2, INPUT);
  pinMode(A1, INPUT); 
  pinMode(A0, INPUT);
  
  digitalWrite(A5, HIGH);  // turn on pullup resistors as voltage dividers
  digitalWrite(A4, HIGH); 
  digitalWrite(A3, HIGH); 
  digitalWrite(A2, HIGH); 
  digitalWrite(A1, HIGH); 
  digitalWrite(A0, HIGH);
  
  pinMode(s4, INPUT); // sets the digital pins 3..5 as inputs for buttons
  pinMode(s3, INPUT); 
  pinMode(s2, INPUT);
  pinMode(s5, INPUT);
  pinMode(s6, INPUT);
  
  digitalWrite(s4, HIGH); // turn on pullup resistors 
  digitalWrite(s3, HIGH);
  digitalWrite(s2, HIGH);
  digitalWrite(s5, HIGH);
  digitalWrite(s6, HIGH);
  
  pinMode(tonePin, OUTPUT); //SPEAKER
  pinMode(ledPin, OUTPUT); //REC LED
  recMode = false;
  
  Serial.begin(9600); // open the serial port for debugging
  Serial.println("Serial OK");
  
  if (!SD.begin(chipSelect)) {
    Serial.println("SD initialization failed!");
    return;
  }
  Serial.println("SD initialization done.");
  digitalWrite(ledPin, HIGH);
  
  startRecTime = millis();
  
  readSettings();
}

void loop(){
  
  bouncer_s5.update();
  
  if(recMode||dataFile)
  {
      unsigned long currentMillis = millis();
      if(currentMillis - previousMillis > interval) {
        // save the last time you blinked the LED 
        previousMillis = currentMillis;   
    
        // if the LED is off turn it on and vice-versa:
        if (ledState == LOW)
          ledState = HIGH;
        else
          ledState = LOW;
    
        // set the LED with the ledState of the variable:
        digitalWrite(ledPin, ledState);
      }  
      
      // read the value from the sensor l1:
      sensorValue = analogRead(l1);
      if ( pl1 && (sensorValue < thl1)){
            pl1 = false;
            thl1 = thl1 + hysteresisFactor; //We add Hysteresis
            start_l1 = millis() - startRecTime;     
       } 
       if (!pl1 && (sensorValue >= thl1)) {
            pl1 = true;  
            thl1 = thl1 - hysteresisFactor; //We remove Hysteresis
            String code = String(String(start_l1)+","+String(l1MIDI)+","+String(100)+","+String(millis()-start_l1-startRecTime));
            dataFile.println(code);
            Serial.println(code);
        }
      
     // read the value from the sensor r1:
    sensorValue = analogRead(r1);
    if ( pr1 && (sensorValue < thr1)){
          pr1 = false;
          thr1 = thr1 + hysteresisFactor; //We add Hysteresis
          start_r1 = millis()-startRecTime;     
     } 
     if (!pr1 && (sensorValue >= thr1)) {
          pr1 = true;  
          thr1 = thr1 - hysteresisFactor; //We remove Hysteresis
          String code = String(String(start_r1)+","+String(r1MIDI)+","+String(100)+","+String(millis()-start_r1-startRecTime));
          dataFile.println(code);
          Serial.println(code);
      }
      
      //buttonState = digitalRead(s5); 
      buttonState = lastState_s5;
      if(bouncer_s5.duration() > durationTime)
      {
          buttonState = bouncer_s5.read();
      } 
   
      if (buttonState != lastState_s5 && buttonState == LOW) //if s5 pressed and hold and low, change to calibration mode
       //if (buttonState != lastState_s5 && buttonState == LOW) //if s5 pressed and hold and low, change to calibration mode
      {
         Serial.println("s5: Calibration");
          // if the file opened okay, write to it:
            if (dataFile)
            {
              int freqNote = 3520;
              int MIDINote = 105;
              int delayMs = 100; 
              for(int i=0; i<3; i++)
              {
                long startBip = millis()-startRecTime;
                tone(tonePin,freqNote); 
                delay(delayMs); 
                String code = String(String(startBip)+","+String(MIDINote)+","+String(100)+","+String(millis()-startBip-startRecTime)); 
                dataFile.println(code);
                noTone(tonePin);
                delay(delayMs/2);
              }
              // close the file:
              dataFile.println("** End test **");
              dataFile.close();
              Serial.println("** End test **");
            } 
            else
            {
              // if the file didn't open, print an error:
              Serial.println("data.txt not closed");
            }
            digitalWrite(ledPin, HIGH);
            recMode = false; 
      }   
      
      lastState_s5 = buttonState;
  }
  
  else
  {
       // read the value from the sensor l1:
      sensorValue = analogRead(r1);
      if ( foot!=0 && pr1 && (sensorValue < thr1))
      {
          pr1 = false;
          tone(tonePin,r1Note,noteDuration);
          thr1 = thr1 + hysteresisFactor; //We add Hysteresis
          delay(noteDuration);     
      }
      if ( foot!=0 && !pr1 && (sensorValue >= thr1))
      {
          pr1 = true;  
          thr1 = thr1 - hysteresisFactor;
      }
      
      // read the value from the sensor l1:
      sensorValue = analogRead(l1);
      if ( foot!=1 && pl1 && (sensorValue < thl1))
      {
          pl1 = false;
          tone(tonePin,l1Note,noteDuration);
          thl1 = thl1 + hysteresisFactor; //We add Hysteresis
          delay(noteDuration);    
      }
      if ( foot!=1 && !pl1 && (sensorValue >= thl1))
      {
          pl1 = true;  
          thl1 = thl1 - hysteresisFactor;
      }
      
      buttonState = digitalRead(s6);
      if (buttonState != lastState_s6&& buttonState == LOW) //if s6 pressed, turn off the device
      {
            Serial.println("s6: Off");
            if (dataFile)
            {
              // close the file:
              dataFile.println("** End test **");
              dataFile.close();
              Serial.println("** End test **");
            } 
            
            wdt_enable(WDTO_1S); 
      }
       
       lastState_s6 = buttonState;
       
       buttonState = digitalRead(s4);
      if (buttonState != lastState_s4&& buttonState == LOW) //if s4 pressed, change the foot to be calibrated
      {
            Serial.println("s4: which foot?");
            Serial.print("foot = ");
            foot = (foot+1)%3;
            Serial.print(foot);
            Serial.print("\n");
            tone(tonePin,(foot+1)*500); 
            delay(100); 
            noTone(tonePin);
      }
       
       lastState_s4 = buttonState;
       
        buttonState = digitalRead(s3);
      // compare the buttonState to its previous state
      if (buttonState != lastState_s3 && buttonState == LOW) //if s3 pressed, threshold down
      {
          Serial.println("s3: down");
          switch (foot)
           {
              case 0:
                if (thl1>minThreshold) 
                {   
                    thl1 = thl1-thresholdStep;
                    l1MIDI = l1MIDI-1;
                    l1Note = l1Note/1.059;  //pow(2,1/12) = 1.059
                } 
                
                Serial.print("threshold l1 -> ");
                Serial.print(thl1);
                Serial.print("\n"); 
                tone(tonePin,l1Note); 
                delay(100); 
                noTone(tonePin);  
                break;
              case 1:
                if (thr1>minThreshold) 
                {     
                    thr1 = thr1 - thresholdStep;
                    r1MIDI = r1MIDI-1;
                    r1Note = r1Note/1.059;  //pow(2,1/12) = 1.059
                } 
                
                Serial.print("threshold r1 -> ");
                Serial.print(thr1);
                Serial.print("\n"); 
                tone(tonePin,r1Note); 
                delay(100); 
                noTone(tonePin);      
                break;
              default: 
                if (thl1>minThreshold) 
                {   
                    thl1 = thl1-thresholdStep;
                    l1MIDI = l1MIDI-1;
                    l1Note = l1Note/1.059;  //pow(2,1/12) = 1.059
                } 
                
                if (thr1>minThreshold) 
                {     
                    thr1 = thr1 - thresholdStep;
                    r1MIDI = r1MIDI-1;
                    r1Note = r1Note/1.059;  //pow(2,1/12) = 1.059
                } 
                
                Serial.print("threshold l1 -> ");
                Serial.print(thl1);
                Serial.print("\n");
                Serial.print("threshold r1 -> ");
                Serial.print(thr1);
                Serial.print("\n");
                tone(tonePin,l1Note); 
                delay(50); 
                noTone(tonePin);  
                tone(tonePin,r1Note); 
                delay(50); 
                noTone(tonePin);  
           } 
      }
      
       lastState_s3 = buttonState;
       
       buttonState = digitalRead(s2);
      // compare the buttonState to its previous state
      if (buttonState != lastState_s2&& buttonState == LOW) //if s2 pressed, threshold up
      { 
          Serial.println("s2: up");
           switch (foot)
           {
              case 0:
                if (thl1<maxThreshold) 
                {     
                    thl1 = thl1+ thresholdStep;
                    l1MIDI = l1MIDI+1;
                    l1Note = l1Note*1.059;  //pow(2,1/12) = 1.059
                } 
                Serial.print("threshold l1 -> ");
                Serial.print(thl1);
                Serial.print("\n");
                tone(tonePin,l1Note); 
                delay(50); 
                noTone(tonePin); 
                break;
              case 1:
                if (thr1<maxThreshold) 
                {     
                    thr1 = thr1 + thresholdStep;
                    r1MIDI = r1MIDI+1;
                    r1Note = r1Note*1.059;  //pow(2,1/12) = 1.059
                } 
                Serial.print("threshold r1 -> ");
                Serial.print(thr1);
                Serial.print("\n");
                tone(tonePin,r1Note); 
                delay(50); 
                noTone(tonePin); 
                break;
              default: 
                if (thl1<maxThreshold) 
                {
                    thl1 = thl1 + thresholdStep;
                    l1MIDI = l1MIDI+1;
                    l1Note = l1Note*1.059;  //pow(2,1/12) = 1.059
                } 
                
                if (thr1<maxThreshold) 
                {
                    thr1 = thr1 + thresholdStep;
                    r1MIDI = r1MIDI+1;
                    r1Note = r1Note*1.059;  //pow(2,1/12) = 1.059
                }     
                
                Serial.print("threshold l1 -> ");
                Serial.print(thl1);
                Serial.print("\n"); 
                Serial.print("threshold r1 -> ");
                Serial.print(thr1);
                Serial.print("\n");
                tone(tonePin,l1Note); 
                delay(50); 
                noTone(tonePin);  
                tone(tonePin,r1Note); 
                delay(50); 
                noTone(tonePin);  
            }      
     }
      
     lastState_s2 = buttonState;
       
       //buttonState = digitalRead(s5);
       buttonState = lastState_s5;
      if(bouncer_s5.duration() > durationTime)
      {
          buttonState = bouncer_s5.read();
      } 
      // compare the buttonState to its previous state
      if (buttonState != lastState_s5 && buttonState == LOW) { 
            Serial.println("s5: REC");
            // open the file. note that only one file can be open at a time,
            // so you have to close this one before opening another.
            dataFile = SD.open(dataFileName, FILE_WRITE);
            
            // if the file opened okay, write to it:
            if (dataFile) {
              Serial.println("-- New test --");
              dataFile.println("-- New test --");
          	// close the file:
            } else {
              // if the file didn't open, print an error:
              Serial.println("error opening data.txt");
            }
            
            startRecTime = millis();
            int freqNote = 3520;
            int MIDINote = 105;
            int delayMs = 500;
            Serial.println("Start REC"); 
            noTone(tonePin);
            String code = String(String(0)+","+String(MIDINote)+","+String(100)+","+String(delayMs));
            dataFile.println(code);
            tone(tonePin,freqNote); 
            delay(delayMs);
            noTone(tonePin); 
            recMode = true;   
      }
     
     lastState_s5 = buttonState;
     
  } 
         
}
