#include "essential/kgdt"

// GDT_row (Segment Descriptor)

// Default constructor for a null GDT_row entry.
essential::GDT_row::GDT_row() : limit_low(0), base_low(0), base_middle(0), access(0), granularity(0), base_high(0) {}

essential::GDT_row::GDT_row(uint32_t base, uint32_t limit, uint8_t access_byte, uint8_t gran_byte) {
    this->limit_low   = (limit & 0xFFFF);
    this->base_low    = (base & 0xFFFF);
    this->base_middle = (base >> 16) & 0xFF;
    this->base_high   = (base >> 24) & 0xFF;

    // The upper 4 bits of the 20-bit limit go into the lower 4 bits of the granularity byte.
    this->granularity = ((limit >> 16) & 0x0F);
    // The granularity flags (G, D/B, L, AVL) go into the upper 4 bits.
    this->granularity |= (gran_byte & 0xF0);
    // Set the access flags for this segment.
    this->access = access_byte;
}

essential::GDT_row::~GDT_row() {}


// GDT (Global Descriptor Table)
essential::GDT::GDT()
    // Use an initializer list to construct each GDT entry.
    : nullSegment(), // First entry must be a null descriptor.
      kernel_CS(0, 0xFFFFFFFF, GDT_ACCESS_CODE_PL0, GDT_GRAN_FLAGS),
      kernel_DS(0, 0xFFFFFFFF, GDT_ACCESS_DATA_PL0, GDT_GRAN_FLAGS),
      user_CS(0, 0xFFFFFFFF, GDT_ACCESS_CODE_PL3, GDT_GRAN_FLAGS),
      user_DS(0, 0xFFFFFFFF, GDT_ACCESS_DATA_PL3, GDT_GRAN_FLAGS)
{
    // The GDT limit is its total size in bytes minus 1.
    // 5 entries * 8 bytes/entry = 40 bytes. Limit = 39 (0x27).
    limit = (sizeof(essential::GDT_row) * 5) - 1;
    
    // The base address is the address of the first entry in our table.
    base = (uintptr_t)&this->nullSegment;
}

essential::GDT::~GDT() {}


// Loads this GDT into the CPU's GDTR and reloads segment registers.
void essential::GDT::installTable() {
    // A structure that matches the 6-byte format required by the 'lgdt' instruction.
    struct GDT_Pointer {
        uint16_t limit;
        uint32_t base; // Use uint32_t for our i686 (32-bit) target
    } __attribute__((packed));

    GDT_Pointer gdt_ptr;
    gdt_ptr.limit = this->limit;
    gdt_ptr.base = this->base;
    
    asm volatile(
        // Load the address of our GDT_Pointer structure into the GDTR.
        "lgdt (%0)\n\t"

        // Reload all data segment registers with the kernel data selector.
        "mov $0x10, %%ax\n\t"   // 0x10 is the selector for kernel_DS
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"

        // Perform a 32-bit far jump to reload the CS register and flush the CPU pipeline.
        "jmp $0x08, $flush_cs\n\t" // 0x08 is the selector for kernel_CS
        "flush_cs:\n\t"
        : // No output operands
        : "r"(&gdt_ptr) // Input: address of our GDT_Pointer
        : "memory", "eax" // Clobbered registers
    );
    basic::printf("GDT Installed\n");
}

/**
 * @brief (Static) Prints the currently loaded GDT by reading the GDTR.
 */
void essential::GDT::printLoadedTable() {
    struct GDT_Pointer {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    GDT_Pointer gdtr;
    // Store the current GDT register contents into our struct.
    asm volatile("sgdt %0" : "=m"(gdtr));

    basic::printf("---\n");
    basic::printf("INFO about : Currently Loaded GDT\n");
    basic::printf("Base Address: %#x\n", gdtr.base);
    basic::printf("Limit: %#x (%d bytes)\n", gdtr.limit, gdtr.limit);
    basic::printf("Entries: %d\n", (gdtr.limit + 1) / sizeof(essential::GDT_row));
    basic::printf("---\n");

    essential::GDT_row* gdt_entries = (essential::GDT_row*)gdtr.base;
    int num_entries = (gdtr.limit + 1) / sizeof(essential::GDT_row);

    for (int i = 0; i < num_entries; i++) {
        essential::GDT_row entry = gdt_entries[i];

        // Reconstruct the base and limit from the scattered fields.
        uint32_t base = entry.base_high << 24 | entry.base_middle << 16 | entry.base_low;
        uint32_t limit = (entry.granularity & 0x0F) << 16 | entry.limit_low;
        
        // If the granularity bit is set, the limit is in 4 KiB pages.
        if ((entry.granularity & 0x80) != 0) {
            limit = (limit << 12) | 0xFFF;
        }

        basic::printf("GDT Entry %d: Base=%p, Limit=%#x, Access=%#x, Granularity=%#x\n",
               i, (void*)(uintptr_t)base, limit, entry.access, entry.granularity);
    }
    basic::printf("---\n");
}

void essential::GDT::printLoadedTableHeader() {
    struct GDT_Pointer {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    GDT_Pointer gdtr;
    // Store the current GDT register contents into our struct.
    asm volatile("sgdt %0" : "=m"(gdtr));

    basic::printf("---\n");
    basic::printf("INFO about : Currently Loaded GDT\n");
    basic::printf("Base Address: %#x\n", gdtr.base);
    basic::printf("Limit: %#x (%d bytes)\n", gdtr.limit, gdtr.limit);
    basic::printf("Entries: %d\n", (gdtr.limit + 1) / sizeof(essential::GDT_row));
    basic::printf("---\n");
}

// --- Static functions to get segment selectors ---

uint16_t essential::GDT::kernel_CS_selector() { return sizeof(essential::GDT_row) * 1; } // Selector 0x08
uint16_t essential::GDT::kernel_DS_selector() { return sizeof(essential::GDT_row) * 2; } // Selector 0x10
uint16_t essential::GDT::user_CS_selector()   { return sizeof(essential::GDT_row) * 3; } // Selector 0x18
uint16_t essential::GDT::user_DS_selector()   { return sizeof(essential::GDT_row) * 4; } // Selector 0x20