# Create an INTERFACE library for our C module.
add_library(usermod_ddrive INTERFACE)

# Add our source files to the lib
target_sources(usermod_ddrive INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/ddrive_module.c
)

# Add the current directory as an include directory.
target_include_directories(usermod_ddrive INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link with the ddrive library.
target_link_libraries(usermod_ddrive INTERFACE
    ddrive
    pico_stdlib
    pico_multicore
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_ddrive)
