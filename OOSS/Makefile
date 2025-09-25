kernelCPP = ./src/kernel.cpp
loaderS = ./src/loader.s
linkerLD = ./src/linker.ld

objects = loader.o kernel.o


GPPPARAMS = -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -fno-pic
ASPARAMS = --32
LDPARAMS = -melf_i386


.PHONY: all clean # to tell that all and clean are not files (they are just names to process)

# This is the first target so for mykernel.bin all dependencies will automatically got run again 
all: mykernel.bin

kernel.o: $(kernelCPP)
	g++ $(GPPPARAMS) -o ./build/kernel.o -c $(kernelCPP)

loader.o: $(loaderS)
	as $(ASPARAMS) -o ./build/loader.o $(loaderS)

mykernel.bin: $(linkerLD) $(objects)
	ld $(LDPARAMS) -T $< -o ./build/$@ ./build/loader.o ./build/kernel.o 
# $< is used for first dependancy file, $<< is used for second dependancy file
# $@ is used for target file ... here target file is mykernel.bin

# install: mykernel.bin
# 	sudo cp $< /boot/mykernel.bin

# It's also good practice to add a "clean" rule
clean:
	rm -f ./build/loader.o ./build/kernel.o ./build/mykernel.bin
