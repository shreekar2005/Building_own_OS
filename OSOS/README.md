# OSOS
> This is actual OSOS from scratch ☺️. Here I am writing 32bit kernel in c++ and kernel loader in x86_64 (not bootloader, I will use grub as bootloader)

---
---

### I am using Ubuntu24.04 to develop OSOS
## Requirements 
### 1. Install reqrired tools
`bash
sudo apt-get install g++ binutils libc6-dev-i386 g++-multilib make qemu-system-x86 mtools xorriso grub-pc-bin
` 
### 2. GCC cross-compiler (i686-elf-TOOL)
Follow the Instructions in `../cross-compiler` and **make your own cross-compiler**. You will need cross-compiler due to some reasons that you will find in cross-compiler README.

#### If you dont want to make cross-compiler (which is not good idea) then you can go to this commit `f500e99459b3d1cf9e592b4be19fe4e2706ef2db` and follow current project and check whether you can build OSOS without cross compiler 🙂

---
---

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
    <!-- <br> <img src="./ScreenShots/image2.png" width="500" alt="OS and Version Settings"> <br> -->
    5. Set Base Memory = 4GB
    <!-- <br> <img src="./ScreenShots/image3.png" width="500" alt="Base Memory Setting"> <br> -->
    6. Finish
### If you have Virtual Machine configured then you can directly run following command to start OSOS_Machine
```bash
make vm # it will build OSOSkernel.iso and boot with Virtual Machine (May ask for your sudo password)
```

### *Read "Instructions" written in "Makefile" for more make procedures.*
---
---

## What things are implemented in OSOS:

1. Custom kernel library headers (checkout `./kernel_src/include` for headers and `./libk_src/` for their source code)
    1. **kiostream** : printf(), clearScreen(), enable/update/disable_cursor()
    2. **kmemory** : printMemoryMap(), new()/delete() baby definitions ***(will update letter)***
    3. **kgdt** : class GDT with 1.methods : installTable | 2.static functions : kernel/user_CS/DS_selector()
    4. **kport** : class Port (which is base for class Port8bit, Port8bitslow, Port16bit, Port32bit) with methods : write(), read()
    5. **kicxxabi** : __callConstructors(), __cxa_finalize()
    6. **kinterrupt** : Have InterruptManager, which can manage interrupts. 
    7. **kkeyboard** : Keyboard driver to handle keyboard interrupt
    8. **kmouse** : Mouse driver to handle mouse interrupts


2. Accessed multiboot info structure provided by grub bootloader.

3. Calling global object constructors and destructors which are listed in `.ctors` and `.dtors` sections of corresponding object files.

4. <br> <img src="./ScreenShots/image6.png" width="300" alt="Base Memory Setting"> <br>
    - Initialized **Global Descriptor Table** (GDT) : Currently I am not separating kernel and user space (Ring0 and Ring3), That is security issue; but I will implement paging in future so there is no need for separating kernel and user space in GDT (currently flat memory)
    - Initialized **Interrupt Descriptor Table** (IDT) to enable interrupts

5. Can handle keyboard input using Interrupt Service Routine.

---
---

## To correctly compile, link and run OSOS

### 1. Using `extern "C"`keyword :
- To prevent "name mangling" or "name decoration" (compiler modifies name of function or variable for some use cases).
- Use `extern "C"` if that function or variable is used by some program which is outside of current C++ file. e.g kernMain, callConstructors, clearScreen

### 2. `kicxxabi.cpp` in libk_src :
- This is **Kernel Internal C++ Application Binary Interface**. Used by compiler while setting up virtual destructors etc.
- **__callConstructors()** and **__cxa_finalize()** are implemented to call global constructors and destructors

---
---

## Using GDB (debugger) with our OSOS (running on qemu)
1. Run QEMU with `-s` (for connecting with GDB on port 1234) and `-S` (To pause VM at very begginning. Resume VM with `c` in GDB).
2. In another terminal start gdb, in gdb connect with remote target (qemu VM) using `target remote localhost:1234`
3. Load symbol table using any symbol file of our OS : `symbol-file kernel_build/OSOSkernel.elf`
4. Set breakpoints : e.g. `break kernelMain`
5. Continue QEMU with `c` (which will pause at next breakpoint).
- For good understanding you can see this "[YouTube Video Tutorial on "Using GDB for OS dev](https://youtu.be/H7SIDK6CiA4?si=swEXoYi1ppiO0EiO)" by me (***HINDI***) :)

---
---

## Some extra :

### 1.  `readelf` and `objdump` : Tools to examine binaries
- Use readelf for understanding the ELF file structure and how it loads into memory. 
- Use objdump for disassembling code and general-purpose inspection.

### 2. Using `ghidra` for examine binaries (reverse engg) :
1. Go to Ghidra github repository : [official Ghidra github link](https://github.com/NationalSecurityAgency/ghidra)
2. Go to releases and download zip, `unzip` it, `cd` to it, run `./ghidraRun`. 

### 3. My personal rules while developing OSOS (IMP if you are thinking to Contribute)
- Using `#include <>` for standard libraries
- Using `#include ""` for OSOS specific libraries, e.g. libraries in `./kernel_src/include/`
- Naming OSOS kernel libraries with prefix `'k'`, e.g. kiostream, kmemory, etc
- **Naming convensions :**  pointer names : `abc_xyz_pqr`, class/struct names :  `AbcXyzPqr` or `ABC_XyzPqr`, function/variable names : `abcXyzPqr`
- all kernel library functions, classes, variables are divided in **`namespaces`** : 
    1. **basic** : kiostream, kmemory
    2. **driver** : kdriver, kkeyboard, kmouse
    3. **essential** : kgdt, kicxxabi
    4. **hardware_communication** : kinterrupt, kport

### 4. Interrupts :
- **PIC (Programmable Interrupt Controller):** It recieves hardware Interrupt Requests (IRQs) and send the interruptNumber to CPU. CPU then go to IDT entry correspondin to interrupt number. There are 2 PICs (Master and Slave) and Slave is attached to one of input line of Master PIC
- There are **Software Interrupts** that are recieved by CPU (e.g. divide by zero with interruptNumber = 0x00). So we should make their handlers also and keep their reference in IDT.
- **Main Issue of PIC** : Actually PIC also recives IRQ number from 0x00 (e.g. Timer IRQnumber=0x00, Keyboard IRQnumber=0x01). So PIC just cannot forward that number to CPU because then CPU will just call same handler for 'divide by zero' and 'Timer' interrupt. So we will add some offset(0x20 for Master PIC and 0x28 for Slave PIC) such that new interrupt number of IRQ will not conflict with Software Interrupt number.

---
---


## One thing which blew my mind :
In early days, many machines have only support to 32bit programs (only 4GB addressable memory). After introduction to 64bit systems some people developed **UEFI** (which is BIOS but it have mini-OS which runs in 64bit). Now I have ***ASUS TUF F15 (GAMING 2022)*** in which there is no option to keep BIOS in **"Legacy Mode"** or turn on **"CSM"**. So on my machine i cannot boot in 32bit OS. In today's date there are many laptops which supports both modern and legacy mode, but my laptop sucks :(. If you want to boot 32bit OSOS then you have to first disable **"Secure boot"** and then turn on **"CSM"** or switch to **"Legacy Mode"** from BIOS

---
---