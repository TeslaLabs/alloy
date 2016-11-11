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

//Based on force directed graph implementation in Prefuse (http://prefuse.org)
//originally written by Jeffrey Heer (http://jheer.org) at UC Berkeley.
#ifndef INCLUDE_CORE_FORCEDIRECTEDGRAPH_H_
#define INCLUDE_CORE_FORCEDIRECTEDGRAPH_H_
#include "AlloyDataFlow.h"
#include "AlloyMath.h"
#include "AlloyUI.h"
#include "AlloyWorker.h"
#include <vector>
#include <array>
#include <memory>
#include <mutex>
namespace aly {
class AlloyContext;
namespace dataflow {
class ForceSimulator;
struct ForceItem {
	float mass;
	float buoyancy;
	float2 force;
	float2 velocity;
	float2 location;
	float2 plocation;
	NodeShape shape;
	RGBAf color;
	std::array<float2, 4> k;
	std::array<float2, 4> l;
	ForceItem(const float2& pt = float2(0.0f)) :
			mass(1.0f), buoyancy(1.0f), force(0.0f), velocity(0.0f), location(
					pt), plocation(pt), shape(NodeShape::Hidden) ,color(1.0f,0.2f,0.2f,1.0f){
	}
	void reset() {
		force = float2(0.0f);
		velocity = float2(0.0f);
		plocation = location;
	}
	virtual void draw(AlloyContext* context, const pixel2& offset, float scale,bool selected);
	virtual ~ForceItem(){}
};
typedef std::shared_ptr<ForceItem> ForceItemPtr;
struct SpringItem {
	ForceItemPtr item1;
	ForceItemPtr item2;
	float kappa;
	float gamma;
	float length;
	float2 direction;
	bool visible;
	
