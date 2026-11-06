cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(rp2040_film_kb C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

# 🔥 更新：所有 .c 文件加上 src/ 前缀
add_executable(rp2040_film_kb
    src/main.c
    src/usb_descriptors.c
    src/keymap.c
    src/matrix.c
    src/hid.c
    src/macro.c
    src/report.c
    src/webhid.c
)

# 🔥 关键：将 include 和 src 目录都加入头文件搜索路径
target_include_directories(rp2040_film_kb PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(rp2040_film_kb
    pico_stdlib
    tinyusb_device
    tinyusb_board
    hardware_flash
)

pico_enable_stdio_usb(rp2040_film_kb 0)
pico_enable_stdio_uart(rp2040_film_kb 0)
pico_add_extra_outputs(rp2040_film_kb)