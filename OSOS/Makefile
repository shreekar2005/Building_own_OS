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
GDT_OBJ := $(LIBK_BUILD_DIR)/gdt.o
MEMORY_OBJ := $(LIBK_BUILD_DIR)/memory.o

OBJECTS    := $(LOADER_OBJ) $(KERNEL_OBJ) $(CONSOLE_OBJ) $(GDT_OBJ) $(MEMORY_OBJ)

##############
# CRTI_OBJ=crti.o
# CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
# CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
# CRTN_OBJ=crtn.o
##############
OBJ_LINK_LIST:=$(CRTI_OBJ) $(CRTBEGIN_OBJ) $(OBJECTS) $(CRTEND_OBJ) $(CRTN_OBJ)

# Target builds
TARGET_BIN := $(KERNEL_BUILD_DIR)/OSOSkernel.bin
TARGET_ISO := $(KERNEL_BUILD_DIR)/OSOSkernel.iso

# .PHONY is for targets that are NOT files
.PHONY: qemu bin iso vm clean

# Default target when you just type 'make'. It start qemu virtual machine
qemu: bin 
	qemu-system-x86_64 --kernel $(TARGET_BIN) -m 4

# Rule to create the kernel binary.
bin: $(TARGET_BIN) 
$(TARGET_BIN): $(OBJECTS) $(KERNEL_SRC_DIR)/linker.ld
	$(CC) -T $(KERNEL_SRC_DIR)/linker.ld $(OBJ_LINK_LIST) $(CCPARAMS) -o $@ 
#FOR LINKING g++ IS BETTER OVER ld

# Rule to create the bootable ISO.
iso: $(TARGET_ISO)
$(TARGET_ISO): $(TARGET_BIN)
	mkdir -p $(KERNEL_BUILD_DIR)/iso/boot/grub
	cp $(TARGET_BIN) $(KERNEL_BUILD_DIR)/iso/boot/
	
	echo 'set timeout=2' > $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo 'set default=0' >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '' >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo 'menuentry "OSOS" {' >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '  multiboot /boot/OSOSkernel.bin' >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '  boot' >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '}' >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg

	grub-mkrescue --output=$@ $(KERNEL_BUILD_DIR)/iso
	rm -rf $(KERNEL_BUILD_DIR)/iso

# Rule to run the OS in VirtualBox, restarting it if it's already running.
vm: $(TARGET_ISO)
	@echo "Unloading KVM modules to run VirtualBox..."
	sudo modprobe -r kvm_intel; sudo modprobe -r kvm

	@if VBoxManage showvminfo "OSOS_Machine" | grep -q "running"; then \
		echo "VM is already running. Powering off first..."; \
		VBoxManage controlvm "OSOS_Machine" poweroff; \
		sleep 1; \
	fi

	@echo "Starting VM... Close the VM window when you are finished."
	@VBoxManage startvm "OSOS_Machine"

	@echo "Waiting for VM to shut down..."
	@while ! VBoxManage showvminfo "OSOS_Machine" | grep -q "powered off"; do \
		sleep 1; \
	done
	@echo "VM has been powered off. Reloading KVM modules."

	sudo modprobe kvm_intel; sudo modprobe kvm

# Rule to clean build directory
clean:
	rm -rf $(KERNEL_BUILD_DIR)
	rm -rf $(LIBK_BUILD_DIR)
	


# It's an "order-only" prerequisite for the object files (note the '|' symbol in Rule to build kernel.o and loader.o).
$(KERNEL_BUILD_DIR):
	mkdir -p $(KERNEL_BUILD_DIR)
$(LIBK_BUILD_DIR):
	mkdir -p $(LIBK_BUILD_DIR)

# Rule to build loader.o.
$(LOADER_OBJ): $(KERNEL_SRC_DIR)/loader.s | $(KERNEL_BUILD_DIR)
	$(AS) $< $(ASPARAMS) -o $@

# Rule to build kernel.o.
$(KERNEL_OBJ): $(KERNEL_SRC_DIR)/kernel.cpp | $(KERNEL_BUILD_DIR)
	$(CC) -c $< $(CCPARAMS) -o $@

# Rule to build console.o.
$(CONSOLE_OBJ): $(LIBK_SRC_DIR)/console.cpp | $(LIBK_BUILD_DIR)
	$(CC) -c $< $(CCPARAMS) -o $@ 

$(GDT_OBJ): $(LIBK_SRC_DIR)/gdt.cpp | $(LIBK_BUILD_DIR)
	$(CC) -c $< $(CCPARAMS) -o $@ 

$(MEMORY_OBJ): $(LIBK_SRC_DIR)/memory.cpp | $(LIBK_BUILD_DIR)
	$(CC) -c $< $(CCPARAMS) -o $@


