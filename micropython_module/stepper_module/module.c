#include "py/obj.h"

#include <pico/stdlib.h>

#include "stepper_class.h"
#include "ddrive_class.h"

static const mp_rom_map_elem_t module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),  MP_ROM_QSTR(MP_QSTR_stepper) },
    { MP_ROM_QSTR(MP_QSTR_Stepper),   MP_ROM_PTR(&type_Stepper)   },
    { MP_ROM_QSTR(MP_QSTR_DiffDrive), MP_ROM_PTR(&type_DiffDrive) },
};
static MP_DEFINE_CONST_DICT(module_globals, module_globals_table);

// Define python module object.
const mp_obj_module_t cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&module_globals,
};

// Register the module to make it available in Python.
MP_REGISTER_MODULE(MP_QSTR_stepper, cmodule);
