# Directories
KERNEL_SRC_DIR := kernel_src
KERNEL_BUILD_DIR := kernel_build
LIBK_SRC_DIR := libk_src
LIBK_BUILD_DIR := libk_build

# Toolchain flags
CC       := i686-elf-g++
CCPARAMS := -I ./$(KERNEL_SRC_DIR)/include -Wall -Wextra -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti -nostdlib -lgcc
AS        := i686-elf-as
ASPARAMS  := 

# --- STEP 1: Automatically find all source files ---
KRNL_SOURCES_S := $(wildcard $(KERNEL_SRC_DIR)/*.s)
KRNL_SOURCES_CPP := $(wildcard $(KERNEL_SRC_DIR)/*.cpp)
LIBK_SOURCES_S := $(wildcard $(LIBK_SRC_DIR)/*.s)
LIBK_SOURCES_CPP := $(wildcard $(LIBK_SRC_DIR)/*.cpp)

# --- STEP 2: Automatically generate object file names from source file names ---
KRNL_OBJS_S   := $(patsubst $(KERNEL_SRC_DIR)/%.s, $(KERNEL_BUILD_DIR)/%.o, $(KRNL_SOURCES_S))
KRNL_OBJS_CPP := $(patsubst $(KERNEL_SRC_DIR)/%.cpp, $(KERNEL_BUILD_DIR)/%.o, $(KRNL_SOURCES_CPP))
LIBK_OBJS_S   := $(patsubst $(LIBK_SRC_DIR)/%.s, $(LIBK_BUILD_DIR)/%.o, $(LIBK_SOURCES_S))
LIBK_OBJS_CPP := $(patsubst $(LIBK_SRC_DIR)/%.cpp, $(LIBK_BUILD_DIR)/%.o, $(LIBK_SOURCES_CPP))

# Combine all object files into one list
OBJECTS := $(KRNL_OBJS_S) $(KRNL_OBJS_CPP) $(LIBK_OBJS_S) $(LIBK_OBJS_CPP)


# Target builds
TARGET_BIN := $(KERNEL_BUILD_DIR)/OSOSkernel.bin
TARGET_ISO := $(KERNEL_BUILD_DIR)/OSOSkernel.iso

# .PHONY is for targets that are NOT files
.PHONY: qemu bin iso vm clean

# Default target when you just type 'make'. It starts qemu virtual machine
qemu: bin 
	qemu-system-x86_64 --kernel $(TARGET_BIN) -m 4096

# Rule to run the OS in VirtualBox, restarting it if it's already running.
vm: iso
	@if VBoxManage showvminfo "OSOS_Machine" | grep -q "running"; then \
		echo "VM is already running. Powering off first..."; \
		VBoxManage controlvm "OSOS_Machine" poweroff; \
		sleep 5; \
	fi
	@echo "Unloading KVM modules to run VirtualBox..."
	sudo modprobe -r kvm_intel; sudo modprobe -r kvm
	@echo "Starting VM... Close the VM window when you are finished."
	@VBoxManage startvm "OSOS_Machine"
	@echo "Waiting for VM to shut down..."
	@while ! VBoxManage showvminfo "OSOS_Machine" | grep -q "powered off"; do \
		sleep 1; \
	done
	@echo "VM has been powered off. Reloading KVM modules."
	sudo modprobe kvm_intel; sudo modprobe kvm

# Rule to create the kernel binary.
bin: $(TARGET_BIN) 
$(TARGET_BIN): $(OBJECTS) $(KERNEL_SRC_DIR)/linker.ld
	$(CC) -T $(KERNEL_SRC_DIR)/linker.ld $(OBJECTS) $(CCPARAMS) -o $@

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


# Rule to clean build directory
clear: clean
	clear
clean:
	rm -rf $(KERNEL_BUILD_DIR)
	rm -rf $(LIBK_BUILD_DIR)

# "Order-only" prerequisites to create build directories
$(KERNEL_BUILD_DIR) $(LIBK_BUILD_DIR):
	mkdir -p $@

# Rule to build kernel assembly files (.s -> .o)
$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SRC_DIR)/%.s | $(KERNEL_BUILD_DIR)
	$(AS) $< $(ASPARAMS) -o $@

# Rule to build kernel C++ files (.cpp -> .o)
$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SRC_DIR)/%.cpp | $(KERNEL_BUILD_DIR)
	$(CC) -c $< $(CCPARAMS) -o $@

# Rule to build libk assembly files (.s -> .o)
$(LIBK_BUILD_DIR)/%.o: $(LIBK_SRC_DIR)/%.s | $(LIBK_BUILD_DIR)
	$(AS) $< $(ASPARAMS) -o $@

# Rule to build libk C++ files (.cpp -> .o)
$(LIBK_BUILD_DIR)/%.o: $(LIBK_SRC_DIR)/%.cpp | $(LIBK_BUILD_DIR)
	$(CC) -c $< $(CCPARAMS) -o $@
