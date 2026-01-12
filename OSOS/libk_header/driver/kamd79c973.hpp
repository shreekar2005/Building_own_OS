#ifndef KAMD79C973_HPP
#define KAMD79C973_HPP

#include "essential/ktypes.hpp"
#include "driver/kdriver.hpp"
#include "hardware_communication/kinterrupt.hpp"
#include "hardware_communication/kport.hpp"
#include "hardware_communication/kpci.hpp"

namespace driver {

    /// @brief Driver for the AMD am79c973 (PCNet-FAST III) Ethernet Controller.
    /// This class manages the initialization, sending, and receiving of network packets
    /// via the PCI bus. It implements both the Driver interface and the InterruptHandler interface.
    class amd_am79c973 : public Driver, public hardware_communication::InterruptHandler {
        
        hardware_communication::Port16Bit MACAddress0Port;
        hardware_communication::Port16Bit MACAddress2Port;
        hardware_communication::Port16Bit MACAddress4Port;
        hardware_communication::Port16Bit RegisterDataPort;
        hardware_communication::Port16Bit RegisterAddressPort;
        hardware_communication::Port16Bit ResetPort;
        hardware_communication::Port16Bit BusControlRegisterDataPort;

        /// @brief structure representing the Initialization Block required by the device.
        /// It contains configuration modes and pointers to the descriptor rings.
        /// Must be 16-byte aligned.
        struct InitializationBlock {
            uint16_t mode;
            unsigned reserved1 : 4;
            unsigned numSendBuffers : 4;
            unsigned reserved2 : 4;
            unsigned numRecvBuffers : 4;
            uint64_t physicalAddress : 48; // MAC Address
            uint16_t reserved3;
            uint64_t logicalAddress;
            uint32_t recvBufferDescrAddress;
            uint32_t sendBufferDescrAddress;
        } __attribute__((packed));

        /// @brief structure representing a Buffer Descriptor in the Ring.
        /// Points to the actual buffer memory and holds status flags.
        /// Must be 16-byte aligned.
        struct BufferDescriptor {
            uint32_t address;
            uint32_t flags;
            uint32_t flags2;
            uint32_t avail;
        } __attribute__((packed));

        // Pointers to the active, aligned structures in memory
        InitializationBlock* initBlock;
        BufferDescriptor* sendBufferDescr;
        BufferDescriptor* recvBufferDescr;

        // --- Memory Management (Fixed to prevent DMA corruption) ---

        /// @brief Raw memory for the 8 Send Buffer Descriptors (needs 128 bytes + alignment).
        uint8_t sendBufferDescrMemory[256];
        
        /// @brief Raw memory for the 8 Receive Buffer Descriptors (needs 128 bytes + alignment).
        uint8_t recvBufferDescrMemory[256];

        /// @brief Actual storage for outgoing packets (8 buffers * 2KB).
        uint8_t sendBuffers[8][2048 + 15];

        /// @brief Actual storage for incoming packets (8 buffers * 2KB).
        uint8_t recvBuffers[8][2048 + 15];

        /// @brief Raw memory for the Initialization Block.
        uint8_t initBlockMem[sizeof(InitializationBlock) + 15];

        /// @brief Index of the current Send Buffer Descriptor we are writing to.
        uint8_t currentSendBuffer;

        /// @brief Index of the current Receive Buffer Descriptor we are reading from.
        uint8_t currentRecvBuffer;

    public:
        /// @brief Constructs the driver instance.
        /// @param dev The PCI Device Descriptor containing port numbers and IRQ.
        /// @param interrupts The global Interrupt Manager to register the handler.
        amd_am79c973(hardware_communication::PCI_DeviceDescriptor* dev, 
                        hardware_communication::InterruptManager* interrupts);
        
        /// @brief Destructor.
        ~amd_am79c973();

        /// @brief Activates the device (enables interrupts and starts the chip).
        void activate();

        /// @brief Resets the device.
        /// @return The wait time required after reset (in ms).
        int reset();

        /// @brief Deactivates the driver (stops the chip).
        void deactivate();

        /// @brief Sends a raw Ethernet packet.
        /// @param buffer Pointer to the data to send.
        /// @param count Number of bytes to send.
        void Send(uint8_t* buffer, int count);
        
        /// @brief Handles hardware interrupts from the network card.
        /// @param esp The stack pointer at the time of interrupt.
        /// @return The new stack pointer (usually unchanged).
        uint32_t handleInterrupt(uint32_t esp);
    };
}

#endif