/********************************************************
  Sipper counter v3.1b
  Modifications by Eric COSTE based on the original design
  April 2020
********************************************************/

void readAlarm( void )
{
short idx = 0;
  Alarm[idx] = EEPROM.read(idx);  
  idx++;
  Alarm[idx] = EEPROM.read(idx);  
  idx++;
  Alarm[idx] = EEPROM.read(idx);  
  idx++;
  Alarm[idx] = EEPROM.read(idx);  
  idx++;
  Alarm[idx] = EEPROM.read(idx);  
  idx++;
  
  if( Alarm[0] != 0x1F) // check signature
  { 
     Alarm[0] = 0;
     Alarm[1] = 0;
     Alarm[2] = 0;
     Alarm[3] = 0;
     Alarm[4] = 0;
  }
}

void writeAlarm( void )
{
short idx = 0;
  EEPROM.write(idx, 0x1F);  
  idx++;
  EEPROM.write(idx, Alarm[idx]);  
  idx++;
  EEPROM.write(idx, Alarm[idx]);  
  idx++;
  EEPROM.write(idx, Alarm[idx]);  
  idx++;
  EEPROM.write(idx, Alarm[idx]);  
  idx++;
  
}


void readTimer( void )
{
short idx = 0;
  Timer[idx] = EEPROM.read(idx+5);  
  idx++;
  Timer[idx] = EEPROM.read(idx+5);  
  idx++;
  Timer[idx] = EEPROM.read(idx+5);  
  idx++;
  Timer[idx] = EEPROM.read(idx+5);  
  idx++;
  Timer[idx] = EEPROM.read(idx+5);  
  idx++;
  
  if( Timer[0] != 0x1F) // check signature
  { 
     Timer[0] = 0;
     Timer[1] = 0;
     Timer[2] = 0;
     Timer[3] = 0;
     Timer[4] = 0;
  }
}

void writeTimer( void )
{
short idx = 0;
  EEPROM.write(idx, 0x1F);  
  idx++;
  EEPROM.write(idx, Timer[idx+5]);  
  idx++;
  EEPROM.write(idx, Timer[idx+5]);  
  idx++;
  EEPROM.write(idx, Timer[idx+5]);  
  idx++;
  EEPROM.write(idx, Timer[idx+5]);  
  idx++;
  
}


char IsButtonPressed(int button)
{
  return( 1-digitalRead(button) );
}


void WaitPressedKey( void )
{
  while( (IsButtonPressed(buttonA) == 0) && (IsButtonPressed(buttonB) == 0) && (IsButtonPressed(buttonC) == 0)&& (IsButtonPressed(buttonD) == 0)&& (IsButtonPressed(buttonE) == 0));
}

void WaitReleasedKey( void )
{
  while( (IsButtonPressed(buttonA) != 0) || (IsButtonPressed(buttonB) != 0) || (IsButtonPressed(buttonC) != 0)|| (IsButtonPressed(buttonD) != 0)|| (IsButtonPressed(buttonE) != 0));
}


int ReturnKeyPressed( void )
{
  if (IsButtonPressed(buttonA) )
    return(1);
  if (IsButtonPressed(buttonB) )
    return(2);
  if (IsButtonPressed(buttonC) )
    return(3);
  if (IsButtonPressed(buttonD) )
    return(4);
  if (IsButtonPressed(buttonE) )
    return(5);
  return(0);
}


void SetClockVars() {
 int x, h, m, s;
  //Display all current data
  current = rtc.now();
  h = current.hour();
  m = current.minute();
  s = current.second();
  
  
  SetSequence = 4;


//*************************************    
//Adjust hours
//*************************************    
  while (SetSequence == 4) 
  {
    WaitPressedKey();
    if (IsButtonPressed(buttonA)) {
      delay (150);
      h++;
      if(h>23) h=0;
    }
    if (IsButtonPressed(buttonC)) {
      delay (150);
      h--;
      if(h<0) h=23;
    }
    if (IsButtonPressed(buttonB)) {
      do // wait for button release
      {
        delay(100);  
      }
      while(IsButtonPressed(buttonB));
      SetSequence++;
    }
    display_time(h, m, s);
  }


//*************************************    
//Adjust minutes
//*************************************    
  while (SetSequence == 5) {

    WaitPressedKey();
    if (IsButtonPressed(buttonA)) {
      delay (150);
      m ++;
      if(m > 59) m=0;
    }
    if (IsButtonPressed(buttonC)) {
      delay (150);
      m --;
      if(m < 0) m=59;
    }
    if (IsButtonPressed(buttonB)) {
      do // wait for button release
      {
        delay(100);  
      }
      while(IsButtonPressed(buttonB));
      SetSequence++;
    }
    display_time(h, m, s);
    
  }

//*************************************    
//Adjust seconds
//*************************************    
#ifndef DISP_MODE_HH_MM
  while (SetSequence == 6) {

    WaitPressedKey();
    if (IsButtonPressed(buttonA)) {
      delay (150);
      s ++;
      if(s > 59) s=0;
    }
    if (IsButtonPressed(buttonC)) {
      delay (150);
      s --;
      if(s < 0) s=59;
    }
    if (IsButtonPressed(buttonB)) {
      do // wait for button release
      {
        delay(100);  
      }
      while(IsButtonPressed(buttonB));
      SetSequence++;
    }
    display_time(h, m, s);
    
  }
#endif
//*************************************    
  rtc.adjust(DateTime(2020, 1, 1, h, m, s));
//*************************************    

  h = Alarm[1];
  m = Alarm[2];
  s = Alarm[3];
  display_time(h, m, s);
//*************************************    
//Adjust ALARM hours
//*************************************    
  while (SetSequence == 7) 
  {
    WaitPressedKey();
    if (IsButtonPressed(buttonA)) {
      delay (150);
      h++;
      if(h>23) h=0;
    }
    if (IsButtonPressed(buttonC)) {
      delay (150);
      h--;
      if(h<0) h=23;
    }
    if (IsButtonPressed(buttonB)) {
      do // wait for button release
      {
        delay(100);  
      }
      while(IsButtonPressed(buttonB));
      SetSequence++;
    }
    display_time(h, m, s);
  }


//*************************************    
//Adjust ALARM minutes
//*************************************    
  while (SetSequence == 8) {

    WaitPressedKey();
    if (IsButtonPressed(buttonA)) {
      delay (150);
      m ++;
      if(m > 59) m=0;
    }
    if (IsButtonPressed(buttonC)) {
      delay (150);
      m --;
      if(m < 0) m=59;
    }
    if (IsButtonPressed(buttonB)) {
      do // wait for button release
      {
        delay(100);  
      }
      while(IsButtonPressed(buttonB));
      SetSequence++;
    }
    display_time(h, m, s);
    
  }

//*************************************    
//Adjust ALARM seconds
//*************************************    
#ifndef DISP_MODE_HH_MM
  while (SetSequence == 9) {

    WaitPressedKey();
    if (IsButtonPressed(buttonA)) {
      delay (150);
      s ++;
      if(s > 59) s=0;
    }
    if (IsButtonPressed(buttonC)) {
      delay (150);
      s --;
      if(s < 0) s=59;
    }
    if (IsButtonPressed(buttonB)) {
      do // wait for button release
      {
        delay(100);  
      }
      while(IsButtonPressed(buttonB));
      SetSequence++;
    }
    display_time(h, m, s);
    
  }
#endif
  Alarm[1] = h;
  Alarm[2] = m;
  Alarm[3] = s;
  
  writeAlarm();


}

