# Cross compiler
## [Why I need Cross Compiler](https://wiki.osdev.org/Why_do_I_need_a_Cross_Compiler%3F)
## I referred this web-page to build cross compiler : [osdev wiki page](https://wiki.osdev.org/GCC_Cross-Compiler)

## Following process is for Ubuntu 24.04.3 LTS (may work on some other linux distros, idk that)

### Requirements
``` bash
sudo apt-get update
sudo apt-get install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo libisl-dev
```
### Steps to make cross-compiler (ensure that you run this script from directory which contains those tar.xz files)
#### 1. Install dependencies that building will require
```bash
sudo apt install build-essential xz-utils libgmp-dev libmpfr-dev libmpc-dev libexpat1-dev texinfo bison flex gawk libtool zlib1g-dev autoconf automake
```
#### 2. You should be in "this" directory where tarballs are there before running script
```bash
cd <path_to_repo>/cross-compiler
```
#### 3. Give execute access and run script
```bash
sudo chmod +x ./build_cross_compiler.sh & ./build_cross_compiler.sh
```

### Libraries will be in `~/cross-compiler/i686-elf/build/bin/`

### You will find these new tools in terminal :) (if you add bin path to environment PATH variable, better to update `~/.bashrc`)
```
osos-addr2line     osos-ar            osos-as            osos-c++           osos-c++filt
osos-cpp           osos-elfedit       osos-g++           osos-gcc           osos-gcc-15.2.0
osos-gcc-ar        osos-gcc-nm        osos-gcc-ranlib    osos-gcov          osos-gcov-dump
osos-gcov-tool     osos-gdb           osos-gdb-add-index osos-gprof         osos-gstack
osos-ld            osos-ld.bfd        osos-lto-dump      osos-nm            osos-objcopy
osos-objdump       osos-ranlib        osos-readelf       osos-size          osos-strings
osos-strip
```