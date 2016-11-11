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

#include "AlloySimulation.h"
#include <sstream>
#include <iostream>
#include <string>
namespace aly {
void ExecuteSimulation(Simulation* sim) {
	try {
		sim->fireUpdateEvent();
		while (sim->step()) {
			sim->fireUpdateEvent();
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		if (sim->isRunning())
			sim->stopRunning();
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

Simulation::Simulation(const std::string& name) :
		running(false), paused(false), name(name), simulationTime(0), timeStep(
				0), simulationDuration(0), computeTimeSeconds(0.0), simulationIteration(
				0), isDrawDirty(false), isInitialized(false) {
	// TODO Auto-generated constructor stub

}
bool Simulation::updateGL() {
	if (isDrawDirty) {

		isDrawDirty = false;
		return true;
	} else
		return false;
}

void Simulation::reset() {
	simulationTime = 0;
	simulationIteration = 0;
	running = false;
	paused = false;
	if (isInitialized) {
		cleanup();
		isInitialized = false;
	}
}
void Simulation::stopRunning() {
	running = false;
	paused = false;
}
bool Simulation::stop() {
	if (running)
		paused = true;
	running = false;
	if (simulationThread.joinable()) {
		simulationThread.join();
	} else {
		return false;
	}
	return true;
}
bool Simulation::start() {
	if (paused) {
		paused = false;
		running = true;
		simulationThread = std::thread(ExecuteSimulation, this);
	} else {
		stop();
		if (isInitialized)
			cleanup();
		isInitialized = false;
		if (!init()) {
			return false;
		}
		isInitialized = true;
		running = true;
		simulationThread = std::thread(ExecuteSimulation, this);
	}
	return true;
}

}

