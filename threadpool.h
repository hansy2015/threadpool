#include <functional>
#include <vector>
#include <queue>
#include <atomic>
#include <future>
#include <thread>
#include <condition_variable>
#include <iostream>

#define THREADPOOL_MAX_NUM 1024

using namespace std;

class threadpool
{
private:
	using Task = std::function<void()>;
	std::vector<std::thread> _pool;
	queue<Task> _task;
	mutex _lock;
	condition_variable _task_cv;
	atomic<bool> _run{ true };
	atomic<int> _Idle{ 0 };

public:
	threadpool()
	{
		AddThread(10);
	}
	threadpool(int size)
	{
		AddThread(size);
	}
	~threadpool()
	{
		_run = false;
		_task_cv.notify_all();
		for (auto& thr : _pool)
		{
			if (thr.joinable())
			{
				thr.join();
			}
		}
	}

	template<class F, class... Args>
	auto commit(F&& f, Args&&... args) -> future<decltype(f(args...))>
	{
		if (!_run)
		{
			throw runtime_error("ThreadPool Stop");
		}
		using RetType = decltype(f(args...));
		auto task = make_shared<packaged_task<RetType>>(
			bind(forward<F>(f), forward<Args>(args)...)
			);

		future<RetType> fure = task->get_future();
		{
			lock_guard<mutex> lock{ _lock };
			_task.emplace([task]() {
				(*task)();
				});
			// 唤醒一个线程
			_task_cv.notify_one();
		}

		return fure;
	};

	int Idle()
	{
		return _Idle;
	}
	int Count()
	{
		return (int)_pool.size();
	}

private:
	void AddThread(int size)
	{
		for (; _pool.size() < THREADPOOL_MAX_NUM && size > 0; size--)
		{
			_pool.emplace_back([this] {
				while (_run)
				{
					Task task;
					unique_lock<mutex> lock(_lock);
					_task_cv.wait(lock, [this] {return !_run || !_task.empty(); });
					if (!_run || _task.empty())
					{
						return;
					}
					task = move(_task.front());
					_task.pop();
					_Idle--;
					task();
					_Idle++;
				}
				});
			_Idle++;
		}
	}

};