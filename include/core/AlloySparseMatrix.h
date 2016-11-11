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
#ifndef ALLOYSPARSEMATRIX_H_
#define ALLOYSPARSEMATRIX_H_
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

template<class T, int C> struct SparseMatrix {
private:
	std::vector<std::map<size_t, vec<T, C>>>storage;
public:
	size_t rows, cols;
	SparseMatrix() :rows(0), cols(0)
	{

	}
	template<class Archive> void serialize(Archive & archive)
	{
		archive(CEREAL_NVP(rows), CEREAL_NVP(cols), cereal::make_nvp(MakeString() << "matrix" << C, storage));
	}
	std::map<size_t, vec<T, C>>& operator[](size_t i)
	{
		if (i >= rows || i < 0)throw std::runtime_error(MakeString() << "Index (" << i << ",*) exceeds matrix bounds [" << rows << "," << cols << "]");
		return storage[i];
	}
	const std::map<size_t, vec<T, C>>& operator[](size_t i) const
	{
		if (i >= rows || i < 0)throw std::runtime_error(MakeString() << "Index (" << i << ",*) exceeds matrix bounds [" << rows << "," << cols << "]");
		return storage[i];
	}
	SparseMatrix(size_t rows, size_t cols) :storage(rows), rows(rows), cols(cols)
	{
	}
	size_t size() const {
		size_t count=0;
		for(const std::map<size_t, vec<T, C>>& vec:storage){
			count+=vec.size();
		}
		return count;
	}
	void resize(size_t rows, size_t cols){
	    this->rows=rows;
	    this->cols=cols;
	    storage.resize(rows);
	}
	void set(size_t i, size_t j, const vec<T, C>& value)
	{
		if (i >= rows || j >= cols || i < 0 || j < 0)throw std::runtime_error(MakeString() << "Index (" << i << "," << j << ") exceeds matrix bounds [" << rows << "," << cols << "]");
		storage[i][j] = value;
	}
	void set(size_t i, size_t j, const T& value)
	{
		if (i >= rows || j >= cols || i < 0 || j < 0)throw std::runtime_error(MakeString() << "Index (" << i << "," << j << ") exceeds matrix bounds [" << rows << "," << cols << "]");
		storage[i][j] = vec<T, C>(value);
	}
	vec<T, C>& operator()(size_t i, size_t j)
	{
		if (i >= rows || j >= cols || i < 0 || j < 0)throw std::runtime_error(MakeString() << "Index (" << i << "," << j << ") exceeds matrix bounds [" << rows << "," << cols << "]");
		return storage[i][j];
	}

	vec<T, C> get(size_t i, size_t j) const
	{
		if (i >= rows || j >= cols || i < 0 || j < 0)throw std::runtime_error(MakeString() << "Index (" << i << "," << j << ") exceeds matrix bounds [" << rows << "," << cols << "]");
		if (storage[i].find(j) == storage[i].end())
		{
			return vec<T, C>(T(0));
		}
		else
		{
			return storage[i].at(j);
		}
	}
	vec<T, C> operator()(size_t i, size_t j) const
	{
		return get(i, j);
	}
	SparseMatrix<T, C> transpose() const
	{
		SparseMatrix<T, C> M(cols, rows);
		for (int i = 0;i < (int)storage.size();i++)
		{
			for (const std::pair<size_t, vec<T, C>>& iv : storage[i])
			{
				M.set(iv.first, i, iv.second);
			}
		}
		return M;
	}
	static SparseMatrix<T, C> identity(size_t M,size_t N)
	{
		SparseMatrix<T, C> A(M,N);
		int K=(int)aly::min(M,N);
#pragma omp parallel for
		for (int k=0;k<K;k++)
		{
			A[k][k]=vec<T,C>(T(1));
		}
		return A;
	}
	static SparseMatrix<T, C> diagonal(const Vector<T,C>& v)
	{
		SparseMatrix<T, C> A(v.size(),v.size());
#pragma omp parallel for
		for (int k = 0;k<(int)v.size();k++)
		{
			A[k][k] = v[k];
		}
		return A;
	}
};
template<class A, class B, class T, int C> std::basic_ostream<A, B> & operator <<(
		std::basic_ostream<A, B> & ss, const SparseMatrix<T, C>& M) {
	for (int i = 0; i < (int)M.rows; i++) {
		ss << "M[" << i << ",*]=";
		for (const std::pair<size_t, vec<T, C>>& pr : M[i]) {
			ss << "<" << pr.first << ":" << pr.second << "> ";
		}
		ss << std::endl;
	}
	return ss;
}

