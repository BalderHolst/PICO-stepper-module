add_library(stepper STATIC
    ${CMAKE_CURRENT_LIST_DIR}/ddrive.c
    ${CMAKE_CURRENT_LIST_DIR}/stepper.c
)

target_include_directories(stepper PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(stepper
    pico_stdlib
    hardware_pwm
)

target_include_directories(stepper PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
