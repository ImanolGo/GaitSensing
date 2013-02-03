
/**
  * TXT -> MIDI converter 
  * 
  * by Imanol Gomez 
  * 
  * 
*/

import arb.soundcipher.*;

String[] data; 
float f; 
PFont fontA;
float tempo = 60;
int spm = 60; //seconds per minute
int numNotes = 0;
SCScore score = new SCScore();

void setup() {
  size(1000, 200);
  background(100); 
  stroke(255); 
  frameRate(12);
  data = loadStrings("/Users/imanolgo/Desktop/DATA.TXT"); 
  score.tempo(tempo);	// public double (default noLoop();	
  noLoop(); // only draw once
}

void draw() { 
  
    fontA = loadFont("AmericanTypewriter-24.vlw"); 
    textFont(fontA); 
    text("TXT to MIDI", 15, 30);
    text("bpm:", 15, 90); 
    text(str((int)tempo), 75, 90);
    text("processing..", 15, 120);
    
    // It might be null the first time we run the program if there is no data.txt file!
    if (data != null) {
      int testNum = 1;
      double initTimeTest = 0.0;
      // Read all the lines from the file and convert to two integers (xy coordinate)
      println("there are " + data.length + " lines");
      for (int i = 0; i < data.length; i++) {        
         println(data[i]);
         String line_ = data[i];
         if (line_.charAt(0)=='-') //New test
         {
             continue;
         }
         else if (line_.charAt(0)=='*')
          {
              if(numNotes>0)
              {
                String fileName = "/Users/imanolgo/Desktop/output_" + str(testNum) + ".mid";
                score.writeMidiFile(fileName); 
                score.empty();
                text("Writing MIDI file ..", 15, 180);
                text(str(testNum) +  ".. ", 200 + testNum*35, 180);
                testNum = testNum +1;
                numNotes = 0;
              }
          }
          
          else
          {
             //String[] PID = split(data[i+1],",");
             String[] PID = split(line_,",");
             if(PID!=null)
             {
                double initTime = (Double.valueOf(PID[0].trim()).doubleValue() - initTimeTest)*tempo/(spm*1000);
                double pitch = Double.valueOf(PID[1].trim()).doubleValue();
                double velocity = Double.valueOf(PID[2].trim()).doubleValue();
                double durationTime = Double.valueOf(PID[3].trim()).doubleValue()/1000;
                score.addNote(initTime, pitch, velocity, durationTime);
                numNotes++;
             }
          }
          
        }
          text("OK", 150, 120);  
    }
    

}