template<class T, int C> SparseMatrix<T, C>& operator*=(
		SparseMatrix<T, C>& A, const vec<T, C>& v) {
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		for (std::pair<size_t, vec<T, C>>& pr : A[i]) {
			A[i][pr.first] = pr.second * v;
		}
	}
	return A;
}
template<class T, int C> SparseMatrix<T, C>& operator/=(
		const SparseMatrix<T, C>& A, const vec<T, C>& v) {
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		for (std::pair<size_t, vec<T, C>>& pr : A[i]) {
			A[i][pr.first] = pr.second / v;
		}
	}
	return A;
}
template<class T, int C> SparseMatrix<T, C>& operator+=(
		SparseMatrix<T, C>& A, const vec<T, C>& v) {
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		for (std::pair<size_t, vec<T, C>>& pr : A[i]) {
			A[i][pr.first] = pr.second + v;
		}
	}
	return A;
}
template<class T, int C> SparseMatrix<T, C>& operator-=(
		SparseMatrix<T, C>& A, const vec<T, C>& v) {
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		for (std::pair<size_t, vec<T, C>>& pr : A[i]) {
			A[i][pr.first] = pr.second - v;
		}
	}
	return A;
}
template<class T, int C> SparseMatrix<T, C>& operator*=(
		SparseMatrix<T, C>& A, const T& v) {
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		for (std::pair<size_t, vec<T, C>>& pr : A[i]) {
			A[i][pr.first] = pr.second * v;
		}
	}
	return A;
}
template<class T, int C> SparseMatrix<T, C>& operator/=(
		SparseMatrix<T, C>& A, const T& v) {
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		for (std::pair<size_t, vec<T, C>>& pr : A[i]) {
			A[i][pr.first] = pr.second / v;
		}
	}
	return A;
}
template<class T, int C> SparseMatrix<T, C>& operator+=(
		SparseMatrix<T, C>& A, const T& v) {
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		for (std::pair<size_t, vec<T, C>>& pr : A[i]) {
			A[i][pr.first] = pr.second + v;
		}
	}
	return A;
}
template<class T, int C> SparseMatrix<T, C>& operator-=(
		SparseMatrix<T, C>& A, const T& v) {
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		for (std::pair<size_t, vec<T, C>>& pr : A[i]) {
			A[i][pr.first] = pr.second - v;
		}
	}
	return A;
}
template<class T, int C> SparseMatrix<T, C> operator*(
		const SparseMatrix<T, C>& A, const SparseMatrix<T, C>& B) {
	if (A.cols != B.rows)
		throw std::runtime_error(
				MakeString()
						<< "Cannot multiply matrices. Inner dimensions do not match. "
						<< "[" << A.rows << "," << A.cols << "] * [" << B.rows
						<< "," << B.cols << "]");
	SparseMatrix<T, C> out(A.rows, B.cols);
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) { //a[i,*]
		for (std::pair<size_t, vec<T, C>> pr1 : A[i]) { //a[i,k]
			int k = (int)pr1.first;
			for (std::pair<size_t, vec<T, C>> pr2 : B[k]) { //b[k,j]
				int j = (int)pr2.first;
				out[i][j] += pr1.second * pr2.second;
			}
		}
	}
	return out;
}

template<class T, int C> SparseMatrix<T, C> operator*(const T& v,
		const SparseMatrix<T, C>& A) {
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : A[i]) {
			out[i][pr.first] = v * pr.second;
		}
	}
	return out;
}
template<class T, int C> SparseMatrix<T, C> operator/(const T& v,
		const SparseMatrix<T, C>& A) {
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : A[i]) {
			out[i][pr.first] = v / pr.second;
		}
	}
	return out;
}
template<class T, int C> SparseMatrix<T, C> operator+(const T& v,
		const SparseMatrix<T, C>& A) {
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : A[i]) {
			out[i][pr.first] = vec<T, C>(v) + pr.second;
		}
	}
	return out;
}
template<class T, int C> SparseMatrix<T, C> operator-(const T& v,
		const SparseMatrix<T, C>& A) {
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : A[i]) {
			out[i][pr.first] = vec<T, C>(v) - pr.second;
		}
	}
	return out;
}
template<class T, int C> SparseMatrix<T, C> operator*(const vec<T, C>& v,
		const SparseMatrix<T, C>& A) {
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : A[i]) {
			out[i][pr.first] = v * pr.second;
		}
	}
	return out;
}
template<class T, int C> SparseMatrix<T, C> operator/(const vec<T, C>& v,
		const SparseMatrix<T, C>& A) {
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : A[i]) {
			out[i][pr.first] = v / pr.second;
		}
	}
	return out;
}
template<class T, int C> SparseMatrix<T, C> operator+(const vec<T, C>& v,
		const SparseMatrix<T, C>& A) {
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : A[i]) {
			out[i][pr.first] = v + pr.second;
		}
	}
	return out;
}
template<class T, int C> SparseMatrix<T, C> operator-(const vec<T, C>& v,
		const SparseMatrix<T, C>& A) {
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : A[i]) {
			out[i][pr.first] = v - pr.second;
		}
	}
	return out;
}

template<class T, int C> SparseMatrix<T, C> operator-(
		const SparseMatrix<T, C>& A, const vec<T, C>& v) {
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : A[i]) {
			out[i][pr.first] = pr.second - v;
		}
	}
	return out;
}
template<class T, int C> SparseMatrix<T, C> operator+(
		const SparseMatrix<T, C>& A, const vec<T, C>& v) {
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : A[i]) {
			out[i][pr.first] = pr.second + v;
		}
	}
	return out;
}
template<class T, int C> SparseMatrix<T, C> operator*(
		const SparseMatrix<T, C>& A, const vec<T, C>& v) {
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : A[i]) {
			out[i][pr.first] = pr.second * v;
		}
	}
	return out;
}

template<class T, int C> SparseMatrix<T, C> operator/(
		const SparseMatrix<T, C>& A, const vec<T, C>& v) {
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : A[i]) {
			out[i][pr.first] = pr.second / v;
		}
	}
	return out;
}

template<class T, int C> SparseMatrix<T, C> operator-(
		const SparseMatrix<T, C>& A, const T& v) {
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : A[i]) {
			out[i][pr.first] = pr.second - vec<T, C>(v);
		}
	}
	return out;
}
template<class T, int C> SparseMatrix<T, C> operator+(
		const SparseMatrix<T, C>& A, const T& v) {
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : A[i]) {
			out[i][pr.first] = pr.second + vec<T, C>(v);
		}
	}
	return out;
}
template<class T, int C> SparseMatrix<T, C> operator*(
		const SparseMatrix<T, C>& A, const T& v) {
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : A[i]) {
			out[i][pr.first] = pr.second * v;
		}
	}
	return out;
}

template<class T, int C> SparseMatrix<T, C> operator/(
		const SparseMatrix<T, C>& A, const T& v) {
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : A[i]) {
			out[i][pr.first] = pr.second / v;
		}
	}
	return out;
}
template<class T, int C> SparseMatrix<T, C> operator-(
		const SparseMatrix<T, C>& A) {
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : A[i]) {
			out[i][pr.first] = -pr.second;
		}
	}
	return out;
}
template<class T, int C> SparseMatrix<T, C> operator+(
		const SparseMatrix<T, C>& A, const SparseMatrix<T, C>& B) {
	if (A.rows != B.rows || A.cols != B.cols)
		throw std::runtime_error(
				MakeString() << "Cannot add matrices. Dimensions do not match. "
						<< "[" << A.rows << "," << A.cols << "] * [" << B.rows
						<< "," << B.cols << "]");
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : B[i]) {
			out[i][pr.first] += pr.second;
		}
	}
	return out;
}
template<class T, int C> SparseMatrix<T, C> operator-(
		const SparseMatrix<T, C>& A, const SparseMatrix<T, C>& B) {
	if (A.rows != B.rows || A.cols != B.cols)
		throw std::runtime_error(
				MakeString()
						<< "Cannot subtract matrices. Dimensions do not match. "
						<< "[" << A.rows << "," << A.cols << "] * [" << B.rows
						<< "," << B.cols << "]");
	SparseMatrix<T, C> out = A;
#pragma omp parallel for
	for (int i = 0; i < (int) out.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : B[i]) {
			out[i][pr.first] -= pr.second;
		}
	}
	return out;
}
template<class T, int C> SparseMatrix<T, C>& operator+=(
		SparseMatrix<T, C>& A, const SparseMatrix<T, C>& B) {
	if (A.rows != B.rows || A.cols != B.cols)
		throw std::runtime_error(
				MakeString() << "Cannot add matrices. Dimensions do not match. "
						<< "[" << A.rows << "," << A.cols << "] * [" << B.rows
						<< "," << B.cols << "]");
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : B[i]) {
			A[i][pr.first] += pr.second;
		}
	}
	return A;
}
template<class T, int C> SparseMatrix<T, C>& operator-=(
		SparseMatrix<T, C>& A, const SparseMatrix<T, C>& B) {
	if (A.rows != B.rows || A.cols != B.cols)
		throw std::runtime_error(
				MakeString()
						<< "Cannot subtract matrices. Dimensions do not match. "
						<< "[" << A.rows << "," << A.cols << "] * [" << B.rows
						<< "," << B.cols << "]");
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		for (std::pair<size_t, vec<T, C>> pr : B[i]) {
			A[i][pr.first] -= pr.second;
		}
	}
	return A;
}
template<class T, int C> void Multiply(Vector<T, C>& out,
		const SparseMatrix<T, 1>& A, const Vector<T, C>& v) {
	out.resize(A.rows);
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		vec<double, C> sum(0.0);
		for (const std::pair<size_t, vec<T, 1>>& pr : A[i]) {
			sum += vec<double, C>(v[pr.first]) * (double) pr.second.x;
		}
		out[i] = vec<T, C>(sum);
	}
}
template<class T, int C> void AddMultiply(Vector<T, C>& out,
		const Vector<T, C>& b, const SparseMatrix<T, 1>& A,
		const Vector<T, C>& v) {
	out.resize(A.rows);
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		vec<double, C> sum(0.0);
		for (const std::pair<size_t, vec<T, 1>>& pr : A[i]) {
			sum += vec<double, C>(v[pr.first]) * (double) pr.second.x;
		}
		out[i] = b[i] + vec<T, C>(sum);
	}
}
template<class T, int C> void SubtractMultiply(Vector<T, C>& out,
		const Vector<T, C>& b, const SparseMatrix<T, 1>& A,
		const Vector<T, C>& v) {
	out.resize(A.rows);
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		vec<double, C> sum(0.0);
		for (const std::pair<size_t, vec<T, 1>>& pr : A[i]) {
			sum += vec<double, C>(v[pr.first]) * (double) pr.second.x;
		}
		out[i] = b[i] - vec<T, C>(sum);
	}
}
template<class T, int C> Vector<T, C> operator*(const SparseMatrix<T, C>& A,
		const Vector<T, C>& v) {
	Vector<T, C> out(A.rows);
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		vec<double, C> sum(0.0);
		for (const std::pair<size_t, vec<T, C>>& pr : A[i]) {
			sum += vec<double, C>(v[pr.first]) * vec<double, C>(pr.second);
		}
		out[i] = vec<T, C>(sum);
	}
	return out;
}
template<class T, int C> void MultiplyVec(Vector<T, C>& out,
		const SparseMatrix<T, C>& A, const Vector<T, C>& v) {
	out.resize(A.rows);
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		vec<double, C> sum(0.0);
		for (const std::pair<size_t, vec<T, C>>& pr : A[i]) {
			sum += vec<double, C>(v[pr.first]) * vec<double, C>(pr.second);
		}
		out[i] = vec<T, C>(sum);
	}
}

template<class T, int C> void AddMultiplyVec(Vector<T, C>& out,
		const Vector<T, C>& b, const SparseMatrix<T, C>& A,
		const Vector<T, C>& v) {
	out.resize(A.rows);
#pragma omp parallel for
	for (int i = 0; i < A.rows; i++) {
		vec<double, C> sum(0.0);
		for (const std::pair<size_t, vec<T, C>>& pr : A[i]) {
			sum += vec<double, C>(v[pr.first]) * vec<double, C>(pr.second);
		}
		out[i] = b[i] + vec<T, C>(sum);
	}
}
template<class T, int C> void SubtractMultiplyVec(Vector<T, C>& out,
		const Vector<T, C>& b, const SparseMatrix<T, C>& A,
		const Vector<T, C>& v) {
	out.resize(A.rows);
#pragma omp parallel for
	for (int i = 0; i < (int) A.rows; i++) {
		vec<double, C> sum(0.0);
		for (const std::pair<size_t, vec<T, C>>& pr : A[i]) {
			sum += vec<double, C>(v[pr.first]) * vec<double, C>(pr.second);
		}
		out[i] = b[i] - vec<T, C>(sum);
	}
}
template<class T, int C> void WriteSparseMatrixToFile(const std::string& file, const SparseMatrix<T, C>& matrix) {
	std::ofstream os(file);
	cereal::PortableBinaryOutputArchive ar(os);
	ar(cereal::make_nvp("sparse_matrix", matrix));
}
template<class T, int C> void ReadSparseMatrixFromFile(const std::string& file, SparseMatrix<T, C>& matrix) {
	std::ifstream os(file);
	cereal::PortableBinaryInputArchive ar(os);
	ar(cereal::make_nvp("sparse_matrix", matrix));
}
typedef SparseMatrix<int, 4> SparseMatrix4i;
typedef SparseMatrix<int, 3> SparseMatrix3i;
typedef SparseMatrix<int, 2> SparseMatrix2i;
typedef SparseMatrix<int, 1> SparseMatrix1i;

typedef SparseMatrix<float, 4> SparseMatrix4f;
typedef SparseMatrix<float, 3> SparseMatrix3f;
typedef SparseMatrix<float, 2> SparseMatrix2f;
typedef SparseMatrix<float, 1> SparseMatrix1f;

typedef SparseMatrix<double, 4> SparseMatrix4d;
typedef SparseMatrix<double, 3> SparseMatrix3d;
typedef SparseMatrix<double, 2> SparseMatrix2d;
typedef SparseMatrix<double, 1> SparseMatrix1d;
}

#endif
