#ifndef __APP_API_H__
#define __APP_API_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "uart.h"
#include "gap.h"

#define BUILD_INS_NUM 17
#define USER_COMMANDS_NUM (BUILD_INS_NUM-3)


//extern const struct cli_command built_ins[BUILD_INS_NUM];

enum appm_error set_ble_mac(struct gap_bdaddr mac);///(uint8_t *mac);
void enter_DMO_mode(uint8_t channal);

void quit_DMO_mode(void);
void ble_name_ini(void);
#endif
