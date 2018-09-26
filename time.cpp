/*
Copyright (C) 2018 Jorge Matricali <jorgematricali@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdint.h> /* uint32_t */
#include <Arduino.h> /* millis */
#include "time.h"

static uint32_t sysTime = 0;
static uint32_t prevMillis = 0;
static uint32_t nextSyncTime = 0;

time_t now()
{
  while (millis() - prevMillis >= 1000) {
    sysTime++;
    prevMillis += 1000;
  }
  return (time_t)sysTime;
}

void setTime(time_t t)
{ 
  sysTime = (uint32_t)t;  
  prevMillis = millis();
}

struct tm *gmtime(const time_t *timer)
{
  static struct tm tmbuf;
  register struct tm *tp = &tmbuf;
  time_t time = *timer;
  register long day, mins, secs, year, leap;
  day = time / (24L * 60 * 60);
  secs = time % (24L * 60 * 60);
  tp->tm_sec = secs % 60;
  mins = secs / 60;
  tp->tm_hour = mins / 60;
  tp->tm_min = mins % 60;
  tp->tm_wday = (day + 4) % 7;
  year = (((day * 4) + 2) / 1461);
  tp->tm_year = year + 70;
  leap = !(tp->tm_year & 3);
  day -= ((year * 1461) + 1) / 4;
  tp->tm_yday = day;
  day += (day > 58 + leap) ? ((leap) ? 1 : 2) : 0;
  tp->tm_mon = ((day * 12) + 6) / 367;
  tp->tm_mday = day + 1 - ((tp->tm_mon * 367) + 5) / 12;
  tp->tm_isdst = 0;
  return (tp);
}
