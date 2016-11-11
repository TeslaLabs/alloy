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
#ifndef ALLOYIMAGE3D_H_INCLUDE_GUARD
#define ALLOYIMAGE3D_H_INCLUDE_GUARD
#include "AlloyCommon.h"
#include "AlloyMath.h"
#include "sha2.h"
#include "AlloyFileUtil.h"
#include "AlloyImage.h"
#include "cereal/types/vector.hpp"
#include <vector>
#include <functional>
#include <fstream>
#include <random>
namespace aly {
	template<class T, int C, ImageType I> struct Volume {
	private:
		std::string hashCode;
		int x, y, z;
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

		int rows;
		int cols;
		int slices;
		uint64_t id;
		const int channels = C;

		const ImageType type = I;
		int3 position() const {
			return int3(x, y, z);
		}
		void setPosition(const int3& pos) {
			x = pos.x;
			y = pos.y;
			z = pos.z;
		}
		std::string updateHashCode(size_t MAX_SAMPLES = 0, HashMethod method =
			HashMethod::SHA256);
		std::string getHashCode() {
			return hashCode;
		}

		template<class Archive> void serialize(Archive & archive) {
			archive(cereal::make_nvp(MakeString() << type << channels, id),
				CEREAL_NVP(rows), CEREAL_NVP(cols), CEREAL_NVP(slices),
				CEREAL_NVP(x), CEREAL_NVP(y), CEREAL_NVP(z),
				CEREAL_NVP(hashCode));
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
		void set(vec<T, C>* val) {
			if (val == nullptr)
				return;
			size_t offset = 0;
			for (vec<T, C>& x : data) {
				x = val[offset++];
			}
		}
		void set(const Volume<T, C, I>& other) {
			resize(other.rows, other.cols, other.slices);
			id = other.id;
			x = other.x;
			y = other.y;
			z = other.z;
			set(&other.data[0]);
		}
		std::string getTypeName() const {
			return MakeString() << type << channels;
		}

		Volume(int r, int c, int s, int x = 0, int y = 0, int z = 0,
			uint64_t id = 0) :
				 x(x), y(y), z(z), data(r * c * s) ,rows(r), cols(c), slices(s), id(id){
		}
		Volume(int r, int c, int s, int3 pos,
			uint64_t id = 0) :
			x(pos.x), y(pos.y), z(pos.z), data(r * c * s) , rows(r), cols(c), slices(s), id(id){
		}
		Volume(T* ptr, int r, int c, int s, int x = 0, int y = 0, int z = 0,
			uint64_t id = 0) :
			Volume(r, c, s, x, y, z, id) {
			set(ptr);
		}
		Volume(vec<T, C>* ptr, int r, int c, int s, int x = 0, int y = 0, int z = 0,
			uint64_t id = 0) :
			Volume(r, c, s, x, y, z, id) {
			set(ptr);
		}
		Volume(std::vector<vec<T, C>>& ref, int r, int c, int s, int x = 0, int y =
			0, int z = 0, uint64_t id = 0) :
			x(x), y(y), z(z), data(ref) , rows(r), cols(c), slices(s), id(id){
		}
		Volume() :
			x(0), y(0), z(0), rows(0), cols(0), slices(0), id(0) {
		}
		Volume(const Volume<T, C, I>& img) :
			Volume(img.rows, img.cols, img.slices, img.position(), img.id) {
			set(img.data);
		}
		Volume<T, C, I>& operator=(const Volume<T, C, I>& rhs) {
			if (this == &rhs)
				return *this;
			this->resize(rhs.rows, rhs.cols, rhs.slices);
			this->setPosition(rhs.position());
			this->id = rhs.id;
			this->set(rhs.data);
			return *this;
		}
		int3 dimensions() const {
			return int3(rows, cols, slices);
		}

		size_t size() const {
			return data.size();
		}
		size_t typeSize() const {
			return sizeof(vec<T, C>);
		}
		void resize(int r, int c, int s) {
			data.resize(r * c * s);
			data.shrink_to_fit();
			rows = r;
			cols = c;
			slices = s;
		}
		inline void clear() {
			data.clear();
			data.shrink_to_fit();
			rows = 0;
			cols = 0;
			slices = 0;
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
		vec<T, C>& operator()(const int i, const int j, const int k) {
			return data[clamp(i, 0, rows - 1) + clamp(j, 0, cols - 1) * rows
				+ clamp(k, 0, slices - 1) * rows * cols];
		}
		vec<T, C>& operator()(const size_t i, const size_t j, const size_t k) {
			return data[clamp((int)i, 0, rows - 1) + clamp((int)j, 0, cols - 1) * rows
				+ clamp((int)k, 0, slices - 1) * rows * cols];
		}
		vec<T, C>& operator()(const int3 ijk) {
			return data[clamp(ijk.x, 0, rows - 1) + clamp(ijk.y, 0, cols - 1) * rows
				+ clamp(ijk.z, 0, slices - 1) * rows * cols];
		}
		const vec<T, C>& operator()(const int i, const int j, const int k) const {
			return data[clamp(i, 0, rows - 1) + clamp(j, 0, cols - 1) * rows
				+ clamp(k, 0, slices - 1) * rows * cols];
		}
		const vec<T, C>& operator()(const size_t i, const size_t j, const size_t k) const {
			return data[clamp((int)i, 0, rows - 1) + clamp((int)j, 0, cols - 1) * rows
				+ clamp((int)k, 0, slices - 1) * rows * cols];
		}
		const vec<T, C>& operator()(const int3 ijk) const {
			return data[clamp(ijk.x, 0, rows - 1) + clamp(ijk.y, 0, cols - 1) * rows
				+ clamp(ijk.z, 0, slices - 1) * rows * cols];
		}
		template<class K> vec<K, C> operator()(const K x, const K y, const K z) {
			int i = static_cast<int>(std::floor(x));
			int j = static_cast<int>(std::floor(y));
			int k = static_cast<int>(std::floor(z));
			vec<K, C> rgb000 = ConvertType<K, T, C>(operator()(i, j, k));
			vec<K, C> rgb100 = ConvertType<K, T, C>(operator()(i + 1, j, k));
			vec<K, C> rgb110 = ConvertType<K, T, C>(operator()(i + 1, j + 1, k));
			vec<K, C> rgb010 = ConvertType<K, T, C>(operator()(i, j + 1, k));

			vec<K, C> rgb001 = ConvertType<K, T, C>(operator()(i, j, k + 1));
			vec<K, C> rgb101 = ConvertType<K, T, C>(operator()(i + 1, j, k + 1));
			vec<K, C> rgb111 = ConvertType<K, T, C>(
				operator()(i + 1, j + 1, k + 1));
			vec<K, C> rgb011 = ConvertType<K, T, C>(operator()(i, j + 1, k + 1));

			K dx = x - i;
			K dy = y - j;
			K dz = z - k;
			vec<K, C> lower = ((rgb000 * (K(1) - dx) + rgb100 * dx) * (K(1) - dy)
				+ (rgb010 * (K(1) - dx) + rgb110 * dx) * dy);
			vec<K, C> upper = ((rgb001 * (K(1) - dx) + rgb101 * dx) * (K(1) - dy)
				+ (rgb011 * (K(1) - dx) + rgb111 * dx) * dy);
			return (K(1) - dz) * lower + dz * upper;

		}
		template<class K> inline vec<K, C> operator()(const vec<K, 3>& pt) {
			return operator()(pt.x, pt.y, pt.z);
		}
		template<class F> void apply(F f) {
			size_t sz = size();
#pragma omp parallel for
			for (int offset = 0; offset < (int)sz; offset++) {
				f(offset, data[offset]);
			}
		}
		void downSample(Volume<T, C, I>& out) const {
			static const double Kernel[3][3][3] = { { { 0, 1, 0 }, { 1, 4, 1 }, { 0,
					1, 0 } }, { { 1, 4, 1 }, { 4, 12, 4 }, { 1, 4, 1 } }, { { 0, 1,
					0 }, { 1, 4, 1 }, { 0, 1, 0 } } };
			out.resize(rows / 2, cols / 2, slices / 2);
#pragma omp parallel for
			for (int i = 0; i < out.rows; i++) {
				for (int j = 0; j < out.cols; j++) {
					for (int k = 0; k < out.slices; k++) {
						vec<double, C> vsum(0.0);
						for (int ii = 0; ii < 3; ii++) {
							for (int jj = 0; jj < 3; jj++) {
								for (int kk = 0; kk < 3; kk++) {
									vsum += Kernel[ii][jj][kk]
										* vec<double, C>(
											operator()(2 * i + ii - 1,
												2 * j + jj - 1,
												2 * k + kk - 1));
								}
							}
						}
						out(i, j, k) = vec<T, C>(vsum / 48.0);
					}
				}
			}
		}
		void upSample(Volume<T, C, I>& out) const {
			static const double Kernel[3][3][3] = { { { 0, 1, 0 }, { 1, 4, 1 }, { 0,
					1, 0 } }, { { 1, 4, 1 }, { 4, 12, 4 }, { 1, 4, 1 } }, { { 0, 1,
					0 }, { 1, 4, 1 }, { 0, 1, 0 } } };
			if (out.size() == 0)
				out.resize(rows * 2, cols * 2, slices * 2);
#pragma omp parallel for
			for (int i = 0; i < out.rows; i++) {
				for (int j = 0; j < out.cols; j++) {
					for (int k = 0; k < out.slices; k++) {
						vec<double, C> vsum(0.0);
						for (int ii = 0; ii < 3; ii++) {
							for (int jj = 0; jj < 3; jj++) {
								for (int kk = 0; kk < 3; kk++) {
									int iii = i + ii - 1;
									int jjj = j + jj - 1;
									int kkk = k + kk - 1;
									if (iii % 2 == 0 && jjj % 2 == 0
										&& kkk % 2 == 0) {
										vsum += Kernel[ii][jj][kk]
											* vec<double, C>(
												operator()(iii / 2, jjj / 2,
													kkk / 2));
									}
								}
							}
							out(i, j, k) = vec<T, C>(vsum / 6.0);
						}
					}
				}
			}
		}
		Volume<T, C, I> downSample() const {
			Volume<T, C, I> out;
			downSample(out);
			return out;
		}
		Volume<T, C, I> upSample() const {
			Volume<T, C, I> out;
			upSample(out);
			return out;
		}
		vec<T, C> min() const {
			vec<T, C> minVal(std::numeric_limits<T>::max());
			for (vec<T, C>& val : data) {
				minVal = aly::minVec(val, minVal);
			}
			return minVal;
		}
		vec<T, C> max() const {
			vec<T, C> maxVal(std::numeric_limits<T>::min());
			for (vec<T, C>& val : data) {
				maxVal = aly::maxVec(val, maxVal);
			}
			return maxVal;
		}
		std::pair<vec<T, C>, vec<T, C>> range() const {
			vec<T, C> maxVal(std::numeric_limits<T>::min());
			vec<T, C> minVal(std::numeric_limits<T>::max());
			for (vec<T, C>& val : data) {
				maxVal = aly::maxVec(val, maxVal);
				minVal = aly::minVec(val, minVal);
			}
			return std::pair<vec<T, C>, vec<T, C>>(minVal, maxVal);
		}
		vec<T, C> mean() const {
			vec<double, C> mean(0.0);
			for (vec<T, C>& val : data) {
				mean += vec<double, C>(val);
			}
			mean = mean / (double)data.size();
			return vec<T, C>(mean);
		}
		vec<T, C> median() const {
			std::vector<T> bands[C];
			for (int c = 0; c < C; c++) {
				bands[c].resize(data.size());
			}
			size_t index = 0;
			for (vec<T, C>& val : data) {
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
						((double)bands[c][data.size() / 2]
							+ (double)bands[c][data.size() / 2 - 1])
						* 0.5f);
				}
			}
			else {
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
			for (vec<T, C>& val : data) {
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
						((double)bands[c][data.size() / 2]
							+ (double)bands[c][data.size() / 2 - 1])
						* 0.5f);
				}
			}
			else {
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
			for (vec<T, C>& val : data) {
				vec<double, C> e = vec<double, C>(val - avg);
				var += e * e;
			}
			var = var / (double)(data.size() - 1);
			return vec<T, C>(aly::sqrt(var));
		}
	}
	;
	template<class T, int C, ImageType I> std::string Volume<T, C, I>::updateHashCode(
		size_t MAX_SAMPLES, HashMethod method) {
		if (MAX_SAMPLES == 0) {
			hashCode = HashCode(data, method);
		}
		else {
			const size_t seed = 8743128921;
			std::mt19937 mt(seed);
			std::uniform_int_distribution<int> wSampler(0, rows - 1);
			std::uniform_int_distribution<int> hSampler(0, cols - 1);
			std::uniform_int_distribution<int> dSampler(0, slices - 1);
			std::vector<vec<T, C>> sample(MAX_SAMPLES);
			for (int i = 0; i < MAX_SAMPLES; i++) {
				sample[i] = this->operator()(wSampler(mt), hSampler(mt),
					dSampler(mt));
			}
			hashCode = HashCode(sample, method);
		}
		return hashCode;
	}
	template<class T, int C, ImageType I> void Transform(Volume<T, C, I>& im1,
		Volume<T, C, I>& im2,
		const std::function<void(vec<T, C>&, vec<T, C>&)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Volume dimensions do not match. "
				<< im1.dimensions() << "!=" << im2.dimensions());
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(im1.data[offset], im2.data[offset]);
		}
	}
	template<class T, int C, ImageType I> void Transform(Volume<T, C, I>& im1,
		const Volume<T, C, I>& im2, const Volume<T, C, I>& im3,
		const Volume<T, C, I>& im4,
		const std::function<
		void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&,
			const vec<T, C>&)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Volume dimensions do not match. "
				<< im1.dimensions() << "!=" << im2.dimensions());
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(im1.data[offset], im2.data[offset], im3.data[offset],
				im4.data[offset]);
		}
	}
	template<class T, int C, ImageType I> void Transform(Volume<T, C, I>& im1,
		const std::function<void(vec<T, C>&)>& func) {
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(im1.data[offset]);
		}
	}
	template<class T, int C, ImageType I> void Transform(Volume<T, C, I>& im1,
		const Volume<T, C, I>& im2,
		const std::function<void(vec<T, C>&, const vec<T, C>&)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Volume dimensions do not match. "
				<< im1.dimensions() << "!=" << im2.dimensions());
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(im1.data[offset], im2.data[offset]);
		}
	}
	template<class T, int C, ImageType I> void Transform(Volume<T, C, I>& im1,
		const Volume<T, C, I>& im2, const Volume<T, C, I>& im3,
		const std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Volume dimensions do not match. "
				<< im1.dimensions() << "!=" << im2.dimensions());
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(im1.data[offset], im2.data[offset], im3.data[offset]);
		}
	}
	template<class T, int C, ImageType I> void Transform(Volume<T, C, I>& im1,
		Volume<T, C, I>& im2,
		const std::function<
		void(int i, int j, int k, vec<T, C>& val1, vec<T, C>& val2)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Volume dimensions do not match. "
				<< im1.dimensions() << "!=" << im2.dimensions());
