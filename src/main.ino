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

// Scheduler
Scheduler ts;

Task t1 (10 * TASK_SECOND, TASK_FOREVER, &displayTemperature, &ts, true);
Task t2 (200 * TASK_MILLISECOND, TASK_FOREVER, &displayTimeDate, &ts, true);
Task t3 (5 * TASK_SECOND, TASK_FOREVER, &displayBannerTextNext, &ts, true);
//Task t11 (1 * TASK_MINUTE, TASK_FOREVER, &displayTimeUntilPartyWeeks, &ts, true);
//Task t12 (1 * TASK_MINUTE, TASK_FOREVER, &displayTimeUntilPartyDays, &ts, true);
Task t13 (1 * TASK_MINUTE, TASK_FOREVER, &displayTimeUntilPartyHours, &ts, true);
//Task t14 (500 * TASK_MILLISECOND, TASK_FOREVER, &displayTimeUntilPartyMinutes, &ts, true);
//Task t15 (500 * TASK_MILLISECOND, TASK_FOREVER, &displayTimeUntilPartySeconds, &ts, true);
//Task t16 (1 * TASK_SECOND, TASK_FOREVER, &displayTimeUntilPartyFull, &ts, true);
//Task t21 (1 * TASK_MINUTE, TASK_FOREVER, &blinkBuiltinLed, &ts, true);
Task t22 (1 * TASK_MINUTE, TASK_FOREVER, &blinkFetLed, &ts, true);


// RTC stuff
#define countof(a) (sizeof(a) / sizeof(a[0])) // Used to get the array size
RtcDateTime epochPartyTime = RtcDateTime("Feb 23 2020", "11:11:00");  // Set the magic date and time

extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];
extern uint8_t SmallFont[];

// TFT display/button stuff
UTFT myGLCD(ITDB32S, 38, 39, 40, 41);   // a 3.2" TFT LCD Screen module, 320*240 (resolution), 65K color
URTouch myTouch(6,5,4,3,2);
UTFT_Buttons myButtons(&myGLCD, &myTouch);

int x, y;
int but1,but2,but3,but4,but5,but6,but7,but8,but9,but10,start1, start2,start3,start4, butClr, butEnt, pressed_button;
char stCurrent[10] = "";
int stCurrentLen = 0;
char stLast[10] = "";

// declaration of variables for flickering of the flamingo
const int max_flicker_time = 100;
const int min_flicker_time = 100;
const int max_flicker_length = 100;
const int min_flicker_length = 100;
const int off_variation_offset = 100;

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

  myGLCD.setBackColor(255,255,0);
  myGLCD.setColor(0,0,0);
  myGLCD.setFont(BigFont);
  myGLCD.print(F("WEKEN"), CENTER, 240);
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
  myGLCD.print(F("DAGEN"), CENTER, 240);
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
  myGLCD.print(F("UUR"), CENTER, 240);
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
  myGLCD.print(F("Minuten"), CENTER, 240);
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
  myGLCD.print(F("Seconden"), CENTER, 240);
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
analogWrite(FET_PIN, 10);
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
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  int32_t compiledCorrected = compiled + 5; // Add a couple of seconds to compensate for upload/flash time

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
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

  // Init LCD and touch screen
  myGLCD.InitLCD(PORTRAIT);
  myGLCD.clrScr();
  myTouch.InitTouch(PORTRAIT);
  myTouch.setPrecision(PREC_MEDIUM);

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
