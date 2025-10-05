# Directories
SRC_DIR := src
BUILD_DIR := build

# Toolchain flags
GPP       := i686-elf-g++
GPPPARAMS := -Wall -Wextra -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti -nostdlib -lgcc
AS        := i686-elf-as
ASPARAMS  := 

# Actually we will not use this as g++ is better than ld for linking :)
LD        := i686-elf-ld
LDPARAMS  := -lgcc

# Define the full paths for the files we will for building.
LOADER_OBJ := $(BUILD_DIR)/loader.o
KERNEL_OBJ := $(BUILD_DIR)/kernel.o
CONSOLE_OBJ := $(BUILD_DIR)/console.o # printf implementation
OBJECTS    := $(LOADER_OBJ) $(CONSOLE_OBJ) $(KERNEL_OBJ)

# Target builds
TARGET_BIN := $(BUILD_DIR)/mykernel.bin
TARGET_ISO := $(BUILD_DIR)/mykernel.iso

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
	rm -rf $(BUILD_DIR)

# It's an "order-only" prerequisite for the object files (note the '|' symbol in Rule to build kernel.o and loader.o).
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Rule to build kernel.o.
$(KERNEL_OBJ): $(SRC_DIR)/kernel.cpp | $(BUILD_DIR)
	$(GPP) -c $< $(GPPPARAMS) -o $@

# Rule to build loader.o.
$(LOADER_OBJ): $(SRC_DIR)/loader.s | $(BUILD_DIR)
	$(AS) $< $(ASPARAMS) -o $@

# Rule to build console.o.
$(CONSOLE_OBJ): $(SRC_DIR)/utils_32bit/console.cpp | $(BUILD_DIR)
	$(GPP) -c $< $(GPPPARAMS) -o $@ 

# Rule to link the final kernel binary. FOR LINKING g++ IS BETTER OVER ld
$(TARGET_BIN): $(OBJECTS) $(SRC_DIR)/linker.ld
	$(GPP) -T $(SRC_DIR)/linker.ld $(OBJECTS) $(GPPPARAMS) -o $@

# Rule to create the bootable ISO.
$(TARGET_ISO): $(TARGET_BIN)
	# Create a temporary directory structure for the ISO content
	mkdir -p $(BUILD_DIR)/iso/boot/grub
	# Copy the kernel binary into the temp boot directory
	cp $(TARGET_BIN) $(BUILD_DIR)/iso/boot/
	# Create the grub configuration file
	echo 'set timeout=5' > $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo 'set default=0' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo 'menuentry "My Operating System" {' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '  multiboot /boot/mykernel.bin' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '  boot' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '}' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	# Build the ISO and clean up the temp directory
	grub-mkrescue --output=$@ $(BUILD_DIR)/iso
	rm -rf $(BUILD_DIR)/iso