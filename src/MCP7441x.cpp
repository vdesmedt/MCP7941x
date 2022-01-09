#include <Arduino.h>
#include <MCP7941x.h>
#include <Wire.h>

static uint8_t timeRegs[5] = { 0x00, 0x0A, 0x11, 0x18, 0x1C };
static uint8_t daysInMonths[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

void MCP7941x::setAddress(uint8_t address) {
  m_address = address;
}

void MCP7941x::startClock() 
{
  writeI2C(0x00, 0x80, 0x80);
  m_started = true;
}   

void MCP7941x::stopClock() 
{
  writeI2C(0x00, 0x80, 0x00);
  m_started = false;
}   

void MCP7941x::enableBatteryMode() 
{
  writeI2C(0x03, 0x08, 0x08);
}

MCP7941x::time_t MCP7941x::readClock(struct dateTime *dt, timeReg timeReg)
{
  time_t epochTime = 0;
  uint8_t baseAdr = timeRegs[timeReg];
  Wire.beginTransmission(m_address);
  Wire.write(baseAdr);
  Wire.endTransmission();
  Wire.requestFrom(m_address, (uint8_t)7);
  //Seconds
  uint8_t v = Wire.read();
  dt->seconds = bcdToDec(v & 0x7F);
  epochTime += dt->seconds;
  //Minutes
  v = Wire.read();
  dt->minutes = bcdToDec(v & 0x7F);
  epochTime += 60 * dt->minutes;
  //Hours
  v = Wire.read();
  dt->fmt12H = (v & 0x40);
  if(dt->fmt12H) 
  {
    dt->hours = bcdToDec(v & 0x1F);
    dt->AM = !(v & 0x20);
  }
  else
  {
    dt->hours = bcdToDec(v & 0x3F);
  }
  epochTime += 3600 * dt->hours;
  //Weekday
  v = Wire.read();
  dt->weekDay = bcdToDec(v & 0x07);
  //Date
  v = Wire.read();
  dt->day = bcdToDec(v & 0x3F);
  epochTime += 86400 * dt->day;
  //Month
  v = Wire.read();
  dt->month = bcdToDec(v & 0x1F);
  
  //Year
  v = Wire.read();
  dt->year = bcdToDec(v) + 1970;
  
  Wire.endTransmission(true);
  return true;
}

void MCP7941x::writeClock(struct dateTime *dt, uint8_t rtcWkDayFirstFives, timeReg timeReg)
{
  rtcWkDayFirstFives &= 0xF8;
  bool wasStarted = false;
  uint8_t baseAdr = timeRegs[timeReg];

  //Read ST Flag if relevant
  uint8_t rtcSec = 0;
  if (timeReg == TIME) {
    rtcSec = readI2C(baseAdr) & 0x80;
    wasStarted = rtcSec == 0x80;
  }
  
  if(timeReg == TIME) stopClock();
  //Write seconds
  Wire.beginTransmission(m_address);
  Wire.write(baseAdr);
  if(timeReg == TIME)
    Wire.write(decToBcd(dt->seconds) | rtcSec);
  else
    Wire.write(decToBcd(dt->seconds));
  
  //Write minutes
  Wire.write(decToBcd(dt->minutes));

  //Write hours
  if(dt->fmt12H && dt->hours > 11)
    dt->hours -= 12;
  uint8_t rtcH = decToBcd(dt->hours);
  if(dt->fmt12H) {
    rtcH |= 0x40;
    if(!dt->AM)
      rtcH |= 0x20;
  }
  else {
    rtcH &= ~0x40;
  }
  Wire.write(rtcH);

  //Weekday
  rtcWkDayFirstFives &= 0xF5;
  rtcWkDayFirstFives |= decToBcd(dt->weekDay);
  Wire.write(rtcWkDayFirstFives);

  //Day
  Wire.write(decToBcd(dt->day));

  //Month
  Wire.write((decToBcd(dt->month) & 0x1F) | isLeapYear(dt->year) << 5);

  //Year
  if(timeReg == TIME)
    Wire.write(decToBcd(dt->year-1970));

  Wire.endTransmission(true);
  if(wasStarted)
    startClock();
}

void MCP7941x::setClock(struct dateTime *dt)
{
  uint8_t rtcWkDay = readI2C(0x03) & 0xF8;
  writeClock(dt, rtcWkDay, TIME);
}

MCP7941x::time_t MCP7941x::getClock(struct dateTime *dt)
{
  //Read WKDAY register which contains other data from bit 3 to 7
  return readClock(dt, TIME);
}

void MCP7941x::setAlarm(enum alarm alarm, struct dateTime *dt, enum alarmMask mask)
{
  writeClock(dt, mask << 4, alarm == Alarm0 ? ALARM_0 : ALARM_1);
}
void MCP7941x::enableAlarm(enum alarm alarm)
{

}
void MCP7941x::disableAlarm(enum alarm alarm)
{

}
void MCP7941x::clearInterruptFlag(enum alarm alarm)
{

}
bool MCP7941x::checkInterruptFlag(enum alarm alarm)
{
  return false;
}

void MCP7941x::setMfpPolarity(enum MCP7941x::mfpPolarity polarity)
{

}

void MCP7941x::setSquareWaveFreq(enum MCP7941x::sqwFreq freq)
{
  
}

void MCP7941x::enableSquareWave()
{
  
}

void MCP7941x::diableSquareWave()
{
  
}

void MCP7941x::setTrimming(int trim, bool coarse)
{
  
}


uint8_t MCP7941x::readI2C(uint8_t reg) 
{
	Wire.beginTransmission(m_address);
	Wire.write(reg);
	Wire.endTransmission();
	Wire.requestFrom(m_address, uint8_t(1));
	uint8_t value = Wire.read();
	Wire.endTransmission(true);
	return value;
}

void MCP7941x::writeI2C(uint8_t reg, uint8_t value) 
{
	Wire.beginTransmission(m_address);
	Wire.write(reg);
	Wire.write(value);
	Wire.endTransmission(true);
}

bool MCP7941x::writeI2C(uint8_t reg, uint8_t mask, uint8_t value)
{
  uint8_t cVal = readI2C(reg);
  
  if((cVal & mask) == (value & mask)) return false;
  uint8_t nVal = (cVal & ~mask) | (value & mask);  
  /*
  Serial.print("Current Val:");Serial.println(cVal, HEX);
  Serial.print("Current Val Masked:");Serial.println(cVal & mask, HEX);
  Serial.print("Value Masked:");Serial.println(value & mask, HEX);
  Serial.print("New Value :");Serial.println(nVal, HEX);
  */
  writeI2C(reg, nVal);
  return true;
}

void MCP7941x::normalize(struct dateTime *dt)
{
  dt->minutes += (dt->seconds / 60);
  dt->seconds =dt->seconds % 60;
  if(dt->seconds < 0) {
    dt->seconds += 60;
    --(dt->minutes);
  }

  dt->hours += (dt->minutes / 60);
  dt->minutes = dt->minutes % 60;
  if(dt->minutes < 0) {
    dt->minutes += 60;
    --(dt->hours);
  }

  dt->day += (dt->hours / 24);
  dt->hours = dt->hours % 24;
  if(dt->hours < 0) {
    dt->hours += 24;
    --(dt->day);
  }

  uint8_t dim = daysInMonth(dt->year, dt->month);
  while(dt->day > dim) {
    dt->day -= dim;
    ++(dt->month);      
  }
}

bool MCP7941x::isLeapYear(uint16_t year)
{
	if((year % 4) != 0) return false;
  return ((year % 1000) == 0) || ((year % 100) != 0);
}

uint8_t MCP7941x::daysInMonth(uint16_t year, uint8_t month)
{
	if (month == 2 && isLeapYear(year))
		return 29;
	return daysInMonths[month-1];
}

uint16_t MCP7941x::dayOfYear(uint16_t year, uint8_t month, uint8_t day)
{
	uint16_t dayCount = 0;
	uint8_t m = 0;
	while (m < month-1)
		dayCount += daysInMonths[m++];
	if(month > 2 && isLeapYear(year))
    dayCount++;
  dayCount += day;
	return dayCount;
}

uint8_t MCP7941x::bcdToDec(uint8_t b)
{
	return ( ((b >> 4)*10U) + (b%16U) );
}

uint8_t MCP7941x::decToBcd(uint8_t d)
{
	return ( ((d/10U) << 4) + (d%10U) );
}