#include "../kernel_src/include/gdt"

/* The struct definitions remain the same */
struct gdtr_t {
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed));

struct gdt_entry_t {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_middle;
    unsigned char  access;
    unsigned char  granularity;
    unsigned char  base_high;
} __attribute__((packed));


/* This is the main inspection function, now using printf */
void inspect_gdt() {
    struct gdtr_t gdtr;
    asm volatile("sgdt %0" : "=m"(gdtr));

    // Print the location and size of the GDT
    printf("GDT is located at: %x\n", gdtr.base);
    printf("Limit (size in bytes): %x\n---\n", gdtr.limit);

    // Get a pointer to the first GDT entry
    struct gdt_entry_t* gdt_entries = (struct gdt_entry_t*)gdtr.base;
    
    // Calculate how many entries there are. The limit is size-1.
    int num_entries = (gdtr.limit + 1) / sizeof(struct gdt_entry_t);

    for (int i = 0; i < num_entries; i++) {
        struct gdt_entry_t entry = gdt_entries[i];

        // Combine the scattered base and limit fields
        unsigned int base = entry.base_high << 24 | entry.base_middle << 16 | entry.base_low;
        unsigned int limit = (entry.granularity & 0x0F) << 16 | entry.limit_low;
        
        // Check the Granularity bit
        if ((entry.granularity & 0x80) != 0) {
            // If the G bit is set, the limit is in 4 KiB pages
            limit = (limit << 12) | 0xFFF;
        }

        // A single, clean printf call to display all the info
        printf("Entry %d: Base=%x, Limit=%x, Access=%x, Granularity=%x\n",
               i, base, limit, entry.access, entry.granularity);
    }
}