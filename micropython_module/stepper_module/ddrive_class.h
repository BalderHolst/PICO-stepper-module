#ifndef DIFF_DRIVE_CLASS_H
#define DIFF_DRIVE_CLASS_H

#include <pico/multicore.h>
#include <pico/stdlib.h>
#include <stdlib.h>

#include "py/obj.h"
#include "py/runtime.h"

#include "ddrive.h"

#include "stepper_class.h"

#define CLAMP(x, lower, upper) ((x) < (lower) ? (lower) : ((x) > (upper) ? (upper) : (x)))

typedef struct _mp_obj_DiffDrive_t {
    mp_obj_base_t base; // For MicroPython object system
    DiffDrive ddrive;
} mp_obj_DiffDrive;

static mp_obj_t DiffDrive_make_new(const mp_obj_type_t *type,
                                 size_t n_args, size_t n_kw,
                                 const mp_obj_t *args) {

    mp_arg_check_num(n_args, n_kw, 3, 3, false);

    mp_obj_t rpins_obj = args[0];
    mp_obj_t lpins_obj = args[1];
    mp_obj_t steps_obj = args[2];

    if (!mp_obj_is_type(rpins_obj, &mp_type_list)) {
        mp_raise_TypeError(MP_ERROR_TEXT("`rpins` must be a list"));
    }

    if (!mp_obj_is_type(lpins_obj, &mp_type_list)) {
        mp_raise_TypeError(MP_ERROR_TEXT("`lpins` must be a list"));
    }

    mp_obj_list_t *rpins_list = MP_OBJ_TO_PTR(rpins_obj);
    mp_obj_list_t *lpins_list = MP_OBJ_TO_PTR(lpins_obj);

    if (rpins_list->len != STEPPER_PINS) mp_raise_ValueError(MP_ERROR_TEXT("rpins list must have 4 items"));
    if (lpins_list->len != STEPPER_PINS) mp_raise_ValueError(MP_ERROR_TEXT("lpins list must have 4 items"));

    int * rpins = m_new(int, STEPPER_PINS);
    int * lpins = m_new(int, STEPPER_PINS);

    if (!rpins || !lpins) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to allocate memory for pins"));
    }

    // Copy pin values from MicroPython lists to C arrays
    for (size_t i = 0; i < STEPPER_PINS; i++) {
        rpins[i] = mp_obj_get_int(rpins_list->items[i]);
        lpins[i] = mp_obj_get_int(lpins_list->items[i]);
    }

    size_t steps = mp_obj_get_int(steps_obj);
    float * buf = m_new(float, steps* STEPPER_PINS);
    PWMSequence seq = stepper_generate_seq(steps, buf);

    mp_obj_DiffDrive *self = mp_obj_malloc(mp_obj_DiffDrive, type);

    ddrive_init_with_seq(&self->ddrive, rpins, lpins, seq);

    return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t DiffDrive_deinit(mp_obj_t self_in) {
    mp_obj_DiffDrive *self = MP_OBJ_TO_PTR(self_in);

    Stepper_deinit(&self->ddrive.rstepper);
    Stepper_deinit(&self->ddrive.lstepper);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(DiffDrive_deinit_method, DiffDrive_deinit);

static mp_obj_t DiffDrive_task_loop(mp_obj_t self) {
    mp_obj_DiffDrive *ddrive_obj = MP_OBJ_TO_PTR(self);

    while (true) {
        ddrive_task(&ddrive_obj->ddrive);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(DiffDrive_task_loop_method, DiffDrive_task_loop);

// ==================== METHODS ====================

static void wait_until_ready(DiffDrive * ddrive) {
    while (ddrive->new_cmd_available) {
        mp_handle_pending(true);
        MICROPY_THREAD_YIELD();
    }
}

// void ddrive_stop(DiffDrive * ddrive);
static mp_obj_t DiffDrive_stop(mp_obj_t self_in) {
    mp_obj_DiffDrive *self = MP_OBJ_TO_PTR(self_in);
    wait_until_ready(&self->ddrive);
    ddrive_stop(&self->ddrive);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(DiffDrive_stop_method, DiffDrive_stop);

// void ddrive_rpm(DiffDrive * ddrive, float rrpm, float lrpm);
static mp_obj_t DiffDrive_rpm(mp_obj_t self_in, mp_obj_t rrpm_obj, mp_obj_t lrpm_obj) {

    mp_obj_DiffDrive *self = MP_OBJ_TO_PTR(self_in);

    float rrpm = mp_obj_get_float(rrpm_obj);
    float lrpm = mp_obj_get_float(lrpm_obj);

    wait_until_ready(&self->ddrive);
    ddrive_rpm(&self->ddrive, rrpm, lrpm);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(DiffDrive_set_rpm_method, DiffDrive_rpm);

// void ddrive_trans_rot(DiffDrive * ddrive, float trans, float rot);
static mp_obj_t DiffDrive_trans_rot(mp_obj_t self_in, mp_obj_t trans_obj, mp_obj_t rot_obj) {

    mp_obj_DiffDrive *self = MP_OBJ_TO_PTR(self_in);

    float trans = mp_obj_get_float(trans_obj);
    float rot = mp_obj_get_float(rot_obj);
    wait_until_ready(&self->ddrive);
    ddrive_trans_rot(&self->ddrive, trans, rot);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(DiffDrive_set_trans_rot_method, DiffDrive_trans_rot);


static const mp_rom_map_elem_t DiffDrive_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__),                MP_ROM_PTR(&DiffDrive_deinit_method)             },
    { MP_ROM_QSTR(MP_QSTR_task_loop),              MP_ROM_PTR(&DiffDrive_task_loop_method)          },
    { MP_ROM_QSTR(MP_QSTR_stop),                   MP_ROM_PTR(&DiffDrive_stop_method)               },
    { MP_ROM_QSTR(MP_QSTR_set_rpm),                MP_ROM_PTR(&DiffDrive_set_rpm_method)            },
    { MP_ROM_QSTR(MP_QSTR_set_trans_rot),          MP_ROM_PTR(&DiffDrive_set_trans_rot_method)      },
};

static MP_DEFINE_CONST_DICT(DiffDrive_locals_dict, DiffDrive_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    type_DiffDrive,
    MP_QSTR_DiffDrive,
    MP_TYPE_FLAG_NONE,
    make_new, DiffDrive_make_new,
    locals_dict, &DiffDrive_locals_dict
);

#endif // DIFF_DRIVE_CLASS_H
