#include "concurrency/kmultitasking.hpp"

using namespace concurrency;

/// @brief Global pointer to the active KThreadManager instance (used by onThreadExit).
static KThreadManager* activeKThreadManager = nullptr;

KThread::KThread(void (*entrypoint)(void*), void* arg)
{
    this->m_entrypoint = entrypoint;
    this->m_arg = arg;
    this->m_codeSegmentSelector = activeKThreadManager->gdt_manager->kernel_CS_selector();
} 

KThread::~KThread(){}

bool KThread::start()
{
    return activeKThreadManager->addThread(this);
}

void KThread::reset()
{
    cpustate = (CPUState*) (stack + 4096 - sizeof(CPUState));
    // stack: the pointer to the start (lowest address) of your 4096-byte array.
    // + 4096: moves the pointer to the very top of stack
    // - sizeof(CPUState): reserves space at the top of the stack to hold our "fake" saved registers (defined in CPUState).

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
    cpustate->esp = (uint32_t)KThreadManager::onThreadExit;
    cpustate->ss  = (uint32_t)m_arg;
}

KThreadManager::KThreadManager(essential::GDT_Manager *gdt_manager)
{
    this->gdt_manager=gdt_manager;
    numThreads = 0;
    currentThread = -1;
    activeKThreadManager = this;
}

KThreadManager::~KThreadManager(){}

bool KThreadManager::addThread(KThread *thread)
{
    // Prevent adding the same thread object twice
    for(int i = 0; i < numThreads; i++) {
        if(threads[i] == thread) {
            return false; // KThread is already running
        }
    }

    thread->reset();
    if (numThreads >= 256)
        return false;
    threads[numThreads++] = thread;
    return true;
}

void KThreadManager::killCurrentThread()
{
    if (numThreads <= 0) return;
    threads[currentThread] = threads[numThreads - 1];
    numThreads--;
    currentThread = -1;
}

void KThreadManager::onThreadExit()
{
    if (activeKThreadManager != nullptr)
        activeKThreadManager->killCurrentThread();
    while(true)
        asm("hlt");
}

#include "basic/kiostream.hpp"
CPUState* KThreadManager::scheduleThreads(CPUState* cpustate, bool forceSwitch)
{
    const int TIME_QUANTUM = 20; 
    static int tick_counter = 0;

    if(numThreads <= 0) return cpustate;

    if(currentThread >= 0)
        threads[currentThread]->cpustate = cpustate;
    
    tick_counter++;

    if (tick_counter < TIME_QUANTUM && !forceSwitch) {
        return cpustate;
    }

    tick_counter = 0;

    // Round Robin Logic
    if(++currentThread >= numThreads)
        currentThread = 0;
    
    return threads[currentThread]->cpustate;
}