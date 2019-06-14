#include "spch.h"
#include "Worker.h"

#include "Pool.h"

namespace Surface {
namespace Thread {

	Worker::Worker(const std::string& name, const Conditional& cond)
		: name(name), cond(cond)
	{ }

	void Worker::Run() // Inside worker thread
	{
		// Must only be called from this worker's thread
		if (thread == nullptr || thread->get_id() != std::this_thread::get_id())
		{
			CORE_ASSERT(false, "Worker::Run() on worker named \"" + name + "\" must not be called from outside its own thread!");
			return;
		}

		// block duplicate run calls
		if (running.exchange(true))
		{
			CORE_ASSERT(false, "Duplicate call to worker \"" + name + "\" while it was already running!");
			return;
		}

		// Allow workers to be initially paused
		if (paused)
			cond.wait_scoped([this]() { return !paused; });

		while (running)
		{
			auto lock = cond.wait_scoped([this]() { return !running || (!paused && ShouldWake()); });

			if (!running) return;
			OnUpdate(lock);
		}
	}

	void Worker::Spawn() // Inside worker thread
	{
		OnSpawn();
		Run();
		OnFinish();
		finished = true;
	}

	void Worker::Finish()
	{
		// this should eventually stop the run loop, and exit the thread
		paused = false;
		running = false;
	}

	bool Worker::Pause()
	{
		// workers can only be paused from the main thread!
		if (std::this_thread::get_id() != Pool::main_thread)
		{
			CORE_ASSERT(false, "Worker \"" + name + "\" can only be paused from the main thread!");
			return;
		}

		if (!paused.exchange(true)) // exchange returns previous value (false if not already paused)
		{
			OnPause();
			return true;
		}
		return false;
	}

	bool Worker::Resume()
	{
		// workers can only be resumed from the main thread!
		if (std::this_thread::get_id() != Pool::main_thread)
		{
			CORE_ASSERT(false, "Worker \"" + name + "\" can only be resumed from the main thread!");
			return;
		}

		if (paused.exchange(false)) // exchange returns previous value (true if currently paused)
		{
			OnResume();
			return true;
		}
		return false;
	}

	bool Worker::IsPaused()
	{
		return paused;
	}

	bool Worker::IsRunning()
	{
		return !finished && running;
	}

	bool Worker::Joinable()
	{
		// this can only be called from the main thread!
		if (std::this_thread::get_id() != Pool::main_thread)
		{
			CORE_ASSERT(false, "Can only call Joinable() on Worker \"" + name + "\" from the main thread!");
			return;
		}

		return finished;
	}

	void Worker::Join()
	{
		// this can only be called from the main thread!
		if (std::this_thread::get_id() != Pool::main_thread)
		{
			CORE_ASSERT(false, "Can only call Join() on Worker \"" + name + "\" from the main thread!");
			return;
		}

		// it is an error to call Join if the worker hasn't finished!
		if (!finished)
		{
			CORE_ASSERT(false, "Cannot Join() Worker \"" + name + "\" because it hasn't finished yet!");
			return;
		}

		thread->join();
	}

}
}
