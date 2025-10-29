#ifndef _OSOS_HARDWARECOMMUNICATION_KPCI_H
    #define _OSOS_HARDWARECOMMUNICATION_KPCI_H
    #include <cstdint>
    #include "hardware_communication/kport.hpp"
    namespace hardware_communication
    {
        /// @brief Peripheral Component Interconnect Controller
        class PCI_Controller{
            private:
                Port32Bit dataPort;
                Port32Bit commandPort;
            public:
                PCI_Controller();
                ~PCI_Controller();
                uint16_t read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registerOffset);
                void write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registerOffset, uint32_t value);
        };
    }
#endif