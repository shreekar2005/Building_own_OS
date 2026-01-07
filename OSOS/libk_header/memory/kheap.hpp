#ifndef _OSOS_MEMORY_KHEAP_H
#define _OSOS_MEMORY_KHEAP_H

#include <cstdint>
#include <stddef.h> 

namespace memory 
{
    struct MemoryBlockHeader {
        size_t size;             // Size of the data part (excluding header)
        bool is_free;            // Is this block usable?
        MemoryBlockHeader* next; // Next block in the list
        MemoryBlockHeader* prev; // Previous block in the list
    };

    class HeapManager {
    private:
        MemoryBlockHeader* head; // The start of THIS specific heap

    public:
        HeapManager() : head(nullptr) {}

        /// @brief Initialize this specific heap instance.
        /// @param startAddress Virtual address where this heap begins.
        /// @param sizeTotal Total size of the heap (in bytes).
        void init(void* startAddress, size_t sizeTotal);

        /// @brief Allocate memory from this specific heap.
        void* allocate(size_t size);

        /// @brief Free memory back to this specific heap.
        void free(void* ptr);

        /// @brief Print debug info for this heap.
        void printHeapInfo();
    };

    // The Global Kernel Heap instance
    extern HeapManager kernel_heap;
}


extern "C" {
    void* malloc(size_t size);
    void free(void* ptr);
    void* calloc(size_t num, size_t size);
    void* realloc(void* ptr, size_t new_size);
}

void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;
void operator delete(void* ptr, size_t size) noexcept;
void operator delete[](void* ptr, size_t size) noexcept;

#endif