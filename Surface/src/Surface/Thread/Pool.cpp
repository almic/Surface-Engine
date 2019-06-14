#include "spch.h"
#include "Pool.h"

#include "Worker.h"
#include "Surface/Application.h"

namespace Surface {
namespace Thread {

	std::vector<Worker*> Pool::workers = {};
	std::thread::id Pool::main_thread = Application::GetMainThreadId();
	unsigned int Pool::worker_count = 0;
	bool Pool::started = false;

	void Pool::StartWorkers(const std::vector<Worker*>& workers)
	{
		if (std::this_thread::get_id() != main_thread)
		{
			CORE_ASSERT(false, "Thread::Pool functions can only be called from the main thread!");
			return;
		}

		if (started)
		{
			CORE_ASSERT(false, "Thread::Pool already started, you cannot start it twice!");
			return;
		}

		started = true;

		// Add all workers before starting, to check names
		for (auto worker : workers)
			AddWorker(worker);

		// Start all workers
		for (auto worker : Pool::workers)
			worker->thread = new std::thread(worker->Spawn);
	}

	void Pool::JoinWorkers()
	{
		// tell all workers to finish
		for (Worker* worker : workers)
		{
			worker->Finish();
		}

		// join and shutdown all workers
		std::vector<Worker*>::iterator index;
		while (workers.size() > 0)
		{
			index = workers.begin();
			for (Worker* worker : workers)
			{
				if (worker->Joinable())
				{
					worker->Join();
					index = workers.erase(index);
				}
				else if (++index == workers.end()) break;
			}
		}
	}

	void Pool::AddWorker(Worker* worker)
	{
		for (Worker* other : workers)
		{
			if (other == worker || other->name == worker->name)
			{
				CORE_ASSERT(false, "Worker named \"" + worker->name + "\" already added to Thread::Pool, or shares a name with another worker!");
				return;
			}
		}
		workers.push_back(worker);
		worker_count++;
	}

}
}
