#include "suble_common.h"
#include "wdt.h"




/*********************************************************************
 * LOCAL CONSTANT
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************
FN: 
*/
void suble_flash_init(void)
{
}

/*********************************************************
FN: 
*/
void suble_flash_read(uint32_t addr, uint8_t *buf, uint32_t size)
{
    wdt_feed(WATCH_DOG_COUNT);
    
    suble_enter_critical();
    flash_read(0, addr, size, buf, NULL);
    suble_exit_critical();
}

/*********************************************************
FN: 
*/
void suble_flash_write(uint32_t addr, uint8_t *buf, uint32_t size)
{
    wdt_feed(WATCH_DOG_COUNT);
    
    suble_enter_critical();
    flash_write(0, addr, size, (void*)buf, NULL);
    suble_exit_critical();
}

/*********************************************************
FN: 
*/
void suble_flash_erase(uint32_t addr, uint32_t num)
{
    wdt_feed(WATCH_DOG_COUNT);
    
    suble_enter_critical();
    flash_erase(0, addr, num*0x1000, NULL);
    suble_exit_critical();
}




















