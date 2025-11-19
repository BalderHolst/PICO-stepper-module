#ifndef DIFF_DRIVE_CLASS_H
#define DIFF_DRIVE_CLASS_H

#include "py/obj.h"
#include "py/runtime.h"

#include <pico/stdlib.h>
#include <stdlib.h>

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

    mp_obj_DiffDrive *self = mp_obj_malloc(mp_obj_DiffDrive, type);

    int steps = DDRIVE_STEPS_PR_SEQ; // TODO: Make dynamic if needed
    float * buf = m_new(float, steps* STEPPER_PINS);
    PWMSequence seq = stepper_generate_seq(steps, buf);

    (void)seq;  // Suppress unused variable warning.

    ddrive_init_with_seq(&ddrive_instance, rpins, lpins, seq);

    ddrive_initialized = true;

    return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t DiffDrive_deinit(mp_obj_t self_in) {
    mp_obj_DiffDrive *self = MP_OBJ_TO_PTR(self_in);

    if (!self->ddrive_ptr) return mp_const_none;

    Stepper_deinit(&self->ddrive_ptr->rstepper);
    Stepper_deinit(&self->ddrive_ptr->lstepper);

    self->ddrive_ptr = NULL;
    ddrive_initialized = false;

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(DiffDrive_deinit_method, DiffDrive_deinit);

static const mp_rom_map_elem_t DiffDrive_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&DiffDrive_deinit_method) },
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
