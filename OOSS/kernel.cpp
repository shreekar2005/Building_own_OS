void printf(char* str){
    // actully from memory location 0xb8000 whatever is in memory that will be written on screen
    // 0xb8000 -> bb|bb|bb|bb|....
    // here for every short the high byte (first b) is reserverd for background color and char color to print
    // low byte of short (second b) is actual character to print 
    // Note : short have 2 bytes
    unsigned short* VideoMemory = (unsigned short*)0xb8000;
    for(int i=0; str[i]!='\0'; i++){
        VideoMemory[i]= (VideoMemory[i]&0xFF00) | str[i]; // we will take high byte from VideoMemory[i] and str[i] will become low byte

    }
}

void clearScreen() {
    unsigned short* VideoMemory = (unsigned short*)0xb8000;

    // The character to fill the screen with: a space with our desired color.
    // 0x07 = Light Grey on Black.
    unsigned short blank = (0x07 << 8) | ' ';

    // The screen has 80 columns and 25 rows.
    int screenSize = 80 * 25;
    for (int i = 0; i < screenSize; i++) {
        VideoMemory[i] = blank;
    }
}

// using extern "C" to preserve name of kernelMain, becasue g++ have some different naming convension so it changes name of functions etc.
extern "C"
void kernelMain(void* multiboot_structure, unsigned int magicnumber)
{
    clearScreen();
    char greeting_from_kernel[] = "Hello world! -- from OOSS";
    printf(greeting_from_kernel);
    while(1); // because kernel cannot stop at the end :)
}