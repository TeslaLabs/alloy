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

#ifndef ALLOYANIMATOR_H_
#define ALLOYANIMATOR_H_
#include "AlloyUnits.h"
#include <functional>
#include <memory>
#include <list>
namespace aly {
struct Interpolant {
	virtual double operator()(double t) const =0;
	virtual inline ~Interpolant() {
	}
	;
};
struct Linear: public Interpolant {
	virtual inline double operator()(double t) const override {
		return t;
	}
};

struct Smoothstep: public Interpolant {
	virtual inline double operator()(double t) const override {
		double scale = t * t * (3.0 - 2.0 * t);
		return scale * 1.0;
	}
};

struct SineIn: public Interpolant {
	virtual inline double operator()(double t) const override {
		return 1.0 - std::cos(t * ALY_PI * 0.5);
	}
};

struct SineOut: public Interpolant {
	virtual inline double operator()(double t) const override {
		return (t < 0.5) ?
				(1.0 - std::cos(t * ALY_PI)) * 0.5 :
				0.5 + std::cos((1.0 - t) * ALY_PI) * 0.5;
	}
};

struct ExponentialIn: public Interpolant {
	virtual inline double operator()(double t) const override {
		return std::pow(2.0, 10.0 * (t - 1.0));
	}
};
struct ExponentialOut: public Interpolant {
	virtual inline double operator()(double t) const override {
		return ((t < 0.5) ?
				std::pow(2.0, 10 * (2.0 * t - 1.0)) * 0.5 :
				(1.0 - std::pow(2.0, 10 * (1.0 - 2.0 * t)) * 0.5));
	}
};
struct QuadraticIn: public Interpolant {
	virtual inline double operator()(double t) const override {
		return t * t;
	}
};
struct QuadraticOut: public Interpolant {
	virtual inline double operator()(double t) const override {
		return (t < 0.5) ?
				(t * t * 2.0) * 0.5 : 1.0 - (2.0 - t * t * 2.0) * 0.5;
	}
};
struct CubicPulse: public Interpolant {
	virtual inline double operator()(double t) const override {
		double c = 1.0, w = 0.9f;
		double x = std::abs(t - c);
		if (x > w)
			return 0.0;
		x /= w;
		return 1.0 - x * x * (3.0 - 2.0 * x);
	}
};
class Tween {
private:
	std::unique_ptr<Interpolant> interpolant;
	double time = 0;
	Tweenable* object;
	double duration = 0;
	bool isCanceled = false;
	bool isCompleted = false;
public:
	std::function<void(Tweenable* object)> onComplete;
	std::function<void(Tweenable* object)> onCancel;
	template<class A> Tween(Tweenable* object, double duration, const A& a) :
			object(object), duration(duration) {
		interpolant = std::unique_ptr<Interpolant>(new A(a));
	}
	template<class F> Tween& addCompleteEvent(F func) {
		onComplete = func;
		return *this;
	}
	template<class F> Tween& addCancelEvent(F func) {
		onCancel = func;
		return *this;
	}
	void reset() {
		object->reset();
		isCanceled = false;
		isCompleted = false;
		time = 0;
	}
	void cancel() {
		isCanceled = true;
	}
	bool step(double dt) {
		if (isCanceled) {
			return false;
		}
		time += dt;
		if (duration > 0 && time < duration) {
			time = std::min(time, duration);
			object->setTweenValue((*interpolant)(time / duration));
			return true;
		} else {
			time = duration;
			object->setTweenValue(1.0);
			isCompleted = true;
			return false;
		}
	}
	void firePostEvents() {
		if (isCompleted) {
			if (onComplete)
				onComplete(object);
		} else if (isCanceled) {
			if (onCancel)
				onCancel(object);
		}
	}
};
class Animator {
private:
	std::list<std::shared_ptr<Tween>> tweens[2];
	std::list<std::shared_ptr<Tween>> finished;
	//Keep reference copies so that Tweenables are not destroyed before animator has time to finish.
	std::list<AUnit2D> coords;
	std::list<AUnit1D> units;
	std::list<AColor> colors;
	int parity = 0;
public:
	std::shared_ptr<Tween>& add(const std::shared_ptr<Tween>& tween);
	void reset();
	bool step(double dt);
	bool firePostEvents();
	template<class A> std::shared_ptr<Tween>& add(AColor& out,
			const Color& start, const Color& end, double duration, const A& a =
					Linear()) {
		ColorTween* ctween = new ColorTween(start, end);
		out = std::shared_ptr<ColorTween>(ctween);
		colors.push_back(out);
		return add(std::shared_ptr<Tween>(new Tween(ctween, duration, a)));
	}
	template<class A> std::shared_ptr<Tween>& add(AUnit2D& out,
			const AUnit2D& start, const AUnit2D& end, double duration,
			const A& a = Linear()) {
		CoordTween* ctween = new CoordTween(start, end);
		out = AUnit2D(ctween);
		coords.push_back(out);
		return add(std::shared_ptr<Tween>(new Tween(ctween, duration, a)));
	}
	template<class A> std::shared_ptr<Tween>& add(AUnit1D& out,
			const AUnit1D& start, const AUnit1D& end, double duration,
			const A& a = Linear()) {
		UnitTween* utween = new UnitTween(start, end);
		out = AUnit1D(utween);
		units.push_back(out);
		return add(std::shared_ptr<Tween>(new Tween(utween, duration, a)));
	}
};
}

#endif /* ALLOYANIMATOR_H_ */
