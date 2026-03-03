cmake_minimum_required(VERSION 3.5)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/external/cmake-microchip/toolchain.cmake)

set(MICROCHIP_MCU PIC18F16Q20) 
set(CMAKE_C_COMPILER_ID_RUN TRUE) 
set(CMAKE_C_COMPILER_WORKS TRUE) 


set(CMAKE_PROJECT_NAME stepper) 
project(${CMAKE_PROJECT_NAME} C ASM)
enable_language(C ASM)
set(CMAKE_C_COMPILER_FORCED TRUE) 

# get path to xc8 for includes and lib
if(NOT DEFINED XC8_PATH)
    set(XC8_PATH "C:/Program Files/Microchip/xc8/v3.10")
endif()

# get Device Support Package
if(NOT DEFINED DFP_PATH)
    set(DFP_PATH "$ENV{USERPROFILE}/.mchp_packs/Microchip/PIC18F-Q_DFP")
    file(GLOB DFP_VERSIONS "${DFP_PATH}/*")
    if(DFP_VERSIONS)
        list(SORT DFP_VERSIONS)
        list(REVERSE DFP_VERSIONS)
        list(GET DFP_VERSIONS 0 DFP_PATH_LATEST)
    else()
        message(FATAL_ERROR "No support pack found for ${MICROCHIP_MCU} in ${DFP_PATH}")
    endif()
    message(STATUS "Using DFP: ${DFP_PATH_LATEST}")
    set(DFP_PATH ${DFP_PATH_LATEST})
endif()

if(NOT CMAKE_BUILD_TYPE) 
    set(CMAKE_BUILD_TYPE Release) 
endif() 

file(GLOB_RECURSE SOURCES
    src/*.c
    core/src/*.c
    core/hw/src/*.c
    stepper/*.c
)
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${XC8_PATH}/pic/include
    ${CMAKE_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/stepper
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/core/hw/inc
)

# required for linking
target_compile_options(${PROJECT_NAME} PRIVATE
    -mcpu=${MICROCHIP_MCU}
    -mdfp=${DFP_PATH}/xc8
)
target_link_options(${PROJECT_NAME} PRIVATE
    -mcpu=${MICROCHIP_MCU}
    -mdfp=${DFP_PATH}/xc8
)

set_target_properties(stepper PROPERTIES SUFFIX ".hex")