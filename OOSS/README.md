# OOSS
This is actual OOSS from scratch ☺️

Here I wrote kernel in c++ and kernel loader in x86_64 (not bootloader, I will use grub as bootloader)

Jobs that current OOSS can do :
1. greet to user by printing Hello message :)

### I am using Ubuntu to develop OOSS
## Requirements 
```bash
sudo apt-get install g++ binutils libc6-dev-i386 g++-multilib make qemu-system-x86 mtools xorriso grub-pc-bin
``` 

## Steps to run OOSS
### To boot from binary
1. ```make bin``` (to build project)
2. ```qemu-system-i386 -kernel ./build/mykernel.bin ```
### To boot from iso
1. ```make iso``` (to build project)
2. ```qemu-system-i386 -cdrom build/mykernel.iso```
### To boot from VirtualBox
1. ```make iso```
2. Create new machine in VirtualBox using iso file made in build/mykernel.iso (This is one time step)
3. KVM (Kernel-based Virtual Machine) have control on VT-x, So to give VT-x to VirualBox we have to remove KVM <br>
    ```sudo modprobe -r kvm_intel; sudo modprobe -r kvm # for intel processors```
4. ```make run```
5. After running you may turn on KVM module again <br>
    ```sudo modprobe kvm_intel; sudo modprobe kvm```


## Learnings or some extra :
### 1. Using ```extern "C"``` in C++ :
1. To prevent "name mangling" or "name decoration" (compiler modifies name of function or variable for some use cases).
2. Use ```extern "C"``` if that function or variable is used by some program which is outside of current C++ file. e.g kernMain, callConstructors, clearScreen
### 2. ```callConstructors``` function :
1. This function is to call constructor of global instances (constructor for local instance is called without any problem but for global instance we need this special function)
### 3.  ```readelf``` and ```objdump``` : Tools to examine binaries
1. Use readelf for understanding the ELF file structure and how it loads into memory. 
2. Use objdump for disassembling code and general-purpose inspection.
    
