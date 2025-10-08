# OSOS
This is actual OSOS from scratch ☺️

Here I am writing 32bit kernel in c++ and kernel loader in x86_64 (not bootloader, I will use grub as bootloader)

### I am using Ubuntu24.04 to develop OSOS
## Requirements 
### 1. Install reqrired tools
`bash
sudo apt-get install g++ binutils libc6-dev-i386 g++-multilib make qemu-system-x86 mtools xorriso grub-pc-bin
` 
### 2. GCC cross-compiler (i686-elf-TOOL)
Follow the Instructions in `../cross-compiler` and **make your own cross-compiler**. You will need cross-compiler due to some reasons that you will find in cross-compiler README.

#### If you dont want to make cross-compiler (which is not good idea) then you can go to this commit `f500e99459b3d1cf9e592b4be19fe4e2706ef2db` and follow current project and check whether you can build OSOS without cross compiler 🙂

## Steps to run OSOS
### To boot from binary (Using QEMU PC System emulator)
```bash
make # it will build OSOSkernel.bin and boot with QEMU 
```
<br> <img src="./ScreenShots/image1.png" width="600" alt="Running OSOS on QEMU"> <br>

### To boot from ISO (Using Virtual Box)
#### Before booting from ISO (VirtualBox) Create new machin using following instructions :
1. `make iso`
2.  Follow following steps to create Virtual Machine in Virtual Box
    1. Create New Machine in Virtual Box
    2. Set its name = "OSOS_Machine"
    3. Set ISO image = "our OSOS kernel ISO" (build by `make iso`)
    4. Set OS= "Other", Set OS Version = "Other/Unknown"
    <br> <img src="./ScreenShots/image2.png" width="600" alt="OS and Version Settings"> <br>
    5. Set Base Memory = 4GB
    <br> <img src="./ScreenShots/image3.png" width="600" alt="Base Memory Setting"> <br>
    6. Finish
### If you have Virtual Machine configured then you can directly run following command to start OSOS_Machine
```bash
make vm # it will build OSOSkernel.iso and boot with Virtual Machine
```


## What things are implemented in OSOS:
1. Custom kernel library headers (checkout `./kernel_src/include` for headers and `./libk_src/` for their source code)
    1. **console** : printf(), keyboard_input_by_polling(), clearScreen(), enable/update/disable_cursor()
    2. **gdt** : inspect_gdt()

2. Accessed multiboot info structure provided by grub bootloader.
3. Calling global object constructors and destructors which are listed in `.init_array` and `.fini_array` sections of corresponding object files. (I am listing them in `.ctors_dtors` section in `OSOSkernel.bin` created by linker script)
4. Can use keyboard input by Polling method. (Without Interrupt Service)

## Learnings or some extra :
### 1. Using `extern "C"` in C++ :
1. To prevent "name mangling" or "name decoration" (compiler modifies name of function or variable for some use cases).
2. Use `extern "C"` if that function or variable is used by some program which is outside of current C++ file. e.g kernMain, callConstructors, clearScreen
### 2. `callConstructors` function :
1. This function is to call constructor of global instances (constructor for local instance is called without any problem but for global instance we need this special function (actually in gcc standard libs, `crt0.o` take care of all this))
### 3.  `readelf` and `objdump` : Tools to examine binaries
1. Use readelf for understanding the ELF file structure and how it loads into memory. 
2. Use objdump for disassembling code and general-purpose inspection.
### 4. Using `ghidra` for examine binaries (reverse engg) :
1. Go to Ghidra github repository : [official Ghidra github link](https://github.com/NationalSecurityAgency/ghidra)
2. Go to releases and download zip, `unzip` it, `cd` to it, run `./ghidraRun`. 