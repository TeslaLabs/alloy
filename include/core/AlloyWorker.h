/*
 * Copyright(C) 2015, Blake C. Lucas, Ph.D. (img.science@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef ALLOYWORKER_H_
#define ALLOYWORKER_H_
#include <thread>
#include <functional>
#include <chrono>
#include <mutex>
namespace aly {
class WorkerTask {
protected:
	std::thread workerThread;
	std::mutex stateChange;
	const std::function<void()> executionTask;
	const std::function<void()> endTask;
	bool running = false;
	bool complete = false;
	bool requestCancel = false;
	virtual void task();
	void done();
public:
	bool isRunning() const;
	bool isCanceled() const;
	inline bool* isCanceledPtr() {
		return &requestCancel;
	}
	bool isComplete() const;

	WorkerTask(const std::function<void()>& func);
	WorkerTask(const std::function<void()>& func, const std::function<void()>& end);
	bool execute(bool block=false);
	bool cancel(bool block=true);
	virtual ~WorkerTask();
};
class RecurrentTask: public WorkerTask {
protected:
	const std::function<bool(uint64_t iteration)> recurrentTask;
	long timeout;
	void step();
public:
	void setTimeout(long milliseconds) {
		timeout = milliseconds;
	}
	RecurrentTask(const std::function<bool(uint64_t iteration)>& func,
			long milliseconds);
	RecurrentTask(const std::function<bool(uint64_t iteration)>& func,
			const std::function<void()>& end, long milliseconds);
};
class TimerTask: public WorkerTask {
protected:
	long timeout;
	long samplingTime;
	virtual void task() override;
public:
	void setTimeout(long milliseconds) {
		timeout = milliseconds;
	}
	TimerTask(const std::function<void()>& successFunc,
			const std::function<void()>& failureFunc, long milliseconds,
			long samplingTime);
};
typedef std::shared_ptr<WorkerTask> WorkerTaskPtr;
typedef std::shared_ptr<RecurrentTask> RecurrentTaskPtr;
typedef std::shared_ptr<TimerTask> TimerTaskPtr;
}
#endif /* ALLOYWORKER_H_ */
