/*

	Special lockable object with magic powers.

	Conditionals act exactly like a unique_lock and condition_variable in one object.

	Conditionals are designed to be copied and shared across threads and objects, such that copied Conditionals share
	the same underlying mutexes and condition_variable but act just like brand new unique_locks which automatically
	release the mutex on destruction.

	Basically, imagine a unique_lock that you can copy, which has all the same functionality as a condition_variable!

	You can even use std::scoped_lock(Conditional a, Conditional b, Conditional c)!

*/

#pragma once

#include "spch.h"

namespace Surface {
namespace Thread {

/*

	Use example, single conditional:

	// use this between threads, pass to workers
	auto main = new Conditional();

	// then for critical section
	{ auto cd = main->wait_scoped();
		...
		...
		cd.unlock();
		...
	} // cd will always be unlocked here once it goes out of scope
	
===============================================================================

	Use example, multiple conditionals with scoped_lock:

	auto a = Conditional();
	auto b = Conditional();
	auto c = Conditional();

	// then for critical section
	{ scoped_lock lock(a, b, c);
		...
		...
	} // all Conditionals released here

*/

	class Conditional
	{
	private:
		std::shared_ptr<std::mutex> mu;

		std::shared_ptr<std::condition_variable> cv;

		std::unique_lock<std::mutex> un;

	public:

		Conditional();

		~Conditional();

		Conditional(const Conditional& other);

		// no, this is not allowed, only copy construction!
		Conditional& operator=(const Conditional&) = delete;

		// Wake up one thread waiting on this Conditional
		void notify_one() noexcept;

		// Wake up all threads waiting on this Conditional
		void notify_all() noexcept;

		// Sleep thread until it is notified from another thread via 'notify_one' or 'notify_all', or by spurious wake
		void wait();

		// Sleep thread while _Pred returns false, and only wakes when 'notify_one' or 'notify_all' is called, or by spurious wake
		template<class _Predicate>
		void wait(_Predicate _Pred);

		/************************************
		* IMPORTANT:
		* I considered using _NODISCARD on these "wait_scoped" functions, but discarding the scoped lock instantly has its uses!
		************************************/

		// Create a new identical Conditional for scoped contexts, useful for scoped destruction and automatic unlocking. See 'wait'.
		Conditional wait_scoped();

		// Create a new identical Conditional for scoped contexts, which sleeps the thread while _Pred returns false. See 'wait'.
		template<class _Predicate>
		Conditional wait_scoped(_Predicate _Pred);

		// Thread sleeps for duration. Returns cv_status::timeout if the duration elapsed, or cv_status::no_timeout if the thread was woken early.
		template<class _Rep, class _Period>
		std::cv_status wait_for(const std::chrono::duration<_Rep, _Period>& _Rel_time);

		// Thread sleeps for duration. Returns false only if the time elapsed and _Pred was still false, otherwise true.
		template<class _Rep, class _Period, class _Predicate>
		bool wait_for(const std::chrono::duration<_Rep, _Period>& _Rel_time, _Predicate _Pred);

		// Thread sleeps until future time is reached. Returns cv_status::timeout if the time was reached, or cv_status::no_timeout if the thread was woken early.
		template<class _Clock, class _Duration>
		std::cv_status wait_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time);

		// Thread sleeps until future time is reached. Returns false only if the time was reached and _Pred was still false, otherwise true.
		template<class _Clock, class _Duration, class _Predicate>
		bool wait_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time, _Predicate _Pred);

		// Thread sleeps until future time is reached. Returns cv_status::timeout if the duration elapsed, or cv_status::no_timeout if the thread was woken early.
		std::cv_status wait_until(const xtime *_Abs_time);

		// Thread sleeps until future time is reached. Returns false only if the time elapsed and _Pred was still false, otherwise true.
		template<class _Predicate>
		bool wait_until(const xtime *_Abs_time, _Predicate _Pred);

		// Stops execution until Conditional lock is aquired.
		void lock() noexcept;

		// Attempts to lock Conditional without blocking, returning true if lock was aquired, false otherwise.
		_NODISCARD bool try_lock() noexcept;

		// Attempts to lock Conditional for duration, returning true if lock was aquired, false otherwise.
		template<class _Rep, class _Period>
		_NODISCARD bool try_lock_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) noexcept;

		// Attempts to lock Conditional until future time, returning true if lock was aquired, false otherwise.
		template<class _Clock, class _Duration>
		_NODISCARD bool try_lock_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time) noexcept;

		// Attempts to lock Conditional until future time, returning true if lock was aquired, false otherwise.
		_NODISCARD bool try_lock_until(const xtime *_Abs_time) noexcept;

		// Unlock the Conditional, see 'unlock_notify' or 'unlock_notify_all' to also wake sleeping threads.
		void unlock() noexcept;
		
		// Attempts to unlock Conditional, returning true if it was previously locked by this thread, false otherwise.
		bool try_unlock() noexcept;

		// Unlock the Condtional and wake up one sleeping thread.
		void unlock_notify() noexcept;

		// Attempts to unlock Conditional and wake up one sleeping thread, returning true if it was previously locked by this thread, false otherwise.
		bool try_unlock_notify() noexcept;

		// Unlock the Condtional and wake up all sleeping threads.
		void unlock_notify_all() noexcept;

		// Attempts to unlock Conditional and wake up all sleeping threads, returning true if it was previously locked by this thread, false otherwise.
		bool try_unlock_notify_all() noexcept;

		// Returns if this particular Conditional owns the lock.
		_NODISCARD bool owns_lock() const noexcept;

		explicit operator bool() const noexcept;
	};

}
}
