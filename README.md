# OOSS
This will be developing OOSS (an OS) from scratch. This will be long journey. Lets hope we will keep this alive for long time :)

### we are using Ubuntu OS to develop OOSS
## Requirements 
1. ```sudo apt install mtools``` (we are using mcopy in Makefile)
2. ```sudo apt install nasm``` (to assemble assembly code)
3. ```sudo apt install qemu-system-x86``` (to run OS from CMD.)

## Steps to run OS
1. clone this repo
1. ```cd OOSS/```
2. ```make``` (to build project)
3. ```qemu-system-i386 -fda build/main_floppy.img``` (to boot OOSS virtually)
