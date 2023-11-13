#include "daytime.h"

const long utcOffsetInSeconds = -3 * 60 * 60; // UTC - 3

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void timeInit()
{
    timeClient.begin();
}

void getDayTime(DayTime *dayTime)
{
    timeClient.update();
    dayTime->day = timeClient.getDay();
    dayTime->hour = timeClient.getHours();
    dayTime->minutes = timeClient.getMinutes();
}

int getDayMinutes(DayTime *dayTime)
{
    return dayTime->hour * 60 + dayTime->minutes;
}