#include "memory/kpaging.hpp"
#include "basic/kiostream.hpp"

namespace memory {

    // Define the static variables
    uint32_t* PagingManager::kernelPageDirectory = nullptr;
    uint32_t* PagingManager::activePageDirectory = nullptr;
    
    // Loads CR3 register
    inline void asm_loadPageDirectory(uint32_t* pdAddress) {
        asm volatile("mov %0, %%cr3" :: "r"(pdAddress));
    }

    // Enables Paging bit (PG) in CR0
    inline void asm_enablePagingMode() {
        uint32_t cr0;
        asm volatile("mov %%cr0, %0" : "=r"(cr0));
        cr0 |= 0x80000000;
        asm volatile("mov %0, %%cr0" :: "r"(cr0));
    }

    void PagingManager::init() {
        basic::printf("Initializing Paging...\n");

        kernelPageDirectory = (uint32_t*)memory::PhysicalMemoryManager::allocate_block();
        
        // Clear it (Set all to "Not Present"), Attributes: Supervisor (User=0), RW=1, Present=0 -> 0x00000002
        for(int i = 0; i < 1024; i++) {
            kernelPageDirectory[i] = 0x00000002;
        }

        // Identity Map the first 16MB of RAM (Kernel Space). This ensures the Kernel code, PMM bitmap, and these Tables are accessible.
        uint32_t currentPhysicalAddress = 0;

        for (int pdIndex = 0; pdIndex < 4; pdIndex++) {
            // Allocate a Page Table for this 4MB region
            uint32_t* pt = (uint32_t*)memory::PhysicalMemoryManager::allocate_block();

            // Fill the table (1024 entries * 4KB = 4MB)
            for(int ptIndex = 0; ptIndex < 1024; ptIndex++) {
                // Attributes: Supervisor, RW, Present (0x3)
                pt[ptIndex] = currentPhysicalAddress | 3; 
                currentPhysicalAddress += 4096; 
            }

            // Install the Table into the Kernel Directory
            kernelPageDirectory[pdIndex] = (uint32_t)pt | 3;
        }

        // Map the Heap Area (16MB to 24MB) = 8MB Heap
        // We continue from where 'currentPhysicalAddress' left off (at 16MB).
        // pdIndex 4 covers 16MB-20MB
        // pdIndex 5 covers 20MB-24MB
        for (int i = 0; i < 2; i++) {
            uint32_t* pt = (uint32_t*)memory::PhysicalMemoryManager::allocate_block();
            
            for(int ptIndex = 0; ptIndex < 1024; ptIndex++) {
                pt[ptIndex] = currentPhysicalAddress | 3; 
                currentPhysicalAddress += 4096; 
            }
            
            // Register at index 4 + i (Indices 4 and 5)
            kernelPageDirectory[4 + i] = (uint32_t)pt | 3;
        }

        // Activate the Kernel Directory
        switchPageDirectory(kernelPageDirectory);
        
        // Turn on the CPU Paging Unit
        asm_enablePagingMode();

        basic::printf("Paging Enabled! (Mapped 0-24MB)\n");
    }

    void PagingManager::switchPageDirectory(uint32_t* newDirectory) {
        activePageDirectory = newDirectory;
        asm_loadPageDirectory(activePageDirectory);
    }

    void* PagingManager::getPhysicalAddress(void* virtualAddress) {
        uintptr_t vAddr = (uintptr_t)virtualAddress;
        
        uint32_t pdIndex = vAddr >> 22;
        uint32_t ptIndex = (vAddr >> 12) & 0x03FF;
        uint32_t offset  = vAddr & 0x0FFF;

        // Check Page Directory Entry
        // We use activePageDirectory because that's what the CPU is seeing right now
        if ((activePageDirectory[pdIndex] & 0x1) == 0) {
            return nullptr; 
        }

        // Get the Page Table address (mask out flags)
        uint32_t* pageTable = (uint32_t*)(activePageDirectory[pdIndex] & 0xFFFFF000);
        
        // Check Page Table Entry
        if ((pageTable[ptIndex] & 0x1) == 0) {
            return nullptr;
        }

        uint32_t physicalFrame = pageTable[ptIndex] & 0xFFFFF000;
        return (void*)(uintptr_t)(physicalFrame + offset);
    }

    void PagingManager::printPageDirectory() {
        if (!activePageDirectory) return;
        
        basic::printf("\n--- Active Page Directory Entries ---\n");
        for (int i = 0; i < 1024; i++) {
            // Check Present Bit (Bit 0)
            if (activePageDirectory[i] & 1) { 
                basic::printf("Index %d: Mapped to Table at 0x%x (Flags: 0x%x)\n", 
                    i, 
                    activePageDirectory[i] & 0xFFFFF000, 
                    activePageDirectory[i] & 0xFFF
                );
            }
        }
        basic::printf("-------------------------------------\n");
    }
}