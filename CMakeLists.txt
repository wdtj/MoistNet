cmake_minimum_required(VERSION 3.21)

include(pico_sdk_import.cmake)

set(PICO_BOARD pico_w)

project(MoistNet)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(MoistNet
    MoistNet.cpp
    pico_http_client.c
    socket.c
    socket_impl.c
)

target_include_directories(MoistNet PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        )

target_link_libraries(MoistNet 
    pico_stdlib
    hardware_adc
    pico_cyw43_arch_lwip_threadsafe_background    
)

# enable usb output, disable uart output
pico_enable_stdio_usb(MoistNet 1) 
pico_enable_stdio_uart(MoistNet 0)

pico_add_extra_outputs(MoistNet)
