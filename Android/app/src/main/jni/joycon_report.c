#include "include/joycon.h"
#include "include/crc8.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <memory.h>


//for more details, please read @https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/USB-HID-Notes.md

#define put_mac_be(buf, mac)    \
    *(buf+0) = mac[0];          \
    *(buf+1) = mac[1];          \
    *(buf+2) = mac[2];          \
    *(buf+3) = mac[3];          \
    *(buf+4) = mac[4];          \
    *(buf+5) = mac[5]

#define put_mac_le(buf, mac)    \
    *(buf+0) = mac[5];          \
    *(buf+1) = mac[4];          \
    *(buf+2) = mac[3];          \
    *(buf+3) = mac[2];          \
    *(buf+4) = mac[1];          \
    *(buf+5) = mac[0]

#define FWV     0x0389  //0x03A6
static const uint8_t MAC_BE[] = {0xdc, 0x68, 0xeb, 0x30, 0x7f, 0x07};

static inline ssize_t read_spi_mem(int fd, off_t addr, size_t len, uint8_t *buf) {
    ssize_t ret;
    ret = lseek(fd, addr, SEEK_SET);
    if (ret < 0)
        return ret;
    ret = read(fd, buf, len);
    if (ret < 0)
        return ret;
    return len;
}

static inline ssize_t write_spi_mem(int fd, uint16_t addr, size_t len, uint8_t *buf) {
    ssize_t ret;
    ret = lseek(fd, addr, SEEK_SET);
    if (ret < 0)
        return ret;
    ret = write(fd, buf, len);
    if (ret < 0)
        return ret;
    return len;
}

/* 80 01 */
static inline void report_output_80_01(uint8_t *report) {
    report[1] = 0x01;
    report[2] = 0;
    report[3] = DEV_PRO;
    put_mac_le(report + 4, MAC_BE);
}

/* 80 02 */
static inline void report_output_80_02(uint8_t *report) {
    report[1] = 0x02;
}

/* 80 03 */
static inline void report_output_80_03(uint8_t *report) {
    report[1] = 0x03;
}

/* 80 04 */
static inline void report_output_80_04(uint8_t *report) {
    report[1] = 0x04;
}

/* 80 05 */
static inline void report_output_80_05(uint8_t *report) {
    report[1] = 0x05;
}

/* 80 06 */
static inline void report_output_80_06(uint8_t *report) {
}


static int report_output_80(joycon_t *jc, uint8_t *output, uint8_t *input) {
    uint8_t *report = input;
    switch (output[1]) {
        case 0x01:
            report_output_80_01(report);
            break;
        case 0x02:
            report_output_80_02(report);
            break;
        case 0x03:
            report_output_80_03(report);
            break;
        case 0x04:
            report_output_80_04(report);
            jc->status->start_push = true;
            break;
        case 0x05:
            report_output_80_05(report);
            break;
        case 0x06:
            report_output_80_06(report);
            break;
        default:
            return -EINVAL;
    }
    report[0] = 0x81;
    return REPORT_INPUT_81_SIZE;
}

/* Sub-command 0x01: Bluetooth manual pairing */
static inline int report_output_01_01(uint8_t cmd, uint8_t *report) {
    report[13] = 0x81;
    report[14] = 0x01;
    switch (cmd) {
        case 0x01:
            report[15] = 0x01;
            put_mac_le(report + 16, MAC_BE);
            break;
        case 0x02: {
            /* Long Term Key (LTK) in Little-Endian. Each byte is XORed with 0xAA. */
            //NS:?              -> JC:cc 30 97 64 91 f4 20 af a5 54 e0 1e 6d 61 6d b7
            //NS:b0 f5 b8 d6 77 -> JC:16 bc 66 aa aa bc ac 11 97 a5 ca 75 ff 1a f5 d9
            //NS:b0 e5 78 5e 6e -> JC:49 69 be 19 e8 ab e4 1a d8 c6 64 7c 13 9d 93 26
            //NS:e5 b8 40 12 00 00 00 00 2b 63 02 65 00 00 00 e4 4c 8d 40 12 00 00 00 d8 e4 b8 40 12 00 00 00
            //   b0 e5 b8 40 12 -> JC:3f 2b 6f f0 c2 74 a7 2b b5 b4 fd 1a 7e 6c 84 e3
            uint8_t tmp[] = {0x49, 0x69, 0xbe, 0x19, 0xe8, 0xab, 0xe4, 0x1a, 0xd8, 0xc6, 0x64, 0x7c, 0x13, 0x9d, 0x93,
                             0x26};
            report[15] = 0x02;
            memmove(report + 16, tmp, sizeof(tmp));
            break;
        }
        case 0x03: {
            /* Joy-Con saves pairing info in x2000 SPI region. */
            report[15] = 0x03;
            break;
        }
        case 0x04: {
            /* MAC + name */
            report[15] = 0x01;
            put_mac_le(report + 16, MAC_BE);
            uint8_t tmp[] = {0x00, 0x25, 0x08, 0x4a, 0x6f, 0x79, 0x2d, 0x43, 0x6f, 0x6e, 0x20, 0x28, 0x52, 0x29, 0x68};
            memmove(report + 22, tmp, sizeof(tmp));
            break;
        }
        default:
            return -1;
    }
    return 23;
}

