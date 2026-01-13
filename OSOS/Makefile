# Instructions to use this makefile
# make                  : To build elf and boot with elf and qemu
# make qemu_elf         : To build elf and boot with elf and qemu
# make qemu_nographic   : To build elf and boot with elf and qemu with serial IO only (attached to terminal of host machine)
# make qemu_serial      : To build elf and boot with elf and qemu with both serial IO and graphical mode
# make qemu_iso         : To build iso and boot with iso and qemu
# make vm               : To build iso and boot with iso and VirtualBox (read README for setting up VM)
# make elf              : To build elf file by compiling and linking source code
# make iso              : To build iso file with existing elf 
# make clean            : To delete all build directories
# make clear            : To delete all build directories and clear the screen
# make setup            : To install all dependencies (qemu-system-x86 virtualbox grub-pc-bin xorriso gnome-terminal)
# make dbg_elf          : To build elf file with -g (keep debug info) and -O0(dont optimize while compiling) 
# make dbg_start_qemu   : To build dbg_elf and just start QEMU (can connect that from VSCode or cli GDB)
# make dbg_cli          : To connect QEMU instance (created by `make dbg_start_qemu`) with cli GDB
# make docs             : To update docs - OSOS documentation (which have technical details and info for project)

# .PHONY is for targets that are NOT files
.PHONY: qemu_elf qemu_iso elf vm iso clear clean debug setup dbg_elf dbg_start_qemu dbg_cli docs

# Directories
KERNEL_SRC_DIR := kernel_src
KERNEL_BUILD_DIR := kernel_build
LIBK_SRC_DIR := libk_src
LIBK_BUILD_DIR := libk_build

# Automatically find all source files ---
KRNL_SOURCES_S := $(wildcard $(KERNEL_SRC_DIR)/*.s)
KRNL_SOURCES_CPP := $(wildcard $(KERNEL_SRC_DIR)/*.cpp)
LIBK_SOURCES_S := $(wildcard $(LIBK_SRC_DIR)/*/*.s)
LIBK_SOURCES_CPP := $(wildcard $(LIBK_SRC_DIR)/*/*.cpp)

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

# Toolchain flags
CC          := 	osos-g++
# Default CCPARAMS. (Note: -DserialMode=1 will be added dynamically below for specific targets)
CCPARAMS    := 	-O2 -I ./libk_header -Wall -Wextra -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti -nostdlib -lgcc
AS          := 	osos-as
ASPARAMS    :=
QEMU 		:= 	qemu-system-i386 -m 4096 \
    			-net nic,model=pcnet \
    			-net tap,ifname=tap0,script=no,downscript=no
GDB         := 	osos-gdb
# -g is for adding debugging symbols

# --- CONFIGURATION LOGIC ---
# If we run 'make qemu_nographic' or 'make qemu_serial', we add the serialMode flag
# and we force a 'clean' so the code actually recompiles with the new flag.
qemu_nographic: CCPARAMS += -DserialMode=1
qemu_serial: CCPARAMS += -DserialMode=1

# Default target when you just type 'make'. It starts qemu virtual machine
qemu: qemu_elf
qemu_elf: elf 
	$(QEMU) -kernel $(TARGET_ELF) -display sdl,full-screen=on

# Boot from ISO file
qemu_iso: iso
	$(QEMU) -cdrom $(TARGET_ISO) -display sdl,full-screen=on

# Have both graphic and serial mode
# DEPENDS ON CLEAN: to ensure we recompile with -DserialMode=1
qemu_serial: clean elf
	$(QEMU) -kernel $(TARGET_ELF) -serial stdio

# Have only serial mode
# DEPENDS ON CLEAN: to ensure we recompile with -DserialMode=1
qemu_nographic : clean elf
	$(QEMU) -kernel $(TARGET_ELF) -nographic

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
		sleep 5; \
	done
	@echo "VM has been powered off. Reloading KVM modules."
	sudo modprobe kvm_intel; sudo modprobe kvm

# Rule to create the bootable ISO.
iso: $(TARGET_ISO)
$(TARGET_ISO): $(TARGET_ELF)
	mkdir -p $(KERNEL_BUILD_DIR)/iso/boot/grub
	cp $(TARGET_ELF) $(KERNEL_BUILD_DIR)/iso/boot/
	echo 'set timeout=2'                    > $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo 'set default=0'                    >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo ''                                 >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo 'menuentry "OSOS" {'               >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '  multiboot /boot/OSOSkernel.elf' >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '  boot'                           >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '}'                                >> $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
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
	@mkdir -p $@

# Rule to build kernel assembly files (.s -> .o)
$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SRC_DIR)/%.s | $(KERNEL_BUILD_DIR)
	$(AS) $< $(ASPARAMS) -o $@

# Rule to build kernel C++ files (.cpp -> .o)
$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SRC_DIR)/%.cpp | $(KERNEL_BUILD_DIR)
	$(CC) -c $< $(CCPARAMS) -o $@

# Rule to build libk assembly files (.s -> .o)
$(LIBK_BUILD_DIR)/%.o: $(LIBK_SRC_DIR)/%.s | $(LIBK_BUILD_DIR)
	@mkdir -p $(dir $@)
	$(AS) $< $(ASPARAMS) -o $@

# Rule to build libk C++ files (.cpp -> .o)
$(LIBK_BUILD_DIR)/%.o: $(LIBK_SRC_DIR)/%.cpp | $(LIBK_BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) -c $< $(CCPARAMS) -o $@



# Install dependencies for OSOS development
setup:
	sudo apt install qemu-system-x86 virtualbox grub-pc-bin xorriso


# -----------------------------DEBUG PURPOSE ONLY (for GDB) ------------------------------------

# This new target builds the ELF file with debug symbols (-g)
# and no optimizations (-O0), which makes debugging much easier.
dbg_elf:
	@$(MAKE) CCPARAMS="$(CCPARAMS) -g -O0" ASPARAMS="$(ASPARAMS) -g" elf


# It builds the ELF and starts QEMU (paused), but does NOT start GDB.
dbg_start_qemu: dbg_elf
	@echo "Starting QEMU in background, paused for GDB..."
	# Use nohup to detach QEMU from the shell
	setsid $(QEMU) -kernel $(TARGET_ELF) -m 4096 -s -S > /dev/null 2>&1 &
	@sleep 1


# To start cli debugging using GDB
dbg_cli: dbg_start_qemu
	@echo "Starting GDB and attaching..."
	$(GDB) $(TARGET_ELF) -ex "target remote localhost:1234"

docs :
	cd ../docs && doxygen && cd ../OSOS