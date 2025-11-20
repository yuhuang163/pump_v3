/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "hal_trace.h"
#include "plat_types.h"
#include "eshell.h"

#define ESHELL_MAXARGS 16
#define ESHELL_BUFF_SIZE 128
#define ESHELL_CHAR_DEL 0x8
#define ESHELL_CHAR_CR '\r'
#define ESHELL_CHAR_LF '\n'
#ifdef SYSTEM_USE_FILE
#define ESHELL_CHAR_END ']'
#endif

typedef struct {
    char debug;
    char echo;
    char running;
    bool need_open_uart;
    int ext_cmd_count;
    unsigned int buff_offset;
    char buff[ESHELL_BUFF_SIZE];
    eshell_cmd_t *ext_cmds;
} eshell_context_t;

static eshell_rpc_run_cmd_func eshell_rpc_run_op_cmd_func_cb = NULL;
static eshell_rpc_run_cmd_func eshell_rpc_run_show_result_func_cb = NULL;
extern unsigned int __eshell_command_table_start[];
extern unsigned int __eshell_command_table_end[];

static eshell_context_t context;

static void help_cmd(int argc, char *argv[]);
static void echo_cmd(int argc, char *argv[]);
static void log_cmd(int argc, char *argv[]);

static const eshell_cmd_t builtin_cmds[] = {
    {ESHELL_CMD_GRP_BUILT_IN, "help", "get all cmds, usage: help -L or help -R", help_cmd},
    {ESHELL_CMD_GRP_BUILT_IN, "echo", "switch on/off command echo", echo_cmd},
    {ESHELL_CMD_GRP_BUILT_IN, "log", "switch on/off debug log output", log_cmd},
    {ESHELL_CMD_GRP_BUILT_IN, NULL, NULL, NULL},
};

// Notes: the str_table index need keep same witch eshell_cmd_grp_id_e definition.
#undef ESHELL_GRP_DEFINE
#define ESHELL_GRP_DEFINE(str) #str
static const char *const grp_id_str_table[] = {
    #include "eshell_grp.list"
};

static const char *grp_id_to_str(unsigned int grp_id)
{
    const char *str;
    if (grp_id < ESHELL_CMD_GRP_ID_MAXNUM) {
        str = grp_id_str_table[grp_id];
    } else {
        str = "GRP_UNKNOW";
    }
    return str;
}

static inline bool check_param_count_valid(uint8_t excepted_cnt, uint8_t valid_cnt,
                                           const char *warn_str)
{
    if (valid_cnt < (excepted_cnt + 1)) {
        eshell_putstring("%s:wrong param number,expect=%d,give=%d!", warn_str, excepted_cnt + 1,
                         valid_cnt);
        return false;
    }
    return true;
}

static void help_cmd(int argc, char *argv[])
{
    eshell_cmd_t *cmds = NULL;
    unsigned int i = 0, j = 0;
    unsigned int grp_id = 0, grp_id_curr = 0;
    eshell_putstring("\n-- Usage:"
                     "\r\n");

    i = 0;
    cmds = (eshell_cmd_t *)builtin_cmds;
    eshell_putstring("    [built-in]"
                     "\r\n");
    while (cmds[i].cmd_str != NULL) {
        eshell_putstring("        %s"
                         "\r\n",
                         cmds[i].cmd_str);
        if (cmds[i].help_str != NULL)
            eshell_putstring("          - %s"
                             "\r\n",
                             cmds[i].help_str);
        eshell_putstring("\r\n");
        ++i;
    }

    cmds = context.ext_cmds;
    for (i = 0; i < context.ext_cmd_count; ++i) {
        grp_id = cmds[i].grp_id;
        if (cmds[i].disp || (grp_id >= ESHELL_CMD_GRP_ID_MAXNUM))
            continue;

        eshell_putstring("    [%s]"
                         "\r\n",
                         grp_id_to_str(grp_id));

        for (j = 0; j < context.ext_cmd_count; ++j) {
            grp_id_curr = cmds[j].grp_id;
            if (cmds[j].disp || (grp_id_curr >= ESHELL_CMD_GRP_ID_MAXNUM))
                continue;
            if (grp_id != grp_id_curr)
                continue;
            eshell_putstring("        %s"
                             "\r\n",
                             cmds[j].cmd_str);
            if (cmds[j].help_str != NULL)
                eshell_putstring("          - %s"
                                 "\r\n",
                                 cmds[j].help_str);
            eshell_putstring("\r\n");
            cmds[j].disp = 1;
        }
    }
    for (i = 0; i < context.ext_cmd_count; ++i) {
        cmds[i].disp = 0;
    }
    // eshell_putstring("-- Info:""\r\n");
    // eshell_putstring("    command echo =[%d]""\r\n", context.echo);
    // eshell_putstring("    ext_cmd_count=[%d]""\r\\n", context.ext_cmd_count);
    // eshell_putstring("    ext_cmds     =[%p]""\r\n", context.ext_cmds);

    eshell_putstring("-- Example:"
                     "\r\n");
    eshell_putstring("    add 1,2"
                     "\r\n");
}

static void echo_cmd(int argc, char *argv[])
{
    char *action = NULL;

    if (!check_param_count_valid(1, argc, "echo_cmd")) {
        return;
    }

    if (argc < 2) {
        eshell_putstring("eshell:echo_cmd,need 2 args");
        return;
    }

    action = argv[1];
    TRACE(1, "eshell:echo_cmd,aciton=%s", action);

    if ((strncmp("on", action, 2)) == 0) {
        eshell_putstring("echo on");
        context.echo = 1;
    } else if ((strncmp("off", action, 3)) == 0) {
        eshell_putstring("echo off");
        context.echo = 0;
    } else {
        eshell_putstring("echo:wrong param!");
    }
}

static void log_cmd(int argc, char *argv[])
{
    char *action = NULL;
    char *param = argv[0];

    if (argc < 2) {
        eshell_putstring("eshell:log_cmd,need 2 args");
        return;
    }

    ESHELL_PARAM_CHECK_COUNT(param, 1, "log_cmd");
    action = argv[1];
    TRACE(1, "eshell:log_cmd,aciton=%s", action);

    if ((strncmp("on", action, 2)) == 0) {
        eshell_putstring("debug log on");
        hal_trace_output_enable(true);
    } else if ((strncmp("off", action, 3)) == 0) {
        eshell_putstring("debug log off");
        hal_trace_output_enable(false);
    } else {
        eshell_putstring("echo:wrong param!");
    }
}

// helper API
static char *_my_strstr(const char *str, const char pt)
{
    int i = 0;
    int len = strlen(str);
    for (i = 0; i < len; ++i) {
        if (str[i] == pt)
            break;
    }
    if (i == len)
        return NULL;
    return (char *)&str[i];
}

static char _my_toupper(char ch)
{
    if (ch >= 'a' && ch <= 'z') {
        return ch - 'a' + 'A';
    }

    return ch;
}

static int _my_is_digit(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return 1;
    }

    return 0;
}

static unsigned char _my_noprefix_2byte_hexstr_to_uchar(char *str)
{
    unsigned char v = 0;

    if (_my_is_digit(str[0])) {
        v |= (str[0] - '0') << 4;
    } else {
        v |= (_my_toupper(str[0]) - 'A' + 10) << 4;
        ;
    }

    if (_my_is_digit(str[1])) {
        v |= (str[1] - '0');
    } else {
        v |= (_my_toupper(str[1]) - 'A' + 10);
    }

    return v;
}

char *eshell_param_get_start(char *params, int param_index)
{
    int counter = 0;
    char *p = (char *)params;
    if (p == NULL)
        return NULL;
    while (p != NULL) {
        if (counter == param_index)
            break;
        p = _my_strstr((const char *)p, ',');
        counter++;
    }

    if (p != NULL && param_index > 0)
        p++;

    return p;
}

int eshell_param_get_int(char *params, int param_index)
{
    char *p = NULL;
    p = eshell_param_get_start(params, param_index);
    if (p != NULL) {
        return atoi(p);
    }
    return 0;
}

unsigned int eshell_param_hexstr_get_uint(char *params, int param_index)
{
    char *p = NULL;
    p = eshell_param_get_start(params, param_index);
    if (p != NULL) {
        return strtol(p, NULL, 16);
    }
    return 0;
}

void eshell_param_macstr_get_array(char *params, int param_index, unsigned char *array_out)
{
    char *p = NULL;
    p = eshell_param_get_start(params, param_index);
    if (p != NULL) {
        array_out[0] = _my_noprefix_2byte_hexstr_to_uchar(p);
        array_out[1] = _my_noprefix_2byte_hexstr_to_uchar(p + 3);
        array_out[2] = _my_noprefix_2byte_hexstr_to_uchar(p + 6);
        array_out[3] = _my_noprefix_2byte_hexstr_to_uchar(p + 9);
        array_out[4] = _my_noprefix_2byte_hexstr_to_uchar(p + 12);
        array_out[5] = _my_noprefix_2byte_hexstr_to_uchar(p + 15);
    }
}

static int eshell_parse_param(char *params, char *argv[])
{
    if (NULL == params) {
        return 0;
    }

    char *line = params;
    int nargs = 0;

    while (nargs < ESHELL_MAXARGS) {
        /* skip any white space */
        while ((*line == ' ') || (*line == '\t')) {
            ++line;
        }

        if (*line == '\0') { /* end of line, no more args	*/
            argv[nargs] = 0;
            return (nargs);
        }

        /* Argument include space should be bracketed by quotation mark */
        if (*line == '\"') {
            /* Skip quotation mark */
            line++;

            /* Begin of argument string */
            argv[nargs++] = line;

            /* Until end of argument */
            while (*line && (*line != '\"')) {
                ++line;
            }
        } else {
            argv[nargs++] = line; /* begin of argument string	*/
            /* find end of string */
            while (*line && (*line != ' ') && (*line != '\t') && (*line != ',')) {
                ++line;
            }
        }

        if (*line == '\0') { /* end of line, no more args	*/
            argv[nargs] = 0;
            return (nargs);
        }

        *line++ = '\0'; /* terminate current arg	 */
    }
    return (nargs);
}

static bool eshell_parse_param_is_remote_cmd(char *str, unsigned int str_len)
{
    char *para_char = str;
    int mark_index = 0;
    bool mark_flag = false;
    bool remote_exe = false;
    unsigned int cnt_index = 0;
    while (*para_char != '\0' || (cnt_index < str_len)) {
        if (mark_flag) {
            if ((*para_char == 'R') || (*para_char == 'r')) {
                str[mark_index] = ' ';
                str[mark_index + 1] = ' ';
                remote_exe = true;
                break;
            } else if ((*para_char == 'L') || (*para_char == 'l')) {
                str[mark_index] = ' ';
                str[mark_index + 1] = ' ';
                remote_exe = false;
                break;
            } else {
                mark_flag = false;
            }
        } else {
            if (*para_char == '-') {
                mark_flag = true;
                mark_index = cnt_index;
            }
        }
        para_char++;
        cnt_index++;
    }
    if (context.debug) {
        TRACE(2, "eshell : str=%s remote_exe=%d", str, remote_exe);
    }
    return remote_exe;
}

// system API
static void eshell_prompt(void)
{
    eshell_putstring("\r\n"
                     "eshell >");
}

static int eshell_execute(char *str, unsigned int str_len, const eshell_cmd_t *cmds, int cmds_count,
                          bool force_local_execute)
{
    int ret = -1;

    if (str == NULL || cmds == NULL || cmds_count < 1)
        return -1;

    if (context.debug) {
        TRACE(1, "eshell_execute:cmds_count=%d", cmds_count);
    }

    if (eshell_parse_param_is_remote_cmd(str, str_len) && !force_local_execute) {
        TRACE(0,"eshell_execute remote cmd");
        if (eshell_rpc_run_op_cmd_func_cb) {
            eshell_rpc_run_op_cmd_func_cb(str, str_len);
        }
        return 0;
    }

    // find command
    for (int i = 0; i < cmds_count; ++i) {
        const char *cmd = cmds[i].cmd_str;

        if (cmd == NULL)
            continue;

        if (context.debug) {
            // TRACE(1, "eshell : index=%d, cmd=%s, str=%s, help=%s", i, cmd, str,
            // cmds[i].help_str);
        }

        int len = strlen(cmd);

        if (strncmp(cmd, str, len) != 0) {
            continue;
        }
        char *argv[ESHELL_MAXARGS + 1]; /* NULL terminated	  */
        int argc = eshell_parse_param(str, argv);
        if (context.debug) {
            TRACE(3, "eshell : argc_num=%d, exe_cmd=%s, help_info=%s", argc, argv[0],
                  cmds[i].help_str);
        }
        cmds[i].cmd_handler(argc, argv);
        ret = 0;
        break;
    }

    return ret;
}

static int eshell_execute_one_command(char *str, unsigned int str_len, bool force_local_execute)
{
    int ret = 0;
    // builtin commands
    ret = eshell_execute(str, str_len, (eshell_cmd_t *)builtin_cmds, sizeof(builtin_cmds)/sizeof(eshell_cmd_t), force_local_execute);
    // ext commands
    if (ret != 0) {
        TRACE(0, "str=%s len=%d", str, str_len);
        ret = eshell_execute(str, str_len, context.ext_cmds, context.ext_cmd_count, force_local_execute);
        if (ret != 0) {
            eshell_putstring("\r\ncommand not supported!\r\n");
        }
    }
    return ret;
}

#ifdef SYSTEM_USE_FILE
#define STR_TRIM(h) \
    do { \
        char *t = h + strlen(h); \
        while (h != t && \
                (*h == ' ' || *h == '\t' || *h == '\r' || *h == '\n' || *h == ']')) \
            h++; \
        while (h != t && \
                (*t == ' ' || *t == '\t' || *t == '\r' || *t == '\n' ||*h == ']')) { \
            *t = '\0'; \
            t--; \
        } \
    } while (0)
#else
#define STR_TRIM(h) \
    do { \
        char *t = h + strlen(h); \
        while (h != t && \
                (*h == ' ' || *h == '\t' || *h == '\r' || *h == '\n')) \
            h++; \
        while (h != t && \
                (*t == ' ' || *t == '\t' || *t == '\r' || *t == '\n')) { \
            *t = '\0'; \
            t--; \
        } \
    } while (0)
#endif

static int eshell_execute_commands_and_check_result(char *str, unsigned int str_len, bool force_local_execute)
{
    int ret = 0, len;
    char *buf, *p, *q;

    if (str_len == 0 || str == NULL)
        return -1;

    buf = malloc(str_len + 1);
    if (!buf)
        return -1;

    memset(buf, 0, str_len+1);
    memcpy(buf, str, str_len);

    p = buf;
    STR_TRIM(p);
    while (1) {
        q = strstr(p, "&&");
        if (q)
            *q = '\0';

        len = strlen(p);
        if (len > 0) {
            ret = eshell_execute_one_command(p, len, force_local_execute);
            if (ret)
                break;
        }

        if (!q)
            break;
        p = q + 2;
        STR_TRIM(p);
        if (p - buf >= str_len)
            break;
    }

    free(buf);
    return ret;
}

