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

class KernelShell {
private:
    multiboot_info_t *mbi;
    char m_line_buffer[256]; 
    int m_buffer_index; 
    
    // --- NEW: Input Queue for buffering keys ---
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
        
        // Initialize Queue
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
        
        // Calculate next write position
        int next_write = (m_write_ptr + 1) % SHELL_BUFFER_SIZE;
        
        // If buffer isn't full, add the character
        if (next_write != m_read_ptr) {
            m_input_queue[m_write_ptr] = c;
            m_write_ptr = next_write;
        }
        // If full, we drop the key (safe behavior for ISR)
    }

    /// @brief Called by the Main Thread. Processes keys with interrupts ENABLED.
    void update()
    {
        // Process all keys currently in the buffer
        while (m_read_ptr != m_write_ptr) {
            char c = m_input_queue[m_read_ptr];
            m_read_ptr = (m_read_ptr + 1) % SHELL_BUFFER_SIZE;
            
            // Now we call the actual logic (safe to sleep here!)
            handle_key_logic(c);
        }
    }

private: 
    // This was your old handle_key, now private and called by update()
    void handle_key_logic(char c)
    {
        switch(c)
        {
            case '\r':
            case '\n':
                basic::printf("\n");
                m_line_buffer[m_buffer_index] = '\0';
                if (m_buffer_index > 0) process_command(m_line_buffer);
                m_buffer_index = 0;
                m_line_buffer[0] = '\0';
                basic::printf("\nOSOS> ");
                break;
            case 8: // Backspace
                if (m_buffer_index > 0){
                    m_buffer_index--;
                    m_line_buffer[m_buffer_index] = '\0';
                    basic::printf("\b \b"); 
                }
                break;
            default:
                if (c >= ' ' && c <= '~' && m_buffer_index < 254) {
                    m_line_buffer[m_buffer_index++] = c;
                    m_line_buffer[m_buffer_index] = '\0';
                    basic::printf("%c", c);
                }
                break;
        }
    }

    void process_command(const char* command)
    {
        if (basic::strcmp(command, "help") == 0) {
            basic::printf("OSOS Kernel Shell Help\n\
help      : list commands\n\
clear     : clear shell\n\
lsmem     : print memory map provided by grub\n\
checkmem  : check how much memory is free (in KB)\n\
checkheap : examine kernel heap\n\
lspagedir : check active page directory entries for kernel\n\
lspci     : list PCI devices\n\
task <i>  : start ith task(thread) from OSOS shell\n\
numtasks  : to see number of tasks in shell list\n\
uptime    : show system uptime\n\
sleep <n> : sleep for n milliseconds\n");
        }
        else if(basic::strcmp(command, "clear") == 0) {
            basic::clearScreen();
        }
        else if (basic::strcmp(command, "lsmem") == 0) {
            memory::printMemoryMap(mbi);
        }
        else if (basic::strcmp(command, "lspci") == 0) {
            basic::printf("Listing PCI devices...\n");
            pciController.scanBus(0);
        }
        else if(basic::strcmp(command, "checkmem") == 0) {
            basic::printf("Total Free Memory : %d KB\n", memory::PhysicalMemoryManager::get_free_memory_kb());
        }
        else if(basic::strcmp(command, "checkheap") == 0) {
            memory::kernel_heap.printHeapInfo();
        }
        else if(basic::strcmp(command, "lspagedir") == 0) {
            memory::PagingManager::printPageDirectory();
        }
        else if(basic::strcmp(command, "numtasks") == 0) {
            basic::printf("Number of tasks in shell list : %d\n", numShellThreads);
        }
        else if(basic::strcmp(command, "uptime") == 0) {
            basic::printf("System Uptime: %d ms\n", (int)essential::Time::getUptimeMS());
        }
        else if (basic::strncmp(command, "sleep", 5) == 0) {
            char* msStr = (char*)command + 5;
            if(*msStr == '\0') {
                basic::printf("Usage: sleep <milliseconds>\n");
            } else {
                int ms = basic::stoi(msStr);
                basic::printf("Sleeping %d ms...\n", ms);
                essential::Time::sleep(ms);
                basic::printf("Done.\n");
            }
        }
        else if (basic::strncmp(command, "task", 4) == 0){
             char* tasknumstr=(char*)command+4;
             int tasknum = basic::stoi(tasknumstr) - 1;
             
             if(tasknum < 0 || tasknum >= numShellThreads) {
                 basic::printf("task number %d not present in task list of OSOS shell :(\n", tasknum+1);
             } else {
                 basic::printf("Starting task number %d...\n", tasknum+1);
                 if (osos_ThreadManager_ptr->addThread(shell_threads_ptr[tasknum]) == false) {
                      basic::printf("Error: failed to start task%d.\n", tasknum+1);
                 }
             }
        }
        else {
            basic::printf("Unknown command: '%s'\n", command);
        }
    }
};

#endif