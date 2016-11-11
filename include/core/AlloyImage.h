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
#ifndef ALLOYIMAGE2D_H_INCLUDE_GUARD
#define ALLOYIMAGE2D_H_INCLUDE_GUARD
#include "AlloyCommon.h"
#include "AlloyMath.h"
#include "sha2.h"
#include "AlloyFileUtil.h"
#include "cereal/types/vector.hpp"
#include <vector>
#include <functional>
#include <fstream>
#include <random>
namespace aly {
bool SANITY_CHECK_IMAGE();
bool SANITY_CHECK_IMAGE_IO();
bool SANITY_CHECK_PYRAMID();
enum class ImageType {
	UNKNOWN=-1,
	BYTE = 0,
	UBYTE = 1,
	SHORT = 2,
	USHORT = 3,
	INT = 4,
	UINT = 5,
	FLOAT = 6,
	DOUBLE = 7
};
template<class L, class R> std::basic_ostream<L, R>& operator <<(
		std::basic_ostream<L, R> & ss, const ImageType& type) {
	switch (type) {
	case ImageType::UNKNOWN:
		return ss << "unknown";
	case ImageType::BYTE:
		return ss << "byte";
	case ImageType::UBYTE:
		return ss << "ubyte";
	case ImageType::SHORT:
		return ss << "short";
	case ImageType::USHORT:
		return ss << "ushort";
	case ImageType::INT:
		return ss << "int";
	case ImageType::UINT:
		return ss << "uint";
	case ImageType::FLOAT:
		return ss << "float";
	case ImageType::DOUBLE:
		return ss << "double";
	}
	return ss;
}
template<class T, int C, ImageType I> struct Image;
template<class T, int C, ImageType I> void WriteImageToRawFile(
		const std::string& fileName, const Image<T, C, I>& img);
template<class T, int C, ImageType I> struct Image {
protected:
	int x, y;
	std::string hashCode;
public:
	std::vector<vec<T, C>> data;
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
	reverse_iterator rbegin() const {
		return data.rbegin();
	}
	reverse_iterator rend() const {
		return data.rend();
	}

	int width;
	int height;
	uint64_t id;
	const int channels;
	const ImageType type;
	std::string updateHashCode(size_t MAX_SAMPLES = 0, HashMethod method =
		HashMethod::SHA256);
	std::string getHashCode() {
		return hashCode;
	}
	template<class Archive> void serialize(Archive & archive) {
		archive(cereal::make_nvp(MakeString() << type << channels, id),
			CEREAL_NVP(width), CEREAL_NVP(height), CEREAL_NVP(x),
			CEREAL_NVP(y), CEREAL_NVP(hashCode));
	}

	void writeToXML(const std::string& fileName) const {
		WriteImageToRawFile(fileName, *this);
	}
	void set(const T& val) {
		data.assign(data.size(), vec<T, C>(val));
	}
	void set(const vec<T, C>& val) {
		data.assign(data.size(), val);
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
	void set(const std::vector<vec<T, C>>& val) {
		data = val;
	}
	void set(vec<T, C>* val) {
		if (val == nullptr)
			return;
		size_t offset = 0;
		for (vec<T, C>& x : data) {
			x = val[offset++];
		}
	}
	void set(const Image<T, C, I>& other) {
		resize(other.width, other.height);
		id = other.id;
		x = other.x;
		y = other.y;
		set(other.data);
	}
	std::string getTypeName() const {
		return MakeString() << type << channels;
	}

	Image(int w, int h, int x = 0, int y = 0, uint64_t id = 0) :
		x(x), y(y), data(w * h), width(w), height(h), id(id), channels(C), type(
			I) {
	}
	Image(int w, int h, int2 pos, uint64_t id = 0) :
		x(pos.x), y(pos.y), data(w * h), width(w), height(h), id(id), channels(
			C), type(I) {
	}
	Image(T* ptr, int w, int h, int x = 0, int y = 0, uint64_t id = 0) :
		Image(w, h, x, y, id) {
		set(ptr);
	}
	Image(vec<T, C>* ptr, int w, int h, int x = 0, int y = 0, uint64_t id = 0) :
		Image(w, h, x, y, id) {
		set(ptr);
	}
	Image(int w, int h, vec<T, C>* ptr) :
		Image(w, h, 0, 0, 0) {
		set(ptr);
	}
	Image(std::vector<vec<T, C>>& ref, int w, int h, int x = 0, int y = 0,
		uint64_t id = 0) :
		x(x), y(y), data(ref), width(w), height(h), id(id), channels(C), type(
			I) {
	}
	Image() :
		x(0), y(0), width(0), height(0), id(0), channels(C), type(I) {
	}
	Image(const Image<T, C, I>& img) :
		Image(img.width, img.height, img.x, img.y, img.id) {
		set(img.data);
	}

	Image<T, C, I>& operator=(const Image<T, C, I>& rhs) {
		if (this == &rhs)
			return *this;
		this->resize(rhs.width, rhs.height);
		this->x = rhs.x;
		this->y = rhs.y;
		this->id = rhs.id;
		this->set(rhs.data);
		return *this;
	}
	int2 dimensions() const {
		return int2(width, height);
	}
	int2 position() const {
		return int2(x, y);
	}
	void setPosition(int xx, int yy) {
		x = xx;
		y = yy;
	}
	void setPosition(const int2& pos) {
		x = pos.x;
		y = pos.y;
	}
	size_t size() const {
		return data.size();
	}
	size_t typeSize() const {
		return sizeof(vec<T, C>);
	}
	void resize(int w, int h) {
		data.resize(w * h);
		data.shrink_to_fit();
		width = w;
		height = h;
	}
	inline void clear() {
		data.clear();
		data.shrink_to_fit();
		width = 0;
		height = 0;
	}
	vec<T, C>* vecPtr() {
		if (data.size() == 0)
			return nullptr;
		return data.data();
	}
	const vec<T, C>* vecPtr() const {
		if (data.size() == 0)
			return nullptr;
		return data.data();
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
		data.assign(data.size(), vec<T, C>((T)0));
	}
	const vec<T, C>& operator[](const size_t i) const {
		return data[i];
	}
	vec<T, C>& operator[](const size_t i) {
		return data[i];
	}
	vec<T, C>& operator()(int i, int j) {
		return data[clamp(i, 0, width - 1) + clamp(j, 0, height - 1) * width];
	}
	vec<T, C>& operator()(const int2 ij) {
		return data[clamp(ij.x, 0, width - 1)
			+ clamp(ij.y, 0, height - 1) * width];
	}
	const vec<T, C>& operator()(int i, int j) const {
		return data[clamp(i, 0, width - 1) + clamp(j, 0, height - 1) * width];
	}
	const vec<T, C>& operator()(const int2 ij) const {
		return data[clamp(ij.x, 0, width - 1)
			+ clamp(ij.y, 0, height - 1) * width];
	}

	vec<float, C> operator()(float x, float y) {
		int i = static_cast<int>(std::floor(x));
		int j = static_cast<int>(std::floor(y));
		vec<float, C> rgb00 = vec<float, C>(operator()(i, j));
		vec<float, C> rgb10 = vec<float, C>(operator()(i + 1, j));
		vec<float, C> rgb11 = vec<float, C>(operator()(i + 1, j + 1));
		vec<float, C> rgb01 = vec<float, C>(operator()(i, j + 1));
		float dx = x - i;
		float dy = y - j;
		return ((rgb00 * (1.0f - dx) + rgb10 * dx) * (1.0f - dy)
			+ (rgb01 * (1.0f - dx) + rgb11 * dx) * dy);
	}
	vec<float, C> operator()(float x, float y) const {
		int i = static_cast<int>(std::floor(x));
		int j = static_cast<int>(std::floor(y));
		vec<float, C> rgb00 = vec<float, C>(operator()(i, j));
		vec<float, C> rgb10 = vec<float, C>(operator()(i + 1, j));
		vec<float, C> rgb11 = vec<float, C>(operator()(i + 1, j + 1));
		vec<float, C> rgb01 = vec<float, C>(operator()(i, j + 1));
		float dx = x - i;
		float dy = y - j;
		return ((rgb00 * (1.0f - dx) + rgb10 * dx) * (1.0f - dy)
			+ (rgb01 * (1.0f - dx) + rgb11 * dx) * dy);
	}
	matrix<float, C, 2> gradient(float x, float y) const {
		vec<float, C> v21 = vec<float, C>(operator()(x + 1, y));
		vec<float, C> v12 = vec<float, C>(operator()(x, y + 1));
		vec<float, C> v10 = vec<float, C>(operator()(x, y - 1));
		vec<float, C> v01 = vec<float, C>(operator()(x - 1, y));
		vec<float, C> dx = ((v21 - v01) * 0.5f);
		vec<float, C> dy = ((v12 - v10) * 0.5f);
		matrix<float, C, 2> G;
		G.x = dx;
		G.y = dy;
		return G;
	}
	matrix<double, C, 2> gradient(double x, double y) const {
		vec<double, C> v21 = vec<double, C>(operator()(x + 1, y));
		vec<double, C> v12 = vec<double, C>(operator()(x, y + 1));
		vec<double, C> v10 = vec<double, C>(operator()(x, y - 1));
		vec<double, C> v01 = vec<double, C>(operator()(x - 1, y));
		vec<double, C> dx = ((v21 - v01) * 0.5);
		vec<double, C> dy = ((v12 - v10) * 0.5);
		matrix<double, C, 2> G;
		G.x = dx;
		G.y = dy;
		return G;
	}
	matrix<T, C, 2> gradient(int x, int y) const {
		vec<T, C> v21 = operator()(x + 1, y);
		vec<T, C> v12 = operator()(x, y + 1);
		vec<T, C> v10 = operator()(x, y - 1);
		vec<T, C> v01 = operator()(x - 1, y);
		vec<T, C> dx = ((v21 - v01) * T(0.5));
		vec<T, C> dy = ((v12 - v10) * T(0.5));
		matrix<T, C, 2> G;
		G.x = dx;
		G.y = dy;
		return G;
	}
	vec<double, C> operator()(double x, double y) {
		int i = static_cast<int>(std::floor(x));
		int j = static_cast<int>(std::floor(y));
		vec<double, C> rgb00 = vec<double, C>(operator()(i, j));
		vec<double, C> rgb10 = vec<double, C>(operator()(i + 1, j));
		vec<double, C> rgb11 = vec<double, C>(operator()(i + 1, j + 1));
		vec<double, C> rgb01 = vec<double, C>(operator()(i, j + 1));
		double dx = x - i;
		double dy = y - j;
		return ((rgb00 * (1.0 - dx) + rgb10 * dx) * (1.0 - dy)
				+ (rgb01 * (1.0 - dx) + rgb11 * dx) * dy);
	}
	vec<double, C> operator()(double x, double y) const {
		int i = static_cast<int>(std::floor(x));
		int j = static_cast<int>(std::floor(y));
		vec<double, C> rgb00 = vec<double, C>(operator()(i, j));
		vec<double, C> rgb10 = vec<double, C>(operator()(i + 1, j));
		vec<double, C> rgb11 = vec<double, C>(operator()(i + 1, j + 1));
		vec<double, C> rgb01 = vec<double, C>(operator()(i, j + 1));
		double dx = x - i;
		double dy = y - j;
		return ((rgb00 * (1.0 - dx) + rgb10 * dx) * (1.0 - dy)
				+ (rgb01 * (1.0 - dx) + rgb11 * dx) * dy);
	}
	inline vec<float, C> operator()(const vec<float, 2>& pt) {
		return operator()(pt.x, pt.y);
	}
	inline vec<double, C> operator()(const vec<double, 2>& pt) {
		return operator()(pt.x, pt.y);
	}
	inline vec<float, C> operator()(const vec<float, 2>& pt) const {
		return operator()(pt.x, pt.y);
	}
	inline vec<double, C> operator()(const vec<double, 2>& pt) const {
		return operator()(pt.x, pt.y);
	}
	template<class F> void apply(F f) {
		size_t sz = size();
#pragma omp parallel for
		for (int offset = 0; offset < (int) sz; offset++) {
			f(offset, data[offset]);
		}
	}
	void downSample(Image<T, C, I>& out) const {
		static const double Kernel[5][5] = { { 1, 4, 6, 4, 1 }, { 4, 16, 24, 16,
				4 }, { 6, 24, 36, 24, 6 }, { 4, 16, 24, 16, 4 },
				{ 1, 4, 6, 4, 1 } };
		out.resize(width / 2, height / 2);
#pragma omp parallel for
		for (int i = 0; i < out.width; i++) {
			for (int j = 0; j < out.height; j++) {
				vec<double, C> vsum(0.0);
				for (int ii = 0; ii < 5; ii++) {
					for (int jj = 0; jj < 5; jj++) {
						vsum += Kernel[ii][jj]
								* vec<double, C>(
										operator()(2 * i + ii - 2,
												2 * j + jj - 2));
					}
				}
				out(i, j) = vec<T, C>(vsum / 256.0);
			}
		}
	}
	void upSample(Image<T, C, I>& out) const {
		static const double Kernel[5][5] = { { 1, 4, 6, 4, 1 }, { 4, 16, 24, 16,
				4 }, { 6, 24, 36, 24, 6 }, { 4, 16, 24, 16, 4 },
				{ 1, 4, 6, 4, 1 } };
		if (out.size() == 0)
			out.resize(width * 2, height * 2);
#pragma omp parallel for
		for (int i = 0; i < out.width; i++) {
			for (int j = 0; j < out.height; j++) {
				vec<double, C> vsum(0.0);
				for (int ii = 0; ii < 5; ii++) {
					for (int jj = 0; jj < 5; jj++) {
						int iii = i + ii - 2;
						int jjj = j + jj - 2;
						if (iii % 2 == 0 && jjj % 2 == 0) {
							vsum += Kernel[ii][jj]
									* vec<double, C>(
											operator()(iii / 2, jjj / 2));
						}
					}
				}
				out(i, j) = vec<T, C>(vsum / 64.0);
			}
		}
	}
	Image<T, C, I> downSample() const {
		Image<T, C, I> out;
		downSample(out);
		return out;
	}
	Image<T, C, I> upSample() const {
		Image<T, C, I> out;
		upSample(out);
		return out;
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
	vec<T, C> absMean() const {
		vec<double, C> mean(0.0);
		for (const vec<T, C>& val : data) {
			mean += vec<double, C>(aly::abs(val));
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
template<class T, int C, ImageType I> std::string Image<T, C, I>::updateHashCode(
		size_t MAX_SAMPLES, HashMethod method) {
	if (MAX_SAMPLES == 0) {
		hashCode = HashCode(data, method);
	} else {
		const size_t seed = 83128921L;
		std::mt19937 mt(seed);
		std::uniform_int_distribution<int> wSampler(0, width - 1);
		std::uniform_int_distribution<int> hSampler(0, height - 1);
		std::vector<vec<T, C>> sample(MAX_SAMPLES);
		for (int i = 0; i < (int) MAX_SAMPLES; i++) {
			sample[i] = this->operator()(wSampler(mt), hSampler(mt));
		}
		hashCode = HashCode(sample, method);
	}
	return hashCode;
}
template<class T, int C, ImageType I> void Transform(Image<T, C, I>& im1,
		Image<T, C, I>& im2,
		const std::function<void(vec<T, C>&, vec<T, C>&)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "Image dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
	size_t sz = im1.size();
#pragma omp parallel for
	for (int offset = 0; offset < (int) sz; offset++) {
		func(im1.data[offset], im2.data[offset]);
	}
}
template<class T, int C, ImageType I> void Transform(Image<T, C, I>& im1,
		const std::function<void(vec<T, C>&)>& func) {
	size_t sz = im1.size();
#pragma omp parallel for
	for (int offset = 0; offset < (int) sz; offset++) {
		func(im1.data[offset]);
	}
}
template<class T, int C, ImageType I> void Transform(Image<T, C, I>& im1,
		const Image<T, C, I>& im2,
		const std::function<void(vec<T, C>&, const vec<T, C>&)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "Image dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
	size_t sz = im1.size();
#pragma omp parallel for
	for (int offset = 0; offset < (int) sz; offset++) {
		func(im1.data[offset], im2.data[offset]);
	}
}
template<class T, int C, ImageType I> void Transform(Image<T, C, I>& im1,
		const Image<T, C, I>& im2, const Image<T, C, I>& im3,
		const std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "Image dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
	size_t sz = im1.size();
#pragma omp parallel for
	for (int offset = 0; offset < (int) sz; offset++) {
		func(im1.data[offset], im2.data[offset], im3.data[offset]);
	}
}
template<class T, int C, ImageType I> void Transform(Image<T, C, I>& im1,
		const Image<T, C, I>& im2, const Image<T, C, I>& im3,
		const Image<T, C, I>& im4,
		const std::function<
				void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&,
						const vec<T, C>&)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "Image dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
	size_t sz = im1.size();
#pragma omp parallel for
	for (int offset = 0; offset < (int) sz; offset++) {
		func(im1.data[offset], im2.data[offset], im3.data[offset],
				im4.data[offset]);
	}
}
template<class T, int C, ImageType I> void Transform(Image<T, C, I>& im1,
		Image<T, C, I>& im2,
		const std::function<void(int i, int j, vec<T, C>& val1, vec<T, C>& val2)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "Image dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
#pragma omp parallel for
	for (int j = 0; j < im1.height; j++) {
		for (int i = 0; i < im1.width; i++) {
			size_t offset = i + j * im1.width;
			func(i, j, im1.data[offset], im2.data[offset]);
		}
	}
}
template<class T, int C, ImageType I> void Transform(Image<T, C, I>& im1,
		Image<T, C, I>& im2,
		const std::function<
				void(size_t offset, vec<T, C>& val1, vec<T, C>& val2)>& func) {
	if (im1.dimensions() != im2.dimensions())
		throw std::runtime_error(
				MakeString() << "Image dimensions do not match. "
						<< im1.dimensions() << "!=" << im2.dimensions());
	size_t sz = im1.size();
#pragma omp parallel for
	for (int offset = 0; offset < (int) sz; offset++) {
		func(offset, im1.data[offset], im2.data[offset]);
	}
}
template<class T, class L, class R, int C, ImageType I> std::basic_ostream<L, R> & operator <<(
		std::basic_ostream<L, R> & ss, const Image<T, C, I> & A) {
	ss << "Image (" << A.getTypeName() << "): " << A.id << " Position: "
			<< A.position() << " Dimensions: [" << A.width << "," << A.height
			<< "]";
	return ss;
}
template<class T, int C, ImageType I> Image<T, C, I> operator+(
		const vec<T, C>& scalar, const Image<T, C, I>& img) {
	Image<T, C, I> out(img.width, img.height, img.position());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T,C>& val1,const vec<T,C>& val2) {val1=scalar+val2;};
	Transform(out, img, f);
	return out;
}

template<class T, int C, ImageType I> Image<T, C, I> operator-(
		const vec<T, C>& scalar, const Image<T, C, I>& img) {
	Image<T, C, I> out(img.width, img.height, img.position());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T,C>& val1,const vec<T,C>& val2) {val1=scalar-val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C, ImageType I> Image<T, C, I> operator*(
		const vec<T, C>& scalar, const Image<T, C, I>& img) {
	Image<T, C, I> out(img.width, img.height, img.position());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T,C>& val1,const vec<T,C>& val2) {val1=scalar*val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C, ImageType I> Image<T, C, I> operator/(
		const vec<T, C>& scalar, const Image<T, C, I>& img) {
	Image<T, C, I> out(img.width, img.height, img.position());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T,C>& val1,const vec<T,C>& val2) {val1=scalar/val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C, ImageType I> Image<T, C, I> operator+(
		const Image<T, C, I>& img, const vec<T, C>& scalar) {
	Image<T, C, I> out(img.width, img.height, img.position());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T,C>& val1,const vec<T,C>& val2) {val1=val2+scalar;};
	Transform(out, img, f);
	return out;
}
template<class T, int C, ImageType I> Image<T, C, I> operator-(
		const Image<T, C, I>& img, const vec<T, C>& scalar) {
	Image<T, C, I> out(img.width, img.height, img.position());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T,C>& val1,const vec<T,C>& val2) {val1=val2-scalar;};
	Transform(out, img, f);
	return out;
}
template<class T, int C, ImageType I> Image<T, C, I> operator*(
		const Image<T, C, I>& img, const vec<T, C>& scalar) {
	Image<T, C, I> out(img.width, img.height, img.position());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T,C>& val1,const vec<T,C>& val2) {val1=val2*scalar;};
	Transform(out, img, f);
	return out;
}
template<class T, int C, ImageType I> Image<T, C, I> operator/(
		const Image<T, C, I>& img, const vec<T, C>& scalar) {
	Image<T, C, I> out(img.width, img.height, img.position());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T,C>& val1,const vec<T,C>& val2) {val1=val2/scalar;};
	Transform(out, img, f);
	return out;
}
template<class T, int C, ImageType I> Image<T, C, I> operator-(
		const Image<T, C, I>& img) {
	Image<T, C, I> out(img.width, img.height, img.position());
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T,C>& val1,const vec<T,C>& val2) {val1=-val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C, ImageType I> Image<T, C, I> operator+=(
		Image<T, C, I>& out, const Image<T, C, I>& img) {
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T,C>& val1,const vec<T,C>& val2) {val1+=val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C, ImageType I> Image<T, C, I> operator-=(
		Image<T, C, I>& out, const Image<T, C, I>& img) {
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T,C>& val1,const vec<T,C>& val2) {val1-=val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C, ImageType I> Image<T, C, I> operator*=(
		Image<T, C, I>& out, const Image<T, C, I>& img) {
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T,C>& val1,const vec<T,C>& val2) {val1*=val2;};
	Transform(out, img, f);
	return out;
}
template<class T, int C, ImageType I> Image<T, C, I> operator/=(
		Image<T, C, I>& out, const Image<T, C, I>& img) {
	std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T,C>& val1,const vec<T,C>& val2) {val1/=val2;};
	Transform(out, img, f);
	return out;
}

template<class T, int C, ImageType I> Image<T, C, I> operator+=(
		Image<T, C, I>& out, const vec<T, C>& scalar) {
	std::function<void(vec<T, C>&)> f = [=](vec<T,C>& val1) {val1+=scalar;};
	Transform(out, f);
	return out;
}
template<class T, int C, ImageType I> Image<T, C, I> operator-=(
		Image<T, C, I>& out, const vec<T, C>& scalar) {
	std::function<void(vec<T, C>&)> f = [=](vec<T,C>& val1) {val1-=scalar;};
	Transform(out, f);
	return out;
}
template<class T, int C, ImageType I> Image<T, C, I> operator*=(
		Image<T, C, I>& out, const vec<T, C>& scalar) {
	std::function<void(vec<T, C>&)> f = [=](vec<T,C>& val1) {val1*=scalar;};
	Transform(out, f);
	return out;
}
template<class T, int C, ImageType I> Image<T, C, I> operator/=(
		Image<T, C, I>& out, const vec<T, C>& scalar) {
	std::function<void(vec<T, C>&)> f = [=](vec<T,C>& val1) {val1/=scalar;};
	Transform(out, f);
	return out;
}

template<class T, int C, ImageType I> Image<T, C, I> operator+(
		const Image<T, C, I>& img1, const Image<T, C, I>& img2) {
	Image<T, C, I> out(img1.width, img1.height);
	std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T,C>& val1,const vec<T,C>& val2,const vec<T,C>& val3) {val1=val2+val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T, int C, ImageType I> Image<T, C, I> operator-(
		const Image<T, C, I>& img1, const Image<T, C, I>& img2) {
	Image<T, C, I> out(img1.width, img1.height);
	std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T,C>& val1,const vec<T,C>& val2,const vec<T,C>& val3) {val1=val2-val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T, int C, ImageType I> Image<T, C, I> operator*(
		const Image<T, C, I>& img1, const Image<T, C, I>& img2) {
	Image<T, C, I> out(img1.width, img1.height);
	std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T,C>& val1,const vec<T,C>& val2,const vec<T,C>& val3) {val1=val2*val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T, int C, ImageType I> Image<T, C, I> operator/(
		const Image<T, C, I>& img1, const Image<T, C, I>& img2) {
	Image<T, C, I> out(img1.width, img1.height);
	std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T,C>& val1,const vec<T,C>& val2,const vec<T,C>& val3) {val1=val2/val3;};
	Transform(out, img1, img2, f);
	return out;
}
template<class T, int C, ImageType I> void WriteImageToRawFile(
		const std::string& file, const Image<T, C, I>& img) {
	std::ostringstream vstr;
	std::string fileName = GetFileWithoutExtension(file);
	vstr << fileName << ".raw";
	FILE* f = fopen(vstr.str().c_str(), "wb");
	if (f == NULL) {
		throw std::runtime_error(
				MakeString() << "Could not open " << vstr.str().c_str()
						<< " for writing.");
	}
	for (int c = 0; c < img.channels; c++) {
		for (int j = 0; j < img.height; j++) {
			for (int i = 0; i < img.width; i++) {
				T val = img(i, j)[c];
				fwrite(&val, sizeof(T), 1, f);
			}
		}
	}
	fclose(f);
	std::string typeName = "";
	switch (img.type) {
	case ImageType::BYTE:
		typeName = "Byte";
		break;
	case ImageType::UBYTE:
		typeName = "Unsigned Byte";
		break;
	case ImageType::SHORT:
		typeName = "Short";
		break;
	case ImageType::USHORT:
		typeName = "Unsigned Short";
		break;
	case ImageType::INT:
		typeName = "Integer";
		break;
	case ImageType::UINT:
		typeName = "Unsigned Integer";
		break;
	case ImageType::FLOAT:
		typeName = "Float";
		break;
	case ImageType::DOUBLE:
		typeName = "Double";
		break;
	case ImageType::UNKNOWN:
		typeName = "Unknown";
		break;
	default:
		break;
	}
//std::cout << vstr.str() << std::endl;
	std::stringstream sstr;
	sstr << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	sstr << "<!-- MIPAV header file -->\n";
	sstr
			<< "<image xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" nDimensions=\"3\">\n";
	sstr << "	<Dataset-attributes>\n";
	sstr << "		<Image-offset>0</Image-offset>\n";
	sstr << "		<Data-type>" << typeName << "</Data-type>\n";
	sstr << "		<Endianess>Little</Endianess>\n";
	sstr << "		<Extents>" << img.width << "</Extents>\n";
	sstr << "		<Extents>" << img.height << "</Extents>\n";
	sstr << "		<Extents>" << img.channels << "</Extents>\n";
	sstr << "		<Resolutions>\n";
	sstr << "			<Resolution>1.0</Resolution>\n";
	sstr << "			<Resolution>1.0</Resolution>\n";
	sstr << "			<Resolution>1.0</Resolution>\n";
	sstr << "		</Resolutions>\n";
	sstr << "		<Slice-spacing>1.0</Slice-spacing>\n";
	sstr << "		<Slice-thickness>0.0</Slice-thickness>\n";
	sstr << "		<Units>Millimeters</Units>\n";
	sstr << "		<Units>Millimeters</Units>\n";
	sstr << "		<Units>Millimeters</Units>\n";
	sstr << "		<Compression>none</Compression>\n";
	sstr << "		<Orientation>Unknown</Orientation>\n";
	sstr << "		<Subject-axis-orientation>Unknown</Subject-axis-orientation>\n";
	sstr << "		<Subject-axis-orientation>Unknown</Subject-axis-orientation>\n";
	sstr << "		<Subject-axis-orientation>Unknown</Subject-axis-orientation>\n";
	sstr << "		<Origin>0.0</Origin>\n";
	sstr << "		<Origin>0.0</Origin>\n";
	sstr << "		<Origin>0.0</Origin>\n";
	sstr << "		<Modality>Unknown Modality</Modality>\n";
	sstr << "	</Dataset-attributes>\n";
	sstr << "</image>\n";
	std::ofstream myfile;
	std::stringstream xmlFile;
	xmlFile << fileName << ".xml";
	myfile.open(xmlFile.str().c_str(), std::ios_base::out);
	if (!myfile.is_open()) {
		throw std::runtime_error(
				MakeString() << "Could not open " << xmlFile.str()
						<< " for writing.");
	}
	myfile << sstr.str();
	myfile.close();
}
typedef Image<uint8_t, 4, ImageType::UBYTE> ImageRGBA;
typedef Image<int, 4, ImageType::INT> ImageRGBAi;
typedef Image<float, 4, ImageType::FLOAT> ImageRGBAf;

typedef Image<uint8_t, 3, ImageType::UBYTE> ImageRGB;
typedef Image<int, 3, ImageType::INT> ImageRGBi;
typedef Image<float, 3, ImageType::FLOAT> ImageRGBf;

typedef Image<uint8_t, 1, ImageType::UBYTE> ImageA;
typedef Image<int, 1, ImageType::INT> ImageAi;
typedef Image<float, 1, ImageType::FLOAT> ImageAf;

typedef Image<int8_t, 4, ImageType::BYTE> Image4b;
typedef Image<uint8_t, 4, ImageType::UBYTE> Image4ub;
typedef Image<uint16_t, 4, ImageType::USHORT> Image4us;
typedef Image<int16_t, 4, ImageType::SHORT> Image4s;
typedef Image<int, 4, ImageType::INT> Image4i;
typedef Image<uint32_t, 4, ImageType::UINT> Image4ui;
typedef Image<float, 4, ImageType::FLOAT> Image4f;

typedef Image<int8_t, 3, ImageType::BYTE> Image3b;
typedef Image<uint8_t, 3, ImageType::UBYTE> Image3ub;
typedef Image<uint16_t, 3, ImageType::USHORT> Image3us;
typedef Image<int16_t, 3, ImageType::SHORT> Image3s;
typedef Image<int, 3, ImageType::INT> Image3i;
typedef Image<uint32_t, 3, ImageType::UINT> Image3ui;
typedef Image<float, 3, ImageType::FLOAT> Image3f;

typedef Image<int8_t, 2, ImageType::BYTE> Image2b;
typedef Image<uint8_t, 2, ImageType::UBYTE> Image2ub;
typedef Image<uint16_t, 2, ImageType::USHORT> Image2us;
typedef Image<int16_t, 2, ImageType::SHORT> Image2s;
typedef Image<int, 2, ImageType::INT> Image2i;
typedef Image<uint32_t, 2, ImageType::UINT> Image2ui;
typedef Image<float, 2, ImageType::FLOAT> Image2f;


typedef Image<int8_t, 1, ImageType::BYTE> Image1b;
typedef Image<uint8_t, 1, ImageType::UBYTE> Image1ub;
typedef Image<uint16_t, 1, ImageType::USHORT> Image1us;
typedef Image<int16_t, 1, ImageType::SHORT> Image1s;

typedef Image<int, 1, ImageType::INT> Image1i;
typedef Image<uint32_t, 1, ImageType::UINT> Image1ui;
typedef Image<float, 1, ImageType::FLOAT> Image1f;

void WriteImageToFile(const std::string& file, const ImageRGBA& img);
void WriteImageToFile(const std::string& file, const ImageRGB& img);
void WriteImageToFile(const std::string& file, const ImageRGBAf& img);
void WriteImageToFile(const std::string& file, const ImageRGBf& img);

void WriteImageToFile(const std::string& file, const Image1f& img);
void WriteImageToFile(const std::string& file, const Image1ub& img);
void ReadImageFromFile(const std::string& file, Image1ub& img);
void ReadImageFromFile(const std::string& file, Image1f& img);

void ReadImageFromFile(const std::string& file, ImageRGBA& img);
void ReadImageFromFile(const std::string& file, ImageRGB& img);
void ReadImageFromFile(const std::string& file, ImageRGBAf& img);
void ReadImageFromFile(const std::string& file, ImageRGBf& img);

void ConvertImage(const ImageRGBAf& in, ImageRGBA& out);
void ConvertImage(const ImageRGBf& in, ImageRGB& out);
void ConvertImage(const ImageRGBA& in, ImageRGBAf& out);
void ConvertImage(const ImageRGB& in, ImageRGBf& out);
void ConvertImage(const ImageRGBA& in, ImageRGB& out);
void ConvertImage(const ImageRGBAf& in, ImageRGBf& out);
void ConvertImage(const ImageRGB& in, ImageRGBA& out);
void ConvertImage(const ImageRGBf& in, ImageRGBAf& out);

template<class T, ImageType I> void ConvertImage(const Image<T, 4, I>& in,
		Image<T, 1, I>& out, bool sRGB = true) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	int N = (int) out.size();

	if (sRGB) {
#pragma omp parallel for
		for (int i = 0; i < N; i++) {
			vec<T, 4> c = in[i];
			out[i] = vec<T, 1>(T(0.21 * c.x + 0.72 * c.y + 0.07 * c.z));
		}
	} else {
#pragma omp parallel for
		for (int i = 0; i < N; i++) {
			vec<T, 4> c = in[i];
			out[i] = vec<T, 1>(T(0.30 * c.x + 0.59 * c.y + 0.11 * c.z));
		}
	}
}
template<class T, ImageType I> void ConvertImage(const Image<T, 4, I>& in,
		Image<T, 2, I>& out, bool sRGB = true) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	int N = out.size();

	if (sRGB) {
#pragma omp parallel for
		for (int i = 0; i < N; i++) {
			vec<T, 4> c = in[i];
			out[i] = vec<T, 2>(T(0.21 * c.x + 0.72 * c.y + 0.07 * c.z), c.w);
		}
	} else {
#pragma omp parallel for
		for (int i = 0; i < N; i++) {
			vec<T, 4> c = in[i];
			out[i] = vec<T, 2>(T(0.30 * c.x + 0.59 * c.y + 0.11 * c.z), c.w);
		}
	}
}
template<class T, ImageType I> void ConvertImage(const Image<T, 3, I>& in,
		Image<T, 1, I>& out, bool sRGB = true) {
	out.resize(in.width, in.height);
	out.id = in.id;
	out.setPosition(in.position());
	int N = (int) out.size();

	if (sRGB) {
#pragma omp parallel for
		for (int i = 0; i < N; i++) {
			vec<T, 3> c = in[i];
			out[i] = vec<T, 1>(T(0.21 * c.x + 0.72 * c.y + 0.07 * c.z));
		}
	} else {
#pragma omp parallel for
		for (int i = 0; i < N; i++) {
			vec<T, 3> c = in[i];
			out[i] = vec<T, 1>(T(0.30 * c.x + 0.59 * c.y + 0.11 * c.z));
		}
	}
}
inline void MakeCheckerBoard(ImageRGBA& img, int horizTiles,int vertTiles){
	const int width=img.width;
	const int height=img.height;
	const int cellWidth = width / horizTiles;
	const int cellHeight = height / vertTiles;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			bool vt = (i / cellWidth) % 2 == 0;
			bool ht = (j / cellHeight) % 2 == 0;
			img(i, j) =((vt && !ht) || (!vt && ht)) ?aly::RGBA(0, 0, 0, 0) : aly::RGBA(255, 255, 255, 255);
		}
	}
}
inline void MakeCheckerBoard(ImageRGB& img, int horizTiles,int vertTiles){
	const int width=img.width;
	const int height=img.height;
	const int cellWidth = width / horizTiles;
	const int cellHeight = height / vertTiles;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			bool vt = (i / cellWidth) % 2 == 0;
			bool ht = (j / cellHeight) % 2 == 0;
			img(i, j) =((vt && !ht) || (!vt && ht)) ?aly::ubyte3(0, 0, 0) : aly::ubyte3(255, 255, 255);
		}
	}
}
inline void MakeCheckerBoard(ImageRGBAf& img, int horizTiles,int vertTiles){
	const int width=img.width;
	const int height=img.height;
	const int cellWidth = width / horizTiles;
	const int cellHeight = height / vertTiles;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			bool vt = (i / cellWidth) % 2 == 0;
			bool ht = (j / cellHeight) % 2 == 0;
			img(i, j) =((vt && !ht) || (!vt && ht)) ?RGBAf(0, 0, 0, 0) : RGBAf(1.0f,1.0f,1.0f,1.0f);
		}
	}
}
inline void MakeCheckerBoard(ImageRGBf& img, int horizTiles,int vertTiles){
	const int width=img.width;
	const int height=img.height;
	const int cellWidth = width / horizTiles;
	const int cellHeight = height / vertTiles;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			bool vt = (i / cellWidth) % 2 == 0;
			bool ht = (j / cellHeight) % 2 == 0;
			img(i, j) =((vt && !ht) || (!vt && ht)) ?RGBf(0, 0, 0) : RGBf(1.0f,1.0f,1.0f);
		}
	}
}
template<class T, int C, ImageType I> void Crop(const Image<T, C, I>& in,
		Image<T, C, I>& out, int2 pos, int2 dims) {
	out.setPosition(pos);
	out.resize(dims.x, dims.y);
	for (int i = 0; i < dims.x; i++) {
		for (int j = 0; j < dims.y; j++) {
			out(i, j) = in(pos.x + i, pos.y + j);
		}
	}
}
template<class T, int C, ImageType I> void FlipVertical(Image<T, C, I>& in) {
#pragma omp parallel for
	for (int i = 0; i < in.width; i++) {
		for (int j = 0; j < in.height / 2; j++) {
			std::swap(in(i, j), in(i, in.height - 1 - j));
		}
	}
}
template<class T, int C, ImageType I> void FlipHorizontal(Image<T, C, I>& in) {
#pragma omp parallel for
	for (int j = 0; j < in.height; j++) {
		for (int i = 0; i < in.width / 2; i++) {
			std::swap(in(i, j), in(in.width - 1 - i, j));
		}
	}
}
template<class T, int C, ImageType I> void DownSample(const Image<T, C, I>& in,
		Image<T, C, I>& out) {
	static const double Kernel[5][5] = { { 1, 4, 6, 4, 1 },
			{ 4, 16, 24, 16, 4 }, { 6, 24, 36, 24, 6 }, { 4, 16, 24, 16, 4 }, {
					1, 4, 6, 4, 1 } };
	out.resize(in.width / 2, in.height / 2);
#pragma omp parallel for
	for (int i = 0; i < out.width; i++) {
		for (int j = 0; j < out.height; j++) {
			vec<double, C> vsum(0.0);
			for (int ii = 0; ii < 5; ii++) {
				for (int jj = 0; jj < 5; jj++) {
					vsum += Kernel[ii][jj]
							* vec<double, C>(
									in(2 * i + ii - 2, 2 * j + jj - 2));
				}
			}
			out(i, j) = vec<T, C>(vsum / 256.0);
		}
	}
}
template<class T, int C, ImageType I> void UpSample(const Image<T, C, I>& in,
		Image<T, C, I>& out) {
	static const double Kernel[5][5] = { { 1, 4, 6, 4, 1 },
			{ 4, 16, 24, 16, 4 }, { 6, 24, 36, 24, 6 }, { 4, 16, 24, 16, 4 }, {
					1, 4, 6, 4, 1 } };
	if (out.size() == 0)
		out.resize(in.width * 2, in.height * 2);
#pragma omp parallel for
	for (int i = 0; i < out.width; i++) {
		for (int j = 0; j < out.height; j++) {
			vec<double, C> vsum(0.0);
			for (int ii = 0; ii < 5; ii++) {
				for (int jj = 0; jj < 5; jj++) {
					int iii = i + ii - 2;
					int jjj = j + jj - 2;
					if (iii % 2 == 0 && jjj % 2 == 0) {
						vsum += Kernel[ii][jj]
								* vec<double, C>(in(iii / 2, jjj / 2));
					}
				}
			}
			out(i, j) = vec<T, C>(vsum / 64.0);
		}
	}
}
template<class T, int C, ImageType I> void Set(const Image<T, C, I>& in,
		Image<T, C, I>& out, int2 pos) {
	for (int i = 0; i < in.width; i++) {
		for (int j = 0; j < in.height; j++) {
			if (pos.x + i >= 0 && pos.x + i < out.width && pos.y + j >= 0
					&& pos.y + j < out.height) {
				out(pos.x + i, pos.y + j) = in(i, j);
			}
		}
	}
}
template<class T, int C, ImageType I> void Tile(
		const std::vector<Image<T, C, I>>& in, Image<T, C, I>& out, int rows,
		int cols) {
	int index = 0;
	int maxX = 0;
	int maxY = 0;
	std::vector<int> lines(rows, 0);
	for (int r = 0; r < rows; r++) {
		int runX = 0;
		int runY = 0;
		for (int c = 0; c < cols; c++) {
			if (index >= (int)in.size())
				break;
			const Image<T, C, I>& img = in[index++];
			runX += img.width;
			runY = std::max(runY, img.height);
		}
		maxX = std::max(runX, maxX);
		lines[r] = maxY;
		maxY += runY;
		if (index >= (int)in.size())
			break;
	}
	out.resize(maxX, maxY);
	out.set(vec<T, C>(T(0)));
	index = 0;
	for (int r = 0; r < rows; r++) {
		int runX = 0;
		int runY = lines[r];
		for (int c = 0; c < cols; c++) {
			if (index >= (int)in.size())
				break;
			const Image<T, C, I>& img = in[index++];
			Set(img, out, int2(runX, runY));
			runX += img.width;
		}
		if (index >= (int)in.size())
			break;
	}
}
template<class T, int C, ImageType I> void Tile(
		const std::initializer_list<Image<T, C, I>>& in, Image<T, C, I>& out,
		int rows, int cols) {
	int maxX = 0;
	int maxY = 0;
	std::vector<int> lines(rows, 0);
	{
		auto iter = in.begin();
		for (int r = 0; r < rows; r++) {
			int runX = 0;
			int runY = 0;
			for (int c = 0; c < cols; c++) {
				if (iter == in.end())
					break;
				const Image<T, C, I>& img = *iter;
				iter++;
				runX += img.width;
				runY = std::max(runY, img.height);
			}
			maxX = std::max(runX, maxX);
			lines[r] = maxY;
			maxY += runY;
			if (iter == in.end())
				break;
		}
	}
	out.resize(maxX, maxY);
	out.set(vec<T, C>(T(0)));
	{
		auto iter = in.begin();
		for (int r = 0; r < rows; r++) {
			int runX = 0;
			int runY = lines[r];
			for (int c = 0; c < cols; c++) {
				if (iter == in.end())
					break;
				const Image<T, C, I>& img = *iter;
				iter++;
				Set(img, out, int2(runX, runY));
				runX += img.width;
			}
			if (iter == in.end())
				break;
		}
	}
}
void ConvertImage(const ImageRGBAf& in, Image1ub& out, bool sRGB = true);
void ConvertImage(const ImageRGBf& in, Image1ub& out, bool sRGB = true);
void ConvertImage(const ImageRGB& in, Image1f& out, bool sRGB = true);
void ConvertImage(const ImageRGBA& in, Image1f& out, bool sRGB = true);

void ConvertImage(const Image1f& in, ImageRGBAf& out);
void ConvertImage(const Image2f& in, ImageRGBAf& out);
void ConvertImage(const Image1f& in, ImageRGBf& out);
void ConvertImage(const Image1b& in, ImageRGBAf& out);
void ConvertImage(const Image1b& in, ImageRGBf& out);
void ConvertImage(const Image1b& in, ImageRGBA& out);
void ConvertImage(const Image1b& in, ImageRGB& out);
void ConvertImage(const Image1f& in, ImageRGBA& out);
void ConvertImage(const Image1f& in, ImageRGB& out);
}
;

#endif
