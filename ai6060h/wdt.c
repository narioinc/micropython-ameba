/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2015 Daniel Campora
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

#include "wdt.h"

/*****************************************************************************
 *                              External variables
 * ***************************************************************************/

/*****************************************************************************
 *                              Internal functions
 * ***************************************************************************/
STATIC const mp_obj_base_t wdt_obj = {&wdt_type};

STATIC mp_obj_t wdt_start(mp_obj_t self_in, mp_obj_t msec_in) {
    wdt_obj_t *self = self_in;
    mp_int_t msec = mp_obj_get_int(msec_in);
    if (msec > 0) {
        api_wdog_period_cutommed(msec);
        self->msec = msec;
    } else {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "millisecond must > 0"));
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(wdt_start_obj, wdt_start);

STATIC mp_obj_t wdt_stop(mp_obj_t self_in) {
    api_wdog_stop();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(wdt_stop_obj, wdt_stop);

STATIC mp_obj_t wdt_refresh(mp_obj_t self_in) {
    wdt_obj_t *self = self_in;
    api_wdog_period_cutommed(self->msec);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(wdt_refresh_obj, wdt_refresh);

STATIC const mp_map_elem_t wdt_locals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),        MP_OBJ_NEW_QSTR(MP_QSTR_wdt) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_start),           (mp_obj_t)&wdt_start_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_stop),            (mp_obj_t)&wdt_stop_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_refresh),         (mp_obj_t)&wdt_refresh_obj },
};
STATIC MP_DEFINE_CONST_DICT(wdt_locals_dict, wdt_locals_table);

STATIC mp_obj_t wdt_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // return singleton object
    return (mp_obj_t)&wdt_obj;
}

const mp_obj_type_t wdt_type = {
    { &mp_type_type },
    .name = MP_QSTR_WDT,
    .make_new = wdt_make_new,
    .locals_dict = (mp_obj_t)&wdt_locals_dict,
};