export PREFIX="$HOME/cross-compiler/build"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"
export BINUTILS_VERSION=2.45
export GDB_VERSION=16.3
export GCC_VERSION=15.2.0

# This line tells all 'make' commands in this script to run in parallel
# $nproc have number of CPU cores
export MAKEFLAGS="-j$(nproc)"

# ------------------ Binutils ------------------
cd $HOME/cross-compiler/src

mkdir build-binutils
cd build-binutils
../binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install


# ------------------ GDB ------------------
../gdb.$GDB_VERSION/configure --target=$TARGET --prefix="$PREFIX" --disable-werror
make all-gdb
make install-gdb


# ------------------ GCC ------------------

cd $HOME/cross-compiler/src

# The $PREFIX/bin dir must be in the PATH. We did that above.
which -- $TARGET-as || echo $TARGET-as is not in the PATH

mkdir build-gcc
cd build-gcc
../gcc-$GCC_VERSION/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers --disable-hosted-libstdcxx
make all-gcc
make all-target-libgcc
make all-target-libstdc++-v3
make install-gcc
make install-target-libgcc
make install-target-libstdc++-v3