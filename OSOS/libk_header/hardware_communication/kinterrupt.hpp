#ifndef _OSOS_HARDWARE_COMMUNICATION_KINTERRUPT_H

    #define _OSOS_HARDWARE_COMMUNICATION_KINTERRUPT_H
    #include <cstdint>
    #include "basic/kiostream.hpp"
    #include "hardware_communication/kport.hpp"
    #include "essential/kgdt.hpp"

    extern "C" {
        // following functions are defined in kinterruptstub.s
        void ignoreInterrupt();
        void handleIRQ0x00(); // Timer
        void handleIRQ0x01(); // Keyboard
        void handleIRQ0x0C(); // PS/2 Mouse 
    }

    namespace hardware_communication{
        // -------------------------- Class structures ------------------------
        
        //making forward definition for this class (needed in InterruptHandler)
        class InterruptManager;
        //Interrupthandler is an INTERFACE
        class InterruptHandler{
            protected:
                uint8_t interruptNumber;
                InterruptManager* interrupt_manager;
                InterruptHandler(uint8_t interruptNumber, InterruptManager* interrupt_manager);
                ~InterruptHandler();
            public:
                virtual uintptr_t handleInterrupt(uintptr_t esp)=0; //pure virtual function
        };

        class IDT_row{
            private:
                uint16_t handlerAddressLowbits;
                uint16_t kernelCodeSegmentSelector;
                uint8_t reserved;
                uint8_t access;
                uint16_t handlerAddressHighbits;
            public:
                IDT_row();
                ~IDT_row();
                friend class InterruptManager;
        }__attribute__((packed));
        
        class InterruptManager{
            
            friend class InterruptHandler;

            public :
                uint16_t limit;
                uint32_t base;
            protected:
                IDT_row interruptDescriptorTable[256];
                //handlers are actually object of corresponding drivers (e.g. keyboard-driver object which have handleInterrupt method)
                InterruptHandler* handlers[256];
                void setIDTEntry(
                    uint8_t interruptNumber,
                    uint16_t codeSegmentSelectorOffset,
                    void (*handler)(),
                    uint8_t DescriptorPrivilegeLever,
                    uint8_t DescriptorType);
                    
                static hardware_communication::Port8BitSlow picMasterCommand;
                static hardware_communication::Port8BitSlow picMasterData;
                static hardware_communication::Port8BitSlow picSlaveCommand;
                static hardware_communication::Port8BitSlow picSlaveData;    
    
            public:
                InterruptManager(essential::GDT* gdt);
                ~InterruptManager();
                void installTable();

                static void activate();
                static void deactivate();
                static void printLoadedTable();
                static void printLoadedTableHeader();
                static uintptr_t handleInterrupt(uint8_t interruptNumber, uintptr_t esp);
        };
    }

#endif