# Cross compiler
## [Why I need Cross Compiler](https://wiki.osdev.org/Why_do_I_need_a_Cross_Compiler%3F)
## I referred this web-page to build cross compiler : [osdev wiki page](https://wiki.osdev.org/GCC_Cross-Compiler)

## Following process is for Ubuntu24.04 (may work on some distro, idk that)

### Requirements
``` bash
sudo apt-get update
sudo apt-get install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo libisl-dev
```
### Steps to make cross-compiler
1. ```bash
    cd cross-compiler
    ```
2. ```bash
    mkdir -p $HOME/cross-compiler/src && tar -xf binutils-2.45.tar.xz -C $HOME/cross-compiler/src
    ```
3. ```bash
    mkdir -p $HOME/cross-compiler/src && tar -xf gdb-16.3.tar.xz -C $HOME/cross-compiler/src
    ```
4. ```bash
    mkdir -p $HOME/cross-compiler/src && tar -xf gcc-15.2.0.tar.xz -C $HOME/cross-compiler/src
    ```
5. ```bash
    sudo chmod +x ./build_cross_compiler.sh
    ```
6. ```bash
    ./build_cross_compiler.sh
    ```

### Now add ```$HOME/cross-compiler/build/bin``` to your environment PATH variable (if you use bash, update ```~/.bashrc```)

### You will find these new tools in terminal :) 
```
i686-elf-addr2line  i686-elf-ar         i686-elf-as         i686-elf-c++
i686-elf-c++filt    i686-elf-cpp        i686-elf-elfedit    i686-elf-g++
i686-elf-gcc        i686-elf-gcc-15.2.0 i686-elf-gcc-ar     i686-elf-gcc-nm
i686-elf-gcc-ranlib i686-elf-gcov       i686-elf-gcov-dump  i686-elf-gcov-tool
i686-elf-gprof      i686-elf-ld         i686-elf-ld.bfd     i686-elf-lto-dump
i686-elf-nm         i686-elf-objcopy    i686-elf-objdump    i686-elf-ranlib
i686-elf-readelf    i686-elf-size       i686-elf-strings    i686-elf-strip
```