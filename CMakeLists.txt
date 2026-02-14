cmake_minimum_required(VERSION 3.5)
set(CMAKE_TOOLCHAIN_FILE external/cmake-microchip/toolchain.cmake)
set(MICROCHIP_MCU PIC18F16Q20)

set(ENV{MPLAB_PACK_ROOT} "C:/Users/Gard/.mchp_packs")
set(CMAKE_C_COMPILER_ID_RUN TRUE)
set(CMAKE_C_COMPILER_WORKS TRUE)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_PROJECT_NAME stepper)
project(${CMAKE_PROJECT_NAME} C ASM)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wpedantic")
#set(CMAKE_C_FLAGS_DEBUG "-g")
#set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_C_COMPILER_FORCED TRUE)

include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "C:/Program Files/Microchip/xc8/v3.10/pic/include"
     ${CMAKE_CURRENT_SOURCE_DIR}/inc
     ${CMAKE_CURRENT_SOURCE_DIR}/core
     ${CMAKE_CURRENT_SOURCE_DIR}/core/hw/inc
)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
)