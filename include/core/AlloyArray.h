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
#ifndef INCLUDE_ALLOYARRAY_H_
#define INCLUDE_ALLOYARRAY_H_
#include <cereal/types/array.hpp>
#include "AlloyCommon.h"
#include <cereal/cereal.hpp>
#include "cereal/types/string.hpp"
namespace aly {
template<class T, int C> struct Array: public std::array<T, C> {

	template<class Archive>
	void save(Archive & archive) const {
		archive(cereal::make_nvp(MakeString() << "array" << C, *this));
	}

	template<class Archive>
	void load(Archive & archive) {
		archive(cereal::make_nvp(MakeString() << "array" << C, *this));
	}
	void set(const T& val) {
		for (float& v : *this) {
			v = val;
		}
	}
	Array():std::array<T,C>() {

	}
	Array(const T& val)  {
		set(val);
	}
	T max() const {
		T tmp(std::numeric_limits<T>::min());
		for (int i = 0; i < C; i++) {
			if ((*this)[i] > tmp)
				tmp = (*this)[i];
		}
		return tmp;
	}
	T min() const {
		T tmp(std::numeric_limits<T>::max());
		for (int i = 0; i < C; i++) {
			if ((*this)[i] < tmp)
				tmp = (*this)[i];
		}
		return tmp;
	}
	T mean() const {
		T tmp(0);
		for (int i = 0; i < C; i++) {
			tmp += (*this)[i];
		}
		return tmp/T(C);
	}
	T median() const {
		std::vector<T> tmp(this->begin(),this->end());
		std::sort(tmp.begin(), tmp.end());
		if (C% 2 == 0) {
			return T(((double)tmp[C / 2]+ (double)tmp[C / 2 - 1])* 0.5f);
		} else {
			return tmp[C / 2];
		}
	}
	T stdDev() const {
		if (C < 2) {
			return T(0);
		}
		T avg = mean();
		double var(0.0);
		for (const T& val : *this) {
			double e = (double)(val - avg);
			var += e * e;
		}
		var = var / (double)(C - 1);
		return T(std::sqrt(var));
	}
};

template<class T, int C> void Transform(Array<T, C>& im1, Array<T, C>& im2,
		const std::function<void(T&, T&)>& func) {
	if (im1.size() != im2.size())
		throw std::runtime_error(
				MakeString() << "Array dimensions do not match. " << im1.size()
						<< "!=" << im2.size());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(im1[offset], im2[offset]);
	}
}
template<class T, int C> void Transform(Array<T, C>& im1,
		const std::function<void(T&)>& func) {
	size_t sz = im1.size();
#pragma omp parallel for
	for (int offset = 0; offset < (int) sz; offset++) {
		func(im1[offset]);
	}
}
template<class T, int C> void Transform(Array<T, C>& im1,
		const Array<T, C>& im2, const std::function<void(T&, const T&)>& func) {
	if (im1.size() != im2.size())
		throw std::runtime_error(
				MakeString() << "Array dimensions do not match. " << im1.size()
						<< "!=" << im2.size());
	size_t sz = im1.size();
#pragma omp parallel for
	for (int offset = 0; offset < (int) sz; offset++) {
		func(im1[offset], im2[offset]);
	}
}
template<class T, int C> void Transform(Array<T, C>& im1,
		const Array<T, C>& im2, const Array<T, C>& im3, const Array<T, C>& im4,
		const std::function<void(T&, const T&, const T&, const T&)>& func) {
	if (im1.size() != im2.size())
		throw std::runtime_error(
				MakeString() << "Array dimensions do not match. " << im1.size()
						<< "!=" << im2.size());
	size_t sz = im1.size();
#pragma omp parallel for
	for (int offset = 0; offset < (int) sz; offset++) {
		func(im1[offset], im2[offset], im3[offset],
				im4[offset]);
	}
}
template<class T, int C> void Transform(Array<T, C>& im1,
		const Array<T, C>& im2, const Array<T, C>& im3,
		const std::function<void(T&, const T&, const T&)>& func) {
	if (im1.size() != im2.size())
		throw std::runtime_error(
				MakeString() << "Array dimensions do not match. " << im1.size()
						<< "!=" << im2.size());
	size_t sz = im1.size();
#pragma omp parallel for
	for (int offset = 0; offset < (int) sz; offset++) {
		func(im1[offset], im2[offset], im3[offset]);
	}
}
template<class T, int C> void Transform(Array<T, C>& im1, Array<T, C>& im2,
		const std::function<void(size_t offset, T& val1, T& val2)>& func) {
	if (im1.size() != im2.size())
		throw std::runtime_error(
				MakeString() << "Array dimensions do not match. " << im1.size()
						<< "!=" << im2.size());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(offset, im1[offset], im2[offset]);
	}
}
template<class T, class L, class R, int C> std::basic_ostream<L, R> & operator <<(
		std::basic_ostream<L, R> & ss, const Array<T, C> & A) {
	size_t index = 0;
	for (const T& val : A) {
		ss << std::setw(5) << index++ << ": " << val << std::endl;
	}
	return ss;
}
template<class T, int C> Array<T, C> operator+(const T& scalar,
		const Array<T, C>& img) {
	Array<T, C> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar + val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> void ScaleAdd(Array<T, C>& out, const T& scalar,
		const Array<T, C>& in) {
	out.resize(in.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 += scalar * val2;};
	Transform(out, in, f);
}
template<class T, int C> void ScaleAdd(Array<T, C>& out, const Array<T, C>& in1,
		const T& scalar, const Array<T, C>& in2) {
	out.resize(in1.size());
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2+scalar * val3;};
	Transform(out, in1, in2, f);
}
template<class T, int C> void ScaleAdd(Array<T, C>& out, const Array<T, C>& in1,
		const T& scalar2, const Array<T, C>& in2, const T& scalar3,
		const Array<T, C>& in3) {
	out.resize(in1.size());
	std::function<void(T&, const T&, const T&, const T&)> f = [=](T& out,
			const T& val1,
			const T& val2,
			const T& val3) {
		out = val1+scalar2*val2+scalar3 * val3;};
	Transform(out, in1, in2, in3, f);
}
template<class T, int C> void ScaleSubtract(Array<T, C>& out, const T& scalar,
		const Array<T, C>& in) {
	out.resize(in.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 -= scalar * val2;};
	Transform(out, in, f);
}
template<class T, int C> void ScaleSubtract(Array<T, C>& out,
		const Array<T, C>& in1, const T& scalar, const Array<T, C>& in2) {
	out.resize(in1.size());
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 - scalar * val3;};
	Transform(out, in1, in2, f);
}
template<class T, int C> void Subtract(Array<T, C>& out, const Array<T, C>& v1,
		const Array<T, C>& v2) {
	out.resize(v1.size());
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2-val3;};
	Transform(out, v1, v2, f);
}
template<class T, int C> void Add(Array<T, C>& out, const Array<T, C>& v1,
		const Array<T, C>& v2) {
	out.resize(v1.size());
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 + val3;};
	Transform(out, v1, v2, f);
}
template<class T, int C> Array<T, C> operator-(const T& scalar,
		const Array<T, C>& img) {
	Array<T, C> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar - val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Array<T, C> operator*(const T& scalar,
		const Array<T, C>& img) {
	Array<T, C> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar*val2;};
	Transform(out, img, f);
	return out;
}

