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

#ifndef ONEEUROFILTER_H_
#define ONEEUROFILTER_H_
#include "AlloyMath.h"
#include <chrono>
namespace aly {
	namespace detail {
		template<class T, int C>
		struct low_pass_filter {
			vec<T, C> hatxprev;
			vec<T, C> xprev;
			low_pass_filter() :
				hatxprev(T(0)), xprev(T(0)), hadprev(false) {
			}
			void reset() {
				hatxprev = vec<T, C>(T(0));
				xprev = vec<T, C>(T(0));
				hadprev = false;
			}
			vec<T, C> operator()(const vec<T, C>& x, T alpha) {
				vec<T, C> hatx;
				if (hadprev) {
					hatx = alpha * x + (T(1.0) - alpha) * hatxprev;
				}
				else {
					hatx = x;
					hadprev = true;
				}
				hatxprev = hatx;
				xprev = x;
				return hatx;
			}

			bool hadprev;
		};
	}
	template<class T, int C>
	struct PointFilter {
	private:
		std::chrono::steady_clock::time_point lastTime;
		double frequency;
		T alpha(T cutoff) {
			double tau = 1.0 / (2 * ALY_PI * cutoff);
			double te = 1.0 / frequency;
			return T(1.0 / (1.0 + tau / te));
		}
		detail::low_pass_filter<T, C> xfilt_, dxfilt_;

		T minCutoff, beta, derivativeCutoff;
	public:
		PointFilter(double _freq = 0, T _mincutoff = T(0), T _beta = T(0), T _dcutoff = T(0)) :
			frequency(_freq), minCutoff(_mincutoff), beta(_beta), derivativeCutoff(
				_dcutoff) {
			lastTime = std::chrono::steady_clock::now();
		}
		vec<T, C> evaluate(const vec<T, C>& x, double dt =
			std::numeric_limits<double>::min()) {
			vec<T, C> dx(T(0));

			std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
			if (dt == std::numeric_limits<double>::min()) {
				double elapsed = std::chrono::duration<double>(currentTime-lastTime).count();
				frequency = 1.0 / elapsed;
			}
			else {
				frequency = 1.0 / dt;
			}
			lastTime = currentTime;
			if (xfilt_.hadprev)
				dx = (x - xfilt_.xprev) * T(frequency);
			vec<T, C> edx = dxfilt_(dx, alpha(derivativeCutoff));
			T cutoff = minCutoff + beta * aly::length(edx);
			return xfilt_(x, alpha(cutoff));
		}
		double getFrequency() const {
			return frequency;
		}
		void setMinCutoff(const T& val) {
			minCutoff = val;
		}
		void setDerivativeCutoff(const T& val) {
			derivativeCutoff = val;
		}
		void setBeta(const T& val) {
			beta = val;
		}
		void reset() {
			lastTime = std::chrono::steady_clock::now();
			xfilt_.reset();
			dxfilt_.reset();
		}
	};
	typedef PointFilter<float, 1> PointFilter1f;
	typedef PointFilter<float, 2> PointFilter2f;
	typedef PointFilter<float, 3> PointFilter3f;
	typedef PointFilter<float, 4> PointFilter4f;
	typedef PointFilter<double, 1> PointFilter1d;
	typedef PointFilter<double, 2> PointFilter2d;
	typedef PointFilter<double, 3> PointFilter3d;
	typedef PointFilter<double, 4> PointFilter4d;
}
#endif