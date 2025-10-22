#!/bin/bash

# This will make the script exit immediately if any command fails
set -e

# ------------------ Environment ------------------
# Define a base directory for the whole project
BASE_DIR="$HOME/cross-compiler/i686-elf"

# Final compiler will be installed in $BASE_DIR/build
export PREFIX="$BASE_DIR/build"

# Source code (binutils-2.45, gcc-15.2.0, etc.) is in $BASE_DIR/src
SRC_DIR="$BASE_DIR/src"

# The target must specify the CPU architecture.
export TARGET=i686-elf

export PATH="$PREFIX/bin:$PATH"

export BINUTILS_VERSION=2.45
export GDB_VERSION=16.3
export GCC_VERSION=15.2.0

# Run make in parallel
export MAKEFLAGS="-j$(nproc)"

# ------------------ Clean-up Check ------------------
# Check if the main project directory already exists
if [ -d "$BASE_DIR" ]; then
    echo "⚠️  Existing project directory found at: $BASE_DIR"
    
    read -p "Do you want to remove it completely and start a fresh build? [y/N] " -r REPLY
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "--- Removing existing directory: $BASE_DIR ---"
        rm -rf "$BASE_DIR"
    else
        echo "Aborting." && exit 1
    fi
fi


# Create the source directory
mkdir -p $SRC_DIR

# ------------------ Source Extraction ------------------
echo "--- Checking for source tarballs in current directory... ---"

# Check if all required files exist before starting
if [ ! -f "binutils-$BINUTILS_VERSION.tar.xz" ]; then
    echo "Error: binutils-$BINUTILS_VERSION.tar.xz not found." && exit 1
fi
if [ ! -f "gdb-$GDB_VERSION.tar.xz" ]; then
    echo "Error: gdb-$GDB_VERSION.tar.xz not found." && exit 1
fi
if [ ! -f "gcc-$GCC_VERSION.tar.xz" ]; then
    echo "Error: gcc-$GCC_VERSION.tar.xz not found." && exit 1
fi

echo "--- All source tarballs found. ---"
echo "--- Extracting sources to $SRC_DIR ---"


#--------------------- Extract tarballs ---------------------
# Extract all three tarballs into the source directory
tar -xf binutils-$BINUTILS_VERSION.tar.xz -C $SRC_DIR
tar -xf gdb-$GDB_VERSION.tar.xz -C $SRC_DIR
tar -xf gcc-$GCC_VERSION.tar.xz -C $SRC_DIR

echo "--- Sources extracted. ---"


# ------------------ Binutils ------------------
echo "--- Building Binutils ---"
mkdir -p $BASE_DIR/build-binutils
cd $BASE_DIR/build-binutils
$SRC_DIR/binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install

# ------------------ GDB ------------------
echo "--- Building GDB ---"
mkdir -p $BASE_DIR/build-gdb
cd $BASE_DIR/build-gdb
$SRC_DIR/gdb-$GDB_VERSION/configure --target=$TARGET --prefix="$PREFIX" --disable-werror
make all-gdb
make install-gdb

# ------------------ GCC ------------------
echo "--- Building GCC ---"
which -- $TARGET-as || (echo "$TARGET-as is not in the PATH. Aborting." && exit 1)
mkdir -p $BASE_DIR/build-gcc
cd $BASE_DIR/build-gcc
$SRC_DIR/gcc-$GCC_VERSION/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers --disable-hosted-libstdcxx
echo "--- Making GCC ---"
make all-gcc
make all-target-libgcc
make all-target-libstdc++-v3
echo "--- Installing GCC ---"
make install-gcc
make install-target-libgcc
make install-target-libstdc++-v3


# ------------------ Create Symlinks (All Tools) ------------------
echo "--- Creating custom symlinks in $PREFIX/bin ---"

CUSTOM_NAME="osos"
cd $PREFIX/bin

for file in ${TARGET}-*; do
    suffix="${file#${TARGET}-}"
    ln -s "$file" "${CUSTOM_NAME}-${suffix}"
done

echo "--- ✅ Cross-compiler build complete! ---"
echo "--- Binaries are in: $PREFIX/bin ---"
echo "--- You can use '${TARGET}-gcc' or '${CUSTOM_NAME}-gcc' ---"


# ------------------ Add to .bashrc ------------------
PATH_TO_ADD="$PREFIX/bin"
BASHRC_FILE="$HOME/.bashrc"
PATH_LINE="export PATH=\"$PATH_TO_ADD:\$PATH\""

if grep -Fxq "$PATH_LINE" "$BASHRC_FILE"; then
    echo
    echo "--- Your .bashrc already includes this PATH. No changes made. ---"
else
    echo
    read -p "Do you want to add the compiler to your .bashrc to make it available in all new terminals? [y/N] " -r REPLY
    echo
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "--- Appending to $BASHRC_FILE... ---"
        echo "" >> "$BASHRC_FILE"
        echo "# Add '$CUSTOM_NAME' cross-compiler to PATH" >> "$BASHRC_FILE"
        echo "$PATH_LINE" >> "$BASHRC_FILE"
        echo "--- Done. ---"
        echo
        echo "Please run 'source $BASHRC_FILE' or open a new terminal for the changes to take effect."
    else
        echo "--- Skipping .bashrc update. ---"
        echo "To use the compiler in this session, run:"
        echo "  export PATH=\"$PATH_TO_ADD:\$PATH\""
    fi
fi