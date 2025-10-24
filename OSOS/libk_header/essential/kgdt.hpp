#ifndef _OSOS_ESSENTIAL_GDT_H
#define _OSOS_ESSENTIAL_GDT_H

    #include <cstdint>
    #include "basic/kiostream.hpp"

    // --- Flags for the 8-bit ACCESS BYTE ---
    #define SEG_ACCESS_TYPE(x)  ((x) << 0)  // Type field (e.g., SEG_CODE_EXRD)
    #define SEG_ACCESS_DESCTYPE(x) ((x) << 4) // Descriptor type (0=system, 1=code/data)
    #define SEG_ACCESS_PRIV(x)  (((x) & 0x03) << 5) // Privilege level (0-3)
    #define SEG_ACCESS_PRES(x)  ((x) << 7)  // Present bit

    // --- Flags for the upper 4 bits of the GRANULARITY BYTE ---
    #define SEG_GRAN_SAVL(x)    ((x) << 4)  // Available for system use
    #define SEG_GRAN_LONG(x)    ((x) << 5)  // Long mode (64-bit)
    #define SEG_GRAN_SIZE(x)    ((x) << 6)  // Size (0=16-bit, 1=32-bit)
    #define SEG_GRAN_GRAN(x)    ((x) << 7)  // Granularity (0=1B, 1=4KB pages)

    // --- Segment Type Fields (for the access byte) ---
    #define SEG_DATA_RDWR      0x02 // Read/Write
    #define SEG_CODE_EXRD      0x0A // Execute/Read

    // --- Correctly combined ACCESS byte values ---
    #define GDT_ACCESS_CODE_PL0 SEG_ACCESS_PRES(1) | SEG_ACCESS_PRIV(0) | SEG_ACCESS_DESCTYPE(1) | SEG_ACCESS_TYPE(SEG_CODE_EXRD)
    #define GDT_ACCESS_DATA_PL0 SEG_ACCESS_PRES(1) | SEG_ACCESS_PRIV(0) | SEG_ACCESS_DESCTYPE(1) | SEG_ACCESS_TYPE(SEG_DATA_RDWR)
    #define GDT_ACCESS_CODE_PL3 SEG_ACCESS_PRES(1) | SEG_ACCESS_PRIV(3) | SEG_ACCESS_DESCTYPE(1) | SEG_ACCESS_TYPE(SEG_CODE_EXRD)
    #define GDT_ACCESS_DATA_PL3 SEG_ACCESS_PRES(1) | SEG_ACCESS_PRIV(3) | SEG_ACCESS_DESCTYPE(1) | SEG_ACCESS_TYPE(SEG_DATA_RDWR)

    // --- Correctly combined GRANULARITY byte flags ---
    // For a 32-bit OS in protected mode with 4KB pages
    #define GDT_GRAN_FLAGS      SEG_GRAN_GRAN(1) | SEG_GRAN_SIZE(1) | SEG_GRAN_LONG(0) | SEG_GRAN_SAVL(0)

    namespace essential{
        /// @brief Represents a single 8-byte segment descriptor entry in the Global Descriptor Table (GDT).
        class GDT_Row {
            private :
                uint16_t limit_low;
                uint16_t base_low;
                uint8_t  base_middle;
                uint8_t  access;
                uint8_t  granularity;
                uint8_t  base_high;

            public:
                GDT_Row(uint32_t base, uint32_t limit, uint8_t access_byte, uint8_t gran_byte);
                GDT_Row(); // Default constructor for null segment
                ~GDT_Row();
                friend class GDT_Manager;

        } __attribute__((packed));

        /// @brief Manages the Global Descriptor Table (GDT), holding segment descriptors for the CPU.
        class GDT_Manager {
            private:
                uint16_t limit;
                uintptr_t base;
                GDT_Row nullSegment;
                GDT_Row kernel_CS;
                GDT_Row kernel_DS;
                GDT_Row user_CS;
                GDT_Row user_DS;

            public:
                GDT_Manager();
                ~GDT_Manager();
                void installTable();
                static void printLoadedTable();
                static void printLoadedTableHeader();
                static uint16_t kernel_CS_selector();
                static uint16_t kernel_DS_selector();
                static uint16_t user_CS_selector();
                static uint16_t user_DS_selector();
        } __attribute__((packed));
    }

#endif