template<class T, int C> Array<T, C> operator/(const T& scalar,
		const Array<T, C>& img) {
	Array<T, C> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = scalar / val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Array<T, C> operator+(const Array<T, C>& img,
		const T& scalar) {
	Array<T, C> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2 + scalar;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Array<T, C> operator-(const Array<T, C>& img,
		const T& scalar) {
	Array<T, C> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2 - scalar;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Array<T, C> operator*(const Array<T, C>& img,
		const T& scalar) {
	Array<T, C> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2*scalar;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Array<T, C> operator/(const Array<T, C>& img,
		const T& scalar) {
	Array<T, C> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = val2 / scalar;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Array<T, C> operator-(const Array<T, C>& img) {
	Array<T, C> out(img.size());
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 = -val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Array<T, C> operator+=(Array<T, C>& out,
		const Array<T, C>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 += val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Array<T, C> operator-=(Array<T, C>& out,
		const Array<T, C>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 -= val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Array<T, C> operator*=(Array<T, C>& out,
		const Array<T, C>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 *= val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Array<T, C> operator/=(Array<T, C>& out,
		const Array<T, C>& img) {
	std::function<void(T&, const T&)> f =
			[=](T& val1, const T& val2) {val1 /= val2;};
	Transform(out, img, f);
	return out;
}

template<class T, int C> Array<T, C> operator+=(Array<T, C>& out,
		const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 += scalar;};
	Transform(out, f);
	return out;
}
template<class T, int C> Array<T, C> operator-=(Array<T, C>& out,
		const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 -= scalar;};
	Transform(out, f);
	return out;
}
template<class T, int C> Array<T, C> operator*=(Array<T, C>& out,
		const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 *= scalar;};
	Transform(out, f);
	return out;
}
template<class T, int C> Array<T, C> operator/=(Array<T, C>& out,
		const T& scalar) {
	std::function<void(T&)> f = [=](T& val1) {val1 /= scalar;};
	Transform(out, f);
	return out;
}

template<class T, int C> Array<T, C> operator+(const Array<T, C>& img1,
		const Array<T, C>& img2) {
	Array<T, C> out(img1.size());
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 + val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T, int C> Array<T, C> operator-(const Array<T, C>& img1,
		const Array<T, C>& img2) {
	Array<T, C> out(img1.size());
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 - val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T, int C> Array<T, C> operator*(const Array<T, C>& img1,
		const Array<T, C>& img2) {
	Array<T, C> out(img1.size());
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2*val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T, int C> Array<T, C> operator/(const Array<T, C>& img1,
		const Array<T, C>& img2) {
	Array<T, C> out(img1.size());
	std::function<void(T&, const T&, const T&)> f =
			[=](T& val1, const T& val2, const T& val3) {val1 = val2 / val3;};
	Transform(out, img1, img2, f);
	return out;
}

template<class T, int C> double dot(const Array<T, C>& a,
		const Array<T, C>& b) {
	double ans = 0.0;
	if (a.size() != b.size())
		throw std::runtime_error(
				MakeString() << "Array dimensions do not match. " << a.size()
						<< "!=" << b.size());
	size_t sz = a.size();
#pragma omp parallel for reduction(+:ans)
	for (int i = 0; i < (int) sz; i++) {
		ans += a[i]*b[i];
	}
	return ans;
}

template<class T, int C> T lengthSqr(const Array<T, C>& a) {
	T ans(0);
	size_t sz = a.size();
#pragma omp parallel for reduction(+:ans)
	for (int i = 0; i < (int) sz; i++) {
		ans += a[i]* a[i];
	}
	return ans;
}
template<class T, int C> T distanceSqr(const Array<T, C>& a, const Array<T, C>& b) {
	T ans(0);
	size_t sz = a.size();
#pragma omp parallel for reduction(+:ans)
	for (int i = 0; i < (int)sz; i++) {
		ans += (a[i] - b[i])*(a[i] - b[i]);
	}
	return ans;
}
template<class T, int C> T distanceL1(const Array<T, C>& a, const Array<T, C>& b) {
	T ans(0);
	size_t sz = a.size();
#pragma omp parallel for reduction(+:ans)
	for (int i = 0; i < (int)sz; i++) {
		ans += std::abs(a[i] - b[i]);
	}
	return ans;
}
template<class T, int C> T distance(const Array<T, C>& a, const Array<T, C>& b) {
	return std::sqrt(distanceSqr(a,b));
}
template<class T, int C> T max(const Array<T, C>& a) {
	size_t sz = a.size();
	T tmp(std::numeric_limits<T>::min());
//#pragma omp parallel for reduction(max:tmp)
	for (int i = 0; i < (int) sz; i++) {
		if (a[i] > tmp)
			tmp = a[i];
	}
	return tmp;
}
template<class T, int C> T min(const Array<T, C>& a) {
	size_t sz = a.size();
	T tmp(std::numeric_limits<T>::max());
//#pragma omp parallel for reduction(min:tmp)
	for (int i = 0; i < (int) sz; i++) {
		if (a[i] < tmp)
			tmp = a[i];
	}
	return tmp;
}
template<class T, int C> T length(const Array<T, C>& a) {
	return std::sqrt(lengthSqr(a));
}
}
;
#endif /* INCLUDE_ALLOYARRAY_H_ */
