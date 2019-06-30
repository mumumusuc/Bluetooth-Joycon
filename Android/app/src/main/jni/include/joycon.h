#ifndef BT_PRO_JOYCON_H
#define BT_PRO_JOYCON_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define REPORT_OUTPUT_SIZE      48
#define REPORT_INPUT_SIZE       48
#define REPORT_INPUT_LARGE_SIZE 361

#define REPORT_INPUT_STANDARD_SIZE  (REPORT_INPUT_SIZE+1)
#define REPORT_INPUT_81_SIZE        REPORT_INPUT_STANDARD_SIZE
#define REPORT_INPUT_21_SIZE        REPORT_INPUT_STANDARD_SIZE
#define REPORT_INPUT_30_SIZE        REPORT_INPUT_STANDARD_SIZE
#define REPORT_INPUT_NFC_IR_SIZE    (REPORT_INPUT_LARGE_SIZE+1)
#define REPORT_INPUT_31_SIZE        REPORT_INPUT_NFC_IR_SIZE
#define REPORT_INPUT_32_SIZE        REPORT_INPUT_NFC_IR_SIZE
#define REPORT_INPUT_33_SIZE        REPORT_INPUT_NFC_IR_SIZE

enum NsDevType {
    DEV_JC_L = 1,
    DEV_JC_R,
    DEV_PRO
};

enum NsBatteryLevel {
    LEVEL_EMPTY = 0x00,
    LEVEL_CRITICAL = 0x20,
    LEVEL_LOW = 0x40,
    LEVEL_MEDIUM = 0x60,
    LEVEL_FULL = 0x80,
    LEVEL_CHARGING = 0x09
};

enum NsConnectionType {
    CON_PRO_CG = 0x00,
    CON_JC = 0x0E,
};

enum NsConnectionInfo {
    CON_BT = 0x00,
    CON_SPI_USB = 0x01,
};

/* buttons */
#define NS_BTN_MASK(N)    (0x00000001 << (N))

#define NS_BTN_MASK_Y      NS_BTN_MASK(0)
#define NS_BTN_MASK_X      NS_BTN_MASK(1)
#define NS_BTN_MASK_B      NS_BTN_MASK(2)
#define NS_BTN_MASK_A      NS_BTN_MASK(3)
#define NS_BTN_MASK_LSR    NS_BTN_MASK(4)
#define NS_BTN_MASK_LSL    NS_BTN_MASK(5)
#define NS_BTN_MASK_R      NS_BTN_MASK(6)
#define NS_BTN_MASK_ZR     NS_BTN_MASK(7)
#define NS_BTN_MASK_MINUS  NS_BTN_MASK(8)
#define NS_BTN_MASK_PLUS   NS_BTN_MASK(9)
#define NS_BTN_MASK_RS     NS_BTN_MASK(10)
#define NS_BTN_MASK_LS     NS_BTN_MASK(11)
#define NS_BTN_MASK_HOME   NS_BTN_MASK(12)
#define NS_BTN_MASK_CAP    NS_BTN_MASK(13)
#define NS_BTN_MASK_NOP    NS_BTN_MASK(14)
#define NS_BTN_MASK_CG     NS_BTN_MASK(15)
#define NS_BTN_MASK_DOWN   NS_BTN_MASK(16)
#define NS_BTN_MASK_UP     NS_BTN_MASK(17)
#define NS_BTN_MASK_RIGHT  NS_BTN_MASK(18)
#define NS_BTN_MASK_LEFT   NS_BTN_MASK(19)
#define NS_BTN_MASK_RSR    NS_BTN_MASK(20)
#define NS_BTN_MASK_RSL    NS_BTN_MASK(21)
#define NS_BTN_MASK_L      NS_BTN_MASK(22)
#define NS_BTN_MASK_ZL     NS_BTN_MASK(23)

typedef struct jc_input {
    uint32_t buttons;
    uint16_t sticks[4];
    int16_t axes[18];
    uint8_t nfc[64];
    uint8_t ir[64];
} jc_input_t;
#define JC_INPUT_SIZE sizeof(jc_input_t)    // 4+8+36+64+64=176

typedef struct jc_status {
    uint8_t dev_type;
    uint8_t con_type;
    uint8_t battery;
    uint8_t vibrate;
    uint8_t player;
    uint16_t voltage;
    bool enable_imu;
    bool enable_vib;
    bool enable_nfc;
    bool enable_ir;
    bool start_push;
} jc_status_t;
#define JC_STATUS_SIZE sizeof(jc_status_t)  // 1+1+1+1+1+2+?+?+?+?+?=???

typedef struct joycon {
    jc_input_t *input;
    jc_status_t *status;
    int spi_fd;
    uint8_t timer;
    /* for Android asset */
    off_t offset;
} joycon_t;

int jc_init(joycon_t *);

void jc_free(joycon_t *);

int jc_replay_output_report(joycon_t *, uint8_t *, uint8_t *);

int jc_makeup_input_report(joycon_t *, uint8_t *);

#endif //BT_PRO_JOYCON_H
