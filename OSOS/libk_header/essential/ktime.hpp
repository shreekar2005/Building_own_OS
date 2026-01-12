#ifndef _OSOS_ESSENTIAL_KTIME_HPP
#define _OSOS_ESSENTIAL_KTIME_HPP

#include "essential/ktypes.hpp"
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