cmake_minimum_required(VERSION 3.10)

project(raspi
        LANGUAGES CXX ASM
        VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

add_executable(raspi
        src/common/mem.cpp
        src/common/string.cpp

        src/kernel/boot.S
        src/kernel/kernel.cpp
        src/kernel/uart1.cpp
        src/kernel/exception.cpp
        src/kernel/mailbox.cpp
        src/kernel/buffer.cpp
        src/kernel/gpu.cpp
)
include_directories(include)

target_link_options(raspi PRIVATE
        -T ${CMAKE_SOURCE_DIR}/linker.ld
        -nostdlib
        -nostartfiles
        -ffreestanding)

target_compile_options(raspi PRIVATE -O0)

add_custom_command(
        TARGET raspi POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME} kernel8.img
        COMMAND ${CMAKE_COMMAND} -E copy kernel8.img ${CMAKE_SOURCE_DIR}/kernel8.img
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/kernel.elf
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Convert ELF to binary"
)


