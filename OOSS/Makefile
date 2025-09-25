SRC_DIR=src
BUILD_DIR=build


GPPPARAMS = -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -fno-pic
ASPARAMS = --32 --noexecstack
LDPARAMS = -melf_i386 


.PHONY: all clean # to tell that all and clean are not files (they are just names to process)

# This is the first target so for mykernel.bin all dependencies will automatically got run again 
all: mykernel.bin

kernel.o: $(SRC_DIR)/kernel.cpp always
	g++ $(GPPPARAMS) -o ./build/kernel.o -c $(SRC_DIR)/kernel.cpp

loader.o: $(SRC_DIR)/loader.s always
	as $(ASPARAMS) -o ./build/loader.o $(SRC_DIR)/loader.s

mykernel.bin: $(SRC_DIR)/linker.ld kernel.o loader.o
	ld $(LDPARAMS) -T $< -o $(BUILD_DIR)/$@ $(BUILD_DIR)/loader.o $(BUILD_DIR)/kernel.o 
# $< is used for first dependancy file, $<< is used for second dependancy file
# $@ is used for target file ... here target file is mykernel.bin

always:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)/*
