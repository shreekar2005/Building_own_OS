# OOSS
This is actual OOSS from scratch ☺️

Here I wrote kernel in c++ and kernel loader in x86_64 (not bootloader, I will use grub as bootloader)

Jobs that current OOSS can do :
1. greet to user by printing Hello message :)

### I am using Ubuntu to develop OOSS
## Requirements 
```sudo apt-get install g++ binutils libc6-dev-i386 g++-multilib make qemu-system-x86``` 

## Steps to run OOSS
1. clone this repo
1. ```cd OOSS/OOSS```
2. ```make``` (to build project)
3. ```qemu-system-i386 -kernel ./build/mykernel.bin -m 256M``` (to boot OOSS virtually with 256MB RAM)

## Learnings or some extra :
### 1. Using ```extern "C"``` in C++ :
1. To prevent "name mangling" or "name decoration" (compiler modifies name of function or variable for some use cases).
2. Use ```extern "C"``` if that function or variable is used by some program which is outside of current C++ file. e.g kernMain, callConstructors, clearScreen
### 2. ```callConstructors``` function :
1. This function is to call constructor of global instances (constructor for local instance is called without any problem but for global instance we need this special function)
### 3.  ```readelf``` and ```objdump``` : Tools to examine binaries
1. Use readelf for understanding the ELF file structure and how it loads into memory. 
2. Use objdump for disassembling code and general-purpose inspection.
    
