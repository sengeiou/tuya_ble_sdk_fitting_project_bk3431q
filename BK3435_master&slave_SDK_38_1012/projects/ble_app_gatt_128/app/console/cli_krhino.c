/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "rwble.h"
#include "reg_blecore.h"
#include "BK_reg_Protocol.h"
#include "ke_mem.h"
#include "ke_event.h"
#include "compiler.h"
#include "lld_util.h"
#include "llm_util.h"
#include "appm.h"
#include "appm_task.h"
#include "appc.h"
#include "appc_task.h"
#include "master_app.h"
#include "cli.h"
#include "wdt.h"
#include "nvds.h"
#include "uart.h"
#include "icu.h"

#define CLI_DEBUG 1

#ifndef STDIO_UART
#define STDIO_UART 0
#endif

#define RET_CHAR  '\r'
#define END_CHAR  '\n'
#define PROMPT    "# "
#define EXIT_MSG  "exit"

#define HCI_COMMAND '\x01'

#if CLI_CONSOLE

/*static */struct cli_st *cli = NULL;
/*static */int            cliexit = 0; // 
char                  esc_tag[64] = {0};
static uint8_t        esc_tag_len = 0;



//aos_sem_t  g_cli_sem;
int cli_getchar(char *inbuf);

int cli_putstr(char *msg);
//static void hexstr2bin(const char *macstr, uint8_t *mac, int len);

/* Find the command 'name' in the cli commands table.
* If len is 0 then full match will be performed else upto len bytes.
* Returns: a pointer to the corresponding cli_command struct or NULL.
*/
static const struct cli_command *lookup_command(char *name, int len)
{
    int i = 0;
    int n = 0;

    while (i < MAX_COMMANDS && n < cli->num_commands) {
        if (cli->commands[i]->name == NULL) {
            i++;
            continue;
        }
        /* See if partial or full match is expected */
        if (len != 0) {
            if (!strncmp(cli->commands[i]->name, name, len)) {
                return cli->commands[i];
            }
        } else {
            if (!strcmp(cli->commands[i]->name, name)) {
                return cli->commands[i];
            }
        }

        i++;
        n++;
    }

    return NULL;
}

/* Parse input line and locate arguments (if any), keeping count of the number
* of arguments and their locations.  Look up and call the corresponding cli
* function if one is found and pass it the argv array.
*
* Returns: 0 on success: the input line contained at least a function name and
*          that function exists and was called.
*          1 on lookup failure: there is no corresponding function for the
*          input line.
*          2 on invalid syntax: the arguments list couldn't be parsed
*/
static int handle_input(char *inbuf,int len)
{
    struct {
        unsigned inArg: 1;
        unsigned inQuote: 1;
        unsigned done: 1;
    } stat;
    static char *argv[16];
    int argc = 0;
    int i = 0;
    const struct cli_command *command = NULL;
    const char *p;

    memset((void *)&argv, 0, sizeof(argv));
    memset(&stat, 0, sizeof(stat));

    
    do {
        switch (inbuf[i]) {
            case '\0':
                if (stat.inQuote) {
                    return 2;
                }
                stat.done = 1;
                break;

            case '"':
            case ',':
                if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg) {
                    memcpy(&inbuf[i - 1], &inbuf[i], strlen(&inbuf[i]) + 1);
                    --i;
                    break;
                }
                if (!stat.inQuote && stat.inArg) {
					stat.inArg = 0;
                    stat.inQuote = 0;
                    inbuf[i] = '\0';
                    break;
                }
                if (stat.inQuote && !stat.inArg) {
                    return 2;
                }

                if (!stat.inQuote && !stat.inArg) {
                    stat.inArg = 1;
                    stat.inQuote = 1;
                    argc++;
                    argv[argc - 1] = &inbuf[i + 1];
                } else if (stat.inQuote && stat.inArg) {
                    stat.inArg = 0;
                    stat.inQuote = 0;
                    inbuf[i] = '\0';
                }
                break;

            case ' ':
            case '=':
                if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg) {
                    memcpy(&inbuf[i - 1], &inbuf[i],
                           strlen(&inbuf[i]) + 1);
                    --i;
                    break;
                }
                if (!stat.inQuote && stat.inArg) {
                    stat.inArg = 0;
                    inbuf[i] = '\0';
                }
                break;

            default:
                if (!stat.inArg) {
                    stat.inArg = 1;
                    argc++;
                    argv[argc - 1] = &inbuf[i];
                }
                break;
        }
    } while (!stat.done && (++i < INBUF_SIZE));
    if (stat.inQuote) {
        return 2;
    }

    if (argc < 1) {
        return 0;
    }

    if (!cli->echo_disabled) {
        csp_printf("\r\n");
     //   fflush(stdout);
    }

    /*
    * Some comamands can allow extensions like foo.a, foo.b and hence
    * compare commands before first dot.
    */
    i = ((p = strchr(argv[0], '.')) == NULL) ? 0 : (p - argv[0]);

    command = lookup_command(argv[0], i);
    if (command == NULL) {
        return 1;
    }

    memset(cli->outbuf, 0, OUTBUF_SIZE);

    command->function(cli->outbuf, OUTBUF_SIZE, argc, argv);
    cli_putstr(cli->outbuf);

    return 0;
}

