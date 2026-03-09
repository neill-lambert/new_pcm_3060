cmake_minimum_required(VERSION 3.28)

# Set cross-compilation tools
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Skip compiler check for cross-compilation
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(new_pcm_3060 C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Processor-specific flags for STM32H723ZG
add_compile_options(
    -mcpu=cortex-m7
    -mfpu=fpv5-d16
    -mfloat-abi=hard
    -mthumb
    -ffunction-sections
    -fdata-sections
    -Wall
    -fstack-usage
    --specs=nano.specs
)

# Debug flags
add_compile_options("$<$<CONFIG:Debug>:-g3;-O0>")
add_compile_options("$<$<CONFIG:Release>:-O3>")

# Preprocessor definitions
add_definitions(
    -DSTM32H723xx
)

# Source files
set(SOURCES
    Core/Src/main.c
    Core/Src/pcm3060.c
    Core/Src/system_stm32h7xx.c
    Core/Src/stm32h7xx_it.c
    Core/Src/startup_stm32h723xx.s
    # CMSIS LL source files
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_gpio.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_rcc.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_utils.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_spi.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_dma.c
    Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_dma2d.c # Just in case, but maybe not needed
)

# Include directories
include_directories(
    Core/Inc
    Drivers/STM32H7xx_HAL_Driver/Inc
    Drivers/STM32H7xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32H7xx/Include
    Drivers/CMSIS/Include
)

# Linker flags
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32H723ZGTX_FLASH.ld)
# Note: Ensure the linker script exists or is provided.
add_link_options(
    -mcpu=cortex-m7
    -mfpu=fpv5-d16
    -mfloat-abi=hard
    -mthumb
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    --specs=nosys.specs
)

add_executable(${PROJECT_NAME}.elf ${SOURCES})

# Target binary conversions
# add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
#     COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
#     COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
# )
