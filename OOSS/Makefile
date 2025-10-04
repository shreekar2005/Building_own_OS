# Directories
SRC_DIR := src
BUILD_DIR := build

# Toolchain flags
GPPPARAMS := -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -fno-pic
ASPARAMS  := --32 --noexecstack
LDPARAMS  := -melf_i386

# Define the full paths for the files we are building. This is much cleaner.
KERNEL_OBJ := $(BUILD_DIR)/kernel.o
LOADER_OBJ := $(BUILD_DIR)/loader.o
OBJECTS    := $(LOADER_OBJ) $(KERNEL_OBJ)

# Defining paths for utilities that we are going to use
CONSOLE_OBJ := $(BUILD_DIR)/console.o # printf implementation
UTIL_OBJECTS := $(CONSOLE_OBJ)

TARGET_BIN := $(BUILD_DIR)/mykernel.bin
TARGET_ISO := $(BUILD_DIR)/mykernel.iso

# .PHONY is for targets that are NOT files
.PHONY: default clean run

default: $(TARGET_BIN) # Default target when you just type 'make'.
bin: $(TARGET_BIN)
iso: $(TARGET_ISO)

# Rule to clean build directory
clean:
	rm -rf $(BUILD_DIR)

# Rule to run the OS in VirtualBox, restarting it if it's already running.
run: $(TARGET_ISO)
	@if VBoxManage showvminfo "My Operating System" | grep -q "running"; then \
		echo "VM is running. Powering off..."; \
		VBoxManage controlvm "My Operating System" poweroff; \
		sleep 2; \
	fi
	@echo "Starting VM..."
	@VBoxManage startvm "My Operating System"

# It's an "order-only" prerequisite for the object files (note the '|' symbol in Rule to build kernel.o and loader.o).
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Rule to build kernel.o.
$(KERNEL_OBJ): $(SRC_DIR)/kernel.cpp | $(BUILD_DIR)
	g++ $(GPPPARAMS) -o $@ -c $<

# Rule to build loader.o.
$(LOADER_OBJ): $(SRC_DIR)/loader.s | $(BUILD_DIR)
	as $(ASPARAMS) -o $@ $<

# Rule to build console.o.
$(CONSOLE_OBJ): $(SRC_DIR)/utils_32bit/console.c | $(BUILD_DIR)
	g++ $(GPPPARAMS) -o $@ -c $<

# Rule to link the final kernel binary.
$(TARGET_BIN): $(OBJECTS) $(SRC_DIR)/linker.ld $(UTIL_OBJECTS)
	ld $(LDPARAMS) -T $(SRC_DIR)/linker.ld $(CONSOLE_OBJ) $(OBJECTS) -o $@ 

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