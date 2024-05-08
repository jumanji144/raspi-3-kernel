CC = $(HOME)/opt/cross/bin/aarch64-elf-g++
LD = $(HOME)/opt/cross/bin/aarch64-elf-ld
AS = $(HOME)/opt/cross/bin/aarch64-elf-as
OBJCPY = $(HOME)/opt/cross/bin/aarch64-elf-objcopy

# Broadcom BCM2837B0, Cortex-A53 (ARMv8) 64-bit SoC
CFLAGS = -fpic -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -fno-threadsafe-statics -fpermissive -g -mcpu=cortex-a53+nosimd
LDFLAGS = -nostdlib -g

KERNEL_SRC = src/kernel
KERNEL_INCLUDE = include
COMMON_SRC = src/common

OBJ_DIR = build
IMG_NAME=kernel8.img

KERNEL_C_FILES = $(wildcard $(KERNEL_SRC)/*.cpp)
COMMON_C_FILES = $(wildcard $(COMMON_SRC)/*.cpp)
KERNEL_ASM_FILES = $(wildcard $(KERNEL_SRC)/*.S)

HEADERS = $(wildcard $(KERNEL_INCLUDE)/*.h)


OBJECTS = $(patsubst $(KERNEL_SRC)/%.cpp, $(OBJ_DIR)/%.o, $(KERNEL_C_FILES))
OBJECTS += $(patsubst $(COMMON_SRC)/%.cpp, $(OBJ_DIR)/%.o, $(COMMON_C_FILES))
OBJECTS += $(patsubst $(KERNEL_SRC)/%.S, $(OBJ_DIR)/%.o, $(KERNEL_ASM_FILES))

$(IMG_NAME): $(OBJECTS) $(HEADERS)
	$(LD) -M -T linker.ld -o kernel.elf $(LDFLAGS) $(OBJECTS) > kernel.map
	$(OBJCPY) kernel.elf -O binary $(IMG_NAME)

$(OBJ_DIR)/%.o: $(KERNEL_SRC)/%.cpp
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(KERNEL_INCLUDE) -c $< -o $@

$(OBJ_DIR)/%.o: $(KERNEL_SRC)/%.S
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(KERNEL_SRC) -I$(KERNEL_INCLUDE) -c $< -o $@

$(OBJ_DIR)/%.o: $(COMMON_SRC)/%.cpp
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(KERNEL_INCLUDE) -c $< -o $@

all: $(IMG_NAME)

clean: 
	rm -rf $(OBJ_DIR)
	rm kernel.elf $(IMG_NAME)

-include $(OBJECTS:.o=.d)