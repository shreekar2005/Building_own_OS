#ifndef _OSOS_BASIC_KTIME_H
#define _OSOS_BASIC_KTIME_H

#include "basic/ktypes.hpp"
#include "driver/ktimer.hpp"

namespace essential {
    class Time {
    private:
        static driver::TimerDriver* activeTimerDriver;

    public:
        // Called in kernelMain to register the driver
        static void setTimerDriver(driver::TimerDriver* driver);
        
        /// @brief sleep for milliseconds (Blocking)
        static void sleep(uint32_t ms);

        /// @brief Get system uptime in milliseconds
        static uint64_t getUptimeMS();

        /// @brief Get current timer frequency
        static uint32_t getFrequency();
    };
}

#endif