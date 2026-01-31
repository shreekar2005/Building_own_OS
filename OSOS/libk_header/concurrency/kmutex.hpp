#ifndef _OSOS_BASIC_KMUTEX_H
#define _OSOS_BASIC_KMUTEX_H

#include "basic/ktypes.hpp"

namespace basic {

    void os_yield() {
        asm volatile("int $0x80");// 0x80 is chosen arbitrarily as our "System Call" vector.
    }

    class Mutex {
    private:
        volatile int _locked; // 1 = locked, 0 = unlocked

    public:
        Mutex() {
            _locked = 0;
        }

        // locks the mutex (yields CPU if busy)
        void lock() {
            // we do NOT disable interrupts here because we might sleep/yield
            
            // atomic check: if locked, yield the CPU to another thread
            while (__sync_lock_test_and_set(&_locked, 1)) {
                os_yield(); // this will force to schedule thread
                asm volatile("pause");
            }
        }

        // unlocks the mutex
        void unlock() {
            __sync_lock_release(&_locked);
        }
    };
}

#endif