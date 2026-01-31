#ifndef _OSOS_CONCURRENCY_KMULTITASKING_H
#define _OSOS_CONCURRENCY_KMULTITASKING_H

#include "basic/ktypes.hpp"
#include "essential/kgdt.hpp"

namespace essential
{

/// @brief Represents the state of the CPU registers.
/// @details This structure matches the layout of registers pushed onto the stack 
/// by the interrupt handler (common/stub) before the C++ handler is called.
/// When switching threads, we save the old thread's state here and load the new thread's state from here.
struct CPUState
{
    // General purpose registers pushed by `pusha` (mostly)
    uint32_t eax, ebx, ecx, edx, esi, edi, ebp;
    
    // Segment selectors
    uint32_t gs, fs, es, ds;
    
    // Error code / interrupt number (pushed manually by the interrupt stub)
    uint32_t error;
    
    // Pushed automatically by the CPU upon interrupt/exception
    uint32_t eip, cs, eflags, esp, ss;
}__attribute__((packed));


/// @brief Represents a single unit of execution (a thread or process) in the OS. 
/// @details Each KThread maintains its own private stack memory (4KB) and a pointer to its saved CPU state.
class KThread {
    friend class KThreadManager;
    private:
        /// @brief The dedicated kernel stack for this thread.
        uint8_t stack[4096]; 
        
        /// @brief Pointer to the saved register state inside the stack.
        /// Used by the scheduler to restore this thread's context.
        CPUState* cpustate;
        
        /// @brief The function to execute when the thread starts.
        void (*m_entrypoint)(void*); 

        /// @brief this is argument pointer for entrypoint.
        void *m_arg; 
        
        /// @brief The Code Segment selector to run this thread in (Kernel vs User).
        uint16_t m_codeSegmentSelector;

    public:
        /// @brief Constructs a new KThread.
        /// @param entrypoint The function pointer where execution should begin.
        /// @param arg Void* argument pointer for entrypoint
        KThread(void (*entrypoint)(void*), void* arg); 
        
        ~KThread();

        /// @brief Resets or initializes the thread's stack frame.
        /// @details This function sets up a "fake" interrupt stack frame at the top of the `stack` array.
        /// It sets the instruction pointer (EIP) to the entrypoint and the stack pointer (ESP) to the `onTaskExit` handler.
        /// This must be called before the thread is run for the first time.
        void reset(); 
};

/// @brief Manages multitasking and scheduling of threads.
/// @details Implements a simple Round-Robin scheduler to switch between registered threads.
class KThreadManager{
    private:
        /// @brief Array of pointers to all active threads.
        KThread* threads[256];
        
        /// @brief The current number of active threads.
        int numThreads;
        
        /// @brief The index of the currently running thread in the `threads` array.
        int currentThread;

    public:
        GDT_Manager *gdt_manager;

        /// @brief Initializes the KThread Manager.
        KThreadManager(GDT_Manager *gdt_manager);
        
        ~KThreadManager();

        /// @brief Adds a kernel thread to the scheduling queue.
        /// @details Calls `reset()` on the thread to prepare its stack.
        /// @param thread Pointer to the KThread object to add.
        /// @return true if the thread was added successfully, false if the queue is full or thread is already running.
        bool addThread(KThread* thread);

        /// @brief The core scheduling function called by the timer interrupt.
        /// @details Saves the current thread's state (passed in `cpustate`) and selects the next thread to run.
        /// Uses a Round-Robin algorithm with a time quantum.
        /// @param cpustate The register state of the thread that was just interrupted.
        /// @param forceSwitch if true then just schedule thread without looking at time quantum
        /// @return The register state of the next thread to run (which the interrupt handler will restore).
        CPUState* scheduleThreads(CPUState* cpustate, bool forceSwitch = false);

        /// @brief Removes the currently running thread from the scheduling queue.
        /// @details This is typically called when a thread finishes execution.
        void killCurrentThread();

        /// @brief A static helper function used as the return address for threads.
        /// @details When a thread function returns (finishes), the CPU pops this address from the stack.
        /// This function effectively calls `killCurrentThread()` and halts the CPU until the next scheduler tick.
        static void onThreadExit(); 
};

}
#endif