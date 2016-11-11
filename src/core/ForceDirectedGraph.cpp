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

#include "ForceDirectedGraph.h"
#include "AlloyDataFlow.h"
#include "AlloyContext.h"
#include "AlloyDrawUtil.h"
#include "AlloyApplication.h"
#define NUM_THREADS 3
namespace aly {
namespace dataflow {
const std::string SpringForce::pnames[2] =
		{ "Spring Coefficient", "Rest Length" };
const float SpringForce::DEFAULT_SPRING_COEFF = 1E-4f;
const float SpringForce::DEFAULT_MAX_SPRING_COEFF = 1E-3f;
const float SpringForce::DEFAULT_MIN_SPRING_COEFF = 1E-5f;
const float SpringForce::DEFAULT_SPRING_LENGTH = 50;
const float SpringForce::DEFAULT_MIN_SPRING_LENGTH = 0;
const float SpringForce::DEFAULT_MAX_SPRING_LENGTH = 200;
const int SpringForce::SPRING_COEFF = 0;
const int SpringForce::SPRING_LENGTH = 1;

const std::string DragForce::pnames[1] = { "Drag Coefficient" };
const float DragForce::DEFAULT_DRAG_COEFF = 0.01f;
const float DragForce::DEFAULT_MIN_DRAG_COEFF = 0.0f;
const float DragForce::DEFAULT_MAX_DRAG_COEFF = 0.1f;
const int DragForce::DRAG_COEFF = 0;

const std::string BoxForce::pnames[1] = { "Gravitational Constant" };
const float BoxForce::DEFAULT_GRAV_CONSTANT = -0.1f;
const float BoxForce::DEFAULT_MIN_GRAV_CONSTANT = -1.0f;
const float BoxForce::DEFAULT_MAX_GRAV_CONSTANT = 1.0f;
const int BoxForce::GRAVITATIONAL_CONST = 0;

const std::string CircularWallForce::pnames[1] = { "Gravitational Constant" };
const float CircularWallForce::DEFAULT_GRAV_CONSTANT = -0.1f;
const float CircularWallForce::DEFAULT_MIN_GRAV_CONSTANT = -1.0f;
const float CircularWallForce::DEFAULT_MAX_GRAV_CONSTANT = 1.0f;

const std::string GravitationalForce::pnames[2] = { "Gravitational Constant",
		"Orientation" };
const float GravitationalForce::DEFAULT_FORCE_CONSTANT = 1E-4f;
const float GravitationalForce::DEFAULT_MIN_FORCE_CONSTANT = 1E-5f;
const float GravitationalForce::DEFAULT_MAX_FORCE_CONSTANT = 1E-3f;
const float GravitationalForce::DEFAULT_DIRECTION = (float) ALY_PI / 2.0f;
const float GravitationalForce::DEFAULT_MIN_DIRECTION = (float) -ALY_PI;
const float GravitationalForce::DEFAULT_MAX_DIRECTION = (float) ALY_PI;

const std::string BuoyancyForce::pnames[2] = { "Gravitational Constant",
		"Orientation" };
const float BuoyancyForce::DEFAULT_FORCE_CONSTANT = 1E-4f;
const float BuoyancyForce::DEFAULT_MIN_FORCE_CONSTANT = 1E-5f;
const float BuoyancyForce::DEFAULT_MAX_FORCE_CONSTANT = 1E-3f;
const float BuoyancyForce::DEFAULT_DIRECTION = (float) ALY_PI / 2.0f;
const float BuoyancyForce::DEFAULT_MIN_DIRECTION = (float) -ALY_PI;
const float BuoyancyForce::DEFAULT_MAX_DIRECTION = (float) ALY_PI;

const std::string NBodyForce::pnames[3] = { "Gravitational Constant",
		"Distance", "Barnes-Hut Theta" };
const float NBodyForce::DEFAULT_GRAV_CONSTANT = -1.0f;
const float NBodyForce::DEFAULT_MIN_GRAV_CONSTANT = -10.0f;
const float NBodyForce::DEFAULT_MAX_GRAV_CONSTANT = 10.0f;
const float NBodyForce::DEFAULT_DISTANCE = -1.0f;
const float NBodyForce::DEFAULT_MIN_DISTANCE = -1.0f;
const float NBodyForce::DEFAULT_MAX_DISTANCE = 500.0f;
const float NBodyForce::DEFAULT_THETA = 0.9f;
const float NBodyForce::DEFAULT_MIN_THETA = 0.0f;
const float NBodyForce::DEFAULT_MAX_THETA = 1.0f;

const float ForceSimulator::RADIUS = 20.0f;
const float ForceSimulator::DEFAULT_TIME_STEP = 30.0f;
const int ForceSimulator::DEFAULT_TIME_OUT = 10;
const int ForceSimulator::DEFAULT_INTEGRATION_CYCLES = 2;
void ForceItem::draw(AlloyContext* context, const pixel2& offset, float scale,
		bool selected) {
	if (shape == NodeShape::Hidden)
		return;
	NVGcontext* nvg = context->nvgContext;
	float lineWidth = scale * 4.0f;
	nvgStrokeWidth(nvg, lineWidth);
	if (selected) {
		nvgStrokeColor(nvg, Color(context->theme.LIGHTEST));

		nvgFillColor(nvg, Color(color).toLighter(0.25f));
	} else {

		nvgFillColor(nvg, Color(color));
		nvgStrokeColor(nvg, Color(context->theme.LIGHTER));
	}
	nvgStrokeWidth(nvg, lineWidth);
	nvgBeginPath(nvg);
	float r = scale * ForceSimulator::RADIUS;
	pixel2 location = scale * (this->location + offset);
	if (shape == NodeShape::Circle) {
		nvgCircle(nvg, location.x, location.y, r - lineWidth * 0.5f);
	} else if (shape == NodeShape::Square) {
		nvgRoundedRect(nvg, location.x - r + lineWidth * 0.5f,
				location.y - r + lineWidth * 0.5f, 2 * r - lineWidth,
				2 * r - lineWidth, r * 0.5f);
	} else if (shape == NodeShape::Triangle) {
		nvgLineJoin(nvg, NVG_ROUND);
		nvgMoveTo(nvg, location.x, location.y + r - lineWidth);
		nvgLineTo(nvg, location.x + r + lineWidth * 0.5f,
				location.y - r + lineWidth * 0.5f);
		nvgLineTo(nvg, location.x - r - lineWidth,
				location.y - r + lineWidth * 0.5f);
		nvgClosePath(nvg);
	} else if (shape == NodeShape::Hexagon) {
		nvgLineJoin(nvg, NVG_ROUND);
		float cx = location.x;
		float cy = location.y;
		static const float SCALE = 1.0f / std::sqrt(0.75f);
		float rx = (r - lineWidth * 0.5f) * SCALE;
		float ry = (r - lineWidth * 0.5f);
		nvgMoveTo(nvg, cx + rx, cy);
		nvgLineTo(nvg, cx + rx * 0.5f, cy - ry);
		nvgLineTo(nvg, cx - rx * 0.5f, cy - ry);
		nvgLineTo(nvg, cx - rx * 0.5f, cy - ry);
		nvgLineTo(nvg, cx - rx, cy);
		nvgLineTo(nvg, cx - rx * 0.5f, cy + ry);
		nvgLineTo(nvg, cx + rx * 0.5f, cy + ry);
		nvgClosePath(nvg);
	}
	nvgFill(nvg);
	nvgStroke(nvg);
}
void SpringItem::draw(AlloyContext* context, const pixel2& offset,
		float scale) {
	if (!visible)
		return;
	NVGcontext* nvg = context->nvgContext;
	nvgStrokeColor(nvg, context->theme.NEUTRAL);
	nvgStrokeWidth(nvg, scale * 4.0f);
	nvgLineCap(nvg, NVG_ROUND);
	nvgBeginPath(nvg);
	float2 objectPt = scale * (item2->location + offset);
	float2 subjectPt = scale * (item1->location + offset);
	nvgMoveTo(nvg, objectPt.x, objectPt.y);
	nvgLineTo(nvg, subjectPt.x, subjectPt.y);
	nvgStroke(nvg);
}
void ForceSimulator::erase(const SpringItemPtr& item) {
	std::lock_guard<std::mutex> lockMe(lock);
	for (auto iter = springs.begin(); iter != springs.end(); iter++) {
		if (item.get() == iter->get()) {
			springs.erase(iter);
			break;
		}
	}
}
void ForceSimulator::erase(const ForceItemPtr& item) {
	std::lock_guard<std::mutex> lockMe(lock);
	for (auto iter = items.begin(); iter != items.end(); iter++) {
		if (item.get() == iter->get()) {
			items.erase(iter);
			break;
		}
	}
	std::vector<SpringItemPtr> tmpList;
	for (SpringItemPtr spring : springs) {
		if (spring->item1.get() != item.get()
				&& spring->item2.get() != item.get()) {
			tmpList.push_back(spring);
		}
	}
	springs = tmpList;
}
void ForceSimulator::erase(const std::list<ForceItemPtr>& deleteList) {
	std::lock_guard<std::mutex> lockMe(lock);
	{
		std::vector<ForceItemPtr> tmpList;
		for (ForceItemPtr item : items) {
			bool del = false;
			for (ForceItemPtr ditem : deleteList) {
				if (item.get() == ditem.get()) {
					del = true;
					break;
				}
			}
			if (!del) {
				tmpList.push_back(item);
			}
		}
		items = tmpList;
	}
	{
		std::vector<SpringItemPtr> tmpList;
		for (SpringItemPtr spring : springs) {
			bool del = false;
			for (ForceItemPtr ditem : deleteList) {
				if (spring->item1.get() == ditem.get()
						|| spring->item2.get() == ditem.get()) {
					del = true;
					break;
				}
			}
			if (!del) {
				tmpList.push_back(spring);
			}
		}
		springs = tmpList;
	}
}
void ForceSimulator::erase(const std::list<SpringItemPtr>& deleteList) {
	std::lock_guard<std::mutex> lockMe(lock);
	std::vector<SpringItemPtr> tmpList;
	for (SpringItemPtr item : springs) {
		bool del = false;
		for (SpringItemPtr ditem : deleteList) {
			if (item.get() == ditem.get()) {
				del = true;
				break;
			}
		}
		if (!del) {
			tmpList.push_back(item);
		}
	}
	springs = tmpList;
}
void ForceSimulator::fit() {
	requestFitToBounds = true;
}
ForceSimulator::ForceSimulator(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims, const std::shared_ptr<Integrator>& integr) :
		Region(name, pos, dims), integrator(integr) {
	simWorker = RecurrentTaskPtr(new RecurrentTask([this](uint64_t iter) {
		return update(iter);
	}, DEFAULT_TIME_OUT));
	requestFitToBounds = false;
	draggingNode = false;
	draggingView = false;
	selected = nullptr;
	cursorDownPosition = pixel2(-1, -1);
	lastDragOffset = pixel2(0.0f, 0.0f);
	dragOffset = pixel2(0.0f, 0.0f);
	scale = 1.0f;
}
bool ForceSimulator::onEventHandler(AlloyContext* context,
		const InputEvent& e) {
	if (Region::onEventHandler(context, e))
		return true;

	if (context->isMouseOver(this)) {
		if (e.type == InputType::MouseButton
				&& e.button == GLFW_MOUSE_BUTTON_LEFT && e.isDown()
				&& !draggingNode && !draggingView) {
			selected = nullptr;
			float2 cursor = e.cursor / scale - getBoundsPosition() - dragOffset;
			float r = RADIUS;
			for (int i = (int) items.size() - 1; i >= 0; i--) {
				ForceItemPtr item = items[i];
				float2 dxy = item->location - cursor;
				if (std::abs(dxy.x) < r && std::abs(dxy.y) < r) {
					selected = item.get();
					break;
				}
			}
			if (selected != nullptr) {
				cursorDownPosition = e.cursor;
				lastDragOffset = selected->location - cursor;
				draggingNode = true;
			}
		}
		if (e.type == InputType::MouseButton
				&& e.button == GLFW_MOUSE_BUTTON_RIGHT && e.isDown()
				&& !draggingView && !draggingNode) {
			draggingView = true;
			lastDragOffset = dragOffset;
			cursorDownPosition = e.cursor;
		}
		if (e.type == InputType::Cursor) {
			if (draggingNode) {
				float2 offset = getBoundsPosition() + dragOffset;
				selected->plocation = selected->location = e.cursor / scale
						- offset + lastDragOffset;
				selected->velocity = float2(0.0f);
			}
			if (draggingView) {
				dragOffset = lastDragOffset
						+ (e.cursor - cursorDownPosition) / scale;
			}
		}
		if (e.type == InputType::MouseButton && e.isUp()) {
			draggingNode = false;
			draggingView = false;
			selected = nullptr;
		}
		if (e.type == InputType::Scroll) {
			float2 pw = e.cursor / scale - dragOffset;
			scale = clamp(scale * (1.0f + e.scroll.y * 0.1f), 0.05f, 20.0f);
			dragOffset = e.cursor / scale - pw;
			return true;
		}
	} else {
		draggingNode = false;
		draggingView = false;
		selected = nullptr;
	}
	return false;
}
bool ForceSimulator::update(uint64_t iter) {
	if (renderCount == 0) {
		lastTime = std::chrono::steady_clock::now();
	}
	float maxDisplacement = 0;
	for (int c = 0; c < DEFAULT_INTEGRATION_CYCLES; c++) {
		maxDisplacement = runSimulator();
	}
	renderCount++;
	std::chrono::steady_clock::time_point currentTime =
			std::chrono::steady_clock::now();
	double elapsed =
			std::chrono::duration<double>(currentTime - lastTime).count();
	if (elapsed > 1.0f) {
		lastTime = currentTime;
		frameRate =
				(float) (DEFAULT_INTEGRATION_CYCLES * renderCount / elapsed);
		//std::cout << "Frame Rate " << frameRate<< " fps" << std::endl;
		renderCount = 0;
	}

	if (!simWorker->isCanceled()) {
		if(onStep){
			onStep(maxDisplacement);
		}
		return true;
	} else {
		return false;
	}

}
void ForceSimulator::start() {
	if (!simWorker->isRunning()) {
		renderCount = 0;
		try {
			simWorker->execute();
		} catch (std::exception& e) {
			std::cerr << "Error occurred while trying to start simulator: "
					<< e.what() << std::endl;
		}
	}
}
void ForceSimulator::stop() {
	try {
		simWorker->cancel();
	} catch (std::exception& e) {
		std::cerr << "Error occurred while trying to stop simulator: "
				<< e.what() << std::endl;
	}
	renderCount = 0;
	frameRate = 0;
}
float ForceSimulator::getSpeedLimit() const {
	return speedLimit;
}
void ForceSimulator::setSpeedLimit(float limit) {
	speedLimit = limit;
}
IntegratorPtr ForceSimulator::getIntegrator() const {
	return integrator;
}
void ForceSimulator::setIntegrator(const IntegratorPtr& intgr) {
	integrator = intgr;
}
void ForceSimulator::clear() {
	items.clear();
	springs.clear();
}
void ForceSimulator::addForce(const ForcePtr& f) {
	std::lock_guard<std::mutex> lockMe(lock);
	allforces.push_back(f);
	if (f->isForceItem()) {
		iforces.push_back(f);
	}
	if (f->isSpringItem()) {
		sforces.push_back(f);
	}
	if (f->isBoundaryItem()) {
		bforces.push_back(f);
	}
}
std::vector<ForcePtr>& ForceSimulator::getForces() {
	return allforces;
}
void ForceSimulator::addForceItem(const ForceItemPtr& item) {
	std::lock_guard<std::mutex> lockMe(lock);
	items.push_back(item);
}

bool ForceSimulator::removeItem(ForceItemPtr item) {
	std::lock_guard<std::mutex> lockMe(lock);
	for (size_t i = 0; i < items.size(); i++) {
		if (items[i].get() == item.get()) {
			items.erase(items.begin() + i);
			return true;
		}
	}
	return false;
}

std::vector<ForceItemPtr>& ForceSimulator::getForceItems() {
	return items;
}

std::vector<SpringItemPtr>& ForceSimulator::getSprings() {
	return springs;
}
SpringItemPtr ForceSimulator::addSpringItem(const ForceItemPtr& item1,
		const ForceItemPtr& item2, float coeff, float length) {
	SpringItemPtr s = SpringItemPtr(
			new SpringItem(item1, item2, coeff, length));
	addSpringItem(s);
	return s;
}
void ForceSimulator::addSpringItem(const SpringItemPtr& spring) {
	std::lock_guard<std::mutex> lockMe(lock);
	springs.push_back(spring);
}
SpringItemPtr ForceSimulator::addSpringItem(const ForceItemPtr& item1,
		const ForceItemPtr& item2, float length) {
	return addSpringItem(item1, item2, -1.f, length);
}

SpringItemPtr ForceSimulator::addSpringItem(const ForceItemPtr& item1,
		const ForceItemPtr& item2) {
	return addSpringItem(item1, item2, -1.f, -1.f);
}
void ForceSimulator::draw(AlloyContext* context) {
	if (requestFitToBounds
			&& forceBounds.dimensions.x * forceBounds.dimensions.y > 0) {
		box2px bounds = getBounds();
		float df = std::max(forceBounds.dimensions.x, forceBounds.dimensions.y)
				+ 2 * RADIUS;
		float db = std::min(bounds.dimensions.x, bounds.dimensions.y);
		scale = db / df;
		float2 cf = forceBounds.center();
		float2 cb = bounds.center();
		dragOffset = cb / scale - cf - bounds.position;
		requestFitToBounds = false;
	}
	Region::draw(context);
	float2 offset = getBoundsPosition() + dragOffset;
	NVGcontext* nvg = context->nvgContext;
	pushScissor(nvg, getCursorBounds());
	{
		std::lock_guard<std::mutex> lockMe(lock);
		for (ForcePtr f : iforces) {
			f->draw(context, offset, scale);
		}
		for (ForcePtr f : sforces) {
			f->draw(context, offset, scale);
		}
		for (SpringItemPtr item : springs) {
			item->draw(context, offset, scale);
		}
		for (ForceItemPtr item : items) {
			item->draw(context, offset, scale, item.get() == selected);
		}
	}
	popScissor(nvg);
}

void ForceSimulator::accumulate() {
	for (int i = 0; i < (int) iforces.size(); i++) {
		if (iforces[i]->isEnabled())
			iforces[i]->init(*this);
	}

	for (int i = 0; i < (int) sforces.size(); i++) {
		if (sforces[i]->isEnabled())
			sforces[i]->init(*this);
	}
#pragma omp parallel for num_threads(NUM_THREADS)
	for (int i = 0; i < (int) items.size(); i++) {

		items[i]->force = float2(0.0f);
		for (ForcePtr f : iforces) {
			if (f->isEnabled())
				f->getForce(items[i]);
		}
	}
#pragma omp parallel for num_threads(NUM_THREADS)
	for (int i = 0; i < (int) springs.size(); i++) {
		for (ForcePtr f : sforces) {
			if (f->isEnabled())
				f->getSpring(springs[i]);
		}
	}

}
float ForceSimulator::runSimulator(float timestep) {
	std::lock_guard<std::mutex> lockMe(lock);
	float2 p1(1E30f);
	float2 p2(-1E30f);
	for (ForceItemPtr item : getForceItems()) {
		float2 p = item->location;
		p1 = aly::min(p, p1);
		p2 = aly::max(p, p2);
	}
	forceBounds = box2f(p1, p2 - p1);
	accumulate();
	integrator->integrate(*this, timestep);
	enforceBoundaries();
	float maxDisplacement = 0.0f;
	for (ForceItemPtr item : getForceItems()) {
		float2 p1 = item->location;
		float2 p2 = item->plocation;
		maxDisplacement = std::max(distanceSqr(p1, p2), maxDisplacement);
	}
	return std::sqrt(maxDisplacement);
}
void ForceSimulator::optimize(float tolerance, int maxIterations,
		float timestep) {
	for (int i = 0; i < maxIterations; i++) {
		float maxDisplacement = runSimulator(timestep);
		if (i > 0 && maxDisplacement <= tolerance) {
			break;
		}
	}
}
void ForceSimulator::enforceBoundaries() {
#pragma omp parallel for num_threads(NUM_THREADS)
	for (int i = 0; i < (int) items.size(); i++) {
		if (items[i].get() == selected)
			continue;
		for (ForcePtr f : bforces) {
			if (f->isEnabled())
				f->enforceBoundary(items[i]);
		}
	}
}
void EulerIntegrator::integrate(ForceSimulator& sim, float timestep) const {
	float speedLimit = sim.getSpeedLimit();
	std::vector<ForceItemPtr>& items = sim.getForceItems();
#pragma omp parallel for num_threads(NUM_THREADS)
	for (int i = 0; i < (int) items.size(); i++) {
		float coeff, len;
		ForceItemPtr item = items[i];
		if (item.get() == sim.selected)
			continue;
		item->plocation = item->location;
		item->location = item->plocation + timestep * item->velocity;
		coeff = timestep / item->mass;
		item->velocity += coeff * item->force;
		float2 vel = item->velocity;
		len = length(vel);
		if (len > speedLimit) {
			item->velocity = speedLimit * vel / len;
		}
	}
}
void RungeKuttaIntegrator::integrate(ForceSimulator& sim,
		float timestep) const {
	float speedLimit = sim.getSpeedLimit();

	std::vector<ForceItemPtr>& items = sim.getForceItems();
#pragma omp parallel for num_threads(NUM_THREADS)
	for (int i = 0; i < (int) items.size(); i++) {
		float coeff;
		std::array<float2, 4> k, l;
		ForceItemPtr item = items[i];
		if (item.get() == sim.selected)
			continue;
		coeff = timestep / item->mass;
		k = item->k;
		l = item->l;
		item->plocation = item->location;
		k[0] = timestep * item->velocity;
		l[0] = coeff * item->force;
		item->location += 0.5f * k[0];
	}
	sim.accumulate();
#pragma omp parallel for num_threads(NUM_THREADS)
	for (int i = 0; i < (int) items.size(); i++) {
		float coeff;
		float len;
		std::array<float2, 4> k, l;
		ForceItemPtr item = items[i];
		if (item.get() == sim.selected)
			continue;
		coeff = timestep / item->mass;
		k = item->k;
		l = item->l;
		float2 vel = item->velocity + 0.5f * l[0];
		len = length(vel);
		if (len > speedLimit) {
			vel = speedLimit * vel / len;
		}
		k[1] = timestep * vel;
		l[1] = coeff * item->force;
		// Set the position to the new predicted position
		item->location = item->plocation + 0.5f * k[1];
	}
	// recalculate forces
	sim.accumulate();
#pragma omp parallel for num_threads(NUM_THREADS)
	for (int i = 0; i < (int) items.size(); i++) {
		float coeff;
		float len;
		std::array<float2, 4> k, l;
		ForceItemPtr item = items[i];
		if (item.get() == sim.selected)
			continue;
		coeff = timestep / item->mass;
		k = item->k;
		l = item->l;
		float2 vel = item->velocity + 0.5f * l[1];
		len = length(vel);
		if (len > speedLimit) {
			vel = speedLimit * vel / len;
		}
		k[2] = timestep * vel;
		l[2] = coeff * item->force;
		item->location = item->plocation + 0.5f * k[2];
	}
	// recalculate forces
	sim.accumulate();
#pragma omp parallel for num_threads(NUM_THREADS)
	for (int i = 0; i < (int) items.size(); i++) {
		float coeff;
		float len;
		std::array<float2, 4> k, l;
		ForceItemPtr item = items[i];
		if (item.get() == sim.selected)
			continue;
		coeff = timestep / item->mass;
		k = item->k;
		l = item->l;
		float2 p = item->plocation;
		float2 vel = item->velocity + 0.5f * l[1];
		len = length(vel);
		if (len > speedLimit) {
			vel = speedLimit * vel / len;
		}
		k[3] = timestep * vel;
		l[3] = coeff * item->force;
		item->location = p + (k[0] + k[3]) / 6.0f + (k[1] + k[2]) / 3.0f;
		vel = (l[0] + l[3]) / 6.0f + (l[1] + l[2]) / 3.0f;
		len = length(vel);
		if (len > speedLimit) {
			vel = speedLimit * vel / len;
		}
		item->velocity += vel;
	}
}
void SpringForce::getSpring(const SpringItemPtr& s) {
	ForceItemPtr item1 = s->item1;
	ForceItemPtr item2 = s->item2;
	float len = (s->length < 0 ? params[SPRING_LENGTH] : s->length);
	float2 p1 = item1->location;
	float2 p2 = item2->location;
	float2 dxy = p2 - p1;
	float r = aly::length(dxy);
	if (r == 0.0f) {
		dxy = float2(RandomUniform(-0.5f, 0.5f) / 50.0f,
				RandomUniform(-0.5f, 0.5f) / 50.0f);
		r = aly::length(dxy);
	}
	float d = r - len;
	float coeff = (s->kappa < 0 ? params[SPRING_COEFF] : s->kappa) * d / r;

	item1->force += coeff * dxy;
	item2->force -= coeff * dxy;

	float2 dir = s->direction;
	if (s->gamma > 0.0f) {
		float2 pivot = 0.5f * (p1 + p2);
		dxy = normalize(dxy);
		float2 ortho = float2(-dxy.y, dxy.x);
		float2 arm = s->gamma * crossMag(normalize(p2 - pivot), dir) * ortho
				/ r;
		item1->force -= arm;
		item2->force += arm;
	}
}
int relativeCCW(float x1, float y1, float x2, float y2, float px, float py) {
	x2 -= x1;
	y2 -= y1;
	px -= x1;
	py -= y1;
	float ccw = px * y2 - py * x2;
	if (ccw == 0.0f) {
		ccw = px * x2 + py * y2;
		if (ccw > 0.0f) {
			px -= x2;
			py -= y2;
			ccw = px * x2 + py * y2;
			if (ccw < 0.0f) {
				ccw = 0.0f;
			}
		}
	}
	return (ccw < 0.0f) ? -1 : ((ccw > 0.0f) ? 1 : 0);
}
float ptSegDistSq(float x1, float y1, float x2, float y2, float px, float py) {
	x2 -= x1;
	y2 -= y1;
	px -= x1;
	py -= y1;
	float dotprod = px * x2 + py * y2;
	float projlenSq;
	if (dotprod <= 0.0f) {

		projlenSq = 0.0f;
	} else {

		px = x2 - px;
		py = y2 - py;
		dotprod = px * x2 + py * y2;
		if (dotprod <= 0.0f) {

			projlenSq = 0.0f;
		} else {

			projlenSq = dotprod * dotprod / (x2 * x2 + y2 * y2);
		}
	}
	float lenSq = px * px + py * py - projlenSq;
	if (lenSq < 0) {
		lenSq = 0;
	}
	return lenSq;
}
void BoxForce::setBounds(const box2f& box) {
	pts[0] = box.position;
	pts[1] = box.position + float2(box.dimensions.x, 0.0f);
	pts[2] = box.position + box.dimensions;
	pts[3] = box.position + float2(0.0f, box.dimensions.y);

	for (int k = 0; k < 4; k++) {
		float2 p1 = pts[k];
		float2 p2 = pts[(k + 1) % 4];
		float2 dxy = p2 - p1;
		float r = length(dxy);
		if (dxy.x != 0.0)
			dxy.x /= r;
		if (dxy.y != 0.0)
			dxy.y /= r;
		this->dxy[k] = dxy;
	}
}
void BoxForce::enforceBoundary(const std::shared_ptr<ForceItem>& forceItem) {
	box2f box(pts[0], pts[2] - pts[0]);
	forceItem->location = box.clamp(forceItem->location);
}
BoxForce::BoxForce(float gravConst, const box2f& box) {
	params = std::vector<float> { gravConst };
	minValues = std::vector<float> { DEFAULT_MIN_GRAV_CONSTANT };
	maxValues = std::vector<float> { DEFAULT_MAX_GRAV_CONSTANT };
	setBounds(box);
}
void BoxForce::draw(AlloyContext* context, const pixel2& offset, float scale) {
	if (!enabled || !visible)
		return;
	NVGcontext* nvg = context->nvgContext;
	nvgStrokeWidth(nvg, scale * 4.0f);
	nvgStrokeColor(nvg, Color(0.8f, 0.8f, 0.8f, 1.0f));
	nvgBeginPath(nvg);
	nvgMoveTo(nvg, scale * (pts[0].x + offset.x + 2.0f),
			scale * (pts[0].y + offset.y + 2.0f));
	nvgLineTo(nvg, scale * (pts[1].x + offset.x - 2.0f),
			scale * (pts[1].y + offset.y + 2.0f));
	nvgLineTo(nvg, scale * (pts[2].x + offset.x - 2.0f),
			scale * (pts[2].y + offset.y - 2.0f));
	nvgLineTo(nvg, scale * (pts[3].x + offset.x + 2.0f),
			scale * (pts[3].y + offset.y - 2.0f));
	nvgClosePath(nvg);
	nvgStroke(nvg);
}
void BoxForce::getForce(const ForceItemPtr& item) {
	float2 n = item->location;
	box2f box(pts[0], pts[2] - pts[0]);
	if (!box.contains(n)) {
		float2 dxy = float2(RandomUniform(-0.5f, 0.5f) / 50.0f,
				RandomUniform(-0.5f, 0.5f) / 50.0f);
		item->force += dxy * distance(item->location, item->plocation);
	}
	for (int k = 0; k < 4; k++) {
		float2 p1 = pts[k];
		float2 p2 = pts[(k + 1) % 4];
		float2 dxy = this->dxy[k];
		int ccw = relativeCCW(p1.x, p1.y, p2.x, p2.y, n.x, n.y);
		float r = (float) std::sqrt(
				ptSegDistSq(p1.x, p1.y, p2.x, p2.y, n.x, n.y));
		if (r < 1E-5f)
			r = (float) RandomUniform(1E-5f, 0.01f);
		float v = params[GRAVITATIONAL_CONST] * item->mass / (r * r * r);
		if (n.x >= std::min(p1.x, p2.x) && n.x <= std::max(p1.x, p2.x))
			item->force.y += ccw * v * dxy.x;
		if (n.y >= std::min(p1.y, p2.y) && n.y <= std::max(p1.y, p2.y))
			item->force.x += -1.0f * ccw * v * dxy.y;
	}
}
void CircularWallForce::getForce(const ForceItemPtr& item) {
	float2 n = item->location;
	float2 dxy = p - n;
	float d = length(dxy);
	float dr = r - d;
	float c = (dr > 0) ? -1.0f : 1.0f;
	float v = c * params[GRAVITATIONAL_CONST] * item->mass / (dr * dr);
	if (d < 1E-5f || d > r) {
		dxy = float2(RandomUniform(-0.5f, 0.5f) / 50.0f,
				RandomUniform(-0.5f, 0.5f) / 50.0f);
		d = length(dxy);
	}
	item->force += v * dxy / d;
}
void CircularWallForce::enforceBoundary(
		const std::shared_ptr<ForceItem>& forceItem) {
	float2 n = forceItem->location;
	float2 dxy = n - p;
	float d = length(dxy);
	if (d > r) {
		forceItem->location = p + dxy * r / d;
	}
}
void GravitationalForce::getForce(const ForceItemPtr& item) {
	float coeff = params[GRAVITATIONAL_CONST] * item->mass;
	item->force += gDirection * coeff;
}
void BuoyancyForce::getForce(const ForceItemPtr& item) {
	//1 means it sinks, 0 neutrally buoyant, -1 it floats
	float coeff = params[GRAVITATIONAL_CONST] * item->mass * item->buoyancy;
	item->force += gDirection * coeff;
}
void QuadTreeNode::reset() {
	com = float2(0.0f);
	mass = 0;
	hasChildren = false;
	values.clear();
	for (QuadTreeNodePtr& child : children) {
		child.reset();
	}
}
void QuadTreeNode::insert(const ForceItemPtr& p) {
	if (values.size() < MAX_LEAFS && !hasChildren) {
		values.push_back(p);
	} else {
		if (depth >= MAX_DEPTH) {
			values.push_back(p);
		} else {
			if (!hasChildren) {
				//move items from leafs into one of the children.
				values.push_back(p);
				for (ForceItemPtr& item : values) {
					float2 pt = item->location;
					float2 split = bounds.center();
					int i = (pt.x >= split.x ? 1 : 0)
							+ (pt.y >= split.y ? 2 : 0);
					if (children[i].get() == nullptr) {
						children[i] = QuadTreeNodePtr(new QuadTreeNode());
						float2 pt1 = bounds.position;
						float2 pt2 = bounds.position + bounds.dimensions;
						if (i == 1 || i == 3)
							pt1.x = split.x;
						else
							pt2.x = split.x;
						if (i > 1)
							pt1.y = split.y;
						else
							pt2.y = split.y;
						children[i]->bounds = box2f(pt1, pt2 - pt1);
						children[i]->depth = depth + 1;
					}
					children[i]->insert(item);
				}
				values.clear();
				hasChildren = true;
			} else {
				float2 pt = p->location;
				float2 split = bounds.center();
				int i = (pt.x >= split.x ? 1 : 0) + (pt.y >= split.y ? 2 : 0);
				if (children[i].get() == nullptr) {
					children[i] = QuadTreeNodePtr(new QuadTreeNode());
					float2 pt1 = bounds.position;
					float2 pt2 = bounds.position + bounds.dimensions;
					if (i == 1 || i == 3)
						pt1.x = split.x;
					else
						pt2.x = split.x;
					if (i > 1)
						pt1.y = split.y;
					else
						pt2.y = split.y;
					children[i]->bounds = box2f(pt1, pt2 - pt1);
					children[i]->depth = depth + 1;
				}
				children[i]->insert(p);
			}
		}
	}
}
void NBodyForce::clear() {
	root = QuadTreeNodePtr(new QuadTreeNode());
}
void NBodyForce::init(ForceSimulator& fsim) {
	root.reset(new QuadTreeNode());
	box2f bounds = fsim.getForceItemBounds();
	float2 dxy = bounds.dimensions;
	float2 center = bounds.center();
	float maxDim = std::max(dxy.x, dxy.y);
	root->bounds = box2f(center - float2(maxDim * 0.5f), float2(maxDim));
	root->depth = 0;
	for (ForceItemPtr item : fsim.getForceItems()) {
		root->insert(item);
	}
	root->update();
}
void QuadTreeNode::update() {
	com = float2(0.0f);
	mass = 0;
	for (QuadTreeNodePtr child : children) {
		if (child.get() != nullptr) {
			child->update();
			mass += child->mass;
			com += child->mass * child->com;
		}
	}
	for (ForceItemPtr item : values) {
		mass += item->mass;
		com += item->mass * item->location;
	}
	if (mass > 0) {
		com = com / mass;
	}
}
void NBodyForce::getForce(const ForceItemPtr& item) {
	std::queue<QuadTreeNode*> q;
	q.push(root.get());
	double2 forceTotal = double2(0.0f);
	const float ZERO_TOL = 1E-6f;
	//if (item->group != nullptr)return;
	while (q.size() > 0) {
		QuadTreeNode* n = q.front();
		q.pop();
		box2f box = n->bounds;
		float d = std::max(box.dimensions.x, box.dimensions.y);
		float2 dxy = n->com - item->location;
		double r = length(dxy);
		//True if distance to center of mass is grater than threshold and thresholding enabled
		bool minDist = params[MIN_DISTANCE] > 0.0f && r > params[MIN_DISTANCE];
		if (r > ZERO_TOL && d < params[BARNES_HUT_THETA] * r
				&& !box.contains(item->location)) {
			//Make sure box does not contain location or else we'll accumulate force twice
			if (!minDist) {
				forceTotal += double2(dxy * n->mass * item->mass) / (r * r * r);
			}
		} else {
			for (QuadTreeNodePtr child : n->children) {
				if (child.get() != nullptr) {
					q.push(child.get());
				}
			}
			if (!minDist) {
				//Add up forces from leaf nodes.
				for (ForceItemPtr& leaf : n->values) {
					if (leaf.get() != item.get()) {
						dxy = leaf->location - item->location;
						r = length(dxy);
						if (r > ZERO_TOL) {
							forceTotal += double2(dxy * leaf->mass * item->mass)
									/ (r * r * r);
						}
					}
				}
			}
		}
	}
	//apply update to item force
	item->force += float2(forceTotal * (double) params[GRAVITATIONAL_CONST]);
}
void QuadTreeNode::draw(AlloyContext* context, const pixel2& offset,
		float scale) {
	static std::vector<Color> colors;
	if (colors.size() == 0) {
		colors.resize(QuadTreeNode::MAX_DEPTH);
		std::srand(123181);
		for (int i = 0; i < MAX_DEPTH; i++) {
			colors[i] = HSVAtoColor(
					HSVA((std::rand() % 256) / 255.0f, 0.8f, 0.7f, 1.0f));
		}
	}
	Color c = colors[depth];
	NVGcontext* nvg = context->nvgContext;
	nvgFillColor(nvg, c);
	nvgStrokeColor(nvg, Color(255, 255, 255));
	nvgStrokeWidth(nvg, scale * 2.0f);
	nvgBeginPath(nvg);
	nvgRect(nvg, scale * (bounds.position.x + offset.x),
			scale * (bounds.position.y + offset.y), scale * bounds.dimensions.x,
			scale * bounds.dimensions.y);
	nvgFill(nvg);
	nvgStroke(nvg);
	for (QuadTreeNodePtr& child : children) {
		if (child.get() != nullptr) {
			child->draw(context, offset, scale);
		}
	}
	nvgFillColor(nvg, c);
	nvgStrokeColor(nvg, Color(255, 255, 255));
	if (hasChildren) {
		nvgBeginPath(nvg);
		nvgCircle(nvg, scale * (com.x + offset.x), scale * (com.y + offset.y),
				scale * 6.0f);
		nvgFill(nvg);
		nvgStroke(nvg);
	}
}
void NBodyForce::draw(AlloyContext* context, const pixel2& offset,
		float scale) {
	if (!enabled || !visible)
		return;
	if (root.get() != nullptr)
		root->draw(context, offset, scale);
}

void CircularWallForce::draw(AlloyContext* context, const pixel2& offset,
		float scale) {
	if (!enabled || !visible)
		return;
	NVGcontext* nvg = context->nvgContext;
	nvgStrokeWidth(nvg, scale * 4.0f);
	nvgStrokeColor(nvg, Color(0.8f, 0.8f, 0.8f, 1.0f));
	nvgBeginPath(nvg);
	nvgCircle(nvg, scale * (p.x + offset.x), scale * (p.y + offset.y),
			scale * (r - 2.0f));
	nvgStroke(nvg);
}
}
}

