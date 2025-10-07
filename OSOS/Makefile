# Directories
KERNEL_SRC_DIR := kernel_src
KERNEL_BUILD_DIR := kernel_build
LIBK_SRC_DIR := libk_src
LIBK_BUILD_DIR := libk_build

# Toolchain flags
CC       := i686-elf-g++
CCPARAMS := -Wall -Wextra -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti -nostdlib -lgcc
AS        := i686-elf-as
ASPARAMS  := 

# Actually we will not use this as g++ is better than ld for linking :)
LD        := i686-elf-ld
LDPARAMS  := -lgcc

# Define the full paths for the files we will for building.
LOADER_OBJ := $(KERNEL_BUILD_DIR)/loader.o
KERNEL_OBJ := $(KERNEL_BUILD_DIR)/kernel.o
CONSOLE_OBJ := $(LIBK_BUILD_DIR)/console.o # printf implementation
OBJECTS    := $(LOADER_OBJ) $(CONSOLE_OBJ) $(KERNEL_OBJ)

##############
# CRTI_OBJ=crti.o
# CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
# CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
# CRTN_OBJ=crtn.o

OBJ_LINK_LIST:=$(CRTI_OBJ) $(CRTBEGIN_OBJ) $(OBJECTS) $(CRTEND_OBJ) $(CRTN_OBJ)
############3

# Target builds
TARGET_BIN := $(KERNEL_BUILD_DIR)/mykernel.bin
TARGET_ISO := $(KERNEL_BUILD_DIR)/mykernel.iso

# .PHONY is for targets that are NOT files
.PHONY: default bin iso run clean

default: $(TARGET_BIN) # Default target when you just type 'make'.

bin: $(TARGET_BIN)
iso: $(TARGET_ISO)

# Rule to run the OS in VirtualBox, restarting it if it's already running.
run: $(TARGET_ISO)
	@if VBoxManage showvminfo "My Operating System" | grep -q "running"; then \
		echo "VM is running. Powering off..."; \
		VBoxManage controlvm "My Operating System" poweroff; \
		sleep 2; \
	fi
	@echo "Starting VM..."
	@VBoxManage startvm "My Operating System"

# Rule to clean build directory
clean:
	rm -rf $(KERNEL_BUILD_DIR)
	rm -rf $(LIBK_BUILD_DIR)

# It's an "order-only" prerequisite for the object files (note the '|' symbol in Rule to build kernel.o and loader.o).
$(KERNEL_BUILD_DIR):
	mkdir -p $(KERNEL_BUILD_DIR)
$(LIBK_BUILD_DIR):
	mkdir -p $(LIBK_BUILD_DIR)

# Rule to build kernel.o.
$(KERNEL_OBJ): $(KERNEL_SRC_DIR)/kernel.cpp | $(KERNEL_BUILD_DIR)
	$(CC) -c $< $(CCPARAMS) -o $@

# Rule to build loader.o.
$(LOADER_OBJ): $(KERNEL_SRC_DIR)/loader.s | $(KERNEL_BUILD_DIR)
	$(AS) $< $(ASPARAMS) -o $@

# Rule to build console.o.
$(CONSOLE_OBJ): $(LIBK_SRC_DIR)/console.cpp | $(LIBK_BUILD_DIR)
	$(CC) -c $< $(CCPARAMS) -o $@ 

# Rule to link the final kernel binary. FOR LINKING g++ IS BETTER OVER ld
$(TARGET_BIN): $(OBJECTS) $(KERNEL_SRC_DIR)/linker.ld
	$(CC) -T $(KERNEL_SRC_DIR)/linker.ld $(OBJ_LINK_LIST) $(CCPARAMS) -o $@

# Rule to create the bootable ISO.
$(TARGET_ISO): $(TARGET_BIN)
	# Create a temporary directory structure for the ISO content
	mkdir -p $(KERNEL_BUILD_DIR)/iso/boot/grub
	# Copy the kernel binary into the temp boot directory
	cp $(TARGET_BIN) $(KERNEL_BUILD_DIR)/iso/boot/
	# Create the grub configuration file
	echo 'set timeout=5' > $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo 'set default=0' >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '' >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo 'menuentry "My Operating System" {' >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '  multiboot /boot/mykernel.bin' >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '  boot' >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '}' >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	# Build the ISO and clean up the temp directory
	grub-mkrescue --output=$@ $(KERNEL_BUILD_DIR)/iso
	rm -rf $(KERNEL_BUILD_DIR)/iso