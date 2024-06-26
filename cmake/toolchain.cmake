set(TOOLCHAIN aarch64-elf)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR BCM2837)

# if toolchain path exists, append /bin/ to it
if(EXISTS ${TOOLCHAIN_PATH})
    set(TOOLCHAIN_PATH ${TOOLCHAIN_PATH}/bin/)
endif()


set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}aarch64-elf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}aarch64-elf-g++)

set(CMAKE_OBJCOPY ${TOOLCHAIN_PATH}aarch64-elf-objcopy
    CACHE FILEPATH "objcopy tool" FORCE)
set(CMAKE_OBJDUMP ${TOOLCHAIN_PATH}aarch64-elf-objdump
    CACHE FILEPATH "objdump tool" FORCE)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_CXX_COMPILER_WORKS 1)

set(CMAKE_FIND_ROOT_PATH ${COMPILER_PATH})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# settings
set(CMAKE_CXX_FLAGS_INIT "-fpic -ffreestanding -fno-exceptions -fno-rtti -fno-threadsafe-statics -fpermissive -mcpu=cortex-a53+nosimd -mgeneral-regs-only")
set(CMAKE_SHARED_LINKER_FLAGS "-nostdlib -T ${CMAKE_SOURCE_DIR}/linker.ld -M ${CMAKE_BINARY_DIR}/kernel.map")