/* Perform basic tab-completion on the input buffer by string-matching the
 * current input line against the cli functions table.  The current input line
 * is assumed to be NULL-terminated.
 */
static void tab_complete(char *inbuf, unsigned int *bp)
{
    int i, n, m;
    const char *fm = NULL;

    aos_cli_printf("\r\n");

    /* show matching commands */
    for (i = 0, n = 0, m = 0; i < MAX_COMMANDS && n < cli->num_commands;
         i++) {
        if (cli->commands[i]->name != NULL) {
            if (!strncmp(inbuf, cli->commands[i]->name, *bp)) {
                m++;
                if (m == 1) {
                    fm = cli->commands[i]->name;
                } else if (m == 2)
                    aos_cli_printf("%s %s ", fm,
                                   cli->commands[i]->name);
                else
                    aos_cli_printf("%s ",
                                   cli->commands[i]->name);
            }
            n++;
        }
    }

    /* there's only one match, so complete the line */
    if (m == 1 && fm) {
        n = strlen(fm) - *bp;
        if (*bp + n < INBUF_SIZE) {
            memcpy(inbuf + *bp, fm + *bp, n);
            *bp += n;
            inbuf[(*bp)++] = ' ';
            inbuf[*bp] = '\0';
        }
    }

    /* just redraw input line */
    //aos_cli_printf("%s%s", PROMPT, inbuf);
}

/* Get an input line.
 *
 * Returns: 1 if there is input, 0 if the line should be ignored.
 */
