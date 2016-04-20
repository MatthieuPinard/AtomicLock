/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Created by: Matthieu Pinard, Ecole des Mines de Saint-Etienne, matthieu.pinard@etu.emse.fr
15-04-2016: Initial release
*/

#pragma once

/*!
* \file AtomicLock.h
* \brief Thread Synchronization capabilities using C++11 <atomic>
* \author Matthieu Pinard
*/
#include <atomic>
#include <thread>

const bool UNLOCKED = false, LOCKED = true;

/*! \class AtomicLock
* \brief Class for locking objects.
*
*  The class provides with lock, unlock, try_lock and wait capabilities.
*/
class AtomicLock {
private:
	std::atomic<bool> ThisLock; /*!< The atomic variable containing the Lock state (LOCKED or UNLOCKED) */
public:
	/*!
	*  \brief Acquire the AtomicLock.
	*
	*  This methods spins while the AtomicLock is acquired by another thread.
	*  As soon as it is released, the lock() function tries to acquire it.
	*
	*/
	inline void lock();
	/*!
	*  \brief Immediatly release the AtomicLock.
	*
	*  This methods release the AtomicLock by atomically storing UNLOCKED.
	*/
	inline void unlock();
	/*!
	*  \brief Wait for the AtomicLock to be released.
	*
	*  This method spins while the AtomicLock is acquired by a thread.
	*/
	inline void wait();
	/*!
	*  \brief Try acquiring the AtomicLock.
	*
	*  This method does an unique check on the AtomicLock : if
	*  it is unlocked, it tries to acquire it using CAS.
	*
	*  \return true if the function has acquired the AtomicLock,
	*  false otherwise. (AtomicLock already acquired, or if another thread
	*  has locked it during the call)
	*/
	inline bool try_lock();
	/*!
	*  \brief AtomicLock constructor.
	*
	*  The contructor initializes the AtomicLock as freed.
	*/
	AtomicLock() : ThisLock(UNLOCKED) {}
	/*!
	*  \brief AtomicLock destructor.
	*
	*  The destructor unlocks the AtomicLock.
	*/
	~AtomicLock() {
		unlock();
	}
};

inline bool AtomicLock::try_lock() {
	auto _Unlocked = UNLOCKED;
	// First check the Lock status before trying to acquire it using a CAS operation.
	return (!ThisLock.load(std::memory_order::memory_order_acquire) &&
		ThisLock.compare_exchange_strong(_Unlocked, LOCKED, std::memory_order::memory_order_acquire, std::memory_order::memory_order_relaxed));
}

inline void AtomicLock::wait() {
	// Spin atomically while the Lock is acquired.
	while (ThisLock.load(std::memory_order::memory_order_acquire)) {
		// Wait by issuing a yield() call
		std::this_thread::yield();
	}
}

inline void AtomicLock::lock() {
	do {
		// Spin on atomic Load, and if the lock is free...
		if (!ThisLock.load(std::memory_order::memory_order_acquire)) {
			auto _Unlocked = UNLOCKED;
			// Try to lock it using a CAS operation...
			if (ThisLock.compare_exchange_strong(_Unlocked, LOCKED, std::memory_order::memory_order_acquire, std::memory_order::memory_order_relaxed)) {
				return;
			}
		}
		// Wait by issuing a yield() call
		std::this_thread::yield();
	} while (true);
}

inline void AtomicLock::unlock() {
	// Simply unlock by storing the value atomically.
	ThisLock.store(UNLOCKED, std::memory_order::memory_order_release);
}