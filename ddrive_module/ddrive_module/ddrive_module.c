// Include MicroPython API.
#include "py/runtime.h"

#include "stepper.h"

// `Stepper` class

static mp_obj_t Stepper_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // Allocates the new object and sets the type.
    Stepper *self = mp_obj_malloc(Stepper, type);

    // TODO: Initialize timer members

    return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t Stepper_test(mp_obj_t self_in) {
    Stepper *self = MP_OBJ_TO_PTR(self_in);
    (void)self;  // Suppress unused variable warning.
    return mp_obj_new_int(42);
}
static MP_DEFINE_CONST_FUN_OBJ_1(Stepper_test_obj, Stepper_test);


static const mp_rom_map_elem_t Stepper_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_test), MP_ROM_PTR(&Stepper_test_obj) },
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
