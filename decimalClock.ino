# As is, this sketch with includes is slightly too large for the nano's 32KB memory.
# You can either remove some of the optional modes, or trim down the parola library a little
# bit as described in
# https://arduinoplusplus.wordpress.com/2018/09/23/parola-a-to-z-optimizing-flash-memory/

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <TimeLib.h>
#include <ezButton.h>
#include <DS3232RTC.h>
#include <Timezone.h>

#include "fontData.h"

#define HARDWARE_TYPE MD_MAX72XX::DR1CR0RR0_HW
#define MAX_DEVICES 4
#define CS_PIN 3

MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

TimeChangeRule dstRule = {"CEST", Last, Sun, Mar, 2, 120};
TimeChangeRule stdRule = {"CET", Last, Sun, Oct, 3, 60};
Timezone tz(dstRule, stdRule);

unsigned long millisAtLastSecond = 0;
int lastSecond = 0;
String displayTime = "";

ezButton button(7);

enum Mode { DECIMAL_TIME, TIME_WITH_GRAPHICAL_SECONDS, HEX_TWELVE_HOUR_TIME, HEX_UNIX_TIME, BINARY_TIME };
Mode mode = DECIMAL_TIME;

void setup() {
  Serial.begin(9600);
  myDisplay.begin();
  myDisplay.setIntensity(0);
  myDisplay.displayClear();
  myDisplay.setFont(clockFont);
  myDisplay.setTextAlignment(PA_RIGHT);
  button.setDebounceTime(100);
  setSyncProvider(RTC.get);
}

void loop() {
  button.loop();
  if(button.isPressed()) {
    mode = (mode + 1)%5;
    lastSecond = 61;
  }
  if (Serial.available() >= 19) {
    setTimeFromSerialInput();
  }

  time_t t = tz.toLocal(now());
  switch(mode) {
    case DECIMAL_TIME : decimalTime(t); break;
    case TIME_WITH_GRAPHICAL_SECONDS : timeWithGraphicalSeconds(t); break;
    case HEX_TWELVE_HOUR_TIME : hexTwelveHourTime(t); break;
    case HEX_UNIX_TIME : hexUnixTime(now()); break;
    case BINARY_TIME : binaryTime(t); break;
  }
}

void binaryTime(time_t t) {
  int hours = hour(t);
  int minutes = minute(t);
  int seconds = second(t);
  if (seconds != lastSecond) {
    String hoursBinary = String(hours + 64, BIN).substring(1);
    String minutesBinary = String(minutes + 64, BIN).substring(1);
    String secondsBinary = String(seconds + 64, BIN).substring(1);
    myDisplay.print(binaryToFont(hoursBinary, minutesBinary, secondsBinary));
    lastSecond = seconds;
  }
}

String binaryToFont(String hoursBinary, String minutesBinary, String secondsBinary) {
  String result = "";
  for (int i = 0; i < 6; i++) {
    int columnInt = (String("") + hoursBinary.charAt(i) + minutesBinary.charAt(i) + secondsBinary.charAt(i)).toInt();
    switch(columnInt) {
      case 0 : result += 'H'; break;
      case 1 : result += 'I'; break;
      case 10 : result += 'J'; break;
      case 11 : result += 'K'; break;
      case 100 : result += 'L'; break;
      case 101 : result += 'M'; break;
      case 110 : result += 'N'; break;
      case 111 : result += 'O'; break;
    }
  }
  return result;
}

void timeWithGraphicalSeconds(time_t t) {
  int hours = hour(t);
  int minutes = minute(t);
  int seconds = second(t);
  if (seconds != lastSecond) {
    String hoursString = String(hours + 100).substring(1);
    String minutesString = String(minutes + 100).substring(1);
    char secondsVis = 'g' + (seconds / 3);
    myDisplay.print(hoursString + ":" + minutesString + secondsVis);
    lastSecond = seconds;
  }
}

void hexUnixTime(time_t t) {
  int currentSecond = second(t);
  if (currentSecond != lastSecond) {
    String hexTime = String(t, HEX);
    hexTime.toUpperCase();
    hexTime = shiftCodePoints(hexTime, 32);
    myDisplay.print(hexTime);
    lastSecond = currentSecond;
  }
}

void hexTwelveHourTime(time_t t) {
  int hours = hour(t)%12;
  int minutes = minute(t);
  int seconds = second(t);
  if (seconds != lastSecond) {
    String hoursHex = String(hours, HEX);
    String minutesHex = String(minutes + 256, HEX).substring(1);
    String secondsHex = String(seconds + 256, HEX).substring(1);
    hoursHex.toUpperCase();
    minutesHex.toUpperCase();
    secondsHex.toUpperCase();
    myDisplay.print(hoursHex + ":" + minutesHex + ":" + secondsHex);
    lastSecond = seconds;
  }
}

void decimalTime(time_t t) {
  int secondMillis = 0;
  int currentSecond = second(t);
  if (currentSecond != lastSecond) {
    millisAtLastSecond = millis();
    lastSecond = currentSecond;
  } else {
    unsigned long currentMillis = millis();
    if (currentMillis < millisAtLastSecond) {
      // millis has overflowed, deal with it
      secondMillis = currentMillis + (4294967295UL - millisAtLastSecond);
    } else {
      secondMillis = currentMillis - millisAtLastSecond;
    }
  }
  String decTime = timeToDecimalTime(t, secondMillis);
  if (decTime != displayTime) {
    displayTime = decTime;
    myDisplay.print(decTime);
  }
}

String timeToDecimalTime(time_t localTime, int secondMillis) {
  long milliseconds = ((hour(localTime)*60L + minute(localTime))*60L + second(localTime))*1000L + secondMillis;
  long decimalSecondsInDay = (milliseconds / (24.0*60*60*1000))*100000;
  String decimalSecondsInDayString = String(100000 + decimalSecondsInDay).substring(1);
  return decimalSecondsInDayString.substring(0, 1) + ":" + decimalSecondsInDayString.substring(1, 3) + ":" + decimalSecondsInDayString.substring(3, 5);
}

String shiftCodePoints(String input, int shift) {
  for (int i = 0; i < input.length(); i++) {
    input[i] = input[i] + shift;
  }
  return input;
}

void setTimeFromSerialInput() {
  int dateTimeComponents[6];
  dateTimeComponents[0] = Serial.parseInt();
  Serial.readStringUntil('-');
  dateTimeComponents[1] = Serial.parseInt();
  Serial.readStringUntil('-');
  dateTimeComponents[2] = Serial.parseInt();
  dateTimeComponents[3] = Serial.parseInt();
  dateTimeComponents[4] = Serial.parseInt();
  dateTimeComponents[5] = Serial.parseInt();
  while (Serial.available() > 0) Serial.read();
  setRtcTime(dateTimeComponents);
}

void setRtcTime(int dateTimeComponents[6]) {
  time_t t;
  tmElements_t tm;
  tm.Year = CalendarYrToTm(dateTimeComponents[0]);
  tm.Month = dateTimeComponents[1];
  tm.Day = dateTimeComponents[2];
  tm.Hour = dateTimeComponents[3];
  tm.Minute = dateTimeComponents[4];
  tm.Second = dateTimeComponents[5];
  t = makeTime(tm);
  RTC.set(t);
  setTime(t);
}
