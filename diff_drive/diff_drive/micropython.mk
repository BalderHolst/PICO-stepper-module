DIFF_DRIVE_MOD_DIR := $(USERMOD_DIR)

# Add all C files to SRC_USERMOD.
SRC_USERMOD += $(DIFF_DRIVE_MOD_DIR)/diff_drive_mod.c

# We can add our module folder to include paths if needed
# This is not actually needed in this example.
CFLAGS_USERMOD += -I$(DIFF_DRIVE_MOD_DIR)
