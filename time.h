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

#ifndef TIME_H
#define TIME_H

typedef unsigned long time_t;

/* ISO C `broken-down time' structure.  */
struct tm
{
  int tm_sec;   /* Seconds. [0-60] (1 leap second) */
  int tm_min;   /* Minutes. [0-59] */
  int tm_hour;  /* Hours.   [0-23] */
  int tm_mday;  /* Day.     [1-31] */
  int tm_mon;   /* Month.   [0-11] */
  int tm_year;  /* Year - 1900. */
  int tm_wday;  /* Day of week. [0-6] */
  int tm_yday;  /* Days in year.[0-365] */
  int tm_isdst; /* DST.   [-1/0/1]*/
};

time_t now();
void setTime(time_t t);
void setTime(int hr, int min, int sec, int day, int month, int yr);
struct tm *gmtime(const time_t *timer);

#endif /* TIME_H */
