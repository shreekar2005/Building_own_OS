#ifndef _OSOS_ESSENTIAL_KMULTITASKING_H
#define _OSOS_ESSENTIAL_KMULTITASKING_H

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


class Task {
    friend class TaskManager;
    private:
        uint8_t stack[4096]; 
        CPUState* cpustate;
        void (*m_entrypoint)(); 
        uint16_t m_segmentSelector;

    public:
        Task(GDT_Manager* gdt_manager, void (*entrypoint)()); 
        ~Task();
        void reset(); 
};

class TaskManager{
    private:
        Task* tasks[256];
        int numTasks;
        int currentTask;
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