/* Sub-command 0x02: Request device info */
static inline int report_output_01_02(uint8_t *report) {
    report[13] = 0x82;
    report[14] = 0x02;
    report[15] = (FWV >> 8) & 0xFF;
    report[16] = (FWV >> 0) & 0xFF;
    report[17] = DEV_PRO;
    report[18] = 0x02;
    put_mac_be(report + 19, MAC_BE);
    report[25] = 0x03;
    report[26] = 0x01;
    return 27;
}

/* Sub-command 0x03: Set input report mode */
static inline int report_output_01_03(jc_status_t *status, uint8_t cmd, uint8_t *report) {
    report[13] = 0x80;
    report[14] = 0x03;
    switch (cmd) {
        case 00:
            /* Used with cmd x11. Active polling for NFC/IR camera data. 0x31 data format must be set first. */
        case 01:
            /* Same as 00. Active polling mode for NFC/IR MCU configuration data. */
        case 02:
            /* Same as 00. Active polling mode for NFC/IR data and configuration. For specific NFC/IR modes */
        case 03:
            /* Same as 00. Active polling mode for IR camera data. For specific IR modes */
        case 23:
            /* MCU update state report? */
            return -1;
        case 0x30:
            /* Standard full mode. Pushes current state @60Hz */
            status->start_push = true;
            break;
        case 0x31:
            /* NFC/IR mode. Pushes large packets @60Hz */
            status->enable_nfc = true;
            break;
            //standard_report_id = 0x31;
        case 0x33:
            /* Unknown mode. */
        case 0x35:
            /* Unknown mode. */
        case 0x3F:
            /* Simple HID mode. Pushes updates with every button press */
            break;
        default:
            return -1;
    }
    return 15;
}

/* Sub-command 0x04: Trigger buttons elapsed time */
static inline int report_output_01_04(uint8_t *report) {
    uint8_t tmp[] = {0x00, 0x00, 0xfe, 0x86, 0x00, 0x00, 0xff, 0xff, 0x55, 0x00, 0x56, 0x00, 0x00, 0x00};
    report[13] = 0x83;
    report[14] = 0x04;
    memmove(report + 15, tmp, sizeof(tmp));
    return 29;
}

/* Sub-command 0x05: Get page list state */
static inline int report_output_01_05(uint8_t *report) {
    return -1;
}

/* Sub-command 0x06: Set HCI state (disconnect/page/pair/turn off) */
//Causes the controller to change power state.
//x00	Disconnect (sleep mode / page scan mode)
//x01	Reboot and Reconnect (page mode)
//x02	Reboot and enter Pair mode (discoverable)
//x04	Reboot and Reconnect (page mode / HOME mode?)
// 01 0c 00 00 00 00 00 00 00 00 06 00 00 00 00 00 00
static inline int report_output_01_06(uint8_t *report) {
    return -1;
}

/* Sub-command 0x07: Reset pairing info */
static inline int report_output_01_07(uint8_t *report) {
    return -1;
}

/* Sub-command 0x08: Set shipment low power state */
static inline int report_output_01_08(uint8_t *report) {
    report[13] = 0x80;
    report[14] = 0x08;
    return 15;
}

/* Sub-command 0x10: SPI flash read */
// see @https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/spi_flash_notes.md
/* 0x6050 : color reg
 * #323232 - black
 * #313232 - Splatoon
 * #323132 - Xenoblade
 */
static inline int report_output_01_10(int fd, off_t offset, uint8_t *args, uint8_t *report) {
    report[13] = 0x90;
    report[14] = 0x10;
    report[15] = args[0];
    report[16] = args[1];
    report[19] = args[4];
    uint16_t addr = (uint16_t) ((args[0] & 0x00FF) | ((args[1] << 8) & 0xFF00));
    if (0 > read_spi_mem(fd, offset + addr, args[4], report + 20))
        return -errno;
    return 20 + args[4];
}

/* Sub-command 0x11: SPI flash Write */
static inline int report_output_01_11(uint8_t *report) {
    return -1;
}

/* Sub-command 0x12: SPI sector erase */
static inline int report_output_01_12(uint8_t *report) {
    return -1;
}

