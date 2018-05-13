#include <kernel.h>
#include "kernel_cfg.h"
#include "app.h"

#include "mbed.h"

#include "app_config.h"

#include "Zumo.h"

Serial pc(USBTX, USBRX);
Zumo zumo;

static int speed = 80;
static int steer = 0;

void task_main(intptr_t exinf) {
	zumo.driveTank(100, 100);
	dly_tsk(1000);
	zumo.driveTank(0, 0);
}
