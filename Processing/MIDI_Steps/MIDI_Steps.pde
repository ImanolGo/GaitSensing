
/**
  * MIDI -> info converter 
  * 
  * by Imanol Gomez 
  * 
  * 
*/

import arb.soundcipher.*;

import javax.sound.midi.Sequencer;
import javax.sound.midi.MidiDevice;
import javax.sound.midi.MidiSystem;
import javax.sound.midi.MidiEvent;
import javax.sound.midi.MidiMessage;
import javax.sound.midi.MidiUnavailableException;
import javax.sound.midi.InvalidMidiDataException;
import java.io.IOException;
import javax.sound.midi.Sequence;
import javax.sound.midi.Track;
import javax.sound.midi.ShortMessage;

import javax.swing.*; 
import javax.swing.filechooser.FileNameExtensionFilter;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

ArrayList Notes_;
PFont fontA;
float tempo = 60;
int spm = 60; //seconds per minute
ArrayList m_Durations = new ArrayList();
ArrayList lines = new ArrayList();
String absolutePath;
String fileName;

//name of the file

class Step
{
  float m_fTime;
  String m_sNote;
  float m_fDuration;
}

void setup() {
  size(400, 400);
  //background(100); 
  stroke(255); 
  frameRate(12);
  Notes_ = new ArrayList();  // Create an empty ArrayList	
 
  // set system look and feel 
 
  try { 
    UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName()); 
  } catch (Exception e) { 
    e.printStackTrace();  
   
  } 
   
  // create a file chooser 
  final JFileChooser fc = new JFileChooser(); 
  FileNameExtensionFilter filter = new FileNameExtensionFilter("MIDI Files", "mid", "midi");
  fc.setFileFilter(filter);
   
  // in response to a button click: 
  int returnVal = fc.showOpenDialog(this); 
   
  if (returnVal == JFileChooser.APPROVE_OPTION) { 
    File file = fc.getSelectedFile(); 
    //println(file.getName());
    //println(file.getPath()); 
    String[] listName = split(file.getName(), ".");
    String[] pathList = split(file.getPath(), "/");
    fileName = listName[0];
    absolutePath= "";
    for(int i= 0;i< pathList.length-1;i++)
    {
       absolutePath += pathList[i] + "/";
    }
    
    println(absolutePath); 
    println(fileName);
    // see if it's an image 
    // (better to write a function and check for all supported extensions) 
    if (file.getName().endsWith("mid")) { 
      Notes_ = loadMidi(file.getPath() ,tempo);
    } else { 
      // just print the contents to the console 
      println("Not opened a midi file."); 
    } 
  } else { 
    println("Open command cancelled by user."); 
  }
  
  noLoop(); // only draw once
  
}

void draw() {    
    background(255);
  
    fontA = loadFont("AmericanTypewriter-24.vlw"); 
    textFont(fontA); 
    text("MIDI Steps", 15, 30);
    text("bpm:", 15, 90); 
    text(str((int)tempo), 75, 90);
    text("processing..", 15, 120);
    
    Map<String,Integer> mpNumber=new HashMap<String, Integer>();
    Map<String,Step> mpSteps=new HashMap<String, Step>();
    
    for(int i=0; i<Notes_.size();i++)
    {  
       Note note =(Note) Notes_.get(i);
        if(!mpNumber.containsKey(note.m_sNote)){
            mpNumber.put(note.m_sNote, 0);
        }
        else{
          if(note.m_on)
          {
            mpNumber.put(note.m_sNote,mpNumber.get(note.m_sNote)+1);
          }
        }
        
        if(!mpSteps.containsKey(note.m_sNote)){
           if(note.m_on){
              Step step = new Step();
              step.m_fTime = note.m_fTime;
              step.m_sNote = note.m_sNote;
              step.m_fDuration = 0.0;  
              mpSteps.put(note.m_sNote, step);
           }
            
        }
        else{
          if(note.m_on)
          {
             Step step = mpSteps.get(note.m_sNote);
             step.m_fTime = note.m_fTime;
             mpSteps.put(note.m_sNote,step);
          }
          else{
             Step step = mpSteps.get(note.m_sNote);
             step.m_fDuration += note.m_fTime - step.m_fTime;
             mpSteps.put(note.m_sNote,step);
          }
        }
        
    }
   
   lines.add("-- STATISTICS --\n");
   for (int i=0; i < mpNumber.size(); i++) {
      String key = (String)mpNumber.keySet().toArray()[i]; 
      Integer val = (Integer)mpNumber.values().toArray()[i];
      System.out.println("key,val: " + key + "," + val);
      Step step = mpSteps.get(key);
      float duration = step.m_fDuration/val;
      String line_ = "Step: "+ key + ": total number of steps-> " + val + ", mean duration-> " + duration + "s";
      lines.add(line_);
    }
    
   lines.add("\n");
   lines.add("-- DETAILED --");
   lines.add("\n");
   mpSteps.clear();
  
   for(int i=0; i<Notes_.size();i++)
    {  
       Note note =(Note) Notes_.get(i);
       if(!mpSteps.containsKey(note.m_sNote)){
           if(note.m_on){
              Step step = new Step();
              step.m_fTime = note.m_fTime;
              step.m_sNote = note.m_sNote;
              step.m_fDuration = 0.0;  
              mpSteps.put(note.m_sNote, step);
           }
            
        }
        else{
          if(note.m_on)
          {
             Step step = mpSteps.get(note.m_sNote);
             step.m_fTime = note.m_fTime;
             mpSteps.put(note.m_sNote,step);
          }
          else{
             Step step = mpSteps.get(note.m_sNote);
             step.m_fDuration = note.m_fTime - step.m_fTime;
             mpSteps.put(note.m_sNote,step);
             String line_ = "Step "+ step.m_sNote + ": @" + step.m_fTime + ", " + step.m_fDuration + "s";
            //println(line_);
            lines.add(line_);
          }
        }
    }

   String linesArray[] = new String[lines.size()];
   linesArray = (String[]) lines.toArray(linesArray); 
   String filePath = absolutePath + fileName + ".txt";
   println("Saved data at: " + filePath);
   saveStrings(filePath, linesArray);
   System.out.println("HashMap - " + mpNumber);
   
   text("OK", 150, 120); 
}
