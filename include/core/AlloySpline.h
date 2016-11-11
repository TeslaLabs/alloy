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
#ifndef ALLOYSPLINE_H_
#define ALLOYSPLINE_H_
#include <cstring>
#include "tinysplinecpp.h"
#include "AlloyVector.h"
namespace aly {
	enum class SplineType {
		Open = TS_OPENED,
		Clamp = TS_CLAMPED,
		Undefined = TS_NONE,
	};
	bool SANITY_CHECK_BSPLINE();
	template<int C> class BSpline {
	protected:
		TsBSpline spline;
	public:
		size_t getDegree() const {
			return spline.deg();
		}
		size_t getOrder() const {
			return spline.order();
		}
		size_t getDimension() const {
			return spline.dim();
		}
		size_t getNumControlPoints() const {
			return spline.nCtrlp();
		}
		size_t getNumKnots() const {
			return spline.nKnots();
		}
		void setBuckle(const float b) {
			spline.buckle(b);
		}
		void insertKnot(float u, size_t multiplicity) {
			spline.insertKnot(u, multiplicity);
		}
		void splitAt(const float u) {
			spline.split(u);
		}
		vec<float,C> evaluate(const float u) const {
			TsDeBoorNet net=spline.evaluate(u);
			vec<float, C> out;
			std::memcpy(&out[0], net.result(), sizeof(vec<float, C>));
			return out;
		}
		float getKnot(size_t n) const {
			if (n >= spline.nKnots()) {
				throw std::runtime_error(MakeString()<<"Knot index out of range " << n << "/" << spline.nKnots());
			}
			return spline.knots()[n];
		}
		void setKnot(size_t n, float value) {
			if (n >= spline.nKnots()) {
				throw std::runtime_error(MakeString() << "Knot index out of range " << n << "/" << spline.nKnots());
			}
			spline.knots()[n]=value;
		}
		void setControlPoint(size_t n, vec<float,C> value) {
			if (n >= spline.nCtrlp()) {
				throw std::runtime_error(MakeString() << "Control point index out of range " << n << "/" << spline.nCtrlp());
			}
			std::memcpy(&spline.ctrlp()[C*n], &value[0], sizeof(vec<float, C>));
		}
		vec<float, C> getControlPoint(size_t n) const {
			if (n >= spline.nCtrlp()) {
				throw std::runtime_error(MakeString() << "Control point index out of range " << n << "/" << spline.nCtrlp());
			}
			vec<float, C> out;
			std::memcpy(&out[0], &spline.ctrlp()[C*n], sizeof(vec<float, C>));
		}
		void getKnots(std::vector<float>& v) const {
			v.resize(spline.nKnots());
			std::memcpy(v.data(),spline.knots(),v.size()*sizeof(float));
		}
		void getKnots(Vector1f& v) const {
			v.resize(spline.nKnots());
			std::memcpy(v.ptr(), spline.knots(), v.size()*v.typeSize());
		}
		void getControlPoints(Vector<float,C>& v) const {
			v.resize(spline.nCtrlp());
			std::memcpy(v.ptr(), spline.ctrlp(),v.size()*v.typeSize());
		}
		void convertToBeziers() {
			spline.toBeziers();
		}
		BSpline() {
		}
		void build(const Vector<float, C>& V, const SplineType& type, int deg) {
			spline = TsBSpline(deg, C, V.size(), static_cast<tsBSplineType>(type));
			std::memcpy(spline.ctrlp(), V.ptr(), V.size()*V.typeSize());
			spline.setupKnots(static_cast<tsBSplineType>(type));
		}
		BSpline(const Vector<float, C>& V,const SplineType& type, int deg):spline(deg, C, V.size(), static_cast<tsBSplineType>(type)) {
			std::memcpy(spline.ctrlp(), V.ptr(), V.size()*V.typeSize());
			spline.setupKnots(static_cast<tsBSplineType>(type));
		}
	};
	typedef BSpline<2> BSpline2f;
	typedef BSpline<3> BSpline3f;
}

#endif
