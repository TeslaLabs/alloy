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

#ifndef ALLOYLINEARALGEBRA_H_
#define ALLOYLINEARALGEBRA_H_

#include "AlloyMath.h"
#include <vector>
#include <functional>
#include <iomanip>
#include <limits>
#include <algorithm>
#include "cereal/types/vector.hpp"
#include "cereal/types/string.hpp"


namespace aly {
bool SANITY_CHECK_LINALG();

template<class T, int C> struct Vector {
public:
	std::vector<vec<T, C>> data;
	const int channels = C;
	typedef vec<T, C> ValueType;
	typedef typename std::vector<ValueType>::iterator iterator;
	typedef typename std::vector<ValueType>::const_iterator const_iterator;
	typedef typename std::vector<ValueType>::reverse_iterator reverse_iterator;

	iterator begin() {
		return data.begin();
	}
	iterator end() {
		return data.end();
	}
	const_iterator cbegin() const {
		return data.cbegin();
	}
	const_iterator cend() const {
		return data.cend();
	}
	reverse_iterator rbegin() {
		return data.rbegin();
	}
	reverse_iterator rend() {
		return data.rend();
	}
	template<class Archive>
	void save(Archive & archive) const {
		archive(cereal::make_nvp(MakeString() << "vector" << C, data));
	}

	template<class Archive>
	void load(Archive & archive) {
		archive(cereal::make_nvp(MakeString() << "vector" << C, data));
	}

	void set(const T& val) {
		data.assign(data.size(), vec<T, C>(val));
	}
	void set(const vec<T, C>& val) {
		data.assign(data.size(), val);
	}
	void set(const std::vector<vec<T, C>>& val) {
		data = val;
	}
	void set(T* val) {
		if (val == nullptr)
			return;
		size_t offset = 0;
		for (vec<T, C>& x : data) {
			for (int c = 0; c < C; c++) {
				x[c] = val[offset++];
			}
		}
	}
	void set(const T* val) {
		if (val == nullptr)
			return;
		size_t offset = 0;
		for (vec<T, C>& x : data) {
			for (int c = 0; c < C; c++) {
				x[c] = val[offset++];
			}
		}
	}
	void set(vec<T, C>* val) {
		if (val == nullptr)
			return;
		size_t offset = 0;
		for (vec<T, C>& x : data) {
			x = val[offset++];
		}
	}
	template<class F> void apply(F f) {
		size_t sz = size();
#pragma omp parallel for
		for (int offset = 0; offset < (int) sz; offset++) {
			f(offset, data[offset]);
		}
	}
	Vector(size_t sz) :
			data(sz) {
	}
	Vector(const Vector<T, C>& img) :
			Vector(img.size()) {
		set(img.data);
	}
	Vector<T, C>& operator=(const Vector<T, C>& rhs) {
		if (this == &rhs)
			return *this;
		if (rhs.size() > 0) {
			this->set(rhs.data);
		} else {
			this->clear();
		}
		return *this;
	}
	Vector() {
	}
	Vector(T* ptr, size_t sz) :
			Vector(sz) {
		set(ptr);
	}
	Vector(vec<T, C>* ptr, size_t sz) :
			Vector(sz) {
		set(ptr);
	}
	Vector(const std::vector<vec<T, C>>& ref) :
			data(ref) {
	}
	size_t size() const {
		return data.size();
	}
	size_t typeSize() const {
		return sizeof(vec<T, C> );
	}
	void resize(size_t sz) {
		data.resize(sz);
		data.shrink_to_fit();
	}
	void resize(size_t sz, const vec<T, C>& val) {
		data.resize(sz, val);
		data.shrink_to_fit();
	}
	void append(const vec<T, C>& val) {
		data.push_back(val);
	}
	void push_back(const vec<T, C>& val) {
		data.push_back(val);
	}
	T* ptr() {
		if (data.size() == 0)
			return nullptr;
		return &(data.front()[0]);
	}
	const T* ptr() const {
		if (data.size() == 0)
			return nullptr;
		return &(data.front()[0]);
	}
	void setZero() {
		data.assign(data.size(), vec<T, C>((T) 0));
	}
	const vec<T, C>& operator[](const size_t i) const {
		if (i >= data.size())
			throw std::runtime_error(
					MakeString() << "Vector index out of bounds " << i << "/"
							<< data.size());
		return data[i];
	}
	vec<T, C>& operator[](const size_t i) {
		if (i >= data.size())
			throw std::runtime_error(
					MakeString() << "Vector index out of bounds " << i << "/"
							<< data.size());
		return data[i];
	}
	inline void clear() {
		data.clear();
		data.shrink_to_fit();
	}
	vec<T, C> min() const {
		vec<T, C> minVal(std::numeric_limits<T>::max());
		for (const vec<T, C>& val : data) {
			minVal = aly::minVec(val, minVal);
		}
		return minVal;
	}
	vec<T, C> max() const {
		vec<T, C> maxVal(std::numeric_limits<T>::min());
		for (const vec<T, C>& val : data) {
			maxVal = aly::maxVec(val, maxVal);
		}
		return maxVal;
	}
	std::pair<vec<T, C>, vec<T, C>> range() const {
		vec<T, C> maxVal(std::numeric_limits<T>::min());
		vec<T, C> minVal(std::numeric_limits<T>::max());
		for (const vec<T, C>& val : data) {
			maxVal = aly::maxVec(val, maxVal);
			minVal = aly::minVec(val, minVal);
		}
		return std::pair<vec<T, C>, vec<T, C>>(minVal, maxVal);
	}
	vec<T, C> mean() const {
		vec<double, C> mean(0.0);
		for (const vec<T, C>& val : data) {
			mean += vec<double, C>(val);
		}
		mean = mean / (double) data.size();
		return vec<T, C>(mean);
	}
	vec<T, C> median() const {
		std::vector<T> bands[C];
		for (int c = 0; c < C; c++) {
			bands[c].resize(data.size());
		}
		size_t index = 0;
		for (const vec<T, C>& val : data) {
			for (int c = 0; c < C; c++) {
				bands[c][index] = val[c];
			}
			index++;
		}
#pragma omp parallel for
		for (int c = 0; c < C; c++) {
			std::sort(bands[c].begin(), bands[c].end());
		}
		vec<T, C> med;
		if (data.size() % 2 == 0) {
			for (int c = 0; c < C; c++) {
				med[c] = T(
						((double) bands[c][data.size() / 2]
								+ (double) bands[c][data.size() / 2 - 1])
								* 0.5f);
			}
		} else {
			for (int c = 0; c < C; c++) {
				med[c] = bands[c][data.size() / 2];
			}
		}
		return med;
	}
	vec<T, C> mad() const {
		if (data.size() <= 2)
			return vec<T, C>(T(0));
		vec<T, C> med = median();
		std::vector<T> bands[C];
		for (int c = 0; c < C; c++) {
			bands[c].resize(data.size());
		}
		size_t index = 0;
		for (const vec<T, C>& val : data) {
			vec<T, C> e = aly::abs(val - med);
			for (int c = 0; c < C; c++) {
				bands[c][index] = e[c];
			}
			index++;
		}
#pragma omp parallel for
		for (int c = 0; c < C; c++) {
			std::sort(bands[c].begin(), bands[c].end());
		}
		vec<T, C> mad;
		if (data.size() % 2 == 0) {
			for (int c = 0; c < C; c++) {
				mad[c] = T(
						((double) bands[c][data.size() / 2]
								+ (double) bands[c][data.size() / 2 - 1])
								* 0.5f);
			}
		} else {
			for (int c = 0; c < C; c++) {
				mad[c] = bands[c][data.size() / 2];
			}
		}
		return mad;
	}
	vec<T, C> madStdDev() const {
		return vec<T, C>(1.4826 * vec<double, C>(mad()));
	}
	vec<T, C> stdDev() const {
		if (data.size() < 2) {
			return vec<T, C>(T(0));
		}
		vec<T, C> avg = mean();
		vec<double, C> var(0.0);
		for (const vec<T, C>& val : data) {
			vec<double, C> e = vec<double, C>(val - avg);
			var += e * e;
		}
		var = var / (double) (data.size() - 1);
		return vec<T, C>(aly::sqrt(var));
	}
};
void WriteVectorToFile(const std::string& file, const Vector<float, 4>& vector);
void ReadVectorFromFile(const std::string& file, Vector<float, 4>& vector);

void WriteVectorToFile(const std::string& file, const Vector<float, 3>& vector);
void ReadVectorFromFile(const std::string& file, Vector<float, 3>& vector);

void WriteVectorToFile(const std::string& file, const Vector<float, 2>& vector);
void ReadVectorFromFile(const std::string& file, Vector<float, 2>& vector);

void WriteVectorToFile(const std::string& file, const Vector<float, 1>& vector);
void ReadVectorFromFile(const std::string& file, Vector<float, 1>& vector);

void WriteVectorToFile(const std::string& file, const Vector<int, 4>& vector);
void ReadVectorFromFile(const std::string& file, Vector<int, 4>& vector);

void WriteVectorToFile(const std::string& file, const Vector<int, 3>& vector);
void ReadVectorFromFile(const std::string& file, Vector<int, 3>& vector);

void WriteVectorToFile(const std::string& file, const Vector<int, 2>& vector);
void ReadVectorFromFile(const std::string& file, Vector<int, 2>& vector);

void WriteVectorToFile(const std::string& file, const Vector<int, 1>& vector);
void ReadVectorFromFile(const std::string& file, Vector<int, 1>& vector);

void WriteVectorToFile(const std::string& file, const Vector<double, 4>& vector);
void ReadVectorFromFile(const std::string& file, Vector<double, 4>& vector);

void WriteVectorToFile(const std::string& file, const Vector<double, 3>& vector);
void ReadVectorFromFile(const std::string& file, Vector<double, 3>& vector);

void WriteVectorToFile(const std::string& file, const Vector<double, 2>& vector);
void ReadVectorFromFile(const std::string& file, Vector<double, 2>& vector);

void WriteVectorToFile(const std::string& file, const Vector<double, 1>& vector);
void ReadVectorFromFile(const std::string& file, Vector<double, 1>& vector);

template<class T, int C> void Transform(Vector<T, C>& im1, Vector<T, C>& im2,
		const std::function<void(vec<T, C>&, vec<T, C>&)>& func) {
	if (im1.size() != im2.size())
		throw std::runtime_error(
				MakeString() << "Vector dimensions do not match. " << im1.size()
						<< "!=" << im2.size());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(im1.data[offset], im2.data[offset]);
	}
}
template<class T, int C> void Transform(Vector<T, C>& im1,
		const std::function<void(vec<T, C>&)>& func) {
	size_t sz = im1.size();
#pragma omp parallel for
	for (int offset = 0; offset < (int) sz; offset++) {
		func(im1.data[offset]);
	}
}
template<class T, int C> void Transform(Vector<T, C>& im1,
		const Vector<T, C>& im2,
		const std::function<void(vec<T, C>&, const vec<T, C>&)>& func) {
	if (im1.size() != im2.size())
		throw std::runtime_error(
				MakeString() << "Vector dimensions do not match. " << im1.size()
						<< "!=" << im2.size());
	size_t sz = im1.size();
#pragma omp parallel for
	for (int offset = 0; offset < (int) sz; offset++) {
		func(im1.data[offset], im2.data[offset]);
	}
}
template<class T, int C> void Transform(Vector<T, C>& im1,
		const Vector<T, C>& im2, const Vector<T, C>& im3,
		const Vector<T, C>& im4,
		const std::function<
				void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&,
						const vec<T, C>&)>& func) {
	if (im1.size() != im2.size())
		throw std::runtime_error(
				MakeString() << "Vector dimensions do not match. " << im1.size()
						<< "!=" << im2.size());
	size_t sz = im1.size();
#pragma omp parallel for
	for (int offset = 0; offset < (int) sz; offset++) {
		func(im1.data[offset], im2.data[offset], im3.data[offset],
				im4.data[offset]);
	}
}
template<class T, int C> void Transform(Vector<T, C>& im1,
		const Vector<T, C>& im2, const Vector<T, C>& im3,
		const std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)>& func) {
	if (im1.size() != im2.size())
		throw std::runtime_error(
				MakeString() << "Vector dimensions do not match. " << im1.size()
						<< "!=" << im2.size());
	size_t sz = im1.size();
#pragma omp parallel for
	for (int offset = 0; offset < (int) sz; offset++) {
		func(im1.data[offset], im2.data[offset], im3.data[offset]);
	}
}
template<class T, int C> void Transform(Vector<T, C>& im1, Vector<T, C>& im2,
		const std::function<
				void(size_t offset, vec<T, C>& val1, vec<T, C>& val2)>& func) {
	if (im1.size() != im2.size())
		throw std::runtime_error(
				MakeString() << "Vector dimensions do not match. " << im1.size()
						<< "!=" << im2.size());
	size_t sz = im1.size();
#pragma omp parallel for
	for (size_t offset = 0; offset < sz; offset++) {
		func(offset, im1.data[offset], im2.data[offset]);
	}
}
template<class T, class L, class R, int C> std::basic_ostream<L, R> & operator <<(
		std::basic_ostream<L, R> & ss, const Vector<T, C> & A) {
	size_t index = 0;
	for (const vec<T, C>& val : A.data) {
		ss << std::setw(5) << index++ << ": " << val << std::endl;
	}
	return ss;
}
template<class T, int C> Vector<T, C> operator+(const vec<T, C>& scalar,
		const Vector<T, C>& img) {
	Vector<T, C> out(img.size());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = scalar + val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> void ScaleAdd(Vector<T, C>& out,
		const vec<T, C>& scalar, const Vector<T, C>& in) {
	out.resize(in.size());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 += scalar * val2;};
	Transform(out, in, f);
}
template<class T, int C> void ScaleAdd(Vector<T, C>& out,
		const T& scalar, const Vector<T, C>& in) {
	out.resize(in.size());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 += scalar * val2;};
	Transform(out, in, f);
}
template<class T, int C> void ScaleAdd(Vector<T, C>& out,
		const Vector<T, C>& in1, const vec<T, C>& scalar,
		const Vector<T, C>& in2) {
	out.resize(in1.size());
	std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2, const vec<T, C>& val3) {val1 = val2+scalar * val3;};
	Transform(out, in1, in2, f);
}
template<class T, int C> void ScaleAdd(Vector<T, C>& out,
		const Vector<T, C>& in1, const vec<T, C>& scalar2,
		const Vector<T, C>& in2, const vec<T, C>& scalar3,
		const Vector<T, C>& in3) {
	out.resize(in1.size());
	std::function<
			void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&,
					const vec<T, C>&)> f = [=](vec<T, C>& out,
			const vec<T, C>& val1,
			const vec<T, C>& val2,
			const vec<T, C>& val3) {
		out = val1+scalar2*val2+scalar3 * val3;};
	Transform(out, in1, in2, in3, f);
}
template<class T, int C> void ScaleSubtract(Vector<T, C>& out,
		const vec<T, C>& scalar, const Vector<T, C>& in) {
	out.resize(in.size());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 -= scalar * val2;};
	Transform(out, in, f);
}
template<class T, int C> void ScaleSubtract(Vector<T, C>& out,
		const Vector<T, C>& in1, const vec<T, C>& scalar,
		const Vector<T, C>& in2) {
	out.resize(in1.size());
	std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2, const vec<T, C>& val3) {val1 = val2 - scalar * val3;};
	Transform(out, in1, in2, f);
}
template<class T, int C> void Subtract(Vector<T, C>& out,
		const Vector<T, C>& v1, const Vector<T, C>& v2) {
	out.resize(v1.size());
	std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2, const vec<T, C>& val3) {val1 = val2-val3;};
	Transform(out, v1, v2, f);
}
template<class T, int C> void Add(Vector<T, C>& out, const Vector<T, C>& v1,
		const Vector<T, C>& v2) {
	out.resize(v1.size());
	std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2, const vec<T, C>& val3) {val1 = val2 + val3;};
	Transform(out, v1, v2, f);
}
template<class T, int C> Vector<T, C> operator-(const vec<T, C>& scalar,
		const Vector<T, C>& img) {
	Vector<T, C> out(img.size());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = scalar - val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Vector<T, C> operator*(const vec<T, C>& scalar,
		const Vector<T, C>& img) {
	Vector<T, C> out(img.size());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = scalar*val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Vector<T, C> operator*(const T& scalar,
		const Vector<T, C>& img) {
	Vector<T, C> out(img.size());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = scalar*val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Vector<T, C> operator/(const vec<T, C>& scalar,
		const Vector<T, C>& img) {
	Vector<T, C> out(img.size());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = scalar / val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Vector<T, C> operator+(const Vector<T, C>& img,
		const vec<T, C>& scalar) {
	Vector<T, C> out(img.size());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = val2 + scalar;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Vector<T, C> operator-(const Vector<T, C>& img,
		const vec<T, C>& scalar) {
	Vector<T, C> out(img.size());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = val2 - scalar;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Vector<T, C> operator*(const Vector<T, C>& img,
		const vec<T, C>& scalar) {
	Vector<T, C> out(img.size());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = val2*scalar;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Vector<T, C> operator/(const Vector<T, C>& img,
		const vec<T, C>& scalar) {
	Vector<T, C> out(img.size());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = val2 / scalar;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Vector<T, C> operator-(const Vector<T, C>& img) {
	Vector<T, C> out(img.size());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = -val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Vector<T, C> operator+=(Vector<T, C>& out,
		const Vector<T, C>& img) {
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 += val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Vector<T, C> operator-=(Vector<T, C>& out,
		const Vector<T, C>& img) {
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 -= val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Vector<T, C> operator*=(Vector<T, C>& out,
		const Vector<T, C>& img) {
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 *= val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C> Vector<T, C> operator/=(Vector<T, C>& out,
		const Vector<T, C>& img) {
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 /= val2;};
	Transform(out, img, f);
	return out;
}

template<class T, int C> Vector<T, C> operator+=(Vector<T, C>& out,
		const vec<T, C>& scalar) {
	std::function<void(vec<T, C>&)> f = [=](vec<T, C>& val1) {val1 += scalar;};
	Transform(out, f);
	return out;
}
template<class T, int C> Vector<T, C> operator-=(Vector<T, C>& out,
		const vec<T, C>& scalar) {
	std::function<void(vec<T, C>&)> f = [=](vec<T, C>& val1) {val1 -= scalar;};
	Transform(out, f);
	return out;
}
template<class T, int C> Vector<T, C> operator*=(Vector<T, C>& out,
		const vec<T, C>& scalar) {
	std::function<void(vec<T, C>&)> f = [=](vec<T, C>& val1) {val1 *= scalar;};
	Transform(out, f);
	return out;
}
template<class T, int C> Vector<T, C> operator/=(Vector<T, C>& out,
		const vec<T, C>& scalar) {
	std::function<void(vec<T, C>&)> f = [=](vec<T, C>& val1) {val1 /= scalar;};
	Transform(out, f);
	return out;
}

template<class T, int C> Vector<T, C> operator+(const Vector<T, C>& img1,
		const Vector<T, C>& img2) {
	Vector<T, C> out(img1.size());
	std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2, const vec<T, C>& val3) {val1 = val2 + val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T, int C> Vector<T, C> operator-(const Vector<T, C>& img1,
		const Vector<T, C>& img2) {
	Vector<T, C> out(img1.size());
	std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2, const vec<T, C>& val3) {val1 = val2 - val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T, int C> Vector<T, C> operator*(const Vector<T, C>& img1,
		const Vector<T, C>& img2) {
	Vector<T, C> out(img1.size());
	std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2, const vec<T, C>& val3) {val1 = val2*val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T, int C> Vector<T, C> operator/(const Vector<T, C>& img1,
		const Vector<T, C>& img2) {
	Vector<T, C> out(img1.size());
	std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2, const vec<T, C>& val3) {val1 = val2 / val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T, int C> vec<double, C> dotVec(const Vector<T, C>& a,
		const Vector<T, C>& b) {
	vec<double, C> ans(0.0);
	if (a.size() != b.size())
		throw std::runtime_error(
				MakeString() << "Vector dimensions do not match. " << a.size()
						<< "!=" << b.size());
	size_t sz = a.size();
#pragma omp parallel for
	for (int c = 0; c < C; c++) {
		double cans = 0;
#pragma omp parallel for reduction(+:cans)
		for (int i = 0; i < (int) sz; i++) {
			cans += (double) a[i][c] * (double) b[i][c];
		}
		ans[c] = cans;
	}
	return ans;
}
template<class T, int C> double dot(const Vector<T, C>& a,
		const Vector<T, C>& b) {
	double ans = 0.0;
	if (a.size() != b.size())
		throw std::runtime_error(
				MakeString() << "Vector dimensions do not match. " << a.size()
						<< "!=" << b.size());
	size_t sz = a.size();
#pragma omp parallel for reduction(+:ans)
	for (int i = 0; i < (int) sz; i++) {
		ans += dot(vec<double, C>(a[i]), vec<double, C>(b[i]));
	}
	return ans;
}

template<class T, int C> T lengthSqr(const Vector<T, C>& a) {
	T ans(0);
	size_t sz = a.size();
#pragma omp parallel for reduction(+:ans)
	for (int i = 0; i < (int) sz; i++) {
		ans += dot(a[i], a[i]);
	}
	return ans;
}
template<class T, int C> T lengthL1(const Vector<T, C>& a) {
	T ans(0);
	size_t sz = a.size();
#pragma omp parallel for reduction(+:ans)
	for (int i = 0; i < (int) sz; i++) {
		for (int c = 0; c < C; c++) {
			ans += std::abs(a[i][c]);
		}
	}
	return ans;
}
template<class T, int C> T lengthInf(const Vector<T, C>& a) {
	T ans(0);
	size_t sz = a.size();
	for (int i = 0; i < (int) sz; i++) {
		for (int c = 0; c < C; c++) {
			ans = std::max(ans,std::abs(a[i][c]));
		}
	}
	return ans;
}
template<class T, int C> vec<T, C> lengthVecInf(const Vector<T, C>& a) {
	vec<T, C> ans((T) 0);
	size_t sz = a.size();
#pragma omp parallel for
	for (int c = 0; c < C; c++) {
		for (int i = 0; i < (int) sz; i++) {
			ans[c]=std::max(ans[c], std::abs(a[i][c]));
		}
	}
	return ans;
}
template<class T, int C> vec<T, C> lengthVecL1(const Vector<T, C>& a) {
	vec<T, C> ans((T) 0);
	size_t sz = a.size();
#pragma omp parallel for
	for (int c = 0; c < C; c++) {
		T cans = 0;
#pragma omp parallel for reduction(+:cans)
		for (int i = 0; i < (int) sz; i++) {
			cans += std::abs(a[i][c]);
		}
		ans[c] = cans;
	}
	return ans;
}
template<class T, int C> vec<T, C> maxVec(const Vector<T, C>& a) {
	vec<T, C> ans((T) 0);
	size_t sz = a.size();
#pragma omp parallel for
	for (int c = 0; c < C; c++) {
		T tmp(std::numeric_limits<T>::min());
//#pragma omp parallel for reduction(max:tmp)
		for (int i = 0; i < (int) sz; i++) {
			if (a[i][c] > tmp)
				tmp = a[i][c];
		}
		ans[c] = tmp;
	}
	return ans;
}
template<class T, int C> vec<T, C> minVec(const Vector<T, C>& a) {
	vec<T, C> ans((T) 0);
	size_t sz = a.size();
#pragma omp parallel for
	for (int c = 0; c < C; c++) {
		T tmp(std::numeric_limits<T>::max());
//#pragma omp parallel for reduction(min:tmp)
		for (int i = 0; i < (int) sz; i++) {
			if (a[i][c] < tmp)
				tmp = a[i][c];
		}
		ans[c] = tmp;
	}
	return ans;
}
template<class T, int C> T max(const Vector<T, C>& a) {
	size_t sz = a.size();
	T tmp(std::numeric_limits<T>::min());
//#pragma omp parallel for reduction(max:tmp)
	for (int i = 0; i < (int) sz; i++) {
		for (int c = 0; c < C; c++) {
			if (a[i][c] > tmp)
				tmp = a[i][c];
		}
	}
	return tmp;
}
template<class T, int C> T min(const Vector<T, C>& a) {
	size_t sz = a.size();
	T tmp(std::numeric_limits<T>::max());
//#pragma omp parallel for reduction(min:tmp)
	for (int i = 0; i < (int) sz; i++) {
		for (int c = 0; c < C; c++) {
			if (a[i][c] < tmp)
				tmp = a[i][c];
		}
	}
	return tmp;
}
template<class T, int C> T length(const Vector<T, C>& a) {
	return std::sqrt(lengthSqr(a));
}
template<class T, int C> vec<double, C> lengthVecSqr(const Vector<T, C>& a) {
	vec<double, C> ans(0.0);
	size_t sz = a.size();
#pragma omp parallel for
	for (int c = 0; c < C; c++) {
		double cans = 0;
#pragma omp parallel for reduction(+:cans)
		for (int i = 0; i < (int) sz; i++) {
			double val = a[i][c];
			cans += val * val;
		}
		ans[c] = cans;
	}
	return ans;
}
template<class T, int C> vec<double, C> lengthVec(const Vector<T, C>& a) {
	return aly::sqrt(lengthVecSqr(a));
}

template<class T> Vector<T, 3> Transform(const matrix<T, 3, 3>& M,
		const Vector<T, 3>& v) {
	int sz=(int)v.size();
	Vector<T, 3> out(sz);
#pragma omp parallel for
	for(int idx=0;idx<sz;idx++){
		out[idx]=Transform(M,v[idx]);
	}
	return out;
}
template<class T> Vector<T, 4> Transform(const matrix<T, 3, 3>& M,
		const Vector<T, 4>& v) {
	int sz=(int)v.size();
	Vector<T, 4> out(sz);
#pragma omp parallel for
	for(int idx=0;idx<sz;idx++){
		out[idx]=Transform(M,v[idx]);
	}
	return out;
}
template<class T> Vector<T, 3> Transform(const matrix<T, 4, 4>& M,
		const Vector<T, 3>& v) {
	int sz=(int)v.size();
	Vector<T, 3> out(sz);
#pragma omp parallel for
	for(int idx=0;idx<sz;idx++){
		out[idx]=Transform(M,v[idx]);
	}
	return out;
}
template<class T> Vector<T, 4> Transform(const matrix<T, 4, 4>& M,
		const Vector<T, 4>& v) {
	int sz=(int)v.size();
	Vector<T, 4> out(sz);
#pragma omp parallel for
	for(int idx=0;idx<sz;idx++){
		out[idx]=Transform(M,v[idx]);
	}
	return out;
}
typedef Vector<uint8_t, 4> VectorRGBA;
typedef Vector<int, 4> VectorRGBAi;
typedef Vector<float, 4> VectorRGBAf;

typedef Vector<uint8_t, 3> VectorRGB;
typedef Vector<int, 3> VectorRGBi;
typedef Vector<float, 3> VectorRGBf;

typedef Vector<uint8_t, 1> VectorA;
typedef Vector<int, 1> VectorAi;
typedef Vector<float, 1> VectorAf;

typedef Vector<uint8_t, 4> Vector4b;
typedef Vector<uint16_t, 4> Vector4us;
typedef Vector<int16_t, 4> Vector4s;
typedef Vector<int, 4> Vector4i;
typedef Vector<uint32_t, 4> Vector4ui;
typedef Vector<float, 4> Vector4f;
typedef Vector<double, 4> Vector4d;

typedef Vector<uint8_t, 3> Vector3b;
typedef Vector<uint16_t, 3> Vector3us;
typedef Vector<int16_t, 3> Vector3s;
typedef Vector<int, 3> Vector3i;
typedef Vector<uint32_t, 3> Vector3ui;
typedef Vector<float, 3> Vector3f;
typedef Vector<double, 3> Vector3d;

typedef Vector<uint8_t, 2> Vector2b;
typedef Vector<uint16_t, 2> Vector2us;
typedef Vector<int16_t, 2> Vector2s;
typedef Vector<int, 2> Vector2i;
typedef Vector<uint32_t, 2> Vector2ui;
typedef Vector<float, 2> Vector2f;
typedef Vector<double, 2> Vector2d;

typedef Vector<uint8_t, 1> Vector1b;
typedef Vector<uint16_t, 1> Vector1us;
typedef Vector<int16_t, 1> Vector1s;
typedef Vector<int, 1> Vector1i;
typedef Vector<uint32_t, 1> Vector1ui;
typedef Vector<float, 1> Vector1f;
typedef Vector<double, 1> Vector1d;

}
;

#endif /* ALLOYLINEARALGEBRA_H_ */
