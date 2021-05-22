#include <SPI.h>
#include <Wire.h> 
#include <EEPROM.h>
#include "RTClib.h"
#include <Adafruit_NeoPixel.h>
#include <TimerOne.h>

 #define _DEBUG_

//#define DISP_MODE_HH_MM     // HH MM, else HH MM SS

int ColorDiv=1;
int HideSeconds = 0;

#ifdef DISP_MODE_HH_MM
  #define NUMPIXELS 80
  #define LED_PIN 2
  #define MAX_COLOR_INDEX  9
  #define STATUS_LED  -1
  #define NB_DIGIT  4
#else
  #define NUMPIXELS 121 
  #define LED_PIN 2
  #define MAX_COLOR_INDEX  9
  #define STATUS_LED  120
  #define NB_DIGIT  6
#endif

// first model wired connection
#define COLOR_CHRONO 99
/*
#define buttonA   5     
#define buttonB   3     
#define buttonC   4     
*/

// second model : base on SIPPER V3.0 PCB
/*
#define buttonA   A1     
#define buttonB   A0     
#define buttonC   A2     
*/

// third model : decicated PCB control board

#define buttonA   3     
#define buttonB   4     
#define buttonC   5     
#define buttonD   6     
#define buttonE   7     

#define BUZZER    8     




// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_RGB + NEO_KHZ800);

uint32_t _counter_mode_step;


DateTime current;  
RTC_DS1307 rtc;
short index = 0;
int color_index = 7;
uint32_t RGB[6];
uint32_t FOCUS_CLOCK;
uint32_t FOCUS_ALARM;
uint32_t UNFOCUS;

uint32_t STATUS_NONE;
uint32_t STATUS_ALARM;
uint32_t STATUS_TIMER;
uint32_t STATUS_CHRONO;
uint32_t STATUS_SETUP;

int _update = 0;
int SetupMode = 0;
int SetSequence = 4;
int _12HMode = 0;
uint8_t Alarm[5];  // 0x1f : flag ; Hours ; Min ; Second ; 0x01 Alarm Enable - 0x80 Alarm Triggered / BEEP ON - 0x40 Alamr manually stopped
uint8_t Timer[5];  // 0x1f : flag ; Hours ; Min ; Second ; 0x80 End of timer Triggered / BEEP ON
int Blink = 0;
int AlarmSecCounter;
unsigned long CHRONO_START, CHRONO_COUNTER;
int CHRONO_COUNTING = 0;


#define RUNNING_CLOCK    0
#define RUNNING_TIMER    1
#define RUNNING_CHRONO   2
#define RUNNING_SETUP    3

int current_running_mode = RUNNING_CLOCK;


void TimerCallBback() 
{

  _update = 1;
  Blink ++;
  if(Blink > 9)
    Blink = 0;
}



uint32_t color_wheel(uint8_t pos, int div) 
{
uint32_t r, g, b;  
  pos = 255 - pos;
  
  if(pos < 85) 
  {
    r = (uint32_t)(255 - pos * 3); 
    g = (uint32_t)(0);
    b = (uint32_t)(pos * 3);
  } 
  else if(pos < 170) 
    {
      pos -= 85;
      r = (uint32_t)(0);
      g = (uint32_t)(pos * 3);
      b = (uint32_t)(255 - pos * 3);
    } 
    else 
    {
      pos -= 170;
      r = (uint32_t)(pos * 3);
      g = (uint32_t)(255 - pos * 3);
      b = (0);
    }
    /*
  if(pos < 85) 
  {
    return ((uint32_t)(255 - pos * 3) << 16) | ((uint32_t)(0) << 8) | (pos * 3);
  } 
  else if(pos < 170) 
    {
      pos -= 85;
      return ((uint32_t)(0) << 16) | ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
    } 
    else 
    {
      pos -= 170;
      return ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8) | (0);
    }
*/
  r = r/div;
  g = g/div;
  b = b/div;
  return( (r<<16) | (g <<8) | b );
}

void compute_colors( int index_col, int div )
{
int i, r, g, b;
  if( index_col <= 7 )
  {
    for(i=0;i<6;i++)
    {
      if( index_col & 0x01 )
        r = 255/div;
      else
        r = 0;
  
      if( index_col & 0x02 )
        g = 255/div;
      else
        g = 0;
  
      if( index_col & 0x04 )
        b = 255/div;
      else
        b = 0;

      RGB[i] = pixels.Color(r, g, b);
        
    }
  }
  else
  {
    if( index_col == 8 )
    {
      uint32_t color = color_wheel(_counter_mode_step, div);

      for(i=0;i<6;i++)
      {
        RGB[i] = color;
      }
      _counter_mode_step = (_counter_mode_step + 2) % 256;
    }

    if( index_col == 9 )
    {
      uint32_t color;
      int localcolorstep;
      localcolorstep = _counter_mode_step;
      
      for(i=0;i<6;i++)
      {
        color = color_wheel(localcolorstep, div );
        RGB[i] = color;
        localcolorstep = (localcolorstep + 24) % 256;
      }
      _counter_mode_step = (_counter_mode_step + 2) % 256;
    }

    if( index_col == COLOR_CHRONO )
    {
      /*
      for(i=0;i<6;i++)
      {
        RGB[i] = pixels.Color(64, 255, 192);
      }
      */
      #ifdef DISP_MODE_HH_MM
        RGB[0] = pixels.Color(0, 255/div, 0);
        RGB[1] = pixels.Color(255/div, 0, 0);
        RGB[2] = pixels.Color(255/div, 0, 0);
        RGB[3] = pixels.Color(0, 0, 255/div);
        
        RGB[4] = pixels.Color(0, 0, 255/div);
        RGB[5] = pixels.Color(0, 0, 255/div);
      #else
        RGB[0] = pixels.Color(0, 255/div, 0);
        RGB[1] = pixels.Color(0, 255/div, 0);
        RGB[2] = pixels.Color(255/div, 0, 0);
        RGB[3] = pixels.Color(255/div, 0, 0);
        RGB[4] = pixels.Color(0, 0, 255/div);
        RGB[5] = pixels.Color(0, 0, 255/div);
       #endif
        
    }
    
  }
  
}


void display_digit( int val, int pos, int conf = -1 )
{
int i, j, idx;

  idx = 10*pos + val;

  i = idx /2;
  j = idx - 2*i;

  if(SetupMode == 0)
  {
    pixels.setPixelColor((4*i) + j, RGB[pos]);
    pixels.setPixelColor((4*i) + j + 2, RGB[pos]);
  }
  else
  {
      switch(SetSequence)
      {
        case 0 :
          pixels.setPixelColor((4*i) + j, FOCUS_ALARM);
          pixels.setPixelColor((4*i) + j + 2, FOCUS_ALARM);
          break;
        case 4 :
          if( (pos == 0) || (pos == 1) )
          {
            pixels.setPixelColor((4*i) + j, FOCUS_CLOCK);
            pixels.setPixelColor((4*i) + j + 2, FOCUS_CLOCK);
          }
          else
          {
            pixels.setPixelColor((4*i) + j, UNFOCUS);
            pixels.setPixelColor((4*i) + j + 2, UNFOCUS);
          }
          break;
        case 5 :
          if( (pos == 2) || (pos == 3) )
          {
            pixels.setPixelColor((4*i) + j, FOCUS_CLOCK);
            pixels.setPixelColor((4*i) + j + 2, FOCUS_CLOCK);
          }
          else
          {
            pixels.setPixelColor((4*i) + j, UNFOCUS);
            pixels.setPixelColor((4*i) + j + 2, UNFOCUS);
          }
          break;
        case 6 :
          if( (pos == 4) || (pos == 5) )
          {
            pixels.setPixelColor((4*i) + j, FOCUS_CLOCK);
            pixels.setPixelColor((4*i) + j + 2, FOCUS_CLOCK);
          }
          else
          {
            pixels.setPixelColor((4*i) + j, UNFOCUS);
            pixels.setPixelColor((4*i) + j + 2, UNFOCUS);
          }
          break;

          case 7 :
          if( (pos == 0) || (pos == 1) )
          {
            pixels.setPixelColor((4*i) + j, FOCUS_ALARM);
            pixels.setPixelColor((4*i) + j + 2, FOCUS_ALARM);
          }
          else
          {
            pixels.setPixelColor((4*i) + j, UNFOCUS);
            pixels.setPixelColor((4*i) + j + 2, UNFOCUS);
          }
          break;
        case 8 :
          if( (pos == 2) || (pos == 3) )
          {
            pixels.setPixelColor((4*i) + j, FOCUS_ALARM);
            pixels.setPixelColor((4*i) + j + 2, FOCUS_ALARM);
          }
          else
          {
            pixels.setPixelColor((4*i) + j, UNFOCUS);
            pixels.setPixelColor((4*i) + j + 2, UNFOCUS);
          }
          break;
        case 9 :
          if( (pos == 4) || (pos == 5) )
          {
            pixels.setPixelColor((4*i) + j, FOCUS_ALARM);
            pixels.setPixelColor((4*i) + j + 2, FOCUS_ALARM);
          }
          else
          {
            pixels.setPixelColor((4*i) + j, UNFOCUS);
            pixels.setPixelColor((4*i) + j + 2, UNFOCUS);
          }
          break;
        
      }
  }
 

}

void display_time( int h, int m, int s )
{
int d1, d2, d3, d4, d5, d6;

  pixels.clear(); // Set all pixel colors to 'off'

  d1 = h/10;
  d2 = h - 10*d1;

  display_digit(d1, 0);
  display_digit(d2, 1);
  
  d3 = m/10;
  d4 = m - 10*d3;

  display_digit(d3, 2);
  display_digit(d4, 3);

#ifndef DISP_MODE_HH_MM
  if(s>=0)
  {
    d5 = s/10;
    d6 = s - 10*d5;
  
    display_digit(d5, 4);
    display_digit(d6, 5);
  }
#endif

  updateStatusLed();
  pixels.show();

}



void update_display( void )
{
int h, m, s;  
  current = rtc.now();
  h = current.hour();
  if( _12HMode )
  {
    if( h>12 )
    {
      h = h - 12;
    }
  }
  
  m = current.minute();
  s = current.second();
  if(HideSeconds)
    s = -1;
  compute_colors(color_index, ColorDiv);   

  display_time( h, m, s );
  
}

/*
void display_chronos( void )
{
int d1, d2, d3, d4, d5, d6;

  pixels.clear(); // Set all pixel colors to 'off'

  d1 = h/10;
  d2 = h - 10*d1;

  display_digit(d1, 0);
  display_digit(d2, 1);
  
  d3 = m/10;
  d4 = m - 10*d3;

  display_digit(d3, 2);
  display_digit(d4, 3);

#ifndef DISP_MODE_HH_MM
  d5 = s/10;
  d6 = s - 10*d5;

  display_digit(d5, 4);
  display_digit(d6, 5);
#endif
  
  pixels.show();

}
*/

void update_chronos( long chrono )
{
int d1, d2, d3, d4, d5, d6;
int d, s, m;  

  compute_colors(COLOR_CHRONO, ColorDiv);   

  chrono = chrono / 10;
  d = chrono - ( 100 * floor(chrono/100) );

  chrono = chrono / 100;
  s = chrono - ( 60 * floor(chrono/60) );

  chrono = chrono / 60;
  m = chrono - ( 60 * floor(chrono/60) );

  pixels.clear(); // Set all pixel colors to 'off'

  d1 = m/10;
  d2 = m - 10*d1;
  display_digit(d1, 0);
  display_digit(d2, 1);

  d3 = s/10;
  d4 = s - 10*d3;
  display_digit(d3, 2);
  display_digit(d4, 3);

  d5 = d/10;
  d6 = d - 10*d5;
  display_digit(d5, 4);
  display_digit(d6, 5);

  updateStatusLed();
  pixels.show();
 
}


void updateStatusLed( void )
{
  if( STATUS_LED > 0 )
  {
      if( current_running_mode == RUNNING_CLOCK  )
      {
          if( Alarm[4] & 0x01 )
          {
            pixels.setPixelColor(STATUS_LED, STATUS_ALARM);
          }
          else
          {
            pixels.setPixelColor(STATUS_LED, STATUS_NONE);
           
          }
        
      }
      if( current_running_mode == RUNNING_CHRONO  )
      {
          pixels.setPixelColor(STATUS_LED, STATUS_CHRONO);
      }
      if( current_running_mode == RUNNING_TIMER  )
      {
          pixels.setPixelColor(STATUS_LED, STATUS_TIMER);
      }
      if( current_running_mode == RUNNING_SETUP  )
      {
          pixels.setPixelColor(STATUS_LED, STATUS_SETUP);
      }

    
  }
  
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

void setup() {
int x, h, m, s, SetSequence;
   
  Serial.begin(9600);
  Serial.println("Starting");

  pinMode(buttonA, INPUT_PULLUP);
  pinMode(buttonB, INPUT_PULLUP);
  pinMode(buttonC, INPUT_PULLUP);
  pinMode(buttonD, INPUT_PULLUP);
  pinMode(buttonE, INPUT_PULLUP);
  
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER,  1 );
  delay(200);
  digitalWrite(BUZZER,  0 );

/************************/
  ColorDiv=1;
  HideSeconds = 0;
  
  readAlarm();
  readTimer();
  if( Alarm[1] > 23)
    Alarm[1] = 0;  
  if( Alarm[2] > 59)
    Alarm[2] = 0;  
  if( Alarm[3] > 59)
    Alarm[3] = 0;  
    
  if( Timer[1] > 23)
    Timer[1] = 0;  
  if( Timer[2] > 59)
    Timer[2] = 0;  
  if( Timer[3] > 59)
    Timer[3] = 0;  

  rtc.begin(); // initialize RTC
  if (! rtc.begin()) {
    Serial.println("RTC Issue");
  }  
 
  
  current = rtc.now();
  h = current.hour();
  m = current.minute();
  m = current.second();

#ifdef _DEBUG_
  Serial.print("Hour : ");
  Serial.print(h);
  Serial.print(" Min : ");
  Serial.print(m);
  Serial.print(" Sec : ");
  Serial.println(s);
#endif

  pixels.begin(); 
  
  pixels.clear(); // Set all pixel colors to 'off'

  color_index = 9;
  compute_colors(color_index,ColorDiv);

  delay(100);

  Timer1.initialize(100000); // set a timer of length 100000 microseconds (or 0.1 sec - or 10Hz => the led will blink 5 times, 5 cycles of on-and-off, per second)
  Timer1.attachInterrupt(TimerCallBback);  // attaches callback() as a timer overflow interrupt


  compute_colors(9 , ColorDiv );
  SetupMode = 0;

  FOCUS_CLOCK = pixels.Color(255, 0, 0);
  FOCUS_ALARM = pixels.Color(0, 255, 0);
  UNFOCUS = pixels.Color(128, 128, 128);

  STATUS_NONE = pixels.Color(0, 0, 0);
  STATUS_SETUP = pixels.Color(0, 32, 0);
  STATUS_TIMER = pixels.Color(32, 0, 0);
  STATUS_CHRONO = pixels.Color(0, 0, 32);
  STATUS_ALARM = pixels.Color(32, 32, 32);
  

  CHRONO_COUNTING = 0;

  Run_Startup();
  
}




