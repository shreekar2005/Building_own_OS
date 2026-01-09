#ifndef KERNEL_HPP
#define KERNEL_HPP

#include "basic/multiboot.h"
#include "essential/kgdt.hpp"
#include "essential/kicxxabi.hpp"
#include "essential/kmultitasking.hpp"
#include "hardware_communication/kinterrupt.hpp"
#include "hardware_communication/kpci.hpp"
#include "driver/kdriver.hpp"
#include "driver/ktimer.hpp"
#include "driver/kkeyboard.hpp"
#include "driver/kmouse.hpp"
#include "driver/kserial.hpp"

// Include the separated modules
#include "kshell.hpp"
#include "khandlers.hpp"
#include "essential/ktime.hpp" // Include the new Time Library

struct KernelArgs {
    essential::GDT_Manager* gdtManager;
    hardware_communication::InterruptManager* interruptManager;
    driver::DriverManager* driverManager;
    driver::TimerDriver* timer;
    driver::KeyboardDriver* keyboard;
    driver::MouseDriver* mouse;
    driver::SerialDriver* serial;
    KernelShell* shell;
    multiboot_info_t* mbi;
};

#include "kernel.hpp"

/// @brief function for shell task1
/// @param arg necessory argument as per KThread rule
void task_o(void* arg);

/// @brief function for shell task2
/// @param arg necessory argument as per KThread rule
void task_x(void* arg);

/// @brief This is tail of kernelMain whose CPU state will be preserved
/// @param arg 
void kernelTail(void* arg);

/// @brief Entrypoint for C++ code. we will call this function from assembly
/// @param mbi multiboot information provided by grub
/// @param magicnumber magic number (just to varify)
extern "C" void kernelMain(multiboot_info_t *mbi, uint32_t magicnumber);

#endif