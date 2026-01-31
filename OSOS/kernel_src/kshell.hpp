#ifndef KERNEL_SHELL_HPP
#define KERNEL_SHELL_HPP

#include "basic/multiboot.h"
#include "basic/kiostream.hpp" 
#include "basic/kstring.hpp"
#include "essential/kmultitasking.hpp"
#include "hardware_communication/kpci.hpp"
#include "memory/kpmm.hpp"
#include "memory/kheap.hpp"
#include "memory/kpaging.hpp"
#include "essential/ktime.hpp" 

#define maxNumShellTask 100
#define SHELL_BUFFER_SIZE 256

using namespace basic;

class KernelShell {
private:
    multiboot_info_t *mbi;
    char m_line_buffer[256]; 
    int m_buffer_index; 
    char m_input_queue[SHELL_BUFFER_SIZE];
    volatile int m_write_ptr;
    volatile int m_read_ptr;

    hardware_communication::PCI_Controller pciController;
    essential::KThreadManager *osos_ThreadManager_ptr;
    essential::KThread* shell_threads_ptr[maxNumShellTask];
    int numShellThreads;

public:
    KernelShell(essential::KThreadManager *osos_ThreadManager_ptr, multiboot_info_t *mbi)
    {
        this->osos_ThreadManager_ptr = osos_ThreadManager_ptr;
        this->mbi = mbi;
        m_buffer_index = 0;
        m_line_buffer[0] = '\0';
        numShellThreads = 0;
        
        m_write_ptr = 0;
        m_read_ptr = 0;
    }
    ~KernelShell() {}

    bool addShellTask(essential::KThread* task){
        if(numShellThreads >= maxNumShellTask) return false;
        shell_threads_ptr[numShellThreads++] = task;
        return true;
    }

    /// @brief Called by the Interrupt Handler. Only buffers the key!
    void on_key_pressed(char c)
    {
        if (c == 0) return;
        
        int next_write = (m_write_ptr + 1) % SHELL_BUFFER_SIZE;
        
        if (next_write != m_read_ptr) {
            m_input_queue[m_write_ptr] = c;
            m_write_ptr = next_write;
        }
    }

    /// @brief Called by the Main Thread. Processes keys with interrupts ENABLED.
    void update()
    {
        while (m_read_ptr != m_write_ptr) {
            char c = m_input_queue[m_read_ptr];
            m_read_ptr = (m_read_ptr + 1) % SHELL_BUFFER_SIZE;
            handle_key_logic(c);
        }
    }

private: 
    void handle_key_logic(char c)
    {
        switch(c)
        {
            case '\r':
            case '\n':
                printf("\n");
                m_line_buffer[m_buffer_index] = '\0';
                if (m_buffer_index > 0) process_command(m_line_buffer);
                m_buffer_index = 0;
                m_line_buffer[0] = '\0';
                printf("\nOSOS> ");
                break;
            case 8: // Backspace
                if (m_buffer_index > 0){
                    m_buffer_index--;
                    m_line_buffer[m_buffer_index] = '\0';
                    printf("\b \b"); 
                }
                break;
            default:
                if (c >= ' ' && c <= '~' && m_buffer_index < 254) {
                    m_line_buffer[m_buffer_index++] = c;
                    m_line_buffer[m_buffer_index] = '\0';
                    printf("%c", c);
                }
                break;
        }
    }

    void process_command(const char* command)
    {
        if (strcmp(command, "help") == 0) {
            printf("\
OSOS Kernel Shell Help\n\
Here is list of commands that are working in OSOS\n\
help      : list commands\n\
clear     : clear shell\n\
lspci     : list PCI devices\n\
checkmem  : examine physical memory (in KB)\n\
checkheap : examine current heap (in B)\n\
lspagedir : check active page directory entries for kernel\n\
uptime    : show system uptime (Hr:Min:Sec:mSec)\n\
sleep <n> : sleep for n milliseconds\n\
numtasks  : to see number of tasks in shell list\n\
task <i>  : start ith task(thread) from OSOS shell\n\
reboot    : restart the computer\n\
poweroff  : shutdown the computer\n\
exit      : shutdown the computer (this is temporary command for now)\n");
        }
        else if(strcmp(command, "clear") == 0) {
            clearScreen();
        }
        else if (strcmp(command, "lspci") == 0) {
            printf("Listing PCI devices...\n");
            pciController.scanBus(0);
        }
        else if(strcmp(command, "checkmem") == 0) {
            memory::printMemoryMap(mbi);
            printf("Total Free Physical Memory to use : %d KB\n", memory::PhysicalMemoryManager::get_free_memory_kb());
        }
        else if(strcmp(command, "checkheap") == 0) {
            memory::kernel_heap.printHeapInfo();
        }
        else if(strcmp(command, "lspagedir") == 0) {
            memory::PagingManager::printPageDirectory();
        }
        else if(strcmp(command, "uptime") == 0) {
            uint64_t uptime = essential::Time::getUptimeMS();
            uint64_t milliseconds = uptime % 1000;
            uint64_t total_seconds = uptime / 1000;
            uint64_t seconds = total_seconds % 60;
            uint64_t total_minutes = total_seconds / 60;
            uint64_t minutes = total_minutes % 60;
            uint64_t hours = total_minutes / 60;
            printf("System Uptime: %02d:%02d:%02d:%03d\n", (int)hours, (int)minutes, (int)seconds, (int)milliseconds);
        }
        else if (strncmp(command, "sleep", 5) == 0) {
            char* msStr = (char*)command + 5;
            if(*msStr == '\0') {
                printf("Usage: sleep <milliseconds>\n");
            } else {
                int ms = stoi(msStr);
                printf("Sleeping %d ms...\n", ms);
                essential::Time::sleep(ms);
                printf("Done.\n");
            }
        }
        else if(strcmp(command, "numtasks") == 0) {
            printf("Number of tasks in shell list : %d\n", numShellThreads);
        }
        else if (strncmp(command, "task", 4) == 0){
             char* tasknumstr=(char*)command+4;
             int tasknum = stoi(tasknumstr) - 1;
             
             if(tasknum < 0 || tasknum >= numShellThreads) {
                 printf("task number %d not present in task list of OSOS shell :(\n", tasknum+1);
             } else {
                 printf("Starting task number %d...\n", tasknum+1);
                 if (osos_ThreadManager_ptr->addThread(shell_threads_ptr[tasknum]) == false) {
                      printf("Error: failed to start task%d.\n", tasknum+1);
                 }
             }
        }
        else if(strcmp(command, "reboot") == 0) {
            printf("Rebooting system...\n");
            
            // Keyboard Controller Reset
            // Writing 0xFE to Port 0x64 pulses the CPU Reset line.
            // Works on QEMU, Bochs, VirtualBox, and Real Hardware.
            asm volatile ("outb %0, %1" : : "a" ((uint8_t)0xFE), "dN" ((uint16_t)0x64));
            
            // Give it a moment to react
            for(volatile int i = 0; i < 1000000; i++);

            // and triggering an interrupt. The CPU panics and reboots.
            printf("Keyboard reset failed. Forcing Triple Fault...\n");
            
            struct {
                uint16_t limit;
                uint32_t base;
            } __attribute__((packed)) emptyIdt = { 0, 0 };
            
            asm volatile ("lidt %0" : : "m" (emptyIdt));
            asm volatile ("int $3");
            
            while(1) { asm volatile("hlt"); }
        }
        else if(strcmp(command, "poweroff") == 0 || strcmp(command, "exit") == 0) {
            printf("Powering off system...\n");
            
            // QEMU Shutdown
            // Port: 0x604, Value: 0x2000
            asm volatile ("outw %1, %0" : : "dN" ((uint16_t)0x604), "a" ((uint16_t)0x2000));

            // Bochs & Older QEMU Shutdown
            // Port: 0xB004, Value: 0x2000
            asm volatile ("outw %1, %0" : : "dN" ((uint16_t)0xB004), "a" ((uint16_t)0x2000));

            // VirtualBox Shutdown
            // Port: 0x4004, Value: 0x3400
            asm volatile ("outw %1, %0" : : "dN" ((uint16_t)0x4004), "a" ((uint16_t)0x3400));

            printf("Poweroff failed (ACPI not implemented). Halting CPU.\n");
            while(1) { asm volatile("hlt"); }
        }
        else {
            printf("Unknown command: '%s'\n", command);
        }
    }
};

#endif