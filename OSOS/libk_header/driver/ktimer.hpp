#ifndef _OSOS_DRIVER_KTIMER_H
#define _OSOS_DRIVER_KTIMER_H

#include "basic/ktypes.hpp"
#include "hardware_communication/kinterrupt.hpp"
#include "hardware_communication/kport.hpp"
#include "driver/kdriver.hpp"

// Programmable Interval Timer (PIT) Constants
#define PIT_COMMAND_PORT     0x43
#define PIT_CHANNEL0_PORT    0x40
#define PIT_FREQUENCY_BASE   1193182

namespace driver
{

/// @brief Custom Driver for the 8253/8254 Programmable Interval Timer (PIT).
class TimerDriver : public hardware_communication::InterruptHandler, public driver::Driver{
        hardware_communication::Port8Bit dataPort;
        hardware_communication::Port8Bit commandPort;
        
        volatile uint64_t ticks;
        uint32_t frequency;

    public :
        /// @brief Constructor for the Timer Driver.
        /// @param interrupt_manager Pointer to the InterruptManager.
        /// @param frequency The desired frequency in Hz (e.g., 1000 for 1ms precision).
        TimerDriver(hardware_communication::InterruptManager* interrupt_manager, uint32_t frequency = 1000);
        ~TimerDriver();

        /// @brief Handles the timer interrupt (IRQ 0).
        /// @param esp The stack pointer from the interrupt context.
        /// @return The stack pointer.
        uint32_t handleInterrupt(uint32_t esp) override;
        
        /// @brief Activates the timer driver and configures the hardware.
        void activate() override;

        /// @brief Resets the timer. (Stub)
        /// @return Always returns 0.
        int reset() override;

        /// @brief Deactivates the timer driver. (Stub)
        void deactivate() override;

        /// @brief Pauses execution for a specific duration.
        /// @param milliseconds The number of milliseconds to sleep.
        void sleep(uint32_t milliseconds);

        /// @brief Gets the current frequency of the timer.
        /// @return The frequency in Hz.
        uint32_t getFrequency() const;

        /// @brief Gets the total number of ticks occurred since the driver was activated.
        /// @return The total tick count.
        uint64_t getTicks() const;

        /// @brief Calculates the system uptime in milliseconds based on ticks and frequency.
        /// @return The uptime in milliseconds.
        uint64_t getUptimeMS() const;
};
}

#endif