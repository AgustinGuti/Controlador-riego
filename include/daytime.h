#ifndef DAY_TIME_H
#define DAY_TIME_H

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include "WiFiUdp.h"

class DayTime
{
public:
    char day;
    char hour;
    char minutes;
};

void timeInit();
void getDayTime(DayTime *dayTime);
/**
 * @return La cantidad de minutos transcurridos desde las 00:00
 */
int getDayMinutes(DayTime *dayTime);

#endif