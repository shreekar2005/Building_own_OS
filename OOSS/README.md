# OOSS
This is actual OOSS from scratch ☺️

Here we wrote kernel in c++ and kernel loader in x86_64 (not bootloader, we will use grub as bootloader)

Jobs that current OOSS can do :
1. greet to user by printing Hello message :)

### we are using Ubuntu to develop OOSS
## Requirements 
```sudo apt-get install g++ binutils libc6-dev-i386 g++-multilib make qemu-system-x86``` 

## Steps to run baby_OOSS
1. clone this repo
1. ```cd OOSS/OOSS```
2. ```make``` (to build project)
3. ```qemu-system-i386 -kernel mykernel.bin``` (to boot OOSS virtually)
