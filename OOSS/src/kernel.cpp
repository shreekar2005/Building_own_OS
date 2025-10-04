#include "multiboot.h" // for handling multiboot info (provided by grub)
#include "printf.cpp"
#define exC extern "C"

int cursor_x_=0;
int cursor_y_=0;

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



class A{
    public:
        A(){
            printf("constructor called \n");
        }
        ~A(){
            printf("destructor called \n");
        }
};

// constructor will be called by the callConstructors() function.
A a_global_instance;

// using exC to avoid "name mangling" or "name decoration"
exC void kernelMain(void* multiboot_structure, unsigned int magicnumber){
    char greeting_from_kernel[] = "Hello world! -- from OOSS";
    printf("%s\n%d\n",greeting_from_kernel, magicnumber);
    A a_local_instance;

    while(1); // because kernel cannot stop at the end :)
}

