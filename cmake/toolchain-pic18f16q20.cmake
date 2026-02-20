# Tell CMake we are cross compiling
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR pic18)

set(MICROCHIP_MCU PIC18F16Q20) 
set(DFP_PATH "C:/Users/Gard/.mchp_packs/Microchip/PIC18F-Q_DFP/1.28.451/xc8")
#set(ENV{MPLAB_PACK_ROOT} "C:/Users/Gard/.mchp_packs") 

# Prevent CMake from trying to run executables
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Set XC8 compiler
set(XC8_BIN_PATH "C:/Program Files/Microchip/xc8/v3.10/bin")
set(CMAKE_C_COMPILER ${XC8_BIN_PATH}/xc8-cc.exe)
set(AS  ${XC8_BIN_PATH}/xc8-cc.exe)
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_C_COMPILER_FORCED TRUE)
# XC8 flags
set(CMAKE_C_FLAGS_INIT "-mcpu=${MICROCHIP_MCU} -mdfp=${DFP_PATH}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-mcpu=${MICROCHIP_MCU} ")

# Optional: optimization/debug flags
set(CMAKE_C_FLAGS_DEBUG_INIT "--debug")
set(CMAKE_C_FLAGS_RELEASE_INIT "-O2")

# Output HEX instead of ELF
set(CMAKE_EXECUTABLE_SUFFIX ".hex")