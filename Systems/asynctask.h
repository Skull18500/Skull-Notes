#pragma once

#include "log.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <exception>
#include <functional>
#include <mutex>
#include <queue>
#include <ratio>
#include <string>
#include <thread>
#include <utility>
#include <vector>

class TaskSystem {
public:
	static TaskSystem& Get() {
		static TaskSystem instance;
		return instance;
	}

	TaskSystem(const TaskSystem&) = delete;
	TaskSystem& operator=(const TaskSystem&) = delete;
	TaskSystem(TaskSystem&&) = delete;
	TaskSystem& operator=(TaskSystem&&) = delete;

	// Detected worker thread count
	unsigned int numthreads = 4;

	// VERSION 1: Callback runs on the MAIN THREAD (Safe for OpenGL)
	void enqueueMain(std::function<void()> task, std::function<void()> callback) {
		if (!task) return;

		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			tasks.emplace([this, task = std::move(task), callback = std::move(callback)]() mutable {
				task(); // Run heavy work on worker thread

				if (callback) {
					// Push the callback to the main thread queue
					std::lock_guard<std::mutex> main_lock(main_mutex);
					main_callbacks.push_back(std::move(callback));
				}
				});
		}
		condition.notify_one();
	}

	// VERSION 2: Callback runs on the WORKER THREAD (Only use for thread-safe logic)
	void enqueueWorker(std::function<void()> task, std::function<void()> callback) {
		if (!task) return;

		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			tasks.emplace([task = std::move(task), callback = std::move(callback)]() mutable {
				task();
				if (callback) {
					callback();
				}
				});
		}
		condition.notify_one();
	}

	// VERSION 3: Simple task, no callback
	void enqueue(std::function<void()> task) {
		if (!task) return;

		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			tasks.emplace(std::move(task));
		}
		condition.notify_one();
	}

	/**
	 * MUST BE CALLED IN THE MAIN ENGINE LOOP.
	 * Processes callbacks that need to run on the main (OpenGL) thread.
	 * @param maxMilliseconds - Optional time budget limit to prevent long frames.
	 * 0.0f means "process everything".
	 */
	void update(float maxMilliseconds = 0.0f) {
		auto startTime = std::chrono::high_resolution_clock::now();

		while (true) {
			std::function<void()> callback;
			{
				std::lock_guard<std::mutex> lock(main_mutex);
				if (main_callbacks.empty()) break;

				callback = std::move(main_callbacks.front());
				main_callbacks.pop_front();
			}

			if (callback) {
				try {
					callback();
				}
				catch (const std::exception& e) {
					printlog(std::string("[TaskSystem Error] Exception in main callback: ") + e.what());
				}
				catch (...) {
					printlog("[TaskSystem Error] Unknown exception in main callback");
				}
			}

			// Check if we've exceeded our time slice for this frame
			if (maxMilliseconds > 0.0f) {
				auto now = std::chrono::high_resolution_clock::now();
				std::chrono::duration<float, std::milli> elapsed = now - startTime;
				if (elapsed.count() >= maxMilliseconds) break;
			}
		}
	}

private:
	TaskSystem() {
		// Auto-detect available cores (fallback to 4 if detection fails)
		unsigned int hw_threads = std::thread::hardware_concurrency();
		numthreads = (hw_threads > 0) ? hw_threads : 4;

		printlog("Initializing TaskSystem with " + std::to_string(numthreads) + " worker threads");
		workers.reserve(numthreads);

		for (unsigned int i = 0; i < numthreads; ++i) {
			workers.emplace_back([this] {
				while (true) {
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(this->queue_mutex);
						this->condition.wait(lock, [this] {
							return !this->tasks.empty() || this->stop_pool.load();
							});

						if (this->stop_pool && this->tasks.empty()) return;

						task = std::move(this->tasks.front());
						this->tasks.pop();
					}

					if (task) {
						try {
							task();
						}
						catch (const std::exception& e) {
							printlog(std::string("[TaskSystem Error] Exception in worker thread: ") + e.what());
						}
						catch (...) {
							printlog("[TaskSystem Error] Unknown exception in worker thread");
						}
					}
				}
				});
		}
	}

	~TaskSystem() {
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			stop_pool.store(true);
		}
		condition.notify_all();

		for (std::thread& worker : workers) {
			if (worker.joinable()) {
				worker.join();
			}
		}
	}

	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;
	std::mutex queue_mutex;
	std::condition_variable condition;
	std::atomic<bool> stop_pool{ false };

	// Main Thread synchronization
	std::deque<std::function<void()>> main_callbacks;
	std::mutex main_mutex;
};