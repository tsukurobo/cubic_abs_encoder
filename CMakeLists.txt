# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "~/pico/pico-sdk")

set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(cubic_abs_encoder C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(cubic_abs_encoder cubic_abs_encoder.cpp )

pico_set_program_name(cubic_abs_encoder "cubic_abs_encoder")
pico_set_program_version(cubic_abs_encoder "0.1")

pico_enable_stdio_uart(cubic_abs_encoder 0)
pico_enable_stdio_usb(cubic_abs_encoder 1)

# Add the standard library to the build
target_link_libraries(cubic_abs_encoder
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(cubic_abs_encoder PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(cubic_abs_encoder
        hardware_spi
        )

pico_add_extra_outputs(cubic_abs_encoder)