	SpringItem(const ForceItemPtr& fi1, const ForceItemPtr& fi2, float k,
			float len) :
			item1(fi1), item2(fi2), kappa(k), gamma(0.0f), length(len), direction(normalize(fi2->location-fi1->location)),visible(true){
	}
	void update() {
		length = distance(item1->location, item2->location);
		direction = (item2->location - item1->location)/std::max(1E-6f,length);
	}
	void draw(AlloyContext* context, const pixel2& offset,float scale);
};

typedef std::shared_ptr<SpringItem> SpringItemPtr;
struct ForceParameter {
	std::string name;
	float value;
	float min;
	float max;
	ForceParameter(const std::string& name, float value, float min, float max) :
			name(name), value(value), min(min), max(max) {
	}
};
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const ForceParameter& param) {
	return ss << param.name << " : " << param.value << " range: [" << param.min
			<< "," << param.max << "]";
}
class Force {
protected:
	std::vector<float> params;
	std::vector<float> minValues;
	std::vector<float> maxValues;
	bool enabled = true;
	bool visible = true;
public:
	virtual void init(ForceSimulator& fsim) {
	}
	;
	virtual ~Force() {
	}
	;
	size_t getParameterCount() const {
		return params.size();
	}
	virtual void enforceBoundary(const std::shared_ptr<ForceItem>& forceItem){

	}
	void setEnabled(bool b) {
		enabled = b;
	}
	void setVisible(bool b) {
		visible = b;
	}
	bool isVisible() const {
		return visible;
	}
	bool isEnabled() const {
		return enabled;
	}
	float getParameterValue(size_t i) const {
		return params[i];
	}
	float getMinValue(size_t param) const {
		return minValues[param];
	}
	float getMaxValue(size_t param) const {
		return maxValues[param];
	}
	virtual std::string getParameterName(size_t i) const {
		return std::string();
	}
	virtual std::string getName() const = 0;
	ForceParameter getParameter(size_t i) const {
		return ForceParameter(getParameterName(i), getParameterValue(i),
				getMinValue(i), getMaxValue(i));
	}
	virtual void setParameter(size_t i, float val) {
		params[i] = val;
	}
	void setMinValue(size_t i, float val) {
		minValues[i] = val;
	}
	void setMaxValue(size_t i, float val) {
		maxValues[i] = val;
	}
	virtual bool isSpringItem() const {
		return false;
	}
	virtual bool isForceItem() const {
		return false;
	}
	virtual bool isBoundaryItem() const {
		return false;
	}
	virtual void getForce(const ForceItemPtr& item) {
		throw std::runtime_error("Get force item not implemented.");
	}
	;
	virtual void getSpring(const SpringItemPtr& spring) {
		throw std::runtime_error("Get spring item not implemented.");
	}
	virtual void draw(AlloyContext* context, const pixel2& offset, float scale) {

	}
	;
};

typedef std::shared_ptr<Force> ForcePtr;

struct Integrator {
	virtual void integrate(ForceSimulator& sim, float timestep) const = 0;
	Integrator() {
	}
	;
	virtual ~Integrator() {
	}
	
};
typedef std::shared_ptr<Integrator> IntegratorPtr;
class ForceSimulator;
struct RungeKuttaIntegrator: public Integrator {
	virtual void integrate(ForceSimulator& sim, float timestep) const override;
};
struct EulerIntegrator: public Integrator {
	virtual void integrate(ForceSimulator& sim, float timestep) const override;
};
class ForceSimulator: public Region {
protected:
	std::mutex lock;
	std::vector<ForceItemPtr> items;
	std::vector<SpringItemPtr> springs;
	std::vector<ForcePtr> iforces;
	std::vector<ForcePtr> sforces;
	std::vector<ForcePtr> bforces;
	std::vector<ForcePtr> allforces;
	std::shared_ptr<Integrator> integrator;
	float speedLimit = 1.0f;
	int renderCount = 0;
	float frameRate = 0.0f;
	box2f forceBounds;
	pixel2 cursorDownPosition;
	pixel2 dragOffset, lastDragOffset;
	float scale;
	bool draggingNode;
	bool draggingView;
	bool requestFitToBounds;
	std::chrono::steady_clock::time_point lastTime;
	bool update(uint64_t iter);
	float runSimulator(float timestep = DEFAULT_TIME_STEP);
public:
	static const float RADIUS;
	static const float DEFAULT_TIME_STEP;
	static const int DEFAULT_INTEGRATION_CYCLES;
	static const int DEFAULT_TIME_OUT;
	ForceItem* selected;
	std::function<void(float)> onStep;
	void accumulate();
	void fit();
	void erase(const SpringItemPtr& item);
	void erase(const std::list<SpringItemPtr>& item);
	void erase(const ForceItemPtr& item);
	void erase(const std::list<ForceItemPtr>& item);
	void setOffset(const pixel2& offset) {
		requestFitToBounds = false;
		dragOffset = offset;
	}
	pixel2 getOffset() const {
		return dragOffset;
	}
	void setZoom(float z) {
		requestFitToBounds = false;
		scale = z;
	}
	std::mutex& getLock(){
		return lock;
	}
	void setSelected(ForceItem* item) {
		selected = item;
	}
	void optimize(float tolerance = 0.25f, int maxIterations = 32000, float timestep = DEFAULT_TIME_STEP);
	ForceSimulator(const std::string& name, const AUnit2D& pos,
			const AUnit2D& dims, const std::shared_ptr<Integrator>& integr =
					std::shared_ptr<Integrator>(new RungeKuttaIntegrator()));
	void start();
	void stop();
	bool isRunning() const {
		return simWorker->isRunning();
	}
	void cancel(){
		simWorker->cancel(false);
	}
	void enforceBoundaries();
	box2f getForceItemBounds() const {
		return forceBounds;
	}
	RecurrentTaskPtr simWorker;
	float getFrameRate() const {
		return frameRate;
	}
	float getSpeedLimit() const;
	void setSpeedLimit(float limit);
	IntegratorPtr getIntegrator() const;
	void setIntegrator(const IntegratorPtr& intgr);
	void clear();
	void addForce(const ForcePtr& f);
	std::vector<ForcePtr>& getForces();
	void addForceItem(const ForceItemPtr& item);
	bool removeItem(ForceItemPtr item);
	std::vector<ForceItemPtr>& getForceItems();
	std::vector<SpringItemPtr>& getSprings();
	SpringItemPtr addSpringItem(const ForceItemPtr& item1,
			const ForceItemPtr& item2, float coeff, float length);
	SpringItemPtr addSpringItem(const ForceItemPtr& item1,
			const ForceItemPtr& item2, float length);
	SpringItemPtr addSpringItem(const ForceItemPtr& item1,
			const ForceItemPtr& item2);
	void addSpringItem(const SpringItemPtr& spring);
	virtual void draw(AlloyContext* context) override;
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
		override;
};
typedef std::shared_ptr<ForceSimulator> ForceSimulatorPtr;

struct SpringForce: public Force {
	static const std::string pnames[2];
	static const float DEFAULT_SPRING_COEFF;
	static const float DEFAULT_MAX_SPRING_COEFF;
	static const float DEFAULT_MIN_SPRING_COEFF;
	static const float DEFAULT_SPRING_LENGTH;
	static const float DEFAULT_MIN_SPRING_LENGTH;
	static const float DEFAULT_MAX_SPRING_LENGTH;
	static const int SPRING_COEFF;
	static const int SPRING_LENGTH;
	SpringForce(float springCoeff, float defaultLength) {
		params = std::vector<float> { springCoeff, defaultLength };
		minValues = std::vector<float> { DEFAULT_MIN_SPRING_COEFF,
				DEFAULT_MIN_SPRING_LENGTH };
		maxValues = std::vector<float> { DEFAULT_MAX_SPRING_COEFF,
				DEFAULT_MAX_SPRING_LENGTH };
	}
	SpringForce(float springCoeff=DEFAULT_SPRING_COEFF) :
			SpringForce(springCoeff, DEFAULT_SPRING_LENGTH) {
	}
	virtual std::string getName() const override {
		return "Spring Force";
	}
	virtual bool isSpringItem() const override {
		return true;
	}

