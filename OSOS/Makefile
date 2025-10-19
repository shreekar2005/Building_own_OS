# Directories
KERNEL_SRC_DIR := kernel_src
KERNEL_BUILD_DIR := kernel_build
LIBK_SRC_DIR := libk_src
LIBK_BUILD_DIR := libk_build

QEMU := qemu-system-i386
# Toolchain flags
CC 			:= i686-elf-g++
CCPARAMS 	:= -I ./$(KERNEL_SRC_DIR)/include -Wall -Wextra -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti -nostdlib -lgcc
AS        	:= i686-elf-as
ASPARAMS  	:= 

# Automatically find all source files ---
KRNL_SOURCES_S := $(wildcard $(KERNEL_SRC_DIR)/*.s)
KRNL_SOURCES_CPP := $(wildcard $(KERNEL_SRC_DIR)/*.cpp)
LIBK_SOURCES_S := $(wildcard $(LIBK_SRC_DIR)/*.s)
LIBK_SOURCES_CPP := $(wildcard $(LIBK_SRC_DIR)/*.cpp)

# Automatically generate object file names from source file names ---
KRNL_OBJS_S   := $(patsubst $(KERNEL_SRC_DIR)/%.s, $(KERNEL_BUILD_DIR)/%.o, $(KRNL_SOURCES_S))
KRNL_OBJS_CPP := $(patsubst $(KERNEL_SRC_DIR)/%.cpp, $(KERNEL_BUILD_DIR)/%.o, $(KRNL_SOURCES_CPP))
LIBK_OBJS_S   := $(patsubst $(LIBK_SRC_DIR)/%.s, $(LIBK_BUILD_DIR)/%.o, $(LIBK_SOURCES_S))
LIBK_OBJS_CPP := $(patsubst $(LIBK_SRC_DIR)/%.cpp, $(LIBK_BUILD_DIR)/%.o, $(LIBK_SOURCES_CPP))

# Combine all object files into one list
OBJECTS := $(KRNL_OBJS_S) $(KRNL_OBJS_CPP) $(LIBK_OBJS_S) $(LIBK_OBJS_CPP)


# Target builds
TARGET_ELF := $(KERNEL_BUILD_DIR)/OSOSkernel.elf
TARGET_ISO := $(KERNEL_BUILD_DIR)/OSOSkernel.iso

# .PHONY is for targets that are NOT files
.PHONY: qemu elf elf vm iso install_OSOS uninstall_OSOS clean

# Default target when you just type 'make'. It starts qemu virtual machine
qemu_elf: elf 
	$(QEMU) -kernel $(TARGET_ELF) -m 4096

# Boot from ISO file
qemu_iso: iso
	$(QEMU) -cdrom $(TARGET_ISO) -m 4096

# Rule to create the kernel ELF.
elf: $(TARGET_ELF)
$(TARGET_ELF): $(OBJECTS) $(KERNEL_SRC_DIR)/linker.ld
	$(CC) -T $(KERNEL_SRC_DIR)/linker.ld $(OBJECTS) $(CCPARAMS) -o $@

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

# Rule to create the bootable ISO.
iso: $(TARGET_ISO)
$(TARGET_ISO): $(TARGET_ELF)
	mkdir -p $(KERNEL_BUILD_DIR)/iso/boot/grub
	cp $(TARGET_ELF) $(KERNEL_BUILD_DIR)/iso/boot/
	
	echo 'set timeout=2' > $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo 'set default=0' >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '' >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo 'menuentry "OSOS" {' >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '  multiboot /boot/OSOSkernel.elf' >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
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



# ------------------------------------------ GRUB Management Rules ------------------------------------------------

# Rule to add a GRUB menu entry for the kernel.
# 1. Copies the kernel to /boot.
# 2. Checks if the menu entry already exists in /etc/grub.d/40_custom.
# 3. If not, it appends the new menu entry.
# 4. Runs update-grub to apply the changes.
install_OSOS: elf
	@echo "--- Adding GRUB Entry for OSOS ---"
	@echo "Copying kernel to /boot directory..."
	sudo cp $(TARGET_ELF) /boot/OSOSkernel.elf
	@if sudo grep -q 'menuentry "OSOS"' /etc/grub.d/40_custom; then \
		echo "GRUB entry for 'OSOS' already exists. Nothing to do."; \
	else \
		echo "Adding new GRUB entry to /etc/grub.d/40_custom..."; \
		printf '\nmenuentry "OSOS" {\n\tmultiboot /boot/OSOSkernel.elf\n\tboot\n}\n' | sudo tee -a /etc/grub.d/40_custom > /dev/null; \
		echo "Updating GRUB configuration..."; \
		sudo update-grub; \
		echo "Done. GRUB entry added successfully."; \
	fi

# Rule to remove the GRUB menu entry for the kernel.
# 1. Checks if the menu entry exists.
# 2. If it does, it removes the entire 'menuentry "OSOS" {...}' block.
# 3. Removes the kernel files from /boot.
# 4. Runs update-grub to apply the changes.
uninstall_OSOS:
	@echo "--- Removing GRUB Entry for OSOS ---"
	@if sudo grep -q 'menuentry "OSOS"' /etc/grub.d/40_custom; then \
		echo "Found GRUB entry. Removing it..."; \
		sudo perl -i -0777 -pe 's/\n*menuentry "OSOS" \{.*?\}//s' /etc/grub.d/40_custom; \
		echo "Removing kernel files from /boot..."; \
		sudo rm -f /boot/OSOSkernel.elf /boot/OSOSkernel.iso; \
		echo "Updating GRUB configuration..."; \
		sudo update-grub; \
		echo "Done. GRUB entry and kernel files removed."; \
	else \
		echo "GRUB entry for 'OSOS' not found. Nothing to remove."; \
	fi