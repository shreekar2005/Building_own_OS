#include <essential/kmultitasking.hpp>

using namespace essential;

/// @brief Global pointer to the active TaskManager instance (used by onTaskExit).
static TaskManager* activeTaskManager = nullptr;

Task::Task(void (*entrypoint)(void*), void* arg)
{
    this->m_entrypoint = entrypoint;
    this->m_arg = arg;
    this->m_codeSegmentSelector = activeTaskManager->gdt_manager->kernel_CS_selector();
} 

Task::~Task(){}

void Task::reset()
{
    cpustate = (CPUState*) (stack + 4096 - sizeof(CPUState));

    cpustate->eax = 0; cpustate->ebx = 0; cpustate->ecx = 0; cpustate->edx = 0;
    cpustate->esi = 0; cpustate->edi = 0; cpustate->ebp = 0;
    cpustate->gs = 0x10;
    cpustate->fs = 0x10;
    cpustate->es = 0x10;
    cpustate->ds = 0x10;

    cpustate->error = 0;
    cpustate->eip = (uint32_t)m_entrypoint;
    cpustate->cs = m_codeSegmentSelector;
    cpustate->eflags = 0x202; 
    cpustate->esp = (uint32_t)TaskManager::onTaskExit;
    cpustate->ss  = (uint32_t)m_arg;
}

TaskManager::TaskManager(GDT_Manager *gdt_manager)
{
    this->gdt_manager=gdt_manager;
    numTasks = 0;
    currentTask = -1;
    activeTaskManager = this;
}

TaskManager::~TaskManager(){}

bool TaskManager::addTask(Task *task)
{
    // Prevent adding the same task object twice
    for(int i = 0; i < numTasks; i++) {
        if(tasks[i] == task) {
            return false; // Task is already running
        }
    }

    task->reset();
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

CPUState* TaskManager::schedule(CPUState* cpustate)
{
    const int TIME_QUANTUM = 4; 
    static int tick_counter = 0;

    if(numTasks <= 0) 
        return cpustate;

    if(currentTask >= 0)
        tasks[currentTask]->cpustate = cpustate;
    
    tick_counter++;
    
    if (tick_counter < TIME_QUANTUM) {
        return cpustate; 
    }

    tick_counter = 0;

    // Round Robin Logic
    if(++currentTask >= numTasks)
        currentTask = 0;
    
    return tasks[currentTask]->cpustate;
}