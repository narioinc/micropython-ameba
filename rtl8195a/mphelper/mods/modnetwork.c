/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2015 Galen Hazelwood
 * Copyright (c) 2016 Chester Tseng
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*****************************************************************************
 *                              Header includes
 * ***************************************************************************/
#include "modnetwork.h"
#include "objnetif.h"
#include "objpacket.h"

/*****************************************************************************
 *                              External variables
 * ***************************************************************************/

/*****************************************************************************
 *                              Internal variables
 * ***************************************************************************/


/*****************************************************************************
 *                              Local functions
 * ***************************************************************************/

void network_init0(void) {
    // Init modnetwork here
    mp_obj_list_init(&MP_STATE_PORT(netif_list_obj), 0);

    /**
     * tcpip_init is a lwip tcpip stack init function, it create a new task: tcpip_thread
     * to recv message from mailbox
     */
    tcpip_init(NULL, NULL);
}

STATIC mp_obj_t netif_iflist(void) {
    return &MP_STATE_PORT(netif_list_obj);
}
MP_DEFINE_CONST_FUN_OBJ_0(netif_iflist_obj, netif_iflist);

STATIC const mp_map_elem_t mp_module_network_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),     MP_OBJ_NEW_QSTR(MP_QSTR_network) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_iflist),       MP_OBJ_FROM_PTR(&netif_iflist_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_NETIF),        MP_OBJ_FROM_PTR(&netif_type) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_PKT),          MP_OBJ_FROM_PTR(&packet_type) },
};
STATIC MP_DEFINE_CONST_DICT(mp_module_network_globals, mp_module_network_globals_table);

const mp_obj_module_t mp_network_module = {
    .base     = { &mp_type_module },
    .globals  = (mp_obj_dict_t*)&mp_module_network_globals,
};