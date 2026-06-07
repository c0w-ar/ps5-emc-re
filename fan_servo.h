#ifndef FAN_SERVO_H
#define FAN_SERVO_H

#include <linux/init.h>

enum fan_zone {
    MAINSOC,
    LOCAL1,
    LOCAL2,
    LOCAL3
};

enum fan_mode {
    AUTO=1,
    MAXIMUM,
    MINUMUM,
    MANUAL,
    SP1
};

enum servo_setting {
    TARGETTEMP,
    PGAIN,
    IGAIN,
    ILIMIT,
    ULIMIT,
    DLIMIT,
    UPLIMIT,
    DPLIMIT,
    UILIMIT,
    DILIMIT,
    DIFGAIN,
    DIFLIMIT,
    DIFDLIMIT,
    MAXDDUTY,
    OFFSETINVALIDTEMPUPPER,
    OFFSETINVALIDTEMPLOWER,
    OFFSETGAINUPPER,
    OFFSETGAINLOWER,
};

char* servo_setting_name[] = {
    "SetVal",
    "Pgain",
    "Igain",
    "Ilimit",
    "Ulimit",
    "Dlimit"
};

struct setting_group {
    struct {
        u16 max;
        u16 min;
    } zone_duty[4];
    u16 zone_duty_maxlow[4];
    u8  padding[8];
    struct {
        u32 SetVal;                 // 0x00 - < 0xff01
        u32 Pgain;                  // 0x01	- < 0x4001
        u32 Igain;                  // 0x02	- < 0x4001
        u32 Ilimit;                 // 0x03	- < 0x40000000
        u32 Ulimit;                 // 0x04	- < 0x10000000
        u32 Dlimit;                 // 0x05	- < 0x10000000
        u32 UPLimit;                // 0x06
        u32 DPLimit;                // 0x07
        u32 UILimit;                // 0x08
        u32 DILimit;                // 0x09
        u32 DifGain;                // 0x0A
        u32 DifLimit;               // 0x0b
        u32 DifDLimit;              // 0x0c
        u32 MaxDduty;               // 0x0d
        u32 OffsetInvalidTempUpper; // 0x0e
        u32 OffsetInvalidTempLower; // 0x0f
        u32 OffsetGainUpper;        // 0x10
        u32 OffsetGainLower;        // 0x11
        u8  padding[8];
    } zone_servo[4];
};


// This is the structure set by main OS on boot / resume
struct servo_preset {
    struct setting_group group[4];
};

#endif
