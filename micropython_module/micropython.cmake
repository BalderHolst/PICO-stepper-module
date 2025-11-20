# This top-level micropython.cmake is responsible for listing
# the individual modules we want to include.
# Paths are absolute, and ${CMAKE_CURRENT_LIST_DIR} can be
# used to prefix subdirectories.

include(${CMAKE_CURRENT_LIST_DIR}/../stepperlib/CMakeLists.txt)

# Add the C example.
include(${CMAKE_CURRENT_LIST_DIR}/stepper_module/micropython.cmake)

# Generate compile_commands.txt
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
