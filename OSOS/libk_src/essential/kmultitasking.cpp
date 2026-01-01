#include <essential/kmultitasking.hpp>
#include "basic/kiostream.hpp" // For printf if needed

using namespace essential;

TaskManager* TaskManager::activeTaskManager = nullptr;


Task::Task(GDT_Manager *gdt_manager, void entrypoint()){
    // 1. Allocate CPUState normally (No need for the extra -4 offset)
    cpustate = (CPUState*) (stack + 4096 - sizeof(CPUState));

    // 2. Standard Initialization
    cpustate->eax = 0;
    cpustate->ebx = 0;
    cpustate->ecx = 0;
    cpustate->edx = 0;
    cpustate->esi = 0;
    cpustate->edi = 0;
    cpustate->ebp = 0;
    
    // If you uncommented gs/fs/es/ds in struct, set them to 0 or Kernel Data Segment (0x10)
    // cpustate->gs = 0; ... 
    
    cpustate->error = 0; 

    // 3. Instruction Pointer Setup
    cpustate->eip = (uint32_t)entrypoint;
    cpustate->cs = gdt_manager->kernel_CS_selector();
    cpustate->eflags = 0x202;
    
    // 4. THE FIX: Set the Return Address here!
    // Since we are in Ring 0, 'iret' leaves ESP pointing to this field.
    // When 'entrypoint' returns, 'ret' will pop this value into EIP.
    cpustate->esp = (uint32_t)TaskManager::onTaskExit;
    
    // cpustate->ss is irrelevant for Ring 0, it just sits above the return address.
}

Task::~Task(){}

TaskManager::TaskManager()
{
    numTasks = 0;
    currentTask = -1;
    activeTaskManager = this;
}

TaskManager::~TaskManager(){}

bool TaskManager::addTask(Task *task)
{
    if (numTasks >= 256)
        return false;
    tasks[numTasks++] = task;
    return true;
}

void TaskManager::killCurrentTask()
{
    if (numTasks <= 0) return;
    tasks[currentTask] = tasks[numTasks - 1];
    numTasks--;
    currentTask = -1; 
    
    basic::printf("\nTask Finished. Remaining: %d\n", numTasks);
}

void TaskManager::onTaskExit()
{
    if (activeTaskManager != nullptr)
        activeTaskManager->killCurrentTask();
    while(true)
        asm("hlt");
}

CPUState *TaskManager::schedule(CPUState *cpustate)
{
    if(numTasks <= 0) 
        return cpustate;
    
    // Save state of current task
    if(currentTask >= 0)
        tasks[currentTask]->cpustate = cpustate;
    
    // Pick next task (Round Robin)
    if(++currentTask >= numTasks)
        currentTask = 0; // Wrap around to 0
    
    return tasks[currentTask]->cpustate;
}
