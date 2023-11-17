EXAMPLE_MOD_DIR := $(USERMOD_DIR)

# Add all C files to SRC_USERMOD.
SRC_USERMOD += $(EXAMPLE_MOD_DIR)/examplemodule.c
SRC_USERMOD_LIB += $(EXAMPLE_MOD_DIR)/sqlite/sqlite3.c
# We can add our module folder to include paths if needed
# This is not actually needed in this example.
CFLAGS_USERMOD += -I$(EXAMPLE_MOD_DIR)
CEXAMPLE_MOD_DIR := $(USERMOD_DIR)
