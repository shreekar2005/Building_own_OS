GPPPARAMS = -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore
ASPARAMS = --32
LDPARAMS = -melf_i386

objects = loader.o kernel.o

.PHONY: all clean # to tell that all and clean are not files (they are just names to process)

# This is the first target so for mykernel.bin all dependencies will automatically got run again 
all: mykernel.bin

kernel.o: kernel.cpp
	g++ $(GPPPARAMS) -o kernel.o -c kernel.cpp

loader.o: loader.s
	as $(ASPARAMS) -o loader.o loader.s

mykernel.bin: linker.ld $(objects)
	ld $(LDPARAMS) -T $< -o $@ $(objects) 
# $< is used for first dependancy file, $<< is used for second dependancy file
# $@ is used for target file ... here target file is mykernel.bin

# install: mykernel.bin
# 	sudo cp $< /boot/mykernel.bin

# It's also good practice to add a "clean" rule
clean:
	rm -f $(objects) mykernel.bin
