#ifndef _OSOS_ESSENTIAL_KMULTITASKING_H
#define _OSOS_ESSENTIAL_KMULTITASKING_H

#include <cstdint>
#include <essential/kgdt.hpp>

namespace essential
{

/// @brief Represents the state of the CPU registers.
/// @details This structure matches the layout of registers pushed onto the stack 
/// by the interrupt handler (common/stub) before the C++ handler is called.
/// When switching tasks, we save the old task's state here and load the new task's state from here.
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
/// @details Each Task maintains its own private stack memory (4KB) and a pointer to its saved CPU state.
class Task {
    friend class TaskManager;
    private:
        /// @brief The dedicated kernel stack for this task.
        uint8_t stack[4096]; 
        
        /// @brief Pointer to the saved register state inside the stack.
        /// Used by the scheduler to restore this task's context.
        CPUState* cpustate;
        
        /// @brief The function to execute when the task starts.
        void (*m_entrypoint)(); 
        
        /// @brief The Code Segment selector to run this task in (Kernel vs User).
        uint16_t m_segmentSelector;

    public:
        /// @brief Constructs a new Task.
        /// @param gdt_manager Pointer to the GDT Manager to retrieve the correct Code Segment selector.
        /// @param entrypoint The function pointer where execution should begin.
        Task(GDT_Manager* gdt_manager, void (*entrypoint)()); 
        
        ~Task();

        /// @brief Resets or initializes the task's stack frame.
        /// @details This function sets up a "fake" interrupt stack frame at the top of the `stack` array.
        /// It sets the instruction pointer (EIP) to the entrypoint and the stack pointer (ESP) to the `onTaskExit` handler.
        /// This must be called before the task is run for the first time.
        void reset(); 
};

/// @brief Manages multitasking and scheduling of tasks.
/// @details Implements a simple Round-Robin scheduler to switch between registered tasks.
class TaskManager{
    private:
        /// @brief Array of pointers to all active tasks.
        Task* tasks[256];
        
        /// @brief The current number of active tasks.
        int numTasks;
        
        /// @brief The index of the currently running task in the `tasks` array.
        int currentTask;
        
        /// @brief Global pointer to the active TaskManager instance (used by onTaskExit).
        static TaskManager* activeTaskManager; 

    public:
        /// @brief Initializes the Task Manager.
        TaskManager();
        
        ~TaskManager();

        /// @brief Adds a task to the scheduling queue.
        /// @details Calls `reset()` on the task to prepare its stack.
        /// @param task Pointer to the Task object to add.
        /// @return true if the task was added successfully, false if the queue is full or task is already running.
        bool addTask(Task* task);

        /// @brief The core scheduling function called by the timer interrupt.
        /// @details Saves the current task's state (passed in `cpustate`) and selects the next task to run.
        /// Uses a Round-Robin algorithm with a time quantum.
        /// @param cpustate The register state of the task that was just interrupted.
        /// @return The register state of the next task to run (which the interrupt handler will restore).
        CPUState* schedule(CPUState* cpustate);

        /// @brief Removes the currently running task from the scheduling queue.
        /// @details This is typically called when a task finishes execution.
        void killCurrentTask();

        /// @brief A static helper function used as the return address for tasks.
        /// @details When a task function returns (finishes), the CPU pops this address from the stack.
        /// This function effectively calls `killCurrentTask()` and halts the CPU until the next scheduler tick.
        static void onTaskExit(); 
};

}
#endif