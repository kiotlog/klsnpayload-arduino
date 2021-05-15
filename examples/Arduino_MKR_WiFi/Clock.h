#include <RTCZero.h>

extern RTCZero rtc;

void setCompileDateTime(int tz = 0);

void setCompileDate()
{
  String date = __DATE__;
  String monthStr = date.substring(0, 3);
  String dayStr = date.substring(4, 6);
  String yearStr = date.substring(9, 11);

  byte day = dayStr.toInt();
  byte year = yearStr.toInt();

  byte month;
  if (monthStr.compareTo("Jan") == 0) {
    month = 1;
  } else if (monthStr.compareTo("Feb") == 0) {
    month = 2;
  } else if (monthStr.compareTo("Mar") == 0) {
    month = 3;
  } else if (monthStr.compareTo("Apr") == 0) {
    month = 4;
  } else if (monthStr.compareTo("May") == 0) {
    month = 5;
  } else if (monthStr.compareTo("Jun") == 0) {
    month = 6;
  } else if (monthStr.compareTo("Jul") == 0) {
    month = 7;
  } else if (monthStr.compareTo("Aug") == 0) {
    month = 8;
  } else if (monthStr.compareTo("Sep") == 0) {
    month = 9;
  } else if (monthStr.compareTo("Oct") == 0) {
    month = 10;
  } else if (monthStr.compareTo("Nov") == 0) {
    month = 11;
  } else if (monthStr.compareTo("Dec") == 0) {
    month = 12;
  }
  
  rtc.setDate(day, month, year);
}

void setCompileTime(int tz)
{
  String time = __TIME__;
  String hStr = time.substring(0, 2);
  String mStr = time.substring(3, 5);
  String sStr = time.substring(6, 8);
  
  byte hours = hStr.toInt() + tz;
  byte minutes = mStr.toInt();
  byte seconds = sStr.toInt();

  rtc.setTime(hours, minutes, seconds); 
}

void setCompileDateTime(int tz)
{
  setCompileTime(tz);
  setCompileDate();
}
