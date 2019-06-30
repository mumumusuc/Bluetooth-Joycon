#include "include/joycon.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

int jc_init(joycon_t *jc) {
    assert(jc->spi_fd >= 0);
    assert(jc->status);
    assert(jc->input);
    jc_status_t *status = jc->status;
    status->voltage = 0x0690;
    status->con_type = CON_PRO_CG | CON_BT;
    status->battery = LEVEL_FULL;
    status->dev_type = DEV_PRO;
    status->vibrate = 0x09;
    status->enable_imu = false;
    status->start_push = false;
    return 0;
}

void jc_free(joycon_t *jc) {
    if (!jc)
        return;
    close(jc->spi_fd);
    //free(jc->input);
    //free(jc->status);
}