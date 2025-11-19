// Include MicroPython API.
#include "py/runtime.h"

#include <pico/stdlib.h>
#include <stdlib.h>

#include "stepper.h"

typedef struct _mp_obj_Stepper_t {
    mp_obj_base_t base;  // MUST be first
    Stepper stepper;     // Your internal C struct
} mp_obj_Stepper;

// `Stepper` class
static mp_obj_t Stepper_make_new(const mp_obj_type_t *type,
                                 size_t n_args, size_t n_kw,
                                 const mp_obj_t *args) {

    // Expect 2 args: pins list + steps integer
    mp_arg_check_num(n_args, n_kw, 2, 2, false);

    mp_obj_t pins_obj = args[0];
    mp_obj_t steps_obj = args[1];

    // Check pins is list
    if (!mp_obj_is_type(pins_obj, &mp_type_list)) {
        mp_raise_TypeError(MP_ERROR_TEXT("pins must be a list"));
    }

    mp_obj_list_t *pins_list = MP_OBJ_TO_PTR(pins_obj);
    if (pins_list->len != STEPPER_PINS) {
        mp_raise_ValueError(MP_ERROR_TEXT("pins list must have 4 items"));
    }

    int * pins = m_new(int, STEPPER_PINS);
    if (!pins) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to allocate memory for pins"));
    }

    for (size_t i = 0; i < STEPPER_PINS; i++) {
        pins[i] = mp_obj_get_int(pins_list->items[i]);
    }

    int steps = mp_obj_get_int(steps_obj);
    (void)steps;  // Suppress unused variable warning.

    mp_obj_Stepper *self = mp_obj_malloc(mp_obj_Stepper, type);

    float * buf = m_new(float, steps* STEPPER_PINS);

    self->stepper = stepper_init_with_buf(pins, steps, buf);

    return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t Stepper_deinit(mp_obj_t self_in) {
    mp_obj_Stepper *self = MP_OBJ_TO_PTR(self_in);

    m_del(int, self->stepper.pins, STEPPER_PINS);
    m_del(float, self->stepper.sequence.items, self->stepper.sequence.length * STEPPER_PINS);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(Stepper_deinit_method, Stepper_deinit);

static mp_obj_t Stepper_test(mp_obj_t self_in) {
    mp_obj_Stepper *self = MP_OBJ_TO_PTR(self_in);
    (void)self;  // Suppress unused variable warning.
    return mp_obj_new_int(42);
}
static MP_DEFINE_CONST_FUN_OBJ_1(Stepper_test_method, Stepper_test);


static const mp_rom_map_elem_t Stepper_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_test), MP_ROM_PTR(&Stepper_test_method) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&Stepper_deinit_method) },
};
static MP_DEFINE_CONST_DICT(Stepper_locals_dict, Stepper_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    type_Stepper,
    MP_QSTR_Stepper,
    MP_TYPE_FLAG_NONE,
    make_new, Stepper_make_new,
    locals_dict, &Stepper_locals_dict
);


static const mp_rom_map_elem_t module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_ddrive) },
    { MP_ROM_QSTR(MP_QSTR_Stepper),    MP_ROM_PTR(&type_Stepper) },
};
static MP_DEFINE_CONST_DICT(module_globals, module_globals_table);

// Define python module object.
const mp_obj_module_t cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&module_globals,
};

// Register the module to make it available in Python.
MP_REGISTER_MODULE(MP_QSTR_ddrive, cmodule);
