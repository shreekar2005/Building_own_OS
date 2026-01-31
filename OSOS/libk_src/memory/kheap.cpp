#include "memory/kheap.hpp"
#include "basic/kiostream.hpp"

namespace memory {

    // global instance of the kernel heap
    HeapManager kernel_heap;

    void HeapManager::init(void* startAddress, size_t sizeTotal) {
        head = (MemoryBlockHeader*)startAddress;
        head->size = sizeTotal - sizeof(MemoryBlockHeader);
        head->is_free = true;
        head->next = nullptr;
        head->prev = nullptr;

        basic::printf("Heap Initialized at 0x%x (Size: %d bytes)\n", startAddress, sizeTotal);
    }

    void* HeapManager::allocate(size_t size) {
        MemoryBlockHeader* current = head;

        while (current != nullptr) {
            if (current->is_free && current->size >= size) {
                if (current->size > size + sizeof(MemoryBlockHeader) + 1) {
                    MemoryBlockHeader* newBlock = (MemoryBlockHeader*)((uintptr_t)current + sizeof(MemoryBlockHeader) + size);
                    
                    newBlock->size = current->size - size - sizeof(MemoryBlockHeader);
                    newBlock->is_free = true;
                    newBlock->next = current->next;
                    newBlock->prev = current;

                    current->size = size;
                    current->is_free = false;
                    current->next = newBlock;

                    if (newBlock->next != nullptr) {
                        newBlock->next->prev = newBlock;
                    }
                } else {
                    current->is_free = false;
                }

                return (void*)((uintptr_t)current + sizeof(MemoryBlockHeader));
            }
            current = current->next;
        }

        basic::printf("Heap OOM: Cannot allocate %d bytes\n", size);
        return nullptr;
    }

    void HeapManager::free(void* ptr) {
        if (!ptr) return;

        // move pointer back to get the header
        MemoryBlockHeader* block = (MemoryBlockHeader*)((uintptr_t)ptr - sizeof(MemoryBlockHeader));
        
        // mark it as free
        block->is_free = true;

        // merge with next block if it is free
        if (block->next != nullptr && block->next->is_free) {
            block->size += block->next->size + sizeof(MemoryBlockHeader);
            block->next = block->next->next;
            if (block->next != nullptr) {
                block->next->prev = block;
            }
        }

        // merge with previous block if it is free
        if (block->prev != nullptr && block->prev->is_free) {
            block->prev->size += block->size + sizeof(MemoryBlockHeader);
            block->prev->next = block->next;
            if (block->next != nullptr) {
                block->next->prev = block->prev;
            }
        }
    }

    void HeapManager::printHeapInfo() {
        MemoryBlockHeader* current = head;
        size_t total_free = 0;
        int i=0;
        while (current != nullptr) {
            if (current->is_free) total_free += current->size;
            basic::printf("Block %-2d: %s, Size(B): %-7d, Addr: 0x%x\n", i++, current->is_free ? "FREE" : "USED", current->size, current);
            current = current->next;
        }
        basic::printf("Total Free Heap: %d B\n", total_free);
    }
}


extern "C" {
    void* malloc(size_t size) {
        return memory::kernel_heap.allocate(size);
    }

    void free(void* ptr) {
        memory::kernel_heap.free(ptr);
    }

    void* calloc(size_t num, size_t size) {
        size_t total = num * size;
        void* ptr = malloc(total);
        if (ptr) {
            uint8_t* p = (uint8_t*)ptr;
            for (size_t i = 0; i < total; i++) {
                p[i] = 0;
            }
        }
        return ptr;
    }

    void* realloc(void* ptr, size_t new_size) {
        if (!ptr) return malloc(new_size);
        if (new_size == 0) {
            free(ptr);
            return nullptr;
        }

        memory::MemoryBlockHeader* header = (memory::MemoryBlockHeader*)((uintptr_t)ptr - sizeof(memory::MemoryBlockHeader));
        size_t old_size = header->size;

        // Allocate new block
        void* new_ptr = malloc(new_size);
        if (!new_ptr) return nullptr;

        // Copy data (Manual memcpy)
        size_t copy_size = (old_size < new_size) ? old_size : new_size;
        uint8_t* src = (uint8_t*)ptr;
        uint8_t* dst = (uint8_t*)new_ptr;
        for (size_t i = 0; i < copy_size; i++) {
            dst[i] = src[i];
        }

        free(ptr);
        return new_ptr;
    }
}


void* operator new(size_t size) {
    return memory::kernel_heap.allocate(size);
}

void* operator new[](size_t size) {
    return memory::kernel_heap.allocate(size);
}

void operator delete(void* ptr) noexcept {
    memory::kernel_heap.free(ptr);
}

void operator delete[](void* ptr) noexcept {
    memory::kernel_heap.free(ptr);
}

void operator delete(void* ptr, size_t size) noexcept {
    (void)size;
    memory::kernel_heap.free(ptr);
}

void operator delete[](void* ptr, size_t size) noexcept {
    (void)size;
    memory::kernel_heap.free(ptr);
}