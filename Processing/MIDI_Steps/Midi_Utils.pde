
MidiSystem midiSystem;
Sequencer midiSequencer;

String[] Notes = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

class Note
{
  float m_fTime;
  String m_sNote;
  boolean m_on;
}

class Song
{
  
  Song(String sName)
  {
    m_sName = sName;
  }
  
  String m_sName = "";
  ArrayList m_Notes = new ArrayList();
}



int g_iTicksPerBeat = 96;

ArrayList loadMidi(String fileName, float fBPM)
{
  
  String sketchPath = this.sketchPath;
  println(sketchPath);
  
  //File midiFile = new File(sketchPath + "/" + fileName);
  File midiFile = new File(fileName);
  println(midiFile.getAbsolutePath());
  
  if(!midiFile.exists() || midiFile.isDirectory() || !midiFile.canRead())
  {
    println("Error reading file");
  }
  
  Sequence midiSequence = null;
 
  try
  {
    midiSequence = midiSystem.getSequence(midiFile);
  }
  catch (Exception e)
  {
    e.printStackTrace();
  }
  
  if (midiSequence == null)
  {
    return null;
  }

  
  Track[] midiTracks = midiSequence.getTracks();  
  Track midiTrack = midiTracks[0];
  
  int iNumEvents = midiTrack.size();
  
  ArrayList myNotes = new ArrayList();
  ArrayList lines = new ArrayList();
  
  for (int i=0; i<iNumEvents; i++)
  {
    MidiEvent event = midiTrack.get(i);
    
    MidiMessage message = event.getMessage();
    
    if (message.getLength()-1 == 2) //message length - staus byte = 2 (short message)
    {
  
  int iMessage = (int)(message.getMessage()[0] & 0xFF);
  
  if (iMessage == ShortMessage.NOTE_ON)
  {    
    int iFullNote = (int)(message.getMessage()[1] & 0xFF);
    
    int iNote = iFullNote % 12;
    int iOctave = floor(iFullNote/12) - 2;
    
    float fBPS = fBPM / 60.0;
    float fSPB = 1.0 / fBPS;
    
    float fTime = event.getTick() * fSPB / g_iTicksPerBeat;
    
    String line_ = "Note On: " + Notes[iNote] + "-" + iOctave + " @" + fTime + "[" + event.getTick() + "]";
    //println(line_);
    lines.add(line_);
    
    Note n = new Note();
    n.m_sNote = Notes[iNote] + "-" + iOctave;
    n.m_fTime = fTime;
    n.m_on = true;
    myNotes.add(n);
    
    //384 ticks per bar?
  }
  
   if (iMessage == ShortMessage.NOTE_OFF)
  {    
    int iFullNote = (int)(message.getMessage()[1] & 0xFF);
    
    int iNote = iFullNote % 12;
    int iOctave = floor(iFullNote/12) - 2;
    
    float fBPS = fBPM / 60.0;
    float fSPB = 1.0 / fBPS;
    
    float fTime = event.getTick() * fSPB / g_iTicksPerBeat;
    String line_ = "Note Off: " + Notes[iNote] + "-" + iOctave + " @" + fTime + "[" + event.getTick() + "]";
    //println(line_);
    lines.add(line_);
    
    Note n = new Note();
    n.m_sNote =  Notes[iNote] + "-" + iOctave;
    n.m_fTime = fTime;
    n.m_on = false;
    myNotes.add(n);
    
    //384 ticks per bar?
  }
    }
  }
  
  String linesArray[] = new String[lines.size()];
  linesArray = (String[]) lines.toArray(linesArray);
  saveStrings("notes.txt", linesArray);
  return myNotes;
  
}
