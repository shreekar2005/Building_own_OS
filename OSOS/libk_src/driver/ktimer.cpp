#include "driver/ktimer.hpp"
#include "basic/kiostream.hpp"

using namespace driver;

/// @brief function that make systemcall 0x80 to forcefully do thread scheduling to next thread
static void os_yield() {
    asm volatile("int $0x80");// 0x80 : syscall that forcefully schedules thread
}

TimerDriver::TimerDriver(hardware_communication::InterruptManager* interrupt_manager, uint32_t frequency)
: hardware_communication::InterruptHandler(0x20, interrupt_manager), // IRQ 0 maps to 0x20
  dataPort(PIT_CHANNEL0_PORT), 
  commandPort(PIT_COMMAND_PORT),
  ticks(0),
  frequency(frequency)
{
}

TimerDriver::~TimerDriver(){}

uint32_t TimerDriver::handleInterrupt(uint32_t esp)
{
    ticks++;
    return esp;
}

void TimerDriver::activate()
{
    // Calculate the divisor: 1193182 / Target Frequency
    uint32_t divisor = PIT_FREQUENCY_BASE / frequency;

    // Send the command byte 0x36 to Port 0x43:
    // Channel 0, Access mode: lo/hi byte, Mode 3 (Square Wave), Binary mode
    commandPort.write(0x36);

    // Send the divisor low byte
    dataPort.write((uint8_t)(divisor & 0xFF));

    // Send the divisor high byte
    dataPort.write((uint8_t)((divisor >> 8) & 0xFF));
    
    basic::printf("Timer Driver activated!\n");
}

void TimerDriver::sleep(uint32_t milliseconds)
{
    // Calculate how many ticks we need to wait
    uint64_t ticksToWait = (milliseconds * frequency) / 1000;
    uint64_t endTicks = ticks + ticksToWait;

    if (milliseconds < 10) { // 10 is just sweet number, 20ms is time quantum for thread scheduling 
        while(ticks < endTicks) {
            asm volatile("hlt");
        }
    } 
    else {
        while(ticks < endTicks) {
            os_yield();
        }
    }
}

uint32_t TimerDriver::getFrequency() const
{
    return this->frequency;
}

uint64_t TimerDriver::getTicks() const
{
    return this->ticks;
}

uint64_t TimerDriver::getUptimeMS() const
{
    if (frequency == 0) return 0;
    return (ticks * 1000) / frequency;
}

int TimerDriver::reset(){ return 0; }
void TimerDriver::deactivate(){}