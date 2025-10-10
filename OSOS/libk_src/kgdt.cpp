// #include "../kernel_src/include/gdt"
#include "kgdt"

#define GDT_ENTRIES 5 // We will have 5 entries: Null, Kernel Code, Kernel Data, User Code, User Data

/* The GDT entry struct (defined by CPU architecture) */
struct gdt_entry_t {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

/* The GDT Register (GDTR) struct */
struct gdtr_t {
    uint16_t limit;
    uintptr_t base; // Use uintptr_t for an integer type that can hold a pointer
} __attribute__((packed));


// Our GDT and the pointer to it
gdt_entry_t gdt_entries[GDT_ENTRIES];
gdtr_t      gdt_ptr;


/**
 * @brief Loads the specified GDT and reloads all segment registers.
 * @param gdt_ptr_addr The memory address of the GDTR structure.
 */
static void gdt_flush(uintptr_t gdt_ptr_addr) {
    asm volatile(
        // Load the Global Descriptor Table Register (GDTR) with the address
        // of our GDT pointer structure. %0 is the input operand (gdt_ptr_addr).
        "lgdt (%0)\n\t"

        // Reload the data segment registers. 0x10 is the selector for our
        // kernel data segment. We must use '%%' to escape the '%' for registers.
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"

        // Perform a far jump to reload the Code Segment (CS) register.
        // 0x08 is the selector for our kernel code segment.
        // '1f' creates a local label to jump to (f = forward).
        "jmp $0x08, $1f\n\t"
        "1:\n\t"
        : // No output operands
        : "r"(gdt_ptr_addr) // Input operand: gdt_ptr_addr is passed in a general-purpose register
        : "memory", "eax" // Clobbered resources: we modify memory and the EAX register
    );
}


/**
 * @brief Sets a single GDT entry.
 * @param num The index of the entry in the GDT.
 * @param base The base address of the segment.
 * @param limit The limit of the segment.
 * @param access The access flags for the segment.
 * @param gran The granularity flags for the segment.
 */
static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    // Set the upper 4 bits of the limit in the lower 4 bits of the granularity byte
    gdt_entries[num].granularity = ((limit >> 16) & 0x0F);

    // Set the granularity flags (G, D/B, L, AVL) in the upper 4 bits of the granularity byte
    gdt_entries[num].granularity |= (gran & 0xF0);
    gdt_entries[num].access      = access;
}

/**
 * @brief Initializes and installs the GDT.
 */
static void gdt_install() {
    // Set up the GDTR pointer
    gdt_ptr.limit = (sizeof(gdt_entry_t) * GDT_ENTRIES) - 1;
    gdt_ptr.base  = (uintptr_t)&gdt_entries;

    // 1. Null segment (required by CPU)
    // Selector: 0x00
    gdt_set_gate(0, 0, 0, 0, 0);

    // We extract the access byte (lower 8 bits) and the granularity flags (from the higher 8 bits).
    // Note: Parentheses around the GDT_... macros are CRITICAL to avoid operator precedence bugs.
    
    // 2. Kernel Code Segment (Ring 0)
    // Selector: 0x08
    gdt_set_gate(1, 0, 0xFFFFFFFF, ((GDT_CODE_PL0) & 0xFF), ((GDT_CODE_PL0) >> 8));

    // 3. Kernel Data Segment (Ring 0)
    // Selector: 0x10
    gdt_set_gate(2, 0, 0xFFFFFFFF, ((GDT_DATA_PL0) & 0xFF), ((GDT_DATA_PL0) >> 8));

    // 4. User Code Segment (Ring 3)
    // Selector: 0x18
    gdt_set_gate(3, 0, 0xFFFFFFFF, ((GDT_CODE_PL3) & 0xFF), ((GDT_CODE_PL3) >> 8));
    
    // 5. User Data Segment (Ring 3)
    // Selector: 0x20
    gdt_set_gate(4, 0, 0xFFFFFFFF, ((GDT_DATA_PL3) & 0xFF), ((GDT_DATA_PL3) >> 8));

    // Load our new GDT
    gdt_flush((uintptr_t)&gdt_ptr);
}


// Initialize GDT table
void __init_GDT() {
    gdt_install();
}


// Print global descriptor table
void print_GDT() {
    struct gdtr_t gdtr;
    asm volatile("sgdt %0" : "=m"(gdtr));

    // Print the location and size of the GDT
    printf("---\nGDT is located at: %p\n", (void*)gdtr.base);
    printf("Limit (size in bytes): %#x\n---\n", gdtr.limit);

    // Get a pointer to the first GDT entry
    struct gdt_entry_t* gdt_entries_ptr = (struct gdt_entry_t*)gdtr.base;
    
    // Calculate how many entries there are
    int num_entries = (gdtr.limit + 1) / sizeof(struct gdt_entry_t);

    for (int i = 0; i < num_entries; i++) {
        struct gdt_entry_t entry = gdt_entries_ptr[i];

        // Combine the scattered base and limit fields
        uint32_t base = entry.base_high << 24 | entry.base_middle << 16 | entry.base_low;
        uint32_t limit = (entry.granularity & 0x0F) << 16 | entry.limit_low;
        
        // Check the Granularity bit
        if ((entry.granularity & 0x80) != 0) {
            // If the G bit is set, the limit is in 4 KiB pages
            limit = (limit << 12) | 0xFFF;
        }

        printf("Entry %d: Base=%p, Limit=%#x, Access=%#x, Granularity=%#x\n",
               i, 
               (void*)(uintptr_t)base, // Cast base to void* for %p
               limit, 
               entry.access, 
               entry.granularity);
    }
    printf("---\n");
}