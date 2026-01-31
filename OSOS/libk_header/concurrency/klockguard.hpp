#ifndef _OSOS_BASIC_KLOCKGUARD_H
#define _OSOS_BASIC_KLOCKGUARD_H

namespace basic {

    // template allows this to work for both Spinlock and Mutex
    template <typename LockType>
    class LockGuard {
    private:
        LockType& _lock; // reference to the lock we are guarding

    public:
        // acquires the lock immediately upon creation
        LockGuard(LockType& lock) : _lock(lock) {
            _lock.lock();
        }

        // releases the lock automatically when this object goes out of scope
        ~LockGuard() {
            _lock.unlock();
        }
    };
}

#endif