#ifndef _OSOS_ESSENTIAL_MULTITASKING_H
#define _OSOS_ESSENTIAL_MULTITASKING_H

#include <cstdint>
#include <essential/kgdt.hpp>

namespace essential
{

struct CPUState
{
    uint32_t eax, ebx, ecx, edx, esi, edi, ebp;
    uint32_t gs, fs, es, ds;
    uint32_t error;
    uint32_t eip, cs, eflags, esp, ss;
}__attribute__((packed));


class Task{
    friend class TaskManager;
    private:
        uint8_t stack[4096]; 
        CPUState* cpustate;
    public:
        Task(GDT_Manager* gdt_manager, void entrypoint());
        ~Task();
};

class TaskManager{
    private:
        Task* tasks[256];
        int numTasks;
        int currentTask;
        
        // Static pointer to the active manager (so the exit handler can find it)
        static TaskManager* activeTaskManager; 

    public:
        TaskManager();
        ~TaskManager();
        bool addTask(Task* task);
        CPUState* schedule(CPUState* cpustate);
        void killCurrentTask();
        static void onTaskExit(); 
};

}
#endif