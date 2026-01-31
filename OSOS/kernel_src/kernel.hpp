#ifndef KERNEL_HPP
#define KERNEL_HPP

#include "basic/multiboot.h"
#include "essential/kgdt.hpp"
#include "essential/kicxxabi.hpp"
#include "essential/kmultitasking.hpp"
#include "essential/ktime.hpp"
#include "hardware_communication/kinterrupt.hpp"
#include "hardware_communication/kpci.hpp"
#include "driver/kdriver.hpp"
#include "driver/ktimer.hpp"
#include "driver/kkeyboard.hpp"
#include "driver/kmouse.hpp"
#include "driver/kserial.hpp"
#include <driver/kamd79c973.hpp>
#include <net/ketherframe.hpp>

#include "kshell.hpp"
#include "khandlers.hpp"

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

/// @brief function for shell task1 - prints o
/// @param arg necessory argument as per KThread rule
void task_o(void* arg);

/// @brief function for shell task2 - prints x
/// @param arg necessory argument as per KThread rule
void task_X(void* arg);

/// @brief function for shell task3 - send network packets
/// @param arg necessory argument as per KThread rule
void task_Net(void* arg);

/// @brief this is tail of kernelMain whose CPU state will be preserved
/// @param arg 
void kernelTail(void* arg);

/// @brief Entrypoint for C++ code. we will call this function from assembly
/// @param mbi multiboot information provided by grub
/// @param magicnumber magic number (just to varify)
extern "C" void kernelMain(multiboot_info_t *mbi, uint32_t magicnumber);

#endif