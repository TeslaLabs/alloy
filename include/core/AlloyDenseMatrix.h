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
#ifndef ALLOYDENSEMATRIX_H_
#define ALLOYDENSEMATRIX_H_
#include <cereal/types/list.hpp>
#include "AlloyVector.h"
#include "cereal/types/vector.hpp"
#include "cereal/types/tuple.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/memory.hpp"
#include "cereal/archives/portable_binary.hpp"
#include <vector>
#include <list>
#include <map>
namespace aly {
bool SANITY_CHECK_DENSE_MATRIX();
template<class T, int C> struct DenseMatrix {
private:
	std::vector<std::vector<vec<T, C>>>data;
public:
	int rows, cols;
	typedef vec<T, C> ValueType;
	typedef typename std::vector<ValueType>::iterator iterator;
	typedef typename std::vector<ValueType>::const_iterator const_iterator;
	typedef typename std::vector<ValueType>::reverse_iterator reverse_iterator;
	size_t size() const {
		return rows*cols;
	}
	iterator begin(int i) const {
		return data[i].begin();
	}
	iterator end(int i) const {
		return data[i].end();
	}
	iterator begin(int i) {
		return data[i].begin();
	}
	iterator end(int i) {
		return data[i].end();
	}
	const_iterator cbegin(int i) const {
		return data[i].cbegin();
	}
	const_iterator cend(int i) const {
		return data[i].cend();
	}
	reverse_iterator rbegin(int i) {
		return data[i].rbegin();
	}
	reverse_iterator rend(int i) {
		return data[i].rend();
	}
	reverse_iterator rbegin(int i) const {
		return data[i].rbegin();
	}
	reverse_iterator rend(int i) const {
		return data[i].rend();
	}
	template<class Archive> void serialize(Archive & archive) {
		archive(CEREAL_NVP(rows), CEREAL_NVP(cols),
				cereal::make_nvp(
						MakeString() << "matrix"<<C,
						data));
	}
	std::vector<vec<T, C>>& operator[](size_t i) {
		if ((int)i >= rows )
		throw std::runtime_error(
				MakeString() << "Index (" << i
				<< ",*) exceeds matrix bounds [" << rows << ","
				<< cols << "]");
		return data[i];
	}
	const std::vector<vec<T, C>>& operator[](size_t i) const {
		if ((int)i >= rows )
		throw std::runtime_error(
				MakeString() << "Index (" << i
				<< ",*) exceeds matrix bounds [" << rows << ","
				<< cols << "]");
		return data[i];
	}
	DenseMatrix(): rows(0),cols(0) {
	}
	DenseMatrix(int rows, int cols) :
	data(rows,std::vector<vec<T,C>>(cols)),rows(rows), cols(cols) {
	}
	void resize(int rows,int cols) {
		if(this->rows!=rows||this->cols!=cols) {
			data=std::vector<std::vector<vec<T,C>>>(rows,std::vector<vec<T,C>>(cols));
			this->rows=rows;
			this->cols=cols;
		}
	}
	void set(size_t i, size_t j, const vec<T, C>& value) {
		if (i >= (size_t)rows || j >=(size_t) cols || i < 0 || j < 0)
		throw std::runtime_error(
				MakeString() << "Index (" << i << "," << j
				<< ") exceeds matrix bounds [" << rows << ","
				<< cols << "]");
		data[i][j] = value;
	}
	void set(size_t i, size_t j, const T& value) {
		if (i >= (size_t)rows || j >= (size_t)cols || i < 0 || j < 0)
		throw std::runtime_error(
				MakeString() << "Index (" << i << "," << j
				<< ") exceeds matrix bounds [" << rows << ","
				<< cols << "]");
		data[i][j] = vec<T, C>(value);
	}
	vec<T, C>& operator()(size_t i, size_t j) {
		if (i >= (size_t)rows || j >= (size_t)cols || i < 0 || j < 0)
		throw std::runtime_error(
				MakeString() << "Index (" << i << "," << j
				<< ") exceeds matrix bounds [" << rows << ","
				<< cols << "]");
		return data[i][j];
	}

	vec<T, C> get(size_t i, size_t j) const {
		if (i >= (size_t)rows || j >= (size_t)cols || i < 0 || j < 0)
		throw std::runtime_error(
				MakeString() << "Index (" << i << "," << j
				<< ") exceeds matrix bounds [" << rows << ","
				<< cols << "]");
		return data[i][j];
	}
	const vec<T, C>& operator()(size_t i, size_t j) const {
		return data[i][j];
	}
	inline DenseMatrix<T, C> transpose() const {
		DenseMatrix<T, C> M(cols, rows);
		for (int i = 0; i < rows; i++) {
			for (int j=0;j < cols;j++) {
				M[j][i]=data[i][j];
			}
		}
		return M;
	}
	inline static DenseMatrix<T, C> identity(size_t M, size_t N) {
		DenseMatrix<T, C> A(M, N);
		for (int i = 0; i < M; i++) {
			for (int j=0;j < N;j++) {
				A[i][j]=vec<T, C>(T((i==j)?1:0));
			}
		}
		return A;
	}
	inline static DenseMatrix<T, C> zero(size_t M, size_t N) {
		DenseMatrix<T, C> A(M, N);
		for (int i = 0; i < M; i++) {
			for (int j=0;j < N;j++) {
				A[i][j]=vec<T, C>(T(0));
			}
		}
		return A;
	}
	inline static DenseMatrix<T, C> diagonal(const Vector<T, C>& v) {
		DenseMatrix<T, C> A((int)v.size(), (int)v.size());
		for (int i = 0; i < A.rows; i++) {
			for (int j=0;j < A.cols;j++) {
				A[i][j]=vec<T, C>(T((i==j)?v[i]:0));
			}
		}
		return A;
	}
	inline static DenseMatrix<T, C> columnVector(const Vector<T, C>& v) {
		DenseMatrix<T, C> A((int)v.size(),1);
		for (int i = 0; i < A.rows; i++) {
			A[i][0] = v[i];
		}
		return A;
	}
	inline static DenseMatrix<T, C> rowVector(const Vector<T, C>& v) {
		DenseMatrix<T, C> A(1, (int)v.size());
		A.data[0] = v.data;
		return A;
	}
	inline Vector<T, C> getRow(int i) const {
		Vector<T, C> v(data[i].data);
		return v;
	}
	inline Vector<T, C> getColumn(int j) const {
		Vector<T, C> v(rows);
		for (int i = 0; i < rows; i++) {
			v[i]=data[i][j];
		}
		return v;
	}
};
template<class A, class B, class T, int C> std::basic_ostream<A, B> & operator <<(std::basic_ostream<A, B> & ss, const DenseMatrix<T, C>& M) {
	ss << "\n";
	if (C == 1) {
		for (int i = 0; i < M.rows; i++) {
			ss << "[";
			for (int j = 0; j < M.cols; j++) {
				ss << std::setprecision(10) << std::setw(16) << M[i][j] << ((j < M.cols - 1) ? "," : "]\n");
			}
		}
	} else {
		for (int i = 0; i < M.rows; i++) {
			ss << "[";
			for (int j = 0; j < M.cols; j++) {
				ss << M[i][j] << ((j < M.cols - 1) ? "," : "]\n");
			}
		}
	}
	return ss;
}

template<class T, int C> Vector<T, C> operator*(const DenseMatrix<T, C>& A, const Vector<T, C>& v) {
	Vector<T, C> out(A.rows);
	for (int i = 0; i < A.rows; i++) {
		vec<T, C> sum(0.0);
		for (int j = 0; j < A.cols; j++) {
			sum += A[i][j] * v[j];
		}
		out[i] = sum;
	}
	return out;
}
template<class T, int C> DenseMatrix<T, C> operator*(const DenseMatrix<T, C>& A, const DenseMatrix<T, C>& B) {
	if (A.cols != B.rows)
		throw std::runtime_error(
				MakeString() << "Cannot multiply matrices. Inner dimensions do not match. " << "[" << A.rows << "," << A.cols << "] * [" << B.rows << ","
						<< B.cols << "]");
	DenseMatrix<T, C> out(A.rows, B.cols);
	for (int i = 0; i < out.rows; i++) {
		for (int j = 0; j < out.cols; j++) {
			vec<T, C> sum(0.0);
			for (int k = 0; k < A.cols; k++) {
				sum += A[i][k] * B[k][j];
			}
			out[i][j] = sum;
		}
	}
	return out;
}
//Slight abuse of mathematics here. Vectors are always interpreted as column vectors as a convention,
//so this multiplcation is equivalent to multiplying "A" with a diagonal matrix constructed from "W".
//To multiply a matrix with a column vector to get a row vector, convert "W" to a dense matrix.
template<class T, int C> DenseMatrix<T, C> operator*(const Vector<T, C>& W, const DenseMatrix<T, C>& A) {
	if (A.rows != (int) W.size())
		throw std::runtime_error(
				MakeString() << "Cannot scale matrix by vector. Rows must match. " << "[" << W.size() << "] * [" << A.rows << "," << A.cols << "]");
	DenseMatrix<T, C> out(A.rows, A.cols);
	for (int i = 0; i < out.rows; i++) {
		for (int j = 0; j < out.cols; j++) {
			out[i][j] = W[i] * A[i][j];
		}
	}
	return out;
}
template<class T, int C> DenseMatrix<T, C>& operator*=(DenseMatrix<T, C>& A, const Vector<T, C>& W) {
	if (A.rows != W.size())
		throw std::runtime_error(
				MakeString() << "Cannot scale matrix by vector. Rows must match. " << "[" << W.size() << "] * [" << A.rows << "," << A.cols << "]");
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			A[i][j] *= W[i];
		}
	}
	return A;
}
template<class T, int C> DenseMatrix<T, C> operator-(const DenseMatrix<T, C>& A) {
	DenseMatrix<T, C> out(A.rows, A.cols);
	for (int i = 0; i < (int) out.rows; i++) {
		for (int j = 0; j < out.cols; j++) {
			out[i][j] = -A[i][j];
		}
	}
	return out;
}
template<class T, int C> DenseMatrix<T, C> operator-(const DenseMatrix<T, C>& A, const DenseMatrix<T, C>& B) {
	if (A.rows != B.rows || A.cols != B.cols) {
		throw std::runtime_error("Cannot subtract matricies. Matrix dimensions must match.");
	}
	DenseMatrix<T, C> out(A.rows, A.cols);
	for (int i = 0; i < (int) out.rows; i++) {
		for (int j = 0; j < out.cols; j++) {
			out[i][j] = A[i][j] - B[i][j];
		}
	}
	return out;
}
template<class T, int C> DenseMatrix<T, C> operator+(const DenseMatrix<T, C>& A, const DenseMatrix<T, C>& B) {
	if (A.rows != B.rows || A.cols != B.cols) {
		throw std::runtime_error("Cannot add matricies. Matrix dimensions must match.");
	}
	DenseMatrix<T, C> out(A.rows, A.cols);
	for (int i = 0; i < (int) out.rows; i++) {
		for (int j = 0; j < out.cols; j++) {
			out[i][j] = A[i][j] + B[i][j];
		}
	}
	return out;
}
template<class T, int C> DenseMatrix<T, C> operator*(const DenseMatrix<T, C>& A, const vec<T, C>& v) {
	DenseMatrix<T, C> out(A.rows, A.cols);
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			out[i][j] = A[i][j] * v;
		}
	}
	return out;
}
template<class T, int C> DenseMatrix<T, C> operator/(const DenseMatrix<T, C>& A, const vec<T, C>& v) {
	DenseMatrix<T, C> out(A.rows, A.cols);
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			out[i][j] = A[i][j] / v;
		}
	}
	return out;
}
template<class T, int C> DenseMatrix<T, C> operator+(const DenseMatrix<T, C>& A, const vec<T, C>& v) {
	DenseMatrix<T, C> out(A.rows, A.cols);
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			out[i][j] = A[i][j] + v;
		}
	}
	return out;
}
template<class T, int C> DenseMatrix<T, C> operator-(const DenseMatrix<T, C>& A, const vec<T, C>& v) {
	DenseMatrix<T, C> out(A.rows, A.cols);
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			out[i][j] = A[i][j] - v;
		}
	}
	return out;
}
template<class T, int C> DenseMatrix<T, C> operator*(const vec<T, C>& v, const DenseMatrix<T, C>& A) {
	DenseMatrix<T, C> out(A.rows, A.cols);
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			out[i][j] = v * A[i][j];
		}
	}
	return out;
}
template<class T, int C> DenseMatrix<T, C> operator/(const vec<T, C>& v, const DenseMatrix<T, C>& A) {
	DenseMatrix<T, C> out(A.rows, A.cols);
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			out[i][j] = v / A[i][j];
		}
	}
	return out;
}
template<class T, int C> DenseMatrix<T, C> operator+(const vec<T, C>& v, const DenseMatrix<T, C>& A) {
	DenseMatrix<T, C> out(A.rows, A.cols);
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			out[i][j] = v + A[i][j];
		}
	}
	return out;
}
template<class T, int C> DenseMatrix<T, C> operator-(const vec<T, C>& v, const DenseMatrix<T, C>& A) {
	DenseMatrix<T, C> out(A.rows, A.cols);
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			out[i][j] = v - A[i][j];
		}
	}
	return out;
}
template<class T, int C> DenseMatrix<T, C>& operator*=(DenseMatrix<T, C>& A, const vec<T, C>& v) {
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			A[i][j] = A[i][j] * v;
		}
	}
	return A;
}
template<class T, int C> DenseMatrix<T, C>& operator/=(DenseMatrix<T, C>& A, const vec<T, C>& v) {
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			A[i][j] = A[i][j] / v;
		}
	}
	return A;
}
template<class T, int C> DenseMatrix<T, C>& operator+=(DenseMatrix<T, C>& A, const vec<T, C>& v) {
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			A[i][j] = A[i][j] + v;
		}
	}
	return A;
}
template<class T, int C> DenseMatrix<T, C>& operator-=(DenseMatrix<T, C>& A, const vec<T, C>& v) {
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			A[i][j] = A[i][j] - v;
		}
	}
	return A;
}
template<class T, int C> DenseMatrix<T, C>& operator*=(DenseMatrix<T, C>& A, const T& v) {
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			A[i][j] = A[i][j] * v;
		}
	}
	return A;
}
template<class T, int C> DenseMatrix<T, C>& operator/=(DenseMatrix<T, C>& A, const T& v) {
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			A[i][j] = A[i][j] / v;
		}
	}
	return A;
}
template<class T, int C> DenseMatrix<T, C>& operator+=(DenseMatrix<T, C>& A, const T& v) {
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			A[i][j] = A[i][j] + v;
		}
	}
	return A;
}
template<class T, int C> DenseMatrix<T, C>& operator-=(DenseMatrix<T, C>& A, const T& v) {
	for (int i = 0; i < A.rows; i++) {
		for (int j = 0; j < A.cols; j++) {
			A[i][j] = A[i][j] - v;
		}
	}
	return A;
}
template<class T, int C> void WriteDenseMatrixToFile(const std::string& file, const DenseMatrix<T, C>& matrix) {
	std::ofstream os(file);
	cereal::PortableBinaryOutputArchive ar(os);
	ar(cereal::make_nvp("dense_matrix", matrix));
}
template<class T, int C> void ReadDenseMatrixFromFile(const std::string& file, DenseMatrix<T, C>& matrix) {
	std::ifstream os(file);
	cereal::PortableBinaryInputArchive ar(os);
	ar(cereal::make_nvp("dense_matrix", matrix));
}
typedef DenseMatrix<int, 4> DenseMatrix4i;
typedef DenseMatrix<int, 3> DenseMatrix3i;
typedef DenseMatrix<int, 2> DenseMatrix2i;
typedef DenseMatrix<int, 1> DenseMatrix1i;

typedef DenseMatrix<float, 4> DenseMatrix4f;
typedef DenseMatrix<float, 3> DenseMatrix3f;
typedef DenseMatrix<float, 2> DenseMatrix2f;
typedef DenseMatrix<float, 1> DenseMatrix1f;

typedef DenseMatrix<double, 4> DenseMatrix4d;
typedef DenseMatrix<double, 3> DenseMatrix3d;
typedef DenseMatrix<double, 2> DenseMatrix2d;
typedef DenseMatrix<double, 1> DenseMatrix1d;
}

#endif
