#ifndef _MILKCOCOA_COORDINATE_
#define _MILKCOCOA_COORDINATE_

/*======================================================================
 * includes
 *======================================================================*/

/*======================================================================
 * typedefs, structures
 *======================================================================*/
typedef unsigned char   uint8_t;

/*======================================================================
 * constants
 *======================================================================*/
#define TASKPRI_MAIN        8
#define TASKPRI_ST_TRAN     8
#define TASKPRI_ACC_MON     2
#define TASKPRI_NTF_MLK     2

#define TASKSTACKSIZE       (1024)

#ifndef KMM_SIZE
#define KMM_SIZE    (TASKSTACKSIZE * 32)    /* カーネルが割り付ける */
#endif /* KMM_SIZE */                       /* メモリ領域のサイズ */

/*======================================================================
 * typedefs, structures
 *======================================================================*/
/**
 * @enum
 *      status flags.
 */
typedef enum statues
{
    ST_NONE = -1,               /**< no state transition */
    ST_INIT,                    /**< initializing */
    ST_ERR,                     /**< fatal error */
    ST_STOP,                    /**< zumo stop */
    ST_FORWARD,                 /**< zumo going forward */
    ST_END
} state_t;

typedef enum event_types
{
    EV_INIT_DONE,               /**< initialization completed */
    EV_ACC_DETECT,              /**< detected vibration */
    EV_FATAL_ERR,               /**< fatal error occurs */
    EV_FORCE_STOP,              /**< force stop event */
    EV_CTRL_FWD,                /**< forward controlled by other node */
    EV_END
} event_t;

/**
 * @struct
 *      state transition function table structure.
 */
typedef struct tran_tbl_strct
{
    int (*func)();              /**< state-entry function */
} tran_tbl_t;

/*======================================================================
 * functions, variables
 *======================================================================*/
#ifdef __cplusplus
extern "C" {
#endif

extern void task_main(intptr_t exinf);
extern void task_st_tran(intptr_t exinf);
extern void task_acc_mon(intptr_t exinf);
extern void task_ntf_mlk(intptr_t exinf);
extern void cyc_timer(intptr_t exinf);
extern void cyc_led_blink(intptr_t exinf);
extern void alm_stop(intptr_t exinf);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MILKCOCOA_COORDINATE_ */
