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

#ifndef ALLOYPROCESS_H_
#define ALLOYPROCESS_H_
#include <memory>
#include <thread>
#include <list>
#include <vector>
#include <string>
namespace aly {
class Simulation;
class SimulationListener {
public:
	virtual void SimulationEvent(Simulation* simulation,
			int mSimulationIteration, double time)=0;
	virtual ~SimulationListener(){}
};
class Simulation {
protected:
	bool running;
	bool paused;
	std::string name;
	double simulationTime;
	double timeStep;
	double simulationDuration;
	double computeTimeSeconds;
	long simulationIteration;

	bool isDrawDirty;
	bool isInitialized;

	std::thread simulationThread;
	std::list<SimulationListener*> listeners;
public:
	typedef std::chrono::high_resolution_clock Clock;
	inline void addListener(SimulationListener* listener) {
		listeners.push_back(listener);
	}
	inline void fireUpdateEvent() {
		for (SimulationListener* listender : listeners) {
			listender->SimulationEvent(this, simulationIteration,
					simulationTime);
		}
	}
	Simulation(const std::string& name);
	void loadParameters(const std::string& paramFile);
	void saveParameters(const std::string& paramFile);
	bool setSource(const std::string& sourceFileName);
	inline bool isRunning() {
		return running;
	}
	void stopRunning();
	inline bool isDirty() {
		return isDrawDirty;
	}
	virtual bool isPlayback() {
		return false;
	}
	inline const std::string& getName() {
		return name;
	}
	inline void setName(const std::string& name) {
		this->name = name;
	}
	inline double getSimulationTime() {
		return simulationTime;
	}
	inline double getSimulationDuration() {
		return simulationDuration;
	}
	inline long getSimulationIteration() {
		return simulationIteration;
	}
	inline double getComputeTimePerFrame() {
		return computeTimeSeconds;
	}
	virtual bool init()=0;
	virtual bool step()=0;
	virtual void cleanup()=0;
	bool updateGL();
	void reset();
	bool start();
	bool stop();
	inline virtual ~Simulation() {
		stop();
	}
};
}

#endif /* ALLOYPROCESS_H_ */
