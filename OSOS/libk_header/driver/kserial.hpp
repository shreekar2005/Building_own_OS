#ifndef _OSOS_DRIVER_KSERIAL_H
#define _OSOS_DRIVER_KSERIAL_H

#include <cstdint>
#include "driver/kdriver.hpp"
#include "hardware_communication/kinterrupt.hpp"
#include "hardware_communication/kport.hpp"

namespace driver 
{
    // Interface for handling serial port events
    class SerialEventHandler{
        public:
            SerialEventHandler();
            virtual ~SerialEventHandler();
            virtual void onDataReceived(char data)=0;
    };

    // Driver for the COM1 serial port (0x3F8)
    class SerialDriver : public hardware_communication::InterruptHandler, public driver::Driver{
    private:
        hardware_communication::Port8Bit dataPort;
        hardware_communication::Port8Bit intEnablePort;
        hardware_communication::Port8Bit modemControlPort;
        hardware_communication::Port8Bit lineStatusPort;

        SerialEventHandler* eventHandler;
    
    public:
        SerialDriver(hardware_communication::InterruptManager* interrupt_manager, SerialEventHandler* eventHandler);
        ~SerialDriver();

        uint32_t handleInterrupt(uint32_t esp) override;
        void activate() override;
        int reset() override;
        void deactivate() override;
    };
}

#endif

