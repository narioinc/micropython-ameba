/*
 * Copyright (c) 2014, SouthSilicon Valley Corp.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         A very simple Contiki application showing how Contiki programs look
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "py/mpconfig.h"
#include "lib/utils/pyexec.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/compile.h"

#include "readline.h"
#include "pyexec.h"


#include "sys/clock.h"
#include "sys/autostart.h"

#include "mphal.h"
#include "gccollect.h"
#include "flash.h"

#include <stdio.h> /* For printf() */
#include "gpio_api.h"
#include "atcmd.h"
#include "bsp.h"
#include "hw_init.h"
#include "wdog_api.h"
#include "netstack.h"

const char *msg_0 = "Starting to execute main.py\r\n";
const char *msg_1 = "REPL start failed\r\n";
static const char fresh_main_py[] =
"# main.py -- put your code here! The script in main.py will be executed when boot up !\r\n"
;

fs_user_mount_t fs_user_mount_flash;

at_cmd_info atcmd_info_tbl[] = 
{
    {"",    NULL}
};

void flash_vfs_init0 (void) {
    fs_user_mount_t *vfs = &fs_user_mount_flash;
    vfs->str = "/flash";
    vfs->len = 6;
    vfs->flags = 0;
    flash_init0(vfs);
    MP_STATE_PORT(fs_user_mount)[0] = vfs;
    FRESULT res = f_mount(&vfs->fatfs, vfs->str, 1);
    if (res == FR_NO_FILESYSTEM) {
        res = f_mkfs("/flash", 1, 0);
        if (res != FR_OK) {
            MP_STATE_PORT(fs_user_mount)[0] = NULL;
            return;
        }
    } else if (res == FR_OK) {
        // To nothing if vfs mount success
    } else {
        printf("Create fatfs in flash failed, res = %d\r\n", res);
        MP_STATE_PORT(fs_user_mount)[0] = NULL;
        return;
    }

    FIL fp;
    res = f_open(&fp, "/flash/main.py", FA_WRITE | FA_OPEN_ALWAYS);
    UINT n;
    res = f_write(&fp, fresh_main_py, sizeof(fresh_main_py) - 1 /* don't count null terminator */, &n);
    f_close(&fp);
    if (FR_OK != f_chdir ("/flash/lib")) {
        f_mkdir("/flash/lib");
    }
}

COMMON_CONFIG i_config;

char gDisableRTS = 0;

PROCESS_NAME(ssv6200_radio_receive_process);
PROCESS_NAME(temperature_compensator_process);
PROCESS_NAME(tcp_driver_process);
PROCESS_NAME(udp_driver_process);
PROCESS_NAME(dhcp_process);
PROCESS_NAME(watchdog_process);
PROCESS_NAME(resolv_process);
PROCESS_NAME(discover_process);

PROCESS_NAME(os_repl_process);

PROCESS(main_process, "main process");
AUTOSTART_PROCESSES(&main_process);
PROCESS_THREAD(main_process, ev, data)
{
    PROCESS_BEGIN();
#if !MICROPY_REPL_EVENT_DRIVEN
    if(pyexec_friendly_repl() != 0) {
        api_wdog_reboot();
    }
#endif
    PROCESS_END();
}

extern IEEE80211STATUS          gwifistatus;
extern TAG_CABRIO_CONFIGURATION gCabrioConf;

int mp_main (void) {
    memcpy(&i_config, 0x3003000, sizeof(COMMON_CONFIG));
    bsp_init();
    clock_init();
    soft_mac_init();
    NETSTACK_CONF_RADIO.init();
    // Due to console_init would register a callback to icomlib tx handler which is not what I want, so I do the console_init in mphal_init
    //console_init(NULL);
    mphal_init();
    //set_log_single_group_level(7, 1, 2);
    cabrio_flash_init();
    reset_global_conf();
    bsp_get_boot_info(true);
    bsp_get_define_info(true);
    bsp_get_mp_info(true);
    bsp_get_config_info(true, &i_config);
    softap_init();
    gc_init(&__heap_start__, &__heap_end__);
    mp_init();
    mp_obj_list_init(mp_sys_path, 0);
    mp_obj_list_init(mp_sys_argv, 0);
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_));
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_flash));
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_flash_slash_lib));
    readline_init0();
    mp_hal_stdout_tx_strn_cooked(msg_0, strlen(msg_0));
    flash_vfs_init0();
    process_init();
    process_start(&etimer_process, NULL);
    init_global_conf();
    process_start(&ssv6200_radio_receive_process, NULL);
    process_start(&tcpip_process, NULL);
    process_start(&tcp_driver_process, NULL);
    process_start(&udp_driver_process, NULL);
    irq_enable();
    netstack_init();
    //process_start(&watchdog_process, NULL);
    process_start(&dhcp_process, NULL);
    process_start(&temperature_compensator_process, NULL);
    //at_cmd_start();

    process_start(&resolv_process, NULL);
    process_start(&discover_process, NULL);
#if MICROPY_REPL_EVENT_DRIVEN
    pyexec_event_repl_init();
    process_start(&os_repl_process, NULL);
#else
    autostart_start(autostart_processes);
#endif
    for(;;) {
        do {
        } while (process_run() > 0);
    }
    return 0;
}

mp_import_stat_t mp_import_stat(const char *path) {
    return fat_vfs_import_stat(path);
}

mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    return fat_vfs_lexer_new_from_file(filename);
}

mp_obj_t mp_builtin_open(uint n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return fatfs_builtin_open(n_args, args, kwargs);
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);