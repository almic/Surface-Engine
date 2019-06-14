#pragma once

#include "Surface/Core.h"

#include "spch.h"

namespace Surface {
namespace Thread {

	/*
	Thread::Pool is specialized for running threads that last throughout the
	life-time of the Application. The class Worker only functions with Pool to
	ensure this strict coding style. Workers should be treated with care as
	they truely represent code that executes on other threads.

	Please do not try to circumvent these strict limitations, instead use other
	methods if you cannot easily create a Worker to do what you need.
	*/

	class Worker;

	class Pool
	{
	private:
		Pool() {}
		~Pool() {}

	public:
		static std::vector<Worker*> workers;
		static std::thread::id main_thread;
		static unsigned int worker_count;
		static bool started;

		// Adds workers to the pool and spawns them in new threads
		static void StartWorkers(const std::vector<Worker*>& workers);

		// Calls Finish() on all workers and joins their threads, you MUST call "notify_all" on the conditionals for the workers!
		static void JoinWorkers();

	private:
		static void AddWorker(Worker* worker);
	};

}
}
