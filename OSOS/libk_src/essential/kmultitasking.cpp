#include <essential/kmultitasking.hpp>

using namespace essential;

TaskManager* TaskManager::activeTaskManager = nullptr;

Task::Task(GDT_Manager *gdt_manager, void (*entrypoint)()){
    this->m_entrypoint = entrypoint;
    this->m_segmentSelector = gdt_manager->kernel_CS_selector();
    reset(); 
} 

Task::~Task(){}

void Task::reset()
{
    cpustate = (CPUState*) (stack + 4096 - sizeof(CPUState));

    cpustate->eax = 0; cpustate->ebx = 0; cpustate->ecx = 0; cpustate->edx = 0;
    cpustate->esi = 0; cpustate->edi = 0; cpustate->ebp = 0;
    
    cpustate->error = 0;
    cpustate->eip = (uint32_t)m_entrypoint;
    
    cpustate->cs = m_segmentSelector;
    cpustate->eflags = 0x202; 
    cpustate->esp = (uint32_t)TaskManager::onTaskExit;
}

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
    if(currentTask >= 0)
        tasks[currentTask]->cpustate = cpustate;
    
    // Round Robin
    if(++currentTask >= numTasks)
        currentTask = 0;
    
    return tasks[currentTask]->cpustate;
}
