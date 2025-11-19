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

volatile bool ddrive_initialized = false;
static DiffDrive ddrive_instance;

typedef struct _mp_obj_DiffDrive_t {
    mp_obj_base_t base; // For MicroPython object system
    DiffDrive * ddrive_ptr;
} mp_obj_DiffDrive;

static mp_obj_t DiffDrive_make_new(const mp_obj_type_t *type,
                                 size_t n_args, size_t n_kw,
                                 const mp_obj_t *args) {

    mp_arg_check_num(n_args, n_kw, 2, 2, false);

    mp_obj_t rpins_obj = args[0];
    mp_obj_t lpins_obj = args[1];

    if (!mp_obj_is_type(rpins_obj, &mp_type_list)) {
        mp_raise_TypeError(MP_ERROR_TEXT("`rpins` must be a list"));
    }

    if (!mp_obj_is_type(rpins_obj, &mp_type_list)) {
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

    if (ddrive_initialized) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("DiffDrive instance already initialized. Only one instance allowed."));
    }

    int steps = DDRIVE_STEPS_PR_SEQ; // TODO: Make dynamic if needed
    float * buf = m_new(float, steps* STEPPER_PINS);
    PWMSequence seq = stepper_generate_seq(steps, buf);

    ddrive_init_with_seq(&ddrive_instance, rpins, lpins, seq);

    ddrive_initialized = true;

    mp_obj_DiffDrive *self = mp_obj_malloc(mp_obj_DiffDrive, type);
    self->ddrive_ptr = &ddrive_instance;

    return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t DiffDrive_deinit(mp_obj_t self_in) {
    mp_obj_DiffDrive *self = MP_OBJ_TO_PTR(self_in);

    if (!self->ddrive_ptr) return mp_const_none;

    // Stop the second core
    // multicore_reset_core1();

    Stepper_deinit(&self->ddrive_ptr->rstepper);
    Stepper_deinit(&self->ddrive_ptr->lstepper);

    self->ddrive_ptr = NULL;
    ddrive_initialized = false;

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(DiffDrive_deinit_method, DiffDrive_deinit);

static void check_initialized(mp_obj_DiffDrive *self) {
    if (!self->ddrive_ptr) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("DiffDrive instance not initialized."));
    }
}