	virtual std::string getParameterName(size_t i) const override {
		return pnames[i];
	}
	virtual void getSpring(const SpringItemPtr& s) override;
};
typedef std::shared_ptr<SpringForce> SpringForcePtr;
struct DragForce: public Force {
	static const std::string pnames[1];
	static const float DEFAULT_DRAG_COEFF;
	static const float DEFAULT_MIN_DRAG_COEFF;
	static const float DEFAULT_MAX_DRAG_COEFF;
	static const int DRAG_COEFF;
	DragForce(float dragCoeff=DEFAULT_DRAG_COEFF) {
		params = std::vector<float> { dragCoeff };
		minValues = std::vector<float> { DEFAULT_MIN_DRAG_COEFF };
		maxValues = std::vector<float> { DEFAULT_MAX_DRAG_COEFF };
	}
	virtual std::string getName() const override {
		return "Drag Force";
	}
	virtual bool isForceItem() const override {
		return true;
	}
	virtual std::string getParameterName(size_t i) const override {
		return pnames[i];
	}
	virtual void getForce(const ForceItemPtr& item) override {
		item->force -= params[DRAG_COEFF] * item->velocity;
	}
};
typedef std::shared_ptr<DragForce> DragForcePtr;
struct BoxForce: public Force {
	static const std::string pnames[1];
	static const float DEFAULT_GRAV_CONSTANT;
	static const float DEFAULT_MIN_GRAV_CONSTANT;
	static const float DEFAULT_MAX_GRAV_CONSTANT;
	static const int GRAVITATIONAL_CONST;
	float2 pts[4];
	float2 dxy[4];
	BoxForce(float gravConst, const box2f& box);
	BoxForce(const box2f& box) :
			BoxForce(DEFAULT_GRAV_CONSTANT, box) {
	}
	virtual std::string getName() const override {
		return "Box Boundary";
	}
	virtual bool isForceItem() const override {
		return true;
	}
	virtual bool isBoundaryItem() const override {
		return true;
	}
	virtual void enforceBoundary(const std::shared_ptr<ForceItem>& forceItem) override;
	void setBounds(const box2f& bounds);
	virtual std::string getParameterName(size_t i) const override {
		return pnames[i];
	}
	virtual void draw(AlloyContext* context, const pixel2& offset, float scale) override;
	virtual void getForce(const ForceItemPtr& item) override;
};
typedef std::shared_ptr<BoxForce> BoxForcePtr;
struct CircularWallForce: public Force {
	static const std::string pnames[1];
	static const float DEFAULT_GRAV_CONSTANT;
	static const float DEFAULT_MIN_GRAV_CONSTANT;
	static const float DEFAULT_MAX_GRAV_CONSTANT;
	static const int GRAVITATIONAL_CONST = 0;
	float2 p;
	float r;
	CircularWallForce(float gravConst, float2 p, float r) :
			p(p), r(r) {
		params = std::vector<float> { gravConst };
		minValues = std::vector<float> { DEFAULT_MIN_GRAV_CONSTANT };
		maxValues = std::vector<float> { DEFAULT_MAX_GRAV_CONSTANT };
	}
	CircularWallForce(float2 p, float r) :
			CircularWallForce(DEFAULT_GRAV_CONSTANT, p, r) {
	}
	virtual std::string getName() const override {
		return "Circular Boundary";
	}
	virtual bool isForceItem() const override {
		return true;
	}
	virtual bool isBoundaryItem() const override {
		return true;
	}
	virtual void enforceBoundary(const std::shared_ptr<ForceItem>& forceItem) override;
	virtual std::string getParameterName(size_t i) const override {
		return pnames[i];
	}
	virtual void getForce(const ForceItemPtr& item) override;
	virtual void draw(AlloyContext* context, const pixel2& offset, float scale) override;
};
typedef std::shared_ptr<CircularWallForce> CircularWallForcePtr;
struct GravitationalForce: public Force {
	static const std::string pnames[2];
	static const int GRAVITATIONAL_CONST = 0;
	static const int DIRECTION = 1;
	static const float DEFAULT_FORCE_CONSTANT;
	static const float DEFAULT_MIN_FORCE_CONSTANT;
	static const float DEFAULT_MAX_FORCE_CONSTANT;
	static const float DEFAULT_DIRECTION;
	static const float DEFAULT_MIN_DIRECTION;
	static const float DEFAULT_MAX_DIRECTION;
	float2 gDirection;
	GravitationalForce(float forceConstant, float direction) {
		params = std::vector<float> { forceConstant, direction };
		minValues = std::vector<float> { DEFAULT_MIN_FORCE_CONSTANT,
				DEFAULT_MIN_DIRECTION };
		maxValues = std::vector<float> { DEFAULT_MAX_FORCE_CONSTANT,
				DEFAULT_MAX_DIRECTION };
		float theta = params[DIRECTION];
		gDirection = float2(std::cos(theta), std::sin(theta));
	}
	GravitationalForce(float force=DEFAULT_FORCE_CONSTANT) :
			GravitationalForce(force, DEFAULT_DIRECTION) {
	}
	virtual std::string getName() const override {
		return "Gravity";
	}
	virtual void setParameter(size_t i, float val) override {
		params[i] = val;
		if (i == DIRECTION) {
			gDirection = float2(std::cos(val), std::sin(val));
		}
	}
	virtual bool isForceItem() const override {
		return true;
	}
	virtual std::string getParameterName(size_t i) const override {
		return pnames[i];
	}
	virtual void getForce(const ForceItemPtr& item) override;
};
typedef std::shared_ptr<GravitationalForce> GravitationalForcePtr;

struct BuoyancyForce: public Force {
	static const std::string pnames[2];
	static const int GRAVITATIONAL_CONST = 0;
	static const int DIRECTION = 1;
	static const float DEFAULT_FORCE_CONSTANT;
	static const float DEFAULT_MIN_FORCE_CONSTANT;
	static const float DEFAULT_MAX_FORCE_CONSTANT;
	static const float DEFAULT_DIRECTION;
	static const float DEFAULT_MIN_DIRECTION;
	static const float DEFAULT_MAX_DIRECTION;
	float2 gDirection;
	BuoyancyForce(float forceConstant, float direction) {
		params = std::vector<float> { forceConstant, direction };
		minValues = std::vector<float> { DEFAULT_MIN_FORCE_CONSTANT,
				DEFAULT_MIN_DIRECTION };
		maxValues = std::vector<float> { DEFAULT_MAX_FORCE_CONSTANT,
				DEFAULT_MAX_DIRECTION };
		float theta = params[DIRECTION];
		gDirection = float2(std::cos(theta), std::sin(theta));
	}
	BuoyancyForce(float force=DEFAULT_FORCE_CONSTANT) :
			BuoyancyForce(force, DEFAULT_DIRECTION) {
	}
	virtual std::string getName() const override {
		return "Buoyancy";
	}
	virtual void setParameter(size_t i, float val) override {
		params[i] = val;
		if (i == DIRECTION) {
			gDirection = float2(std::cos(val), std::sin(val));
		}
	}
	virtual bool isForceItem() const override {
		return true;
	}
	virtual std::string getParameterName(size_t i) const override {
		return pnames[i];
	}
	virtual void getForce(const ForceItemPtr& item) override;
};
typedef std::shared_ptr<BuoyancyForce> BuoyancyForcePtr;

struct QuadTreeNode {
	static const int MAX_LEAFS = 8;
	static const int MAX_DEPTH = 12;
	float mass;
	float2 com;
	bool hasChildren;
	int depth;
	box2f bounds;
	std::vector<ForceItemPtr> values;
	std::array<std::shared_ptr<QuadTreeNode>, 4> children;
	void update();
	QuadTreeNode(float mass = 0.0f, float2 com = float2(0.0f),int depth=-1) :
			mass(mass), com(com), hasChildren(false),depth(depth),bounds(box2f()) {
	}
	void insert(const ForceItemPtr& item);
	void draw(AlloyContext* context, const pixel2& offset, float scale);
	void reset();
};
typedef std::shared_ptr<QuadTreeNode> QuadTreeNodePtr;

struct NBodyForce: public Force {
	static const std::string pnames[3];
	static const float DEFAULT_GRAV_CONSTANT;
	static const float DEFAULT_MIN_GRAV_CONSTANT;
	static const float DEFAULT_MAX_GRAV_CONSTANT;
	static const float DEFAULT_DISTANCE;
	static const float DEFAULT_MIN_DISTANCE;
	static const float DEFAULT_MAX_DISTANCE;
	static const float DEFAULT_THETA;
	static const float DEFAULT_MIN_THETA;
	static const float DEFAULT_MAX_THETA;
	static const int GRAVITATIONAL_CONST = 0;
	static const int MIN_DISTANCE = 1;
	static const int BARNES_HUT_THETA = 2;
	QuadTreeNodePtr root;
public:
	NBodyForce(float gravConstant, float minDistance, float theta){
		params = {gravConstant, minDistance, theta};
		minValues = {DEFAULT_MIN_GRAV_CONSTANT,
			DEFAULT_MIN_DISTANCE, DEFAULT_MIN_THETA};
		maxValues = {DEFAULT_MAX_GRAV_CONSTANT,
			DEFAULT_MAX_DISTANCE, DEFAULT_MAX_THETA};
		visible = false;
	}
	NBodyForce(float grav=DEFAULT_GRAV_CONSTANT) :NBodyForce(grav, DEFAULT_DISTANCE, DEFAULT_THETA) {
	}

	virtual bool isForceItem() const override {
		return true;
	}
	virtual void draw(AlloyContext* context, const pixel2& offset, float scale) override;
	virtual std::string getParameterName(size_t i) const override {
		return pnames[i];
	}
	virtual std::string getName() const override {
		return "N-Body Force";
	}
	void clear();
	virtual void init(ForceSimulator& fsim) override;
	void getForce(const ForceItemPtr& item) override;

};
typedef std::shared_ptr<NBodyForce> NBodyForcePtr;
}
}
#endif /* INCLUDE_CORE_FORCEDIRECTEDGRAPH_H_ */
