#include "../kernel_src/include/gdt"

/* The GDT Register (GDTR) struct */
struct gdtr_t {
    uint16_t limit;
    uintptr_t base; // Use uintptr_t for an integer type that can hold a pointer
} __attribute__((packed));

/* GDT entry struct (defined by CPU architecture) */
struct gdt_entry_t {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));


/* Print global descriptor table */
void print_gdt() {
    struct gdtr_t gdtr;
    asm volatile("sgdt %0" : "=m"(gdtr));

    // Print the location and size of the GDT
    printf("GDT is located at: %p\n", (void*)gdtr.base);
    printf("Limit (size in bytes): %#x\n---\n", gdtr.limit); // Use hex for a limit

    // Get a pointer to the first GDT entry
    struct gdt_entry_t* gdt_entries = (struct gdt_entry_t*)gdtr.base;
    
    // Calculate how many entries there are
    int num_entries = (gdtr.limit + 1) / sizeof(struct gdt_entry_t);

    for (int i = 0; i < num_entries; i++) {
        struct gdt_entry_t entry = gdt_entries[i];

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
}