/* Sub-command 0x21: Set NFC/IR MCU configuration */
static inline int report_output_01_21(uint8_t *report, uint8_t arg) {
    // 21 21 ->
    // a0 21 01 00 00 00 06 00 1a 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 4c
    report[13] = 0xA0;
    report[14] = 0x21;
    if (arg == 0x21) {
        //01 00 00 00 06 00 1a 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 4c
        //01 00 00 00 06 00 1a 01 00 00 00 00 00 00 00
        report[15] = 0x01;
        report[19] = 0x06;
        report[21] = 0x1A;
        report[22] = 0x01;
        report[48] = crc8(report + 15, 33);
    } else
        memset(report + 15, 0xA5, 34);
    return 15;
}

/* Sub-command 0x22: Set NFC/IR MCU state */
static inline int report_output_01_22(uint8_t arg, uint8_t *report) {
    report[13] = 0x80;
    report[14] = 0x22;
    //report[15] = arg;
    return 15;
}

/* Sub-command 0x30: Set player lights */
static inline int report_output_01_30(jc_status_t *status, uint8_t player, uint8_t *report) {
    report[13] = 0x80;
    report[14] = 0x30;
#define PLAYER_1    0x01
#define PLAYER_2    0x02
#define PLAYER_3    0x04
#define PLAYER_4    0x08
    if (player & PLAYER_4)
        status->player = 4;
    else if (player & PLAYER_3)
        status->player = 3;
    else if (player & PLAYER_2)
        status->player = 2;
    else if (player & PLAYER_1)
        status->player = 1;
    else
        status->player = 0;
    return 15;
}

/* Sub-command 0x38: Set HOME Light */
static inline int report_output_01_38(uint8_t *report) {
    // TODO
    return -1;
}

/* Sub-command 0x40: Enable IMU (6-Axis sensor) */
static inline int report_output_01_40(jc_status_t *status, uint8_t arg, uint8_t *report) {
    report[13] = 0x80;
    report[14] = 0x40;
    status->enable_imu = arg;
    return 16;
}

/* Sub-command 0x41: Set IMU sensitivity */
static inline int report_output_01_41(uint8_t *report) {
    return -1;
}

/* Sub-command 0x43: Read IMU registers */
static inline int report_output_01_43(uint8_t *args, uint8_t *report) {
    report[13] = 0xC0;
    report[14] = 0x43;
    report[15] = args[0];
    report[16] = args[1];
    return 17;
}

/* Sub-command 0x48: Enable vibration */
static inline int report_output_01_48(jc_status_t *status, uint8_t arg, uint8_t *report) {
    status->vibrate = 0x0C;
    report[13] = 0x80;
    report[14] = 0x48;
    status->enable_vib = arg;
    return 15;
}

/* Sub-command 0x50: Get regulated voltage */
static inline int report_output_01_50(jc_status_t *status, uint8_t *report) {
    report[13] = 0xD0;
    report[14] = 0x50;
    report[15] = (uint8_t) ((status->voltage >> 8) & 0xFF);
    report[16] = (uint8_t) ((status->voltage >> 0) & 0xFF);
    return 17;
}

/*
 * response with input_21
 * see @pro-hid-descriptor.txt
 */
static int report_output_01(joycon_t *jc, uint8_t *output, uint8_t *input) {
    int ret;
    uint8_t subcmd = output[10];
    uint8_t *args = &output[11];
    uint8_t *report = input;
    switch (subcmd) {
        case 0x01:
            ret = report_output_01_01(args[0], report);
            break;
        case 0x02:
            ret = report_output_01_02(report);
            break;
        case 0x03:
            ret = report_output_01_03(jc->status, args[0], report);
            break;
        case 0x04:
            ret = report_output_01_04(report);
            break;
        case 0x05:
            ret = report_output_01_05(report);
            break;
        case 0x06:
            ret = report_output_01_06(report);
            break;
        case 0x07:
            ret = report_output_01_07(report);
            break;
        case 0x08:
            ret = report_output_01_08(report);
            break;
        case 0x10:
            ret = report_output_01_10(jc->spi_fd, jc->offset, args, report);
            break;
        case 0x11:
            ret = report_output_01_11(report);
            break;
        case 0x12:
            ret = report_output_01_12(report);
            break;
        case 0x21:
            ret = report_output_01_21(report, args[0]);
            break;
        case 0x22:
            ret = report_output_01_22(args[0], report);
            break;
        case 0x30:
            ret = report_output_01_30(jc->status, args[0], report);
            break;
        case 0x38:
            ret = report_output_01_38(report);
            break;
        case 0x40:
            ret = report_output_01_40(jc->status, args[0], report);
            break;
        case 0x41:
            ret = report_output_01_41(report);
            break;
        case 0x43:
            ret = report_output_01_43(args, report);
            break;
        case 0x48:
            ret = report_output_01_48(jc->status, args[0], report);
            break;
        case 0x50:
            ret = report_output_01_50(jc->status, report);
            break;
        default:
            return -EINVAL;
    }
    if (ret > 0) {
        report[0] = 0x21;
        return REPORT_INPUT_21_SIZE;
    }
    return ret;
}