#pragma omp parallel for
		for (int k = 0; k < im1.slices; k++) {
			for (int j = 0; j < im1.cols; j++) {
				for (int i = 0; i < im1.rows; i++) {
					size_t offset = i + j * im1.rows + k * im1.rows * im1.cols;
					func(i, j, k, im1.data[offset], im2.data[offset]);
				}
			}
		}
	}
	template<class T, int C, ImageType I> void Transform(Volume<T, C, I>& im1,
		Volume<T, C, I>& im2,
		const std::function<
		void(size_t offset, vec<T, C>& val1, vec<T, C>& val2)>& func) {
		if (im1.dimensions() != im2.dimensions())
			throw std::runtime_error(
				MakeString() << "Volume dimensions do not match. "
				<< im1.dimensions() << "!=" << im2.dimensions());
		size_t sz = im1.size();
#pragma omp parallel for
		for (int offset = 0; offset < (int)sz; offset++) {
			func(offset, im1.data[offset], im2.data[offset]);
		}
	}
	template<class T, class L, class R, int C, ImageType I> std::basic_ostream<L, R> & operator <<(
		std::basic_ostream<L, R> & ss, const Volume<T, C, I> & A) {
		ss << "Volume (" << A.getTypeName() << "): " << A.id << " Position: ("
			<< A.x << "," << A.y << ") Dimensions: [" << A.rows << "," << A.cols
			<< "]\n";
		return ss;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator+(
		const vec<T, C>& scalar, const Volume<T, C, I>& img) {
		Volume<T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = scalar + val2;};
		Transform(out, img, f);
		return out;
	}

	template<class T, int C, ImageType I> Volume<T, C, I> operator-(
		const vec<T, C>& scalar, const Volume<T, C, I>& img) {
		Volume<T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = scalar - val2;};
		Transform(out, img, f);
		return out;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator*(
		const vec<T, C>& scalar, const Volume<T, C, I>& img) {
		Volume<T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = scalar*val2;};
		Transform(out, img, f);
		return out;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator/(
		const vec<T, C>& scalar, const Volume<T, C, I>& img) {
		Volume<T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = scalar / val2;};
		Transform(out, img, f);
		return out;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator+(
		const Volume<T, C, I>& img, const vec<T, C>& scalar) {
		Volume<T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = val2 + scalar;};
		Transform(out, img, f);
		return out;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator-(
		const Volume<T, C, I>& img, const vec<T, C>& scalar) {
		Volume<T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = val2 - scalar;};
		Transform(out, img, f);
		return out;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator*(
		const Volume<T, C, I>& img, const vec<T, C>& scalar) {
		Volume<T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = val2*scalar;};
		Transform(out, img, f);
		return out;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator/(
		const Volume<T, C, I>& img, const vec<T, C>& scalar) {
		Volume<T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = val2 / scalar;};
		Transform(out, img, f);
		return out;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator-(
		const Volume<T, C, I>& img) {
		Volume<T, C, I> out(img.rows, img.cols, img.slices, img.position());
		std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 = -val2;};
		Transform(out, img, f);
		return out;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator+=(
		Volume<T, C, I>& out, const Volume<T, C, I>& img) {
		std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 += val2;};
		Transform(out, img, f);
		return out;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator-=(
		Volume<T, C, I>& out, const Volume<T, C, I>& img) {
		std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 -= val2;};
		Transform(out, img, f);
		return out;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator*=(
		Volume<T, C, I>& out, const Volume<T, C, I>& img) {
		std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 *= val2;};
		Transform(out, img, f);
		return out;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator/=(
		Volume<T, C, I>& out, const Volume<T, C, I>& img) {
		std::function<void(vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2) {val1 /= val2;};
		Transform(out, img, f);
		return out;
	}

	template<class T, int C, ImageType I> Volume<T, C, I> operator+=(
		Volume<T, C, I>& out, const vec<T, C>& scalar) {
		std::function<void(vec<T, C>&)> f = [=](vec<T, C>& val1) {val1 += scalar;};
		Transform(out, f);
		return out;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator-=(
		Volume<T, C, I>& out, const vec<T, C>& scalar) {
		std::function<void(vec<T, C>&)> f = [=](vec<T, C>& val1) {val1 -= scalar;};
		Transform(out, f);
		return out;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator*=(
		Volume<T, C, I>& out, const vec<T, C>& scalar) {
		std::function<void(vec<T, C>&)> f = [=](vec<T, C>& val1) {val1 *= scalar;};
		Transform(out, f);
		return out;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator/=(
		Volume<T, C, I>& out, const vec<T, C>& scalar) {
		std::function<void(vec<T, C>&)> f = [=](vec<T, C>& val1) {val1 /= scalar;};
		Transform(out, f);
		return out;
	}

	template<class T, int C, ImageType I> Volume<T, C, I> operator+(
		const Volume<T, C, I>& img1, const Volume<T, C, I>& img2) {
		Volume<T, C, I> out(img1.rows, img1.cols, img1.slices);
		std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2, const vec<T, C>& val3) {val1 = val2 + val3;};
		Transform(out, img1, img2, f);
		return out;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator-(
		const Volume<T, C, I>& img1, const Volume<T, C, I>& img2) {
		Volume<T, C, I> out(img1.rows, img1.cols, img1.slices);
		std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2, const vec<T, C>& val3) {val1 = val2 - val3;};
		Transform(out, img1, img2, f);
		return out;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator*(
		const Volume<T, C, I>& img1, const Volume<T, C, I>& img2) {
		Volume<T, C, I> out(img1.rows, img1.cols, img1.slices);
		std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2, const vec<T, C>& val3) {val1 = val2*val3;};
		Transform(out, img1, img2, f);
		return out;
	}
	template<class T, int C, ImageType I> Volume<T, C, I> operator/(
		const Volume<T, C, I>& img1, const Volume<T, C, I>& img2) {
		Volume<T, C, I> out(img1.rows, img1.cols, img1.slices);
		std::function<void(vec<T, C>&, const vec<T, C>&, const vec<T, C>&)> f =
			[=](vec<T, C>& val1, const vec<T, C>& val2, const vec<T, C>& val3) {val1 = val2 / val3;};
		Transform(out, img1, img2, f);
		return out;
	}
	template<class T, int C, ImageType I> void WriteImageToRawFile(
		const std::string& file, const Volume<T, C, I>& img) {
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
			for (int k = 0; k < img.slices; k++) {
				for (int j = 0; j < img.cols; j++) {
					for (int i = 0; i < img.rows; i++) {
						T val = img(i, j, k)[c];
						fwrite(&val, sizeof(T), 1, f);
					}
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
		}
		//std::cout << vstr.str() << std::endl;
		std::stringstream sstr;
		sstr << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
		sstr << "<!-- MIPAV header file -->\n";
		if (img.channels > 1) {
			sstr << "<image xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" nDimensions=\"4\">\n";
		}
		else {
			sstr << "<image xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" nDimensions=\"3\">\n";
		}
		sstr << "	<Dataset-attributes>\n";
		sstr << "		<Image-offset>0</Image-offset>\n";
		sstr << "		<Data-type>" << typeName << "</Data-type>\n";
		sstr << "		<Endianess>Little</Endianess>\n";
		sstr << "		<Extents>" << img.rows << "</Extents>\n";
		sstr << "		<Extents>" << img.cols << "</Extents>\n";
		sstr << "		<Extents>" << img.slices << "</Extents>\n";
		if (img.channels > 1) {
			sstr << "		<Extents>" << img.channels << "</Extents>\n";
		}
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
	typedef Volume<uint8_t, 4, ImageType::UBYTE> VolumeRGBA;
	typedef Volume<int, 4, ImageType::INT> VolumeRGBAi;
	typedef Volume<float, 4, ImageType::FLOAT> VolumeRGBAf;

	typedef Volume<uint8_t, 3, ImageType::UBYTE> VolumeRGB;
	typedef Volume<int, 3, ImageType::INT> VolumeRGBi;
	typedef Volume<float, 3, ImageType::FLOAT> VolumeRGBf;

	typedef Volume<uint8_t, 1, ImageType::UBYTE> VolumeA;
	typedef Volume<int, 1, ImageType::INT> VolumeAi;
	typedef Volume<float, 1, ImageType::FLOAT> VolumeAf;

	typedef Volume<int8_t, 4, ImageType::BYTE> Volume4b;
	typedef Volume<uint8_t, 4, ImageType::UBYTE> Volume4ub;
	typedef Volume<uint16_t, 4, ImageType::USHORT> Volume4us;
	typedef Volume<int16_t, 4, ImageType::SHORT> Volume4s;
	typedef Volume<int, 4, ImageType::INT> Volume4i;
	typedef Volume<uint32_t, 4, ImageType::UINT> Volume4ui;
	typedef Volume<float, 4, ImageType::FLOAT> Volume4f;

	typedef Volume<int8_t, 3, ImageType::BYTE> Volume3b;
	typedef Volume<uint8_t, 3, ImageType::UBYTE> Volume3ub;
	typedef Volume<uint16_t, 3, ImageType::USHORT> Volume3us;
	typedef Volume<int16_t, 3, ImageType::SHORT> Volume3s;
	typedef Volume<int, 3, ImageType::INT> Volume3i;
	typedef Volume<uint32_t, 3, ImageType::UINT> Volume3ui;
	typedef Volume<float, 3, ImageType::FLOAT> Volume3f;


	typedef Volume<int8_t, 2, ImageType::BYTE> Volume2b;
	typedef Volume<uint8_t, 2, ImageType::UBYTE> Volume2ub;
	typedef Volume<uint16_t, 2, ImageType::USHORT> Volume2us;
	typedef Volume<int16_t, 2, ImageType::SHORT> Volume2s;
	typedef Volume<int, 2, ImageType::INT> Volume2i;
	typedef Volume<uint32_t, 2, ImageType::UINT> Volume2ui;
	typedef Volume<float, 2, ImageType::FLOAT> Volume2f;


	typedef Volume<int8_t, 1, ImageType::BYTE> Volume1b;
	typedef Volume<uint8_t, 1, ImageType::UBYTE> Volume1ub;
	typedef Volume<uint16_t, 1, ImageType::USHORT> Volume1us;
	typedef Volume<int16_t, 1, ImageType::SHORT> Volume1s;

	typedef Volume<int, 1, ImageType::INT> Volume1i;
	typedef Volume<uint32_t, 1, ImageType::UINT> Volume1ui;
	typedef Volume<float, 1, ImageType::FLOAT> Volume1f;
}
;

#endif
