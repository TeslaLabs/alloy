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

#include "AlloyMath.h"
#include "AlloyWorker.h"
namespace aly {
WorkerTask::WorkerTask(const std::function<void()>& func) :
		executionTask(func), endTask() {

}
WorkerTask::WorkerTask(const std::function<void()>& func,
		const std::function<void()>& end) :
		executionTask(func), endTask(end) {

}
bool WorkerTask::isRunning() const {
	return running;
}
bool WorkerTask::isCanceled() const {
	return requestCancel;
}

bool WorkerTask::isComplete() const {
	return complete;
}
void WorkerTask::task() {
	running = true;
	requestCancel = false;
	if (executionTask) {
		try {
			executionTask();
		} catch (std::exception&) {

		}
	}
	if (!requestCancel) {
		done();
	}
	running = false;
	requestCancel = false;
	complete = true;
}
void WorkerTask::done() {
	if (endTask)
		endTask();
}
bool WorkerTask::execute(bool block) {
	if (stateChange.try_lock()) {
		if (block) {
			task();
		} else {
			if (workerThread.joinable()) {
				stateChange.unlock();
				return false;
			}
			workerThread = std::thread(&WorkerTask::task, this);
		}
		stateChange.unlock();
		return true;
	} else {
		return false;
	}
}
WorkerTask::~WorkerTask() {
	cancel();
}
bool WorkerTask::cancel(bool block) {
	if (stateChange.try_lock()) {
		if (block) {
			if (workerThread.joinable()) {
				if (!requestCancel) {
					requestCancel = true;
					workerThread.join();
				}
			} else {
				stateChange.unlock();
				return true;
			}
		} else {
			requestCancel = true;
		}
		stateChange.unlock();
		return true;
	} else {
		return false;
	}
}
RecurrentTask::RecurrentTask(const std::function<bool(uint64_t)>& func,
		long timeout) :
		WorkerTask([this] {this->step();}), recurrentTask(func), timeout(
				timeout) {

}
RecurrentTask::RecurrentTask(const std::function<bool(uint64_t)>& func,
		const std::function<void()>& end, long timeout) :
		WorkerTask([this] {this->step();}, end), recurrentTask(func), timeout(
				timeout) {

}

void RecurrentTask::step() {
	uint64_t iter = 0;
	while (!requestCancel) {
		auto currentTime = std::chrono::steady_clock::now();
		if (recurrentTask) {
			try {
				if (!recurrentTask(iter++))
					break;
			} catch (std::exception&) {
				break;
			}
		}
		if (requestCancel)
			break;
		auto nextTime = std::chrono::steady_clock::now();
		//sleep_until has different behavior on Linux and Windows. Use sleep_for instead.
		long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
				nextTime - currentTime).count();
		std::this_thread::sleep_for(
				std::chrono::milliseconds(aly::max(0, (int) (timeout - ms))));
	}
}

TimerTask::TimerTask(const std::function<void()>& successFunc,
		const std::function<void()>& failureFunc, long timeout,
		long samplingTime) :
		WorkerTask(successFunc, failureFunc), timeout(timeout), samplingTime(
				samplingTime) {

}

void TimerTask::task() {
	running = true;
	requestCancel = false;
	complete = false;
	auto currentTime = std::chrono::steady_clock::now();
	while (!requestCancel) {
		std::this_thread::sleep_for(std::chrono::milliseconds(samplingTime));
		auto nextTime = std::chrono::steady_clock::now();
		long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
				nextTime - currentTime).count();
		if (ms >= timeout)
			break;
	}
	if (requestCancel) {
		if (endTask)
			endTask();
		complete = false;
	} else {
		try {
			if (executionTask)
				executionTask();
			complete = true;
		} catch (std::exception&) {

		}
	}
	running = false;
	requestCancel = false;
}
}

