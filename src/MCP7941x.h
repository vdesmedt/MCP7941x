#ifndef MCP7941x_h
#define MCP7941x_h

class MCP7941x
{
public:
    // Enums
    enum timeReg
    {
        REG_TIME = 0,
        REG_ALARM_0 = 1,
        REG_ALARM_1 = 2,
        REG_PWR_DOWN = 3,
        REG_PWR_UP = 4,
    };
    enum alarm
    {
        Alarm0 = 0,
        Alarm1 = 1,
    };
    enum alarmMask
    {
        seconds = 0,
        minutes = 1,
        hours = 2,
        dayOfWeek = 3,
        date = 4,
        all = 7, // Seconds, Minutes, Hours, DayOfWeek, Date & Month
    };
    enum sqwFreq
    {
        freq32768Hz = 3,
        freq8192Hz = 2,
        freq4096Hz = 1,
        freq1Hz = 0,
    };
    enum mfpPolarity
    {
        matchingPolarity = 1,
        reversePolarity = 0, // In Dual Alarm mode, MFP will be 0 only if both interrupt flags are set.
    };

    // Structs
    struct dateTime
    {
        uint8_t seconds;
        uint8_t minutes;
        uint8_t hours;
        uint8_t day;
        uint8_t weekDay;
        uint8_t month;
        uint16_t year;
        bool fmt12H;
        bool AM;
    };
    // TypeDef
    typedef int32_t time_t;

    // Utility methods
    static bool isLeapYear(uint16_t year);
    static uint8_t daysInMonth(uint16_t year, uint8_t month);
    static uint16_t dayOfYear(uint16_t year, uint8_t month, uint8_t day);

    // Methods
    void setAddress(uint8_t address);
    void startClock();
    void stopClock();
    // Config
    void enableBatteryMode();
    void setTrimming(int trim, bool coarse);
    // Clock
    time_t getClock(struct dateTime *dt);
    void setClock(struct dateTime *dt);
    // Alarm
    void setAlarm(enum alarm alarm, struct dateTime *dt, enum alarmMask mask);
    void enableAlarm(enum alarm alarm);
    void disableAlarm(enum alarm alarm);
    bool clearInterruptFlag(enum alarm alarm);
    bool checkInterruptFlag(enum alarm alarm);
    void setMfpPolarity(enum mfpPolarity polarity);
    // Square Wave
    void setSquareWaveFreq(enum sqwFreq freq);
    void enableSquareWave();
    void disableSquareWave();
    void normalize(struct dateTime *dt);

private:
    bool m_started = false;
    uint8_t m_address = 0x6F;
    time_t readClock(struct dateTime *dt, timeReg timeReg = REG_TIME);
    void writeClock(struct dateTime *dt, uint8_t rtcWkDayFirstFives, timeReg timeReg = REG_TIME);
    uint8_t readI2C(uint8_t reg);
    void writeI2C(uint8_t reg, uint8_t value);
    bool writeI2C(uint8_t reg, uint8_t mask, uint8_t value);
    // returns previous state
    bool setBit(uint8_t reg, uint8_t bit);
    // returns previous state
    bool clearBit(uint8_t reg, uint8_t bit);
    static uint8_t bcdToDec(uint8_t b);
    static uint8_t decToBcd(uint8_t b);
};

#endif