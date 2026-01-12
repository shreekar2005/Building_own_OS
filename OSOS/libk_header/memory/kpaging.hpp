#ifndef _OSOS_MEMORY_KPAGING_H
#define _OSOS_MEMORY_KPAGING_H

#include "essential/ktypes.hpp"
#include "memory/kpmm.hpp" 

namespace memory 
{
    /// @brief Manages Virtual Memory via x86 Paging. Supports switching between multiple Page Directories for multitasking.
    class PagingManager {
    public:
        /// @brief Initializes the Paging system. Creates the Master Kernel Directory, maps Kernel memory (0-16MB), and enables paging.
        static void init();

        /// @brief Loads a specific Page Directory into the CR3 register. This effectively switches the "Memory View" for the CPU.
        /// @param newDirectory Pointer to the Page Directory to load.
        static void switchPageDirectory(uint32_t* newDirectory);

        /// @brief Resolves a Virtual Address to its Physical Address using the CURRENTLY active Page Directory.
        static void* getPhysicalAddress(void* virtualAddress);

        /// @brief Prints the currently active Page Directory entries.
        static void printPageDirectory();

        // The Master Directory containing kernel mappings (shared by all).
        static uint32_t* kernelPageDirectory;

        // The Directory currently loaded in the CPU (CR3).
        static uint32_t* activePageDirectory;
    };
}

#endif