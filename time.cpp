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
