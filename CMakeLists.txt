cmake_minimum_required(VERSION 3.2)
set(CMAKE_TOOLCHAIN_FILE external/cmake-microchip/toolchain.cmake)
set(MICROCHIP_MCU PIC18F16Q20)

set(CMAKE_PROJECT_NAME stepper)
project(${CMAKE_PROJECT_NAME} C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wpedantic")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "C:/Program Files/Microchip/xc8/v3.10/pic/include",
     ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
)