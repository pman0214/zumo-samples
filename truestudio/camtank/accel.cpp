#include <kernel.h>
#include "kernel_cfg.h"
#include "accel.h"

#include "mbed.h"

#include "app_config.h"

#include "Zumo.h"

DigitalOut led(D7);

Serial pc(USBTX, USBRX);
Zumo zumo;
static short x, y, z;

void task_main(intptr_t exinf) {
    pc.baud(57600);
    pc.printf("Ready\r\n");
}

void cyc_timer(intptr_t exinf) {
    iact_tsk(TASKID_ACCEL);
}

void task_accel(intptr_t exinf) {
    led = 1;
    zumo.getAcceleration(&x, &y, &z);
    pc.printf("%6d,%6d,%6d\r\n", x, y, z);
    led = 0;
    ext_tsk();
}