static int get_input(char *inbuf, unsigned int *bp)
{
    //char c;
    //int  esc = 0, key1 = -1, key2 = -1;
    if (inbuf == NULL) {
        aos_cli_printf("inbuf_null\r\n");
        return 0;
    }

    cli->his_idx = (cli->his_cur + HIS_SIZE - 1) % HIS_SIZE;
    while (cli_getchar(&inbuf[*bp]) == 1) {
			#if 1
        if (inbuf[*bp] == RET_CHAR)//0d
				{					
					continue;
        		}
				if(inbuf[*bp] == END_CHAR)//0a
				{
					inbuf[*bp] = '\0';
					*bp = 0;
					uart_ringbuf_clean();
          return 1;
				}
			#else
			if(inbuf[*bp] == RET_CHAR || inbuf[*bp] == END_CHAR)/* end of input line */ 
			{
					inbuf[*bp] = '\0';
					*bp = 0;
          return 1;
			}
      #endif 
        if (inbuf[*bp] == HCI_COMMAND ) {   /* end of input line for hci command*/ // for bk reg tools                    
            //inbuf[*bp] = c;
            *bp = 0;
            return 1;
        }
#if 0
        if (inbuf[*bp] == 0x1b) { /* escape sequence */
            esc = 1;
            key1 = -1;
            key2 = -1;
            continue;
        }

        if (esc) {
            if (key1 < 0) {
                key1 = inbuf[*bp];
                if (key1 != 0x5b) {
                    /* not '[' */
                    inbuf[(*bp)] = 0x1b;
                    (*bp) ++;
                    inbuf[*bp] = key1;
                    (*bp) ++;
                    if (!cli->echo_disabled) {
                        csp_printf("\x1b%c", key1);
                      //  fflush(stdout);									   
                    }
                    esc = 0; /* quit escape sequence */
										
                }
                continue;
            }

            if (key2 < 0) {
                key2 = inbuf[*bp];
                if (key2 == 't') {
                    esc_tag[0] = 0x1b;
                    esc_tag[1] = key1;
                    esc_tag_len = 2;
                }
            }

            if (key2 != 0x41 && key2 != 0x42 && key2 != 't') {
                /*unsupported esc sequence*/
                inbuf[(*bp)] = 0x1b;
                (*bp) ++;
                inbuf[*bp] = key1;
                (*bp) ++;
                inbuf[*bp] = key2;
                (*bp) ++;
                if (!cli->echo_disabled) {
                    csp_printf("\x1b%c%c", key1, key2);
                   // fflush(stdout);
                }
                esc_tag[0] = '\x0';
                esc_tag_len = 0;
                esc = 0; /* quit escape sequence */
                continue;
            }

            if (key2 == 0x41) { /* UP */
                char *cmd = cli->history[cli->his_idx];
                cli->his_idx = (cli->his_idx + HIS_SIZE - 1) % HIS_SIZE;
                strncpy(inbuf, cmd, INBUF_SIZE);
                csp_printf("\r\n" PROMPT "%s", inbuf);
                *bp = strlen(inbuf);
                esc_tag[0] = '\x0';
                esc_tag_len = 0;
                esc = 0; /* quit escape sequence */
                continue;
            }

            if (key2 == 0x42) { /* DOWN */
                char *cmd = cli->history[cli->his_idx];
                cli->his_idx = (cli->his_idx + 1) % HIS_SIZE;
                strncpy(inbuf, cmd, INBUF_SIZE);
                csp_printf("\r\n" PROMPT "%s", inbuf);
                *bp = strlen(inbuf);
                esc_tag[0] = '\x0';
                esc_tag_len = 0;
                esc = 0; /* quit escape sequence */
                continue;
            }


            /* ESC_TAG */
            if (esc_tag_len >= sizeof(esc_tag)) {
                esc_tag[0] = '\x0';
                esc_tag_len = 0;
                esc = 0; /* quit escape sequence */
                csp_printf("Error: esc_tag buffer overflow\r\n");
//                fflush(stdout);
                continue;
            }
            esc_tag[esc_tag_len++] = inbuf[*bp];
            if (inbuf[*bp] == 'm') {
                esc_tag[esc_tag_len++] = '\x0';
                if (!cli->echo_disabled) {
                    csp_printf("%s", esc_tag);
//                    fflush(stdout);
                }
                esc = 0; /* quit escape sequence */
            }
            continue;
        }
#endif
        //inbuf[*bp] = c;
        if ((inbuf[*bp] == 0x08) || /* backspace */
            (inbuf[*bp] == 0x7f)) { /* DEL */
            if (*bp > 0) {
                (*bp)--;
                if (!cli->echo_disabled) {
                    csp_printf("%c %c", 0x08, 0x08);
//                    fflush(stdout);
                }
            }
            continue;
        }

        if (inbuf[*bp] == '\t') {
            inbuf[*bp] = '\0';
            tab_complete(inbuf, bp);
            continue;
        }

        if (!cli->echo_disabled) {
            csp_printf("%c", inbuf[*bp]);
//            fflush(stdout);
        }

        (*bp)++;
        if (*bp >= INBUF_SIZE) {
            aos_cli_printf("Error: input buffer overflow\r\n");
            //aos_cli_printf(PROMPT);
            *bp = 0;
            return 0;
        }
    }

    return 0;
}

/* Print out a bad command string, including a hex
 * representation of non-printable characters.
 * Non-printable characters show as "\0xXX".
 */
static void print_bad_command(char *cmd_string)
{
    if (cmd_string != NULL) {
        aos_cli_printf("command error\r\n", cmd_string);
        //aos_cli_printf("command '%s' not found\r\n", cmd_string);
    }
}

/* Main CLI processing thread
 *
 * Waits to receive a command buffer pointer from an input collector, and
 * then processes.  Note that it must cleanup the buffer when done with it.
 *
 * Input collectors handle their own lexical analysis and must pass complete
 * command lines to CLI.
 */

extern void send_bt_rx_data_process(void);
__STATIC void cli_main(void)
{

    ke_event_clear(KE_EVENT_AOS_CLI);
    //aos_cli_printf("cli_main\r\n");    
    if (!cliexit) 
    {		
        int ret;
        char *msg = NULL;

        if (get_input(cli->inbuf, &cli->bp)) {
            msg = cli->inbuf;
#if 0
            if (strcmp(msg, EXIT_MSG) == 0) {
                break;
            }
#endif
            if (strlen(cli->inbuf) > 0) {
                strncpy(cli->history[cli->his_cur], cli->inbuf, INBUF_SIZE);
                cli->his_cur = (cli->his_cur + 1) % HIS_SIZE;
            }

            ret = handle_input(msg,strlen(cli->inbuf));
			
            if (ret == 1) {
                print_bad_command(msg);
            } else if (ret == 2) {
                aos_cli_printf("syntax error\r\n");
            }

            if(cli->inbuf[0] != HCI_COMMAND) // for bk reg tools
            {   
                //aos_cli_printf("\r\n");
                esc_tag[0] = '\x0';
                esc_tag_len = esc_tag_len;
                esc_tag_len =0;
                //aos_cli_printf(PROMPT);
            }
        }
    }
    else
    {
        if(NULL != strstr((const char *)uart_rx_buf + uart_rx_index - 9,"AT+QDMO"))
        {
            cliexit = 0;
            uart_ringbuf_clean();
            aos_cli_printf("\r\nOK\r\n");///aos_cli_printf("DMO mode %x\r\n",uart_rx_index);
            return;
        }
        //send_bt_rx_data_process();
        //app_uart_clean();
    }
			
    //uart_ringbuf_clean();
}

/* ------------------------------------------------------------------------- */

int aos_cli_register_command(const struct cli_command *cmd)
{
    int i;

    if (!cli) {
        return 1;
    }

    if (!cmd->name || !cmd->function) {
        return -EINVAL;
    }

    if (cli->num_commands < MAX_COMMANDS) {
        /* Check if the command has already been registered.
         * Return 0, if it has been registered.
         */
        for (i = 0; i < cli->num_commands; i++) {
            if (cli->commands[i] == cmd) {
                return 0;
            }
        }
        cli->commands[cli->num_commands++] = cmd;
        return 0;
    }

    return -ENOMEM;
}
//AOS_EXPORT(int, aos_cli_register_command, const struct cli_command *);

int aos_cli_unregister_command(const struct cli_command *cmd)
{
    int i;
    if (!cmd->name || !cmd->function) {
        return -EINVAL;
    }

    for (i = 0; i < cli->num_commands; i++) {
        if (cli->commands[i] == cmd) {
            cli->num_commands--;
            int remaining_cmds = cli->num_commands - i;
            if (remaining_cmds > 0) {
                memmove(&cli->commands[i], &cli->commands[i + 1],
                        (remaining_cmds * sizeof(struct cli_command *)));
            }
            cli->commands[cli->num_commands] = NULL;
            return 0;
        }
    }

    return -ENOMEM;
}
//AOS_EXPORT(int, aos_cli_unregister_command, const struct cli_command *);

int aos_cli_register_commands(const struct cli_command *cmds, int num_cmds)
{
    int i;
    int err;
    for (i = 0; i < num_cmds; i++) {
        if ((err = aos_cli_register_command(cmds++)) != 0) {
            return err;
        }
    }

    return 0;
}
//AOS_EXPORT(int, aos_cli_register_commands, const struct cli_command *, int);

int aos_cli_unregister_commands(const struct cli_command *cmds, int num_cmds)
{
    int i;
    int err;
    for (i = 0; i < num_cmds; i++) {
        if ((err = aos_cli_unregister_command(cmds++)) != 0) {
            return err;
        }
    }

    return 0;
}
//AOS_EXPORT(int, aos_cli_unregister_commands, const struct cli_command *, int);

int aos_cli_stop(void)
{
    cliexit = 1;

    return 0;
}
//AOS_EXPORT(int, aos_cli_stop, void);


 //int cli_main(void)
int aos_cli_init(void)
{
    int ret;
    //aos_cli_printf("cli:0x%x \r\n",cli);
    if(cli == NULL)
    {
        cli = (struct cli_st *)ke_malloc(sizeof(struct cli_st),KE_MEM_NON_RETENTION);
    }
    
    if (cli == NULL) {
			 aos_cli_printf("Error0: Failed to create cli env: %d\r\n",
                       -ENOMEM);
			  aos_cli_printf("Error1: Failed to create cli env: %d\r\n",
                       -ENOMEM);
        return -ENOMEM;
    }else
    {
        //aos_cli_printf("cli:0x%x \r\n",cli);
                       
    }

    memset((void *)cli, 0, sizeof(struct cli_st));

    /* add our built-in commands */
    if ((ret = aos_cli_register_commands(&built_ins[0],
                                         sizeof(built_ins) / sizeof(struct cli_command))) != 0) {
        goto init_general_err;
    }
                                    

    cli->initialized = 1;
    cli->echo_disabled = 1;
    
    ke_event_callback_set(KE_EVENT_AOS_CLI, &cli_main);
    return 0;

init_general_err:
    if (cli) {
        ke_free(cli);
        cli = NULL;
    }
	aos_cli_printf("Error: return: %d\r\n",
                       ret);

    return ret;
}

const char *aos_cli_get_tag(void)
{
    return esc_tag;
}
//AOS_EXPORT(const char *, aos_cli_get_tag, void);

#if defined BUILD_BIN || defined BUILD_KERNEL
int aos_cli_printf(const char *msg, ...)
{
    va_list ap;

    char *pos, message[64];
    int sz;
    int len;

    memset(message, 0, 64);

    sz = 0;
    if (esc_tag_len) {
        strcpy(message, esc_tag);
        sz = strlen(esc_tag);
    }
    pos = message + sz;

    va_start(ap, msg);
    len = vsnprintf(pos, 64 - sz, msg, ap);
    va_end(ap);

    if (len <= 0) {
        return 0;
    }
	
    cli_putstr(message);

    return 0;
}
#endif

int cli_putstr(char *msg)
{
    if (msg[0] != 0) {
			uart_send((unsigned char *)msg,strlen(msg));
    }

    return 0;
}

int cli_getchar(char *inbuf)
{

	//*inbuf = Read_Uart_Buf();
    uint8_t len;
    len = read_uart_ringbuf_data((uint8_t*)inbuf,1);
	if(len != 0)
	{
		return 1;
	}else {
        return 0;
   }		
}

#ifndef csp_printf
__attribute__((weak)) int csp_printf(const char *fmt,...)
{
	
	va_list args;
    int ret;	
    
    char uart_buff[OUTBUF_SIZE];
    
	va_start(args, fmt);
	ret = vsprintf(uart_buff, fmt, args);	
	va_end(args);
    
    uart_putchar(uart_buff);
	if(ret > sizeof(uart_buff))
	{
		uart_putchar("buff full \r\n");
	}
		
    return ret;
}
#endif

#endif
