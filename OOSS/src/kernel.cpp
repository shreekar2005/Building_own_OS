#define exC extern "C"


typedef void (*constructor_pointer)();
// will fill values of start_ctors and end_ctors by linker
exC constructor_pointer start_ctors; // like void (*start_ctors)();
exC constructor_pointer end_ctors; 
// will call below function from loader
exC void callConstructors(){
    for(constructor_pointer* i=&start_ctors; i!=&end_ctors; i++){
        (*i)(); // calling each constructor pointer (like function pointer)
    }
}


typedef void (*destructor_pointer)();
exC destructor_pointer start_dtors; // like void (*start_dtors)();
exC destructor_pointer end_dtors; 
// will call below function from loader
exC void callDestructors(){
    for(destructor_pointer* i=&start_dtors; i!=&end_dtors; i++){
        (*i)(); // calling each constructor pointer (like function pointer)
    }
}



// global variables will track the cursor's position for printf
int cursor_x = 0;
int cursor_y = 0;

exC void printf(char* str){
    // actully from memory location 0xb8000 whatever is in memory that will be written on screen
    // 0xb8000 -> bb|bb|bb|bb|....
    // here for every short the high byte (first b) is reserverd for background color and char color to print
    // low byte of short (second b) is actual character to print 
    // Note : short have 2 bytes
    unsigned short* VideoMemory = (unsigned short*)0xb8000;

    for(int i=0; str[i]!='\0'; i++){
        if(str[i] == '\n') {
            cursor_y++;
            cursor_x = 0;
        } else {
            // calculate the position in video memory from cursor_x and cursor_y
            int offset = cursor_y * 80 + cursor_x;
            VideoMemory[offset]= (VideoMemory[offset]&0xFF00) | str[i]; // we will take high byte from VideoMemory[offset] and str[i] will become low byte
            cursor_x++;
        }

        // if we are at the end of the line, wrap to the next one
        if(cursor_x >= 80) {
            cursor_y++;
            cursor_x = 0;
        }

        // if we are at the bottom of the screen, scroll everything up
        if(cursor_y >= 25) {
            for(int y = 0; y < 24; y++) {
                for(int x = 0; x < 80; x++) {
                    VideoMemory[y*80+x] = VideoMemory[(y+1)*80+x];
                }
            }
            // clear the last line
            for(int x = 0; x < 80; x++) {
                VideoMemory[24*80+x] = (VideoMemory[24*80+x] & 0xFF00) | ' ';
            }
            cursor_y = 24;
        }
    }
}

// will call below function from loader
exC void clearScreen() {
    unsigned short* VideoMemory = (unsigned short*)0xb8000;

    // The character to fill the screen with: a space with our desired color.
    // 0x07 = Light Grey on Black.
    unsigned short blank = (0x07 << 8) | ' ';

    // The screen has 80 columns and 25 rows.
    int screenSize = 80 * 25;
    for (int i = 0; i < screenSize; i++) {
        VideoMemory[i] = blank;
    }
    
    // Reset the cursor to the top-left corner after clearing
    cursor_x = 0;
    cursor_y = 0;
}

class A{
    public:
        A(){
            char constructor_msg[]="constructor called \n";
            printf(constructor_msg);
        }
        ~A(){
            char destructor_msg[]="destructor called \n";
            printf(destructor_msg);
        }
};

// constructor will be called by the callConstructors() function.
A a_global_instance;

// using exC to avoid "name mangling" or "name decoration"
exC void kernelMain(void* multiboot_structure, unsigned int magicnumber){
    // constructor will be called WITHOUT the callConstructors() function.
    A a_local_instance;

    char greeting_from_kernel[] = "Hello world! -- from OOSS\n";
    printf(greeting_from_kernel);

    while(1); // because kernel cannot stop at the end :)
}

