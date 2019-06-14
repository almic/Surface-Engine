#pragma once

#include "Surface/Core.h"
#include "Sync.h"

#include "spch.h"

#include <atomic>

namespace Surface {
namespace Thread {

	/*
	Thread::Worker is designed to work only with Thread::Pool.

	To learn more about Thread::Pool, check out its header file.

	To learn how to implement Worker, just read the comments here.
	*/

	class Pool;

	class Worker
	{
		friend Pool;
	public:
		Worker() = delete;
		Worker(const std::string& name, const Conditional& cond);

	private:
		// Unique identifier for the thread, useful for debugging
		std::string name;

		// The Conditional this Worker uses for doing things
		Conditional cond;

		// The thread on which the Worker is running
		std::thread* thread;

		// Running status of the Worker, if it is false then the Worker is scheduled to finish at some point in the future
		std::atomic_bool running = false;

		// Pause status of the Worker, true if explicitly told to sleep in the future or is currently sleeping,
		// false if it will eventually wake up or is currently running
		std::atomic_bool paused = false;

		// If the Worker is completely done executing and is waiting to be joined.
		std::atomic_bool finished = false;

	public:
		~Worker() {};

		// Sleep the current worker, calling OnPause() in a THREAD UNSAFE manner, the worker must lock its Conditional if needed.
		// Returns "true" if the worker was not previously paused and OnPause() was called.
		virtual bool Pause() final;

		// Un-pauses the current worker, calling OnResume() in a THREAD UNSAFE manner, the worker must lock its Conditional if needed.
		// This will not wake up the worker thread itself, the main thread MUST call "notify_all" on the Conditional to ensure workers resume.
		// Returns "true" if the worker was previously paused and OnResume() was called.
		virtual bool Resume() final;

		// Marks the worker as "finished" and unpauses if it was previously paused, although the worker may still be sleeping.
		// The main thread MUST call "notify_all" on the Conditional to ensure workers finish properly.
		// Once the worker wakes up, it will call OnFinish() before completely exiting.
		virtual void Finish() final;

		// True if the worker will pause in the future or is already sleeping, note that the worker could still be inside OnUpdate() however.
		virtual bool IsPaused() final;

		// True if the worker is running, False if the worker will eventually finish in the future or has already finished, not that the worker could
		// still be inside OnUpdate() or OnFinish() however.
		virtual bool IsRunning() final;

		// True if the worker has completely finished.
		virtual bool Joinable() final;

		// Joins the worker thread, it is an error to call this if Joinable() returns false!
		virtual void Join() final;

	private:
		// INTERNAL USE ONLY! The main runloop for the Worker, which checks some things and calls OnUpdate()
		virtual void Run() final;

		// INTERNAL USE ONLY! The basic lifetime of a Worker, calls OnSpawn(), then Run(), then OnFinish()
		virtual void Spawn() final;

		// Worker Thread! UNSAFE! Allows implementations to do some extra work just before they start running. Conditional would need to be locked if required, and UNLOCKED before returning.
		virtual inline void OnSpawn() noexcept = 0;

		// Worker Thread! UNSAFE! The last code which executes before Worker terminates. Conditional would need to be locked if required, and UNLOCKED before returning.
		virtual inline void OnFinish() noexcept = 0;

		// Worker Thread! SAFE. Do the work, lock will always be held, and can be unlocked early if needed.
		virtual inline void OnUpdate(Conditional& lock) noexcept = 0;

		// Worker Thread! SAFE. Return true if there is work to be done, MUST NOT attempt to lock or unlock the Conditional!
		virtual inline bool ShouldWake() noexcept = 0;

		// MAIN THREAD! UNSAFE! Code which runs on the main thread ONE TIME when worker is initially paused. Conditional would need to be locked if required, and UNLOCKED before returning.
		virtual inline void OnPause() noexcept = 0;

		// MAIN THREAD! UNSAFE! Code which runs on the main thread ONE TIME when worker is resumed after being paused. Conditional would need to be locked if required, and UNLOCKED before returning.
		virtual inline void OnResume() noexcept = 0;

	};

}
}
