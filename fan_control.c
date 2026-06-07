
#include <linux/init.h>
#include <linux/ps5.h>
#include "fan_servo.h"

int icc_query_wrapper(u8 service_id, u8 msg_type, u8 length, u8* data) {
    struct icc_msg *msg = (struct icc_msg *) kzalloc(ICC_MSG_MAX_SIZE, GFP_KERNEL);
    int ret;

    msg->service_id = service_id;
    msg->msg_type = msg_type;
    msg->length = length;
    memcpy(msg->data, data, ICC_MSG_MAX_SIZE - 0x20);

    ret = icc_query((u8*) msg, (u8*) msg);
    if (ret || ((u16*)msg->data)[0]) {
        pr_err("icc_query %02x-%02x failed: ret=%d icc_ret=%04x\n", service_id, msg_type, ret, ((u16*)msg->data)[0]);
        if(!ret)
            ret = ((u16*)msg->data)[0];
    }

    memcpy(data, msg->data, ICC_MSG_MAX_SIZE - 0x20);
    kfree(msg);
    return ret;
}


int ps5_icc_fan_mode_get (u8 zone, u8 fanmode)
{
    if (zone>3 || fanmode<1 || fanmode>6)
        return EINVAL;

    u8* data = (u8*) kzalloc(ICC_MSG_MAX_SIZE, GFP_KERNEL);
    memset(data, 0, ICC_MSG_MAX_SIZE);
    data[0] = zone;

    int ret = icc_query_wrapper(ICC_SERVICE_ID_FAN, 0x03, 0x20, data);
    if (ret)
        pr_err("ps5_icc_fan_mode_get failed - ret=%04x  zone=%d\n", ret, zone);
    else
        pr_info("ps5_icc_fan_mode_get OK - zone=%d fanmode=%d\n", zone, data[2]);

    kfree(data);
    return ret;
}

int ps5_icc_fan_mode_set (u8 zone, u8 fanmode)
{
    if (zone>3 || fanmode<1 || fanmode>6)
        return EINVAL;

    u8* data = (u8*) kzalloc(ICC_MSG_MAX_SIZE, GFP_KERNEL);
    memset(data, 0, ICC_MSG_MAX_SIZE);
    data[0] = zone;
    data[1] = fanmode;

    int ret = icc_query_wrapper(ICC_SERVICE_ID_FAN, 0x02, 0x20, data);
    if (ret)
        pr_err("ps5_icc_fan_mode_set failed - ret=%04x  zone=%d fanmode=%d\n", ret, zone, fanmode);
    else
        pr_info("ps5_icc_fan_mode_set OK - zone=%d fanmode=%d\n", zone, fanmode);

    kfree(data);
    return ret;
}

// Get Fan Servo Current Settings for Zone X
// EMC only provides first 6 params
int icc_fan_zone_curset_get(u8 zone)
{
    if (zone>3)
        return EINVAL;

    u8* data = (u8*) kzalloc(ICC_MSG_MAX_SIZE, GFP_KERNEL);
    memset(data, 0, ICC_MSG_MAX_SIZE);
    data[0] = zone;

    int ret = icc_query_wrapper(ICC_SERVICE_ID_FAN, 0x07, 0x20, data);
    if (ret)
        pr_err("icc_fan_zone_curset_get failed - ret=%04x zone=%d \n", ret, zone);
    else {
        pr_info("icc_fan_zone_curset_get OK - zone=%02d data=%*phN\n", zone, 0x18, (data+4));
        pr_info("\nParsed Fan Servo Settings:\n\n");
        for (int i=0 ; i<6 ; i++) {
            pr_info("%d - %6s : %08x\n", i, servo_setting_name[i], *(u32*)(data+4+i*4));
        }
        pr_info("\n");
    }
    kfree(data);
    return ret;
}

// Set Fan Servo Current Setting Y for Zone X
// EMC only allows setting one of the first 6 params
int icc_fan_zone_curset_set(u8 zone, u8 setting_id, u32 value)
{
    if (zone>3 || setting_id>5)
        return EINVAL;

    u8* data = (u8*) kzalloc(ICC_MSG_MAX_SIZE, GFP_KERNEL);
    memset(data, 0, ICC_MSG_MAX_SIZE);
    data[0] = zone;
    data[1] = setting_id;

    // Write value on its offset starting at 0x04
    *(u32 *)(data + setting_id * 4 + 4) = value;

    int ret = icc_query_wrapper(ICC_SERVICE_ID_FAN, 0x06, 0x40, data);
    if (ret)
        pr_err("icc_fan_zone_curset_set failed - ret=%04x zone=%d setting_id=%d value=%08x\n", ret, zone, setting_id, value);
    else
        pr_info("icc_fan_zone_curset_set OK - zone=%d setting_id=%d value=%08x\n", zone, setting_id, value);

    kfree(data);
    return ret;
}

// Set Target Temperature in ºC
int fan_set_target_temp(u8 temp) {
    // Set on Zone 0 == MainSoc
    return icc_fan_zone_curset_set(MAINSOC, TARGETTEMP, ((u32)temp)<<8);
}


int icc_fan_zone_temp_get(u8 zone)
{
    if (zone>3)
        return EINVAL;

    u8* data = (u8*) kzalloc(ICC_MSG_MAX_SIZE, GFP_KERNEL);
    memset(data, 0, ICC_MSG_MAX_SIZE);
    data[0] = zone;

    int ret = icc_query_wrapper(ICC_SERVICE_ID_THERMAL, 0x01, 0x20, data);
    if (ret)
        pr_err("icc_fan_zone_temp_get failed - ret=%04x zone=%d\n", ret, zone);
    else {
           pr_info("icc_fan_zone_temp_get OK - zone=%d\n", zone);
        pr_info("\n\nRead temperature zone %d : %02d.%02dºC\n\n", zone, data[3], data[2]*100/256);
    }

    kfree(data);
    return ret;
}