static mp_obj_t DiffDrive_task(mp_obj_t self) {
    mp_obj_DiffDrive *ddrive_obj = MP_OBJ_TO_PTR(self);
    check_initialized(ddrive_obj);

    while (ddrive_initialized) {
        ddrive_task(ddrive_obj->ddrive_ptr);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(DiffDrive_task_method, DiffDrive_task);


// ==================== METHODS ====================

// void ddrive_stop(DiffDrive * ddrive);
static mp_obj_t DiffDrive_stop(mp_obj_t self_in) {
    mp_obj_DiffDrive *self = MP_OBJ_TO_PTR(self_in);
    check_initialized(self);
    ddrive_stop(self->ddrive_ptr);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(DiffDrive_stop_method, DiffDrive_stop);

// void ddrive_rpm(DiffDrive * ddrive, float rrpm, float lrpm);
static mp_obj_t DiffDrive_rpm(mp_obj_t self_in, mp_obj_t rrpm_obj, mp_obj_t lrpm_obj) {

    mp_obj_DiffDrive *self = MP_OBJ_TO_PTR(self_in);
    check_initialized(self);

    // Check that they are all floats
    if (!mp_obj_is_float(rrpm_obj)) mp_raise_TypeError(MP_ERROR_TEXT("`rrpm` must be a float"));
    if (!mp_obj_is_float(lrpm_obj)) mp_raise_TypeError(MP_ERROR_TEXT("`lrpm` must be a float"));

    float rrpm = mp_obj_get_float(rrpm_obj);
    float lrpm = mp_obj_get_float(lrpm_obj);
    ddrive_rpm(self->ddrive_ptr, rrpm, lrpm);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(DiffDrive_set_rpm_method, DiffDrive_rpm);

// void ddrive_trans_rot(DiffDrive * ddrive, float trans, float rot);
static mp_obj_t DiffDrive_trans_rot(mp_obj_t self_in, mp_obj_t trans_obj, mp_obj_t rot_obj) {

    mp_obj_DiffDrive *self = MP_OBJ_TO_PTR(self_in);
    check_initialized(self);

    // Check that they are all floats
    if (!mp_obj_is_float(trans_obj)) mp_raise_TypeError(MP_ERROR_TEXT("`trans` must be a float"));
    if (!mp_obj_is_float(rot_obj))   mp_raise_TypeError(MP_ERROR_TEXT("`rot` must be a float"));

    float trans = mp_obj_get_float(trans_obj);
    float rot = mp_obj_get_float(rot_obj);
    ddrive_trans_rot(self->ddrive_ptr, trans, rot);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(DiffDrive_set_trans_rot_method, DiffDrive_trans_rot);

// bool * ddrive_trap_rpm(DiffDrive * ddrive, float rtarget, float ltarget, float time);
static mp_obj_t DiffDrive_trap_rpm(size_t n_args, const mp_obj_t *args, mp_map_t *kw) {

    mp_obj_t self_in     = args[0];
    mp_obj_t rtarget_obj = args[1];
    mp_obj_t ltarget_obj = args[2];
    mp_obj_t time_obj    = args[3];

    mp_obj_DiffDrive *self = MP_OBJ_TO_PTR(self_in);
    check_initialized(self);

    // Check that they are all floats
    if (!mp_obj_is_float(rtarget_obj)) mp_raise_TypeError(MP_ERROR_TEXT("`rtarget` must be a float"));
    if (!mp_obj_is_float(ltarget_obj)) mp_raise_TypeError(MP_ERROR_TEXT("`ltarget` must be a float"));
    if (!mp_obj_is_float(time_obj))    mp_raise_TypeError(MP_ERROR_TEXT("`time` must be a float"));

    float rtarget = mp_obj_get_float(rtarget_obj);
    float ltarget = mp_obj_get_float(ltarget_obj);
    float time = mp_obj_get_float(time_obj);

    bool * active = ddrive_trap_rpm(self->ddrive_ptr, rtarget, ltarget, time);
    return mp_obj_new_bool(*active);
}

static MP_DEFINE_CONST_FUN_OBJ_KW(DiffDrive_set_trap_rpm_method, 4, DiffDrive_trap_rpm);

// bool * ddrive_trap_trans_rot(DiffDrive * ddrive, float trans, float rot, float time);
static mp_obj_t DiffDrive_trap_trans_rot(size_t n_args, const mp_obj_t *args, mp_map_t *kw) {

    mp_obj_t self_in     = args[0];
    mp_obj_t trans_obj   = args[1];
    mp_obj_t rot_obj     = args[2];
    mp_obj_t time_obj    = args[3];

    mp_obj_DiffDrive *self = MP_OBJ_TO_PTR(self_in);
    check_initialized(self);

    // Check that they are all floats
    if (!mp_obj_is_float(trans_obj)) mp_raise_TypeError(MP_ERROR_TEXT("`trans` must be a float"));
    if (!mp_obj_is_float(rot_obj))   mp_raise_TypeError(MP_ERROR_TEXT("`rot` must be a float"));
    if (!mp_obj_is_float(time_obj))  mp_raise_TypeError(MP_ERROR_TEXT("`time` must be a float"));

    float trans = mp_obj_get_float(trans_obj);
    float rot = mp_obj_get_float(rot_obj);
    float time = mp_obj_get_float(time_obj);
    bool * active = ddrive_trap_trans_rot(self->ddrive_ptr, trans, rot, time);
    return mp_obj_new_bool(*active);
}
static MP_DEFINE_CONST_FUN_OBJ_KW(DiffDrive_set_trap_trans_rot_method, 4, DiffDrive_trap_trans_rot);

static const mp_rom_map_elem_t DiffDrive_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__),                MP_ROM_PTR(&DiffDrive_deinit_method)             },
    { MP_ROM_QSTR(MP_QSTR_task),                   MP_ROM_PTR(&DiffDrive_task_method)               },
    { MP_ROM_QSTR(MP_QSTR_stop),                   MP_ROM_PTR(&DiffDrive_stop_method)               },
    { MP_ROM_QSTR(MP_QSTR_set_rpm),                MP_ROM_PTR(&DiffDrive_set_rpm_method)            },
    { MP_ROM_QSTR(MP_QSTR_set_trans_rot),          MP_ROM_PTR(&DiffDrive_set_trans_rot_method)      },
    { MP_ROM_QSTR(MP_QSTR_set_trap_rpm),           MP_ROM_PTR(&DiffDrive_set_trap_rpm_method)       },
    { MP_ROM_QSTR(MP_QSTR_set_trap_trans_rot),     MP_ROM_PTR(&DiffDrive_set_trap_trans_rot_method) },
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
