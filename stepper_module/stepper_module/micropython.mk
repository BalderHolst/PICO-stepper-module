STEPPER_MOD_DIR := $(USERMOD_DIR)

# Add all C files to SRC_USERMOD.
SRC_USERMOD += $(STEPPER_MOD_DIR)/stepper_module.c

# We can add our module folder to include paths if needed
# This is not actually needed in this example.
CFLAGS_USERMOD += -I$(STEPPER_MOD_DIR)