static int report_output_10(joycon_t *jc, uint8_t *output, uint8_t *input) {
    if (++jc->status->vibrate > 0xFF)
        jc->status->vibrate = 0;
    input[0] = 0x30;
    return 0;
}

// 11 05 00 00 00 00 00 00 00 00 01 00
static int report_output_11(joycon_t *jc, uint8_t *output, uint8_t *input) {
    uint8_t subcmd = output[10];
    if (subcmd == 0x01) {
        /* 0x01 == nfc ? */
        jc->status->enable_nfc = true;
    }
    input[0] = 0x31;
    return 0;
}

static inline void update_buttons(uint32_t buttons, uint8_t *input) {
    input[0] = (uint8_t) ((buttons >> 0) & 0xFF);
    input[1] = (uint8_t) ((buttons >> 8) & 0xFF);
    input[2] = (uint8_t) ((buttons >> 16) & 0xFF);
}

static inline void update_sticks(uint16_t *sticks, uint8_t *input) {
    input[0] = (uint8_t) (sticks[0] & 0xFF);
    input[1] = (uint8_t) (((sticks[0] >> 8) & 0x0F) | ((sticks[1] << 4) & 0xF0));
    input[2] = (uint8_t) ((sticks[1] >> 4) & 0xFF);
    input[3] = (uint8_t) (sticks[2] & 0xFF);
    input[4] = (uint8_t) (((sticks[2] >> 8) & 0x0F) | ((sticks[3] << 4) & 0xF0));
    input[5] = (uint8_t) ((sticks[3] >> 4) & 0xFF);
}

static inline void update_axes(int16_t *axes, uint8_t *input) {
#define LINE_LEN    6
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 6; j++) {
            /* gravity */
            input[j + i * 12] = (uint8_t) ((axes[i * LINE_LEN + j / 2] >> (j % 2) * 8) & 0xFF);
            /* gyro */
            input[j + 6 + i * 12] = (uint8_t) ((axes[i * LINE_LEN + j / 2 + 3] >> (j % 2) * 8) & 0xFF);
        }
    }
}

int jc_replay_output_report(joycon_t *jc, uint8_t *output, uint8_t *input) {
    uint8_t report_id = output[0];
    switch (report_id) {
        case 0x01:
            /* replay with input report 0x21 */
            return report_output_01(jc, output, input);
        case 0x80:
            /* replay with input report 0x81 */
            return report_output_80(jc, output, input);
        case 0x10:
            /* replay rumble */
            return report_output_10(jc, output, input);
        case 0x11:
            /* nfc/ir */
            return report_output_11(jc, output, input);
        default:
            break;
    }
    return -EINVAL;
}

static inline bool check_input_report_id(uint8_t report_id) {
    // TODO : check report_id legality
    return true;
}

int jc_makeup_input_report(joycon_t *jc, uint8_t *input) {
    uint8_t report_id = input[0];
    if (!check_input_report_id(report_id))
        return -1;
    if (input[0] == 0x81)
        return REPORT_INPUT_81_SIZE;
    input[1] = jc->timer;
    if (++jc->timer > 0xFF)
        jc->timer = 0x00;
    input[2] = jc->status->battery | jc->status->con_type;
    input[12] = (uint8_t) (jc->status->vibrate & 0x0F);
    update_buttons(jc->input->buttons, input + 3);
    update_sticks(jc->input->sticks, input + 6);
    if (input[0] == 0x21)
        return REPORT_INPUT_21_SIZE;
    if (jc->status->enable_imu)
        update_axes(jc->input->axes, input + 13);
    if (input[0] == 0x31) {
        if (jc->status->enable_nfc) {
            if (jc->timer % 2) {
                input[49] = 0xFF;
            } else {
                // 01 00 ff 00 06 00 1a 06
                // 01 00 00 00 06 00 1a 01
                input[49] = 0x01;
                input[50] = 0x00;
                input[51] = 0x00;
                input[52] = 0x00;
                input[53] = 0x06;
                input[54] = 0x00;
                input[55] = 0x1A;
                input[56] = 0x01;
            }
            input[REPORT_INPUT_31_SIZE - 1] = crc8(input + 49, REPORT_INPUT_31_SIZE - 50);
        }
        return REPORT_INPUT_31_SIZE;
    }
    return REPORT_INPUT_STANDARD_SIZE;
}