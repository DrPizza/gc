#pragma once

#include <atomic>
#include <new>

#if defined(ATOMIC_BOOL_LOCK_FREE) && ATOMIC_BOOL_LOCK_FREE

struct /*alignas(std::hardware_destructive_interference_size)*/ gc_spinlock {
	gc_spinlock() : locked(false) {
	}

	bool try_lock() {
		bool state = false;
		return locked.compare_exchange_weak(state, true, std::memory_order_acquire);
	}

	void lock() {
		for(;;) {
			if(try_lock()) {
				break;
			}
			while(locked.load(std::memory_order_relaxed) == true) {
				std::this_thread::yield();
			}
		}
	}

	void unlock() {
		locked.store(false, std::memory_order_release);
	}

	gc_spinlock(const gc_spinlock&) = delete;
	gc_spinlock& operator=(const gc_spinlock&) = delete;

private:
	std::atomic<bool> locked;
};

#else

struct gc_spinlock {
	bool try_lock() {
		return !flag.test_and_set(std::memory_order_acquire);
	}

	void lock() {
		for(; !try_lock();) {
			std::this_thread::yield();
		}
	}
	void unlock() {
		flag.clear(std::memory_order_release);
	}
private:
	std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

#endif

struct gc_lock {
	gc_lock(gc_spinlock& spinlock_) : spinlock(spinlock_) {
		spinlock.lock();
	}

	~gc_lock() {
		spinlock.unlock();
	}

	gc_lock(const gc_lock&) = delete;
	gc_lock& operator=(const gc_lock&) = delete;

private:
	gc_spinlock& spinlock;
};
