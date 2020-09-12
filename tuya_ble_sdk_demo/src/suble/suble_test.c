#include "suble_common.h"




/*********************************************************************
 * LOCAL CONSTANT
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */
static void* lock_timer[2];


/*********************************************************************
 * VARIABLE
 */
#define TUYA_TEST_LEN  32
#define TUYA_TEST_ADDR 0x70000

uint8_t tmpBuf1[TUYA_TEST_LEN] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x00};
uint8_t tmpBuf2[TUYA_TEST_LEN] = {0};

/*********************************************************************
 * LOCAL FUNCTION
 */




static void suble_test_outtime_cb(void* timer)
{
    SUBLE_PRINTF("1");
}
/*********************************************************
FN: 
*/
void suble_test_func(void)
{
//    suble_timer_create(&lock_timer[0], 1000, SUBLE_TIMER_REPEATED, suble_test_outtime_cb);
//    suble_timer_start(lock_timer[0]);
}