static int eshell_execute_multi_commands(char *str, unsigned int str_len, bool force_local_execute)
{
    int ret = 0, len;
    char *buf, *p, *q;

    if (str_len == 0 || str == NULL)
        return -1;

    buf = malloc(str_len + 1);
    if (!buf)
        return -1;

    memset(buf, 0, str_len+1);
    memcpy(buf, str, str_len);

    p = buf;
    STR_TRIM(p);
    while (1) {
        q = strstr(p, ";");
        if (q)
            *q = '\0';

        len = strlen(p);
        if (len > 0) {
            if (strstr(p, "&&"))
                ret = eshell_execute_commands_and_check_result(p, len, force_local_execute);
            else
                ret = eshell_execute_one_command(p, len, force_local_execute);
        }

        if (!q)
            break;
        p = q + 1;
        STR_TRIM(p);
        if (p - buf >= str_len)
            break;
    }

    free(buf);
    return ret;
}

static int eshell_execute_command(char *str)
{
    return eshell_execute_multi_commands(str, strlen(str), false);
}

int eshell_rpc_execute_command(char *str, unsigned int str_len)
{
    return eshell_execute_multi_commands(str, str_len, true);
}

int eshell_run(char *cmdstr)
{
    return eshell_execute_multi_commands(cmdstr, strlen(cmdstr), false);
}


void eshell_process(void)
{
    unsigned char ch = 0;

#ifdef ESHELL_AUTORUN
    static bool first_boot = true;
    if (first_boot) {
        eshell_run(ESHELL_AUTORUN);
        first_boot = false;
    }
#endif

    eshell_prompt();
    while (context.running) {
        ch = eshell_getchar();
        if (ch == 0) {
            while(!eshell_wait_data_ready());

            continue;
        }

        if (context.debug) {
            if (ch != 0) {
                TRACE(1, "eshell : get char=%c", ch);
            }
        }

#ifdef SYSTEM_USE_FILE
        if (ch == ESHELL_CHAR_END ||ch == ESHELL_CHAR_CR || ch == ESHELL_CHAR_LF) {
#else
        if (ch == ESHELL_CHAR_CR || ch == ESHELL_CHAR_LF) {
#endif
            if (context.buff_offset > 0) {
                eshell_putstring("\r\n");
                eshell_execute_command(context.buff);
                context.buff_offset = 0;
                memset(context.buff, 0, ESHELL_BUFF_SIZE);
            }

            if (ch == ESHELL_CHAR_LF) {
                eshell_prompt();
            }
            continue;
        }

        if (ch == ESHELL_CHAR_DEL) {
            if (context.buff_offset > 0) {
                context.buff_offset--;
                context.buff[context.buff_offset] = '\0';
                eshell_putchar('\b');
                eshell_putchar(' ');
                eshell_putchar('\b');
            }
            continue;
        }

        if (ch > 0 && context.buff_offset < ESHELL_BUFF_SIZE) {
            if (context.echo)
                eshell_putchar(ch);
            context.buff[context.buff_offset++] = ch;
        }
    }
}

void eshell_open(uint8_t uart_id, bool need_open_uart)
{
    context.debug = 0;
    context.echo = 1;
    context.buff_offset = 0;
    context.running = 1;
    context.need_open_uart = need_open_uart;
    context.ext_cmds = (eshell_cmd_t *)__eshell_command_table_start;
    context.ext_cmd_count =
        ((unsigned int)__eshell_command_table_end - (unsigned int)__eshell_command_table_start) /
        sizeof(eshell_cmd_t);

    eshell_platform_init(uart_id, need_open_uart);
}

void eshell_close(void)
{
    context.running = 0;
    eshell_platform_deinit();
}

void eshell_rpc_run_cmd_func_cb_register(eshell_rpc_run_cmd_func op_cb,
                                         eshell_rpc_run_cmd_func result_cb)
{
    eshell_rpc_run_op_cmd_func_cb = op_cb;
    eshell_rpc_run_show_result_func_cb = result_cb;
}

eshell_rpc_run_cmd_func eshell_rpc_run_result_output_func_get(void)
{
    return eshell_rpc_run_show_result_func_cb;
}
