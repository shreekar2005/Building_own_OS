#include "basic/ktime.hpp"

namespace essential {
    
    driver::TimerDriver* Time::activeTimerDriver = 0;

    void Time::setTimerDriver(driver::TimerDriver* driver) {
        activeTimerDriver = driver;
    }

    void Time::sleep(uint32_t ms) {
        if (activeTimerDriver) {
            activeTimerDriver->sleep(ms);
        }
    }

    uint64_t Time::getUptimeMS() {
        if (activeTimerDriver) {
            return activeTimerDriver->getUptimeMS();
        }
        return 0;
    }

    uint32_t Time::getFrequency() {
        if (activeTimerDriver) {
            return activeTimerDriver->getFrequency();
        }
        return 0;
    }
}