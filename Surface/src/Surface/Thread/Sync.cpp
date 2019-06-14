#include "spch.h"
#include "Sync.h"

namespace Surface {
namespace Thread {

	Conditional::Conditional()
	{
		mu = std::make_shared<std::mutex>(new std::mutex());
		cv = std::make_shared<std::condition_variable>(new std::condition_variable());
		un = std::unique_lock<std::mutex>(*mu, std::defer_lock);
	}

	Conditional::~Conditional()
	{
		// thankfully everything will unlock automatically for us because we
		// always create new unique locks for new objects
	}

	Conditional::Conditional(const Conditional& other)
	{
		mu = other.mu;
		cv = other.cv;
		// do not copy the unique lock
		un = std::unique_lock<std::mutex>(*mu, std::defer_lock);
	}

	void Conditional::notify_one() noexcept
	{
		cv->notify_one();
	}

	void Conditional::notify_all() noexcept
	{
		cv->notify_all();
	}

	void Conditional::wait()
	{
		un.lock();
		cv->wait(un);
	}

	template<class _Predicate>
	void Conditional::wait(_Predicate _Pred)
	{
		un.lock();
		cv->wait(un, _Pred);
	}

	Conditional Conditional::wait_scoped()
	{
		auto cond = *this;
		cond.un.lock();
		cond.cv->wait(un);
		return cond;
	}

	template<class _Predicate>
	Conditional Conditional::wait_scoped(_Predicate _Pred)
	{
		auto cond = *this;
		cond.un.lock();
		cond.cv->wait(un, _Pred);
		return cond;
	}

	template<class _Rep, class _Period>
	std::cv_status Conditional::wait_for(const std::chrono::duration<_Rep, _Period>& _Rel_time)
	{
		un.lock();
		return cv->wait_for(un, _Rel_time);
	}

	template<class _Rep, class _Period, class _Predicate>
	bool Conditional::wait_for(const std::chrono::duration<_Rep, _Period>& _Rel_time, _Predicate _Pred)
	{
		un.lock();
		return cv->wait_for(un, _Rel_time, _Pred);
	}

	template<class _Clock, class _Duration>
	std::cv_status Conditional::wait_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time)
	{
		un.lock();
		return cv->wait_until(un, _Abs_time);
	}

	template<class _Clock, class _Duration, class _Predicate>
	bool Conditional::wait_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time, _Predicate _Pred)
	{
		un.lock();
		return cv->wait_until(un, _Abs_time, _Pred);
	}

	std::cv_status Conditional::wait_until(const xtime *_Abs_time)
	{
		un.lock();
		return cv->wait_until(un, _Abs_time);
	}

	template<class _Predicate>
	bool Conditional::wait_until(const xtime *_Abs_time, _Predicate _Pred)
	{
		un.lock();
		return cv->wait_until(un, _Abs_time, _Pred);
	}

	void Conditional::lock()
	{
		// noexcept prevents this from throwing if already locked by this
		un.lock();
	}

	_NODISCARD bool Conditional::try_lock()
	{
		if (!un.owns_lock())
			return un.try_lock();
		return true;
	}

	template<class _Rep, class _Period>
	_NODISCARD bool Conditional::try_lock_for(const std::chrono::duration<_Rep, _Period>& _Rel_time)
	{
		return un.try_lock_for(_Rel_time);
	}

	template<class _Clock, class _Duration>
	_NODISCARD bool Conditional::try_lock_until(const std::chrono::time_point<_Clock, _Duration>& _Abs_time)
	{
		return un.try_lock_until(_Abs_time);
	}

	_NODISCARD bool Conditional::try_lock_until(const xtime *_Abs_time)
	{
		return un.try_lock_until(_Abs_time);
	}

	void Conditional::unlock()
	{
		// noexcept prevents this from throwing if not already locked by this
		un.unlock();
	}

	bool Conditional::try_unlock()
	{
		if (un.owns_lock())
		{
			un.unlock();
			return true;
		}
		return false;
	}

	void Conditional::unlock_notify()
	{
		unlock();
		cv->notify_one();
	}

	bool Conditional::try_unlock_notify()
	{
		bool r = try_unlock();
		cv->notify_one();
		return r;
	}

	void Conditional::unlock_notify_all()
	{
		unlock();
		cv->notify_all();
	}

	bool Conditional::try_unlock_notify_all()
	{
		bool r = try_unlock();
		cv->notify_all();
		return r;
	}

	_NODISCARD bool Conditional::owns_lock() const noexcept
	{
		return un.owns_lock();
	}

	explicit Conditional::operator bool() const noexcept
	{
		return un.owns_lock();
	}

}
}
