#include <avr/pgmspace.h>
#include <Arduino.h>

#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass
#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
#include <TaskScheduler.h>

#include <UTFT.h>
#include <URTouch.h>
#include <UTFT_Buttons.h>

#include <Wire.h> // must be included before RtcDS3231.h so that Arduino library object file references work
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);

// include and define for SD card
#include <SD.h>                      // need to include the SD library
#define SD_ChipSelectPin 53          // SS pin 53 on Mega2560

// includes needed for audio OUTPUT
#include <TMRpcm.h>                  //  PCM playing library...
#include <SPI.h>
#define laudSpeakerPin 11
TMRpcm audio;                        // create an object for playing audio

// includes and variables needed for alarm
const byte interruptPin = 13;
volatile bool alarm = 0;


// Scheduler
Scheduler ts;

Task t1 (10 * TASK_SECOND, TASK_FOREVER, &displayTemperature, &ts, true);
Task t2 (200 * TASK_MILLISECOND, TASK_FOREVER, &displayTimeDate, &ts, true);
Task t3 (5 * TASK_SECOND, TASK_FOREVER, &displayBannerTextNext, &ts, true);
Task t4 (100 * TASK_MILLISECOND, TASK_FOREVER, &scanScreen, &ts, true);
Task t11 (1 * TASK_MINUTE, TASK_FOREVER, &displayTimeUntilPartyWeeks, &ts, false);
Task t12 (1 * TASK_MINUTE, TASK_FOREVER, &displayTimeUntilPartyDays, &ts, false);
Task t13 (1 * TASK_MINUTE, TASK_FOREVER, &displayTimeUntilPartyHours, &ts, false);
Task t14 (500 * TASK_MILLISECOND, TASK_FOREVER, &displayTimeUntilPartyMinutes, &ts, false);
Task t15 (500 * TASK_MILLISECOND, TASK_FOREVER, &displayTimeUntilPartySeconds, &ts, false);
Task t16 (1 * TASK_SECOND, TASK_FOREVER, &displayTimeUntilPartyFull, &ts, false);
//Task t21 (1 * TASK_MINUTE, TASK_FOREVER, &blinkBuiltinLed, &ts, true);
Task t22 (1 * TASK_MINUTE, TASK_FOREVER, &blinkFetLed, &ts, true);
Task t30 (1 * TASK_SECOND, TASK_FOREVER, &checkAlarm, &ts, true);

// RTC stuff
#define countof(a) (sizeof(a) / sizeof(a[0])) // Used to get the array size
RtcDateTime epochPartyTime = RtcDateTime("Feb 23 2020", "11:11:00");  // Set the magic date and time

extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];
extern uint8_t SmallFont[];
extern uint8_t Dingbats1_XL[];

// TFT display/button stuff
UTFT myGLCD(ITDB32S, 38, 39, 40, 41);   // a 3.2" TFT LCD Screen module, 320*240 (resolution), 65K color
URTouch myTouch(6,5,4,3,2);
UTFT_Buttons myButtons(&myGLCD, &myTouch);
// extern unsigned short frog[3600];     // define frog image (60x60)
// extern unsigned short oeteldonk[10160]; // define oeteldonk image(80x127)
extern unsigned short frog[];
extern unsigned short oeteldonk[];

// declaration of variables for flickering of the beautifull pink flamingo
const int max_flicker_time = 100;
const int min_flicker_time = 100;
const int max_flicker_length = 100;
const int min_flicker_length = 100;
const int off_variation_offset = 100;

int but1, but2, but3, pressed_button;
int flicker_length,hold_on,hold_off,flick_off = 100, flick_on = 100;

// Global variables
#define FET_PIN 7
int itemNr = 0;       // Counter voor de actieve spreuk
String spreuken[7] =
{
  "Tot CARNAVAL 2020",
  "Carnaval in Oeteldonk",
  "FF lekker onthaasten",
  "Het is schrikkeljaar !!",
  "Hendrien Rules!!",
  "Tot d'n hoogheid aonkomt",
  "Prins Amadeiro etc. etc."
};

int liedNr = 0;       // Counter voor liedjes
char *liedjes[3] =
{
  "anoesjka.wav",
  "lekker.wav",
  "vingers.wav"
};

int timeItem = 0;     // Counter voor tijdweergave

/**********************************************************************
 * Functions: string formating
 **********************************************************************/
String formatTime(const RtcDateTime& dt)
{
  char datestring[9];
  snprintf_P(datestring,
          countof(datestring),
          PSTR("%02u:%02u:%02u"),
          dt.Hour(),
          dt.Minute(),
          dt.Second() );
  return datestring;
}

String formatDate(const RtcDateTime& dt)
{
  char datestring[11];
  snprintf_P(datestring,
          countof(datestring),
          PSTR("%02u/%02u/%04u"),
          dt.Day(),
          dt.Month(),
          dt.Year() );
  return datestring;
}

/**********************************************************************
 * Functions: display
 **********************************************************************/
 void placeButtons()
 {
 but1 = myButtons.addButton( 0, 0, 239, 105,(char *)"");
 but2 = myButtons.addButton( 0,106,239,106,(char *)"");
 but3 = myButtons.addButton( 0,213,239,106,(char *)"");
 myButtons.drawButtons();
 }

void scanScreen()
{
  if (myTouch.dataAvailable() == true)
  {
    pressed_button = myButtons.checkButtons();
    if (pressed_button==but1)
    {
      Serial.println(" Button2 !!");
      myGLCD.fillRect(0,106,239,212);     // restore white part of the flag
      myGLCD.setBackColor(255,255,255);
      myGLCD.drawBitmap (90,130, 80, 45, oeteldonk);
//      audio.play("anoesjka.wav");
      audio.play(liedjes[liedNr]);
      liedNr = ((liedNr + 1) % countof(liedjes));
    }
    if (pressed_button==but2)
    {
      Serial.println(" Button2 !!");
      myGLCD.fillRect(0,106,239,212);     // restore white part of the flag
      myGLCD.setBackColor(255,255,255);
      myGLCD.drawBitmap (90,130, 60, 60, frog);
      audio.play("frog.wav");     //the sound file "frog.wav" will play
      //audio.play((char *)"grenouille.wav");     //the sound file "frog.wav" will play
    }
    if (pressed_button==but3)
    {
      Serial.println(" Button3 !!");

      myGLCD.setColor(255,255,255);
      myGLCD.fillRect(0,106,239,212);     // restore white part of the flag

      switch (timeItem)
      {
      case 0:
        t16.disable();
        t11.enable();
        break;
      
      case 1:
        t11.disable();
        t12.enable();
        break;
      
      case 2:
        t12.disable();
        t13.enable();
        break;
      
      case 3:
        t13.disable();
        t14.enable();
        break;
      
      case 4:
        t14.disable();
        t15.enable();
        break;
      
      case 5:
        t15.disable();
        t16.enable();
        break;
      
      default:
        break;
      }
      timeItem = ((timeItem + 1) % 6);    // 6 Tasks are defined
      audio.disable();

    }
  }
}

void displayFlagOeteldonk()
{
  myGLCD.setColor(255,0,0);
  myGLCD.fillRect(0,0,239,105);
  myGLCD.setColor(255,255,255);
  myGLCD.fillRect(0,106,239,212);
  myGLCD.setColor(255,255,0);
  myGLCD.fillRect(0,213,239,319);
}

void displayBasicText()
{
  myGLCD.setBackColor(255,0,0);
  myGLCD.setColor(255,255,255);
  myGLCD.setFont(BigFont);
  myGLCD.print(F("NOG"), CENTER, 48);
}

void displayTimeDate()
{
  myGLCD.setBackColor(255,0,0);
  myGLCD.setColor(255,255,255);
  myGLCD.setFont(SmallFont);

  RtcDateTime now = Rtc.GetDateTime();

  myGLCD.print(formatDate(now), LEFT, 0);
  myGLCD.print(formatTime(now), CENTER, 0);
}

void displayTemperature()
{
  myGLCD.setBackColor(255,0,0);
  myGLCD.setColor(255,255,255);
  myGLCD.setFont(SmallFont);

  RtcTemperature temp = Rtc.GetTemperature();
  String currentTemp = String(temp.AsFloatDegC()) + "C";
  myGLCD.print(currentTemp, RIGHT, 0);
}

void displayTimeUntilPartyWeeks()
{
  RtcDateTime now = Rtc.GetDateTime();
  int32_t weeksToGo = ((epochPartyTime - now) / 604800);  // Devide by one week

  myGLCD.setBackColor(255,255,255);
  myGLCD.setColor(0,0,0);
  myGLCD.setFont(SevenSegNumFont);
  myGLCD.print(String(weeksToGo), CENTER, 132);

  myGLCD.setColor(255,255,0);
  myGLCD.fillRect(0,213,239,319);
  myGLCD.setBackColor(255,255,0);
  myGLCD.setColor(0,0,0);
  myGLCD.setFont(BigFont);
  myGLCD.print(F("   WEKEN   "), CENTER, 240);
}

void displayTimeUntilPartyDays()
{
  RtcDateTime now = Rtc.GetDateTime();
  int32_t daysToGo = ((epochPartyTime - now) / 86400);  // Devide by one day

  myGLCD.setBackColor(255,255,255);
  myGLCD.setColor(0,0,0);
  myGLCD.setFont(SevenSegNumFont);
  myGLCD.print(String(daysToGo), CENTER, 132);

  myGLCD.setBackColor(255,255,0);
  myGLCD.setColor(0,0,0);
  myGLCD.setFont(BigFont);
  myGLCD.print(F("   DAGEN   "), CENTER, 240);
}

void displayTimeUntilPartyHours()
{
  RtcDateTime now = Rtc.GetDateTime();
  int32_t hoursToGo = ((epochPartyTime - now) / 3600);  // Devide by one hour

  myGLCD.setBackColor(255,255,255);
  myGLCD.setColor(0,0,0);
  myGLCD.setFont(SevenSegNumFont);
  myGLCD.print(String(hoursToGo), CENTER, 132);

  myGLCD.setBackColor(255,255,0);
  myGLCD.setColor(0,0,0);
  myGLCD.setFont(BigFont);
  myGLCD.print(F("    UUR    "), CENTER, 240);
}

void displayTimeUntilPartyMinutes()
{
  RtcDateTime now = Rtc.GetDateTime();
  int32_t minutesToGo = ((epochPartyTime - now) / 60);  // Devide by one hour

  myGLCD.setBackColor(255,255,255);
  myGLCD.setColor(0,0,0);
  myGLCD.setFont(SevenSegNumFont);
  myGLCD.print(String(minutesToGo), CENTER, 132);

  myGLCD.setBackColor(255,255,0);
  myGLCD.setColor(0,0,0);
  myGLCD.setFont(BigFont);
  myGLCD.print(F("  Minuten  "), CENTER, 240);
}

void displayTimeUntilPartySeconds()
{
  RtcDateTime now = Rtc.GetDateTime();
  int32_t secondsToGo = (epochPartyTime - now);

  myGLCD.setBackColor(255,255,255);
  myGLCD.setColor(0,0,0);
  myGLCD.setFont(SevenSegNumFont);
  myGLCD.print(String(secondsToGo), CENTER, 132);

  myGLCD.setBackColor(255,255,0);
  myGLCD.setColor(0,0,0);
  myGLCD.setFont(BigFont);
  myGLCD.print(F(" Seconden "), CENTER, 240);
}

void displayTimeUntilPartyFull()
{
  RtcDateTime now = Rtc.GetDateTime();
  int32_t secondsToGo = (epochPartyTime - now);
  int32_t minutesToGo = (secondsToGo / 60);  // Devide by one min
  int32_t hoursToGo = (minutesToGo / 60);  // Devide by one hour
  int32_t daysToGo = (hoursToGo / 24);  // Devide by one day

  myGLCD.setBackColor(255,255,255);
  myGLCD.setColor(0,0,0);

  myGLCD.setFont(SevenSegNumFont);
  myGLCD.print(String(daysToGo), 20, 106);
  myGLCD.setFont(BigFont);
  myGLCD.print(F("d"), 82, 140);

  myGLCD.setFont(SevenSegNumFont);
  myGLCD.print(String(hoursToGo), 120, 106);
  myGLCD.setFont(BigFont);
  myGLCD.print(F("h"), 216, 140);

  myGLCD.setFont(SevenSegNumFont);
  myGLCD.print(String(minutesToGo), CENTER, 158);
  myGLCD.setFont(BigFont);
  myGLCD.print(F("m"), 200, 190);

  myGLCD.setBackColor(255,255,0);
  myGLCD.setColor(0,0,0);

  myGLCD.setFont(SevenSegNumFont);
  myGLCD.print(String(secondsToGo), CENTER, 220);
  myGLCD.setFont(BigFont);
  myGLCD.print(F("Seconden"), CENTER, 270);
}

void displayBannerTextNext()
{
  myGLCD.setBackColor(255,255,0);
  myGLCD.setColor(0,0,0);
  myGLCD.setFont(SmallFont);
  myGLCD.print(F("                            "), CENTER, 300);
  myGLCD.print(spreuken[itemNr], CENTER, 300);
  //itemNr = ((itemNr + 1) % spreukenNr);
  itemNr = ((itemNr + 1) % countof(spreuken));
}


/**********************************************************************
 * Functions
 **********************************************************************/
void rtcCheck()   // TODO return error code and use in setup()
{
  if (!Rtc.IsDateTimeValid())
  {
    if (Rtc.LastError() != 0)
    {
      Serial.print(F("RTC communications error = "));
      Serial.println(Rtc.LastError());
    }
    else
    {
      Serial.println(F("RTC lost confidence in the DateTime!"));
    }
  }
}

void rtcResetClock()
{
  RtcDateTime timeReset = RtcDateTime("Jan 01 2010", "11:11:00");  // Set the magic date and time
  Rtc.SetDateTime(timeReset);
}

void blinkBuiltinLed()
{
  digitalWrite(LED_BUILTIN, true);
  delay(500);
  digitalWrite(LED_BUILTIN, false);
  delay(500);
}

void blinkFetLed()
{
flicker_length = random(10,20); // min and max flicker loop length
hold_on = random(500,800);
hold_off = random(800,1000);

for (int i = 0; i < flicker_length; i++)
  {
    analogWrite(FET_PIN,10);
    delay(flick_off);
    analogWrite(FET_PIN,100);
    delay(flick_on);
    flick_off = random(100,200);  //on and off varies during loop
    flick_on = random(50,100);   // Unsure if this adds noticable time to the flicker on
  }
delay(hold_on); // Loop ends with on so this delay then decides sucess hold
digitalWrite(FET_PIN,0); // This is then the faliure
delay(hold_off); // how long to wait before next startup attempt
// analogWrite(FET_PIN, 10);
}


//Some functions needed for Alarm
void handleAlarm() {
  alarm = false;
  audio.play("siren.wav");     //the sound file "frog.wav" will play
  Rtc.LatchAlarmsTriggeredFlags();
}

void handleInterrupt() {
   alarm = true;
}

void checkAlarm(){
  if (alarm == true) {
  handleAlarm();
  }
}

/*
void blinkFetLed()
{
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(FET_PIN, true);
    delay(200);
    digitalWrite(FET_PIN, false);
    delay(200);
  }
  analogWrite(FET_PIN, 10);
}
*/
/**********************************************************************
 * Setup
 **********************************************************************/
void setup()
{
  Serial.begin(115200);

  Serial.print(F("Compiled at: "));
  Serial.print(__DATE__);
  Serial.print(F(" "));
  Serial.println(__TIME__);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FET_PIN, OUTPUT);

  // Init RTC library
  Rtc.Begin();

  //rtcResetClock();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  uint32_t compiledCorrected = compiled + 5; // Add a couple of seconds to compensate for upload/flash time

  // Check if the clock is working
  if (!Rtc.IsDateTimeValid())
  {
    if (Rtc.LastError() != 0)
    {
      Serial.print(F("RTC communications error = "));
      Serial.println(Rtc.LastError());
    }
    else
    {
      Serial.println(F("RTC lost confidence in the DateTime!"));
      Rtc.SetDateTime(compiledCorrected);
    }
  }

  // Be sure the clock is running
  if (!Rtc.GetIsRunning())
  {
    Serial.println(F("RTC was not actively running, starting now"));
    Rtc.SetIsRunning(true);
  }

  // Be sure the clock has a 'semi valid' date/time. if not, set compile time
  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiledCorrected)
  {
    Serial.println(F("RTC is older than compile time!  (Updating DateTime)"));
    Rtc.SetDateTime(compiledCorrected);
  }
  else if (now > compiledCorrected)
  {
    Serial.println(F("RTC is newer than compile time. (this is expected)"));
  }
  else if (now == compiledCorrected)
  {
    Serial.println(F("RTC is the same as compile time! (not expected but all is fine)"));
  }
  // Set the clock to the needed state
  Rtc.Enable32kHzPin(false);
  //Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

  // Settings for alarm
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeAlarmOne);
  DS3231AlarmOne alarm1(
    0,
    11,
    11,
    00,
  DS3231AlarmOneControl_HoursMinutesSecondsMatch);
  Rtc.SetAlarmOne(alarm1);
  Rtc.LatchAlarmsTriggeredFlags();

  // Init LCD and touch screen
  myGLCD.InitLCD(PORTRAIT);
  myGLCD.clrScr();
  myTouch.InitTouch(PORTRAIT);
  myTouch.setPrecision(PREC_MEDIUM);

  // Init audio
  //pinMode(laudSpeakerPin, OUTPUT);
  audio.speakerPin = laudSpeakerPin; //5,6,11 or 46 on Mega, 9 on Uno, Nano, etc
  audio.setVolume(5);    //   0 to 7. Set volume level

  // Check if the SD card is working
  if (!SD.begin(SD_ChipSelectPin)) {  // see if the card is present and can be initialized:
    Serial.println("SD fail");
    return;   // don't do anything more if not
  }
  else{
    Serial.println("SD ok");
  }

  // Put flag, text and time on display
  placeButtons();
  // Say bootup hello ;)
  // blinkFetLed();

  // Display basic default stuff
  displayFlagOeteldonk();
  displayBasicText();
}

/**********************************************************************
 * Main loop, start the scheduler and run forrest, run
 **********************************************************************/
void loop()
{
  // rtcCheck();
  ts.execute();
}
