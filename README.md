# Raspi 3 kernel
This project is a small project i started to see if i could build a small kernel for the raspberry pi 3.
I initially chose C but then i rewrote it in C++, using it's advanced compiler capabilities to make everything
stack based.

## Plans
My plans for this kernel are:
- [x] Simple boot code
- [x] Write to the UART
- [x] Setup mailbox proprety messages
- [ ] Obtain a framebuffer and write to it
- [ ] Initialize the MMU and setup virtual memory
- [ ] Write a simple scheduler
- [ ] Write a simple filesystem
- [ ] Write a simple shell
- [ ] Go over to usermode
- [ ] Syscalls and usermode programs
- [ ] Vpage heap allocator for usermode programs
- [ ] Simple text editor in usermode
- [ ] Window compositor
- [ ] Window manager
- [ ] GUI for my text editor

## Building

### Prerequisites
Firstly you need to have build-devel installed to have the base tools to build a application.
You will need a cross compiler for aarch64, atleast the gcc and binutils packages.

### Creating the image
To create the image you will need to run the following commands:
```bash
make all
```

### Running the image
To run the image either run it in qemu via:
```bash
qemu-system-aarch64 -M raspi3b -serial null -serial stdio -kernel kernel8.img
```
Or to install it on a sd card, follow these instructions:

#### On a real raspberry pi
Firstly you will need a SD Card with enough space to hold the image and the bootloader.

If you don't want the hassle of creating the partition and putting the files in yourself, simply download something like 
raspbian and simply delete all the .img files and put in the kernel8.img file.

If you want to do it yourself, follow these instructions:
- Create a MBR partition table on the SD Card
- Create a primary partition with the type 0x0c (FAT32)
- Format the partition with FAT32
- Download bootcode.bin, start.elf and fixup.dat from the (raspberry pi firmware repository)[https://github.com/raspberrypi/firmware/tree/master/boot]
- Put the kernel8.img file in the root of the partition

Here is a linux script to do it:
```bash
device=/dev/sdX

parted $device mklabel msdos
parted -a optimal $device mkpart primary fat32 0% 100MB
mkfs.fat -F 32 ${device}1

mount ${device}1 /mnt/raspi-sd
wget https://github.com/raspberrypi/firmware/raw/master/boot/bootcode.bin -O /mnt/raspi-sd/bootcode.bin
wget https://github.com/raspberrypi/firmware/raw/master/boot/start.elf -O /mnt/raspi-sd/start.elf
wget https://github.com/raspberrypi/firmware/raw/master/boot/fixup.dat -O /mnt/raspi-sd/fixup.dat

cp kernel8.img /mnt/raspi-sd/kernel8.img

umount /mnt/raspi-sd
```
