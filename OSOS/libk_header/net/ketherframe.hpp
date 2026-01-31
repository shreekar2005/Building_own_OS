#ifndef _OSOS_NET_KETHERFRAME_H
#define _OSOS_NET_KETHERFRAME_H

#include "essential/ktypes.hpp"
#include "driver/kamd79c973.hpp"


namespace net
{
    
    struct EtherFrameHeader
    {
        uint64_t dstMAC_BE : 48;
        uint64_t srcMAC_BE : 48;
        uint16_t etherType_BE;
    } __attribute__ ((packed));
    
    typedef uint32_t EtherFrameFooter;
    
    class EtherFrameProvider;
    
    class EtherFrameHandler
    {
    protected:
        EtherFrameProvider* backend;
        uint16_t etherType_BE;
            
    public:
        EtherFrameHandler(EtherFrameProvider* backend, uint16_t etherType);
        ~EtherFrameHandler();

        virtual bool onEtherFrameReceived(uint8_t* etherframePayload, uint32_t size);
        void send(uint64_t dstMAC_BE, uint8_t* etherframePayload, uint32_t size);
        uint32_t getIPAddress();
    };
    
    
    class EtherFrameProvider : public driver::RawDataHandler
    {
    friend class EtherFrameHandler;
    protected:
        EtherFrameHandler* handlers[65535];
    public:
        EtherFrameProvider(driver::amd_am79c973* backend);
        ~EtherFrameProvider();
        
        bool onRawDataReceived(uint8_t* buffer, uint32_t size);
        void send(uint64_t dstMAC_BE, uint16_t etherType_BE, uint8_t* buffer, uint32_t size);
        
        uint64_t getMACAddress();
        uint32_t getIPAddress();
    };
    
    
}

#endif