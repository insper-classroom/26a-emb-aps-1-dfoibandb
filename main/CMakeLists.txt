add_executable(pico_emb main.c config.c audio.c buttons.c leds.c game.c)

set_target_properties(pico_emb PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_link_libraries(pico_emb pico_stdlib pico_multicore hardware_gpio hardware_pwm hardware_irq hardware_clocks hardware_sync)

pico_add_extra_outputs(pico_emb)
