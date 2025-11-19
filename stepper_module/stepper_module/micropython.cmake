# Create an INTERFACE library for our C module.
add_library(usermod_stepper INTERFACE)

# Add our source files to the lib
target_sources(usermod_stepper INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/stepper_module.c
)

# Add the current directory as an include directory.
target_include_directories(usermod_stepper INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link with the stepper library.
target_link_libraries(usermod_stepper INTERFACE
    stepper
    pico_stdlib
    pico_multicore
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_stepper)