void loop ()
{
int ret ;  
unsigned long T1, T2;
int PressMode = 0;  // 0 : short (<2s) , 1 : long (>2 , <10s) , 2  :XtraLong > 10s
int h, s, m;  

  // ********************************************************
  // MODE CLOCK
  // ********************************************************
  if(current_running_mode == RUNNING_CLOCK)
  {
  
    if( _update )
    {
      _update = 0;
      compute_colors(color_index, ColorDiv);   

      current = rtc.now();
      h = current.hour();
      m = current.minute();
      s = current.second();

//      T1 = millis();
      update_display();
      
//      T2 = millis();
      if(Blink == 0) // display clocks
      {
#ifdef _DEBUG_
        Serial.print("Mode CLOCK - Clock=");
        Serial.print(h);
        Serial.print(":");
        Serial.print(m);
        Serial.print(":");
        Serial.println(s);
        Serial.print("Mode CLOCK - Alam=");
        Serial.print(Alarm[1]);
        Serial.print(":");
        Serial.print(Alarm[2]);
        Serial.print(":");
        Serial.println(Alarm[3]);
#endif
        if( AlarmSecCounter )
          AlarmSecCounter++;
      }

      // check if alarm is to be triggered
      if( Alarm[4] & 0x01 )
      {
        if( (Alarm[1] == h) && (Alarm[2] == m) && (Alarm[3] == s) )
        {
#ifdef _DEBUG_
          Serial.println("Alarm time match");
#endif          
          if( (Alarm[4] & 0x80) == 0 ) // if alarm not yet triggered
          {
#ifdef _DEBUG_
            Serial.println("Alarm trigger");
#endif
            Alarm[4] = Alarm[4] | 0x80; // set trigger on
            Alarm[4] = Alarm[4] & 0xBF; // reset manual switch off
            AlarmSecCounter = 1;
          }
        }


        if( AlarmSecCounter > 65 )
        {
          Alarm[4] = Alarm[4] & 0x3F; // reset manual switch off and alam status
          AlarmSecCounter = 0;
        } 

        
        // as long as alarm is working
        if( (Alarm[4] & 0xC0) == 0x80 ) // if alarm not cancelled
        {
            if( (Blink == 0) || (Blink == 3) )
            {
              digitalWrite(BUZZER,  1 );
            }
            else
            {
              digitalWrite(BUZZER,  0 );
            }
            
        }
      }
    }
  
  //  update_display();
  
    delay(50);
  
    ret = ReturnKeyPressed();
    if(ret)
    {
      T1 = millis();
      if( ( ret == 5 ) &&  (Alarm[4] & 0x01) )
      {
        // si alarm en cours, et non desactivée , alors desactiver
        if( (Alarm[4] & 0xC0) == 0x80 )
        {
            Alarm[4] = Alarm[4] | 0x40; // switch off alarm
#ifdef _DEBUG_
            Serial.println("Alarm manual off");
#endif            
            ret = 0;
        }
      }
      if( ( ret == 5 ) &&  ! (Alarm[4] & 0x01) )
      {
        {
          SetupMode = 1;
          SetSequence = 0;
#ifdef _DEBUG_
          Serial.print("Mode CLOCK - Alam=");
          Serial.print(Alarm[1]);
          Serial.print(":");
          Serial.print(Alarm[2]);
          Serial.print(":");
          Serial.println(Alarm[3]);
#endif          
          display_time( Alarm[1], Alarm[2], Alarm[3] );
          SetupMode = 0;
        }
      }
      WaitReleasedKey();
      T2 = millis();

      if( (T2-T1) < 750 )
      {
        PressMode = 0;
      }
      else
      {
        if( (T2-T1) < 5000 )
        {
          PressMode = 1;
        }
        else
        {
          PressMode = 2;
        }
      }

      if(ret == 5)
        PressMode = 0;

#ifdef _DEBUG_
      Serial.print("Mode CLOCK - Key=");
      Serial.println(ret);
      Serial.print("             Mode=");
      Serial.println(PressMode);
#endif
      
      if( PressMode == 0 )
      {
        if( ret == 1 )
        {
          color_index ++;
          if(color_index>MAX_COLOR_INDEX)
              color_index = 1;
          compute_colors(color_index, ColorDiv);   
        }

        if( ret == 2 )
        {
          current_running_mode = RUNNING_CHRONO;
          CHRONO_COUNTER = 0;
          CHRONO_COUNTING = 0;
        }

        if( ret == 3 )
        {
          ColorDiv++;
          if(ColorDiv > 5)
            ColorDiv = 1;
          Serial.print("Mode CLOCK - Alarm=");
          Serial.println(Alarm[4]);
        }


        if( ret == 5 ) // switch alarm mode
        {
            if( Alarm[4] & 0x01 )
              Alarm[4] = 0; // RAZ all
            else
              Alarm[4] = Alarm[4] | 0x01;
#ifdef _DEBUG_
            Serial.print("Mode CLOCK - Alarm=");
            Serial.println(Alarm[4]);
#endif
            writeAlarm();
        }

  
      }
  
      if( PressMode == 1 ) // LONG PRESS
      {
        if( ret == 3 ) // switch 12/24
        {
          _12HMode = 1 - _12HMode;
        }
        
        if( ret == 1 ) // switch HideSeconds
        {
          HideSeconds = 1 - HideSeconds;
        }


          
        if( ret == 2 ) //setup hours
        {
          SetupMode = 1;
          SetSequence = 4;

          current_running_mode = RUNNING_SETUP;

          update_display();
          Serial.println("Setup time");
          SetClockVars();
          compute_colors(color_index, ColorDiv);   
          Serial.println("End Setup time");
          SetupMode = 0;
          update_display();
          current_running_mode = RUNNING_CLOCK;
        }
      }
    }
  }

  // ********************************************************
  // MODE TIMER
  // ********************************************************
  if(current_running_mode == RUNNING_TIMER)
  {

  }
  // ********************************************************
  // MODE CHRONO
  // ********************************************************
  if(current_running_mode == RUNNING_CHRONO)
  {
    if( CHRONO_COUNTING )
    {
      update_chronos(CHRONO_COUNTER + millis() - CHRONO_START);
      
    }
    else
    {
      update_chronos(CHRONO_COUNTER);
//      CHRONO_COUNTER += millis() - CHRONO_START;
    }
    delay(50);


    ret = ReturnKeyPressed();
    if(ret)
    {
      if( ret == 5 )
      {
        CHRONO_COUNTING = 1 - CHRONO_COUNTING;
        if( CHRONO_COUNTING )
        {
          CHRONO_START = millis();
        }
        else
        {
          CHRONO_COUNTER += millis() - CHRONO_START;
        }
      }
      
      if( ( ret == 3 ) || ( ret == 1 ) )
      {
          CHRONO_COUNTER = 0;
          CHRONO_START = millis();
      }

      if( ret == 2 )
      {
        current_running_mode = RUNNING_CLOCK;
      }

      
      WaitReleasedKey();
    }
  }
  
}


int ret=0 ;  

void test_loop ()
{

  display_time( ret, ret , ret );

  delay(100);
  ret++;
  if(ret>99)
    ret = 0;
  
}


void  Run_Startup( void )
{
int val, pos;


  for(pos = 0;pos <NB_DIGIT ; pos ++)
  {
      for(val=0;val<10;val++)
      {
        uint32_t color = color_wheel(4*val*pos,2);
        for(int i=0;i<6;i++)
        {
          RGB[i] = color;
        }
        pixels.clear(); // Set all pixel colors to 'off'
        pixels.setPixelColor(STATUS_LED, STATUS_ALARM);
        display_digit(  val,  pos  );
        pixels.show();
        delay(40);
      }
  }
  for(val=0;val<10;val++)
  {
      uint32_t color = color_wheel(25*val,2);
      for(int i=0;i<NB_DIGIT;i++)
      {
        RGB[i] = color;
      }
      pixels.clear(); // Set all pixel colors to 'off'
      for(pos = 0;pos <NB_DIGIT ; pos ++)
      {
        display_digit(  val,  pos  );
      }
      pixels.show();
      delay(75);
  }

}


      
