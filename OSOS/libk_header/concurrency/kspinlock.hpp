#ifndef _OSOS_BASIC_KSPINLOCK_H
#define _OSOS_BASIC_KSPINLOCK_H

#include "basic/ktypes.hpp"

namespace basic {

    class Spinlock {
    private:
        volatile int _locked; // 1 = locked, 0 = unlocked
        
        // we need to save the interrupt state (eflags) because spinlocks disable interrupts
        // to prevent the scheduler from switching tasks while we hold the lock
        uint32_t _saved_eflags;

    public:
        Spinlock() {
            _locked = 0;
            _saved_eflags = 0;
        }

        // locks the spinlock (disables interrupts)
        void lock() {
            // save current interrupt state (EFLAGS) and Disable Interrupts
            uint32_t eflags;
            asm volatile("pushf; pop %0; cli" : "=r"(eflags));

            // atomic check: if locked, spin (burn CPU) until free
            while (__sync_lock_test_and_set(&_locked, 1)) {
                asm volatile("pause"); // optimize the spin-loop by slowing down loop speed (to reduce heating)
            }
            
            // store the flags only after we successfully acquired the lock
            _saved_eflags = eflags;
        }

        // unlocks the spinlock (restores interrupts)
        void unlock() {
            // release the lock atomically
            __sync_lock_release(&_locked);

            // restore Interrupts: if they were ON before lock(), turn them ON.
            if (_saved_eflags & 0x200) {
                asm volatile("sti");
            }
        }
    };
}

#endif