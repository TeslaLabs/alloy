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
#ifndef ALLOYSPARSESOLVE_H_
#define ALLOYSPARSESOLVE_H_
#include "AlloyMath.h"
#include "AlloyVector.h"
#include "AlloySparseMatrix.h"
namespace aly {
bool SANITY_CHECK_ALGO();
bool SANITY_CHECK_SPARSE_SOLVE();
template<class T, int C> void SolveVecCG(const Vector<T, C>& b,
		const SparseMatrix<T, C>& A, Vector<T, C>& x, int iters = 100,
		T tolerance = 1E-6f,
		const std::function<bool(int, double)>& iterationMonitor = nullptr) {
	const double ZERO_TOLERANCE = 1E-16;
	vec<double, C> err(0.0);
	size_t N = b.size();
	Vector<T, C> p(N);
	Vector<T, C> Ap(N);
	MultiplyVec(Ap, A, x);
	Vector<T, C> tmp1(N), tmp2(N);
	Vector<T, C>* rcurrent = &tmp1;
	Vector<T, C>* rnext = &tmp2;
	Subtract(*rcurrent, b, Ap);
	p = *rcurrent;
	err = lengthVecSqr(*rcurrent);
	double e = lengthL1(err) / N;
	if (iterationMonitor) {
		if (!iterationMonitor(0, e))return;
	}
	for (int iter = 0; iter < iters; iter++) {
		MultiplyVec(Ap, A, p);
		vec<double, C> denom = dotVec(p, Ap);
		for (int c = 0; c < C; c++) {
			if (std::abs(denom[c]) < ZERO_TOLERANCE) {
				denom[c] = (denom[c] < 0) ? -ZERO_TOLERANCE : ZERO_TOLERANCE;
			}
		}
		vec<double, C> alpha = lengthVecSqr(*rcurrent) / denom;
		ScaleAdd(x, vec<T, C>(alpha), p);
		ScaleSubtract(*rnext, *rcurrent, vec<T, C>(alpha), Ap);
		vec<double, C> err = lengthVecSqr(*rnext);
		double e = lengthL1(err) / N;
		if (iterationMonitor) {
			if (!iterationMonitor(iter + 1, e))return;
		}
		if (e < tolerance)
			break;
		denom = lengthVecSqr(*rcurrent);
		for (int c = 0; c < C; c++) {
			if (std::abs(denom[c]) < ZERO_TOLERANCE) {
				denom[c] = (denom[c] < 0) ? -ZERO_TOLERANCE : ZERO_TOLERANCE;
			}
		}
		vec<double, C> beta = err / denom;
		ScaleAdd(p, *rnext, vec<T, C>(beta), p);
		std::swap(rcurrent, rnext);
	}
}
template<class T, int C> void SolveCG(const Vector<T, C>& b,
		const SparseMatrix<T, 1>& A, Vector<T, C>& x, int iters = 100,
		T tolerance = 1E-6f,
		const std::function<bool(int, double)>& iterationMonitor = nullptr) {
	const double ZERO_TOLERANCE = 1E-16;
	vec<double, C> err(0.0);
	size_t N = b.size();
	Vector<T, C> p(N);
	Vector<T, C> Ap(N);
	Multiply(Ap, A, x);
	Vector<T, C> tmp1(N), tmp2(N);
	Vector<T, C>* rcurrent = &tmp1;
	Vector<T, C>* rnext = &tmp2;
	Subtract(*rcurrent, b, Ap);
	p = *rcurrent;
	err = lengthVecSqr(*rcurrent);
	double e = lengthL1(err) / N;
	if (iterationMonitor) {
		if (!iterationMonitor(0, e))return;
	}
	for (int iter = 0; iter < iters; iter++) {
		Multiply(Ap, A, p);
		vec<double, C> denom = dotVec(p, Ap);
		for (int c = 0; c < C; c++) {
			if (std::abs(denom[c]) < ZERO_TOLERANCE) {
				denom[c] = (denom[c] < 0) ? -ZERO_TOLERANCE : ZERO_TOLERANCE;
			}
		}
		vec<double, C> alpha = lengthVecSqr(*rcurrent) / denom;
		ScaleAdd(x, vec<T, C>(alpha), p);
		ScaleSubtract(*rnext, *rcurrent, vec<T, C>(alpha), Ap);
		vec<double, C> err = lengthVecSqr(*rnext);
		double e = lengthL1(err) / N;
		if (iterationMonitor) {
			if (!iterationMonitor(iter + 1, e))return;
		}
		if (e < tolerance)
			break;
		denom = lengthVecSqr(*rcurrent);
		for (int c = 0; c < C; c++) {
			if (std::abs(denom[c]) < ZERO_TOLERANCE) {
				denom[c] = (denom[c] < 0) ? -ZERO_TOLERANCE : ZERO_TOLERANCE;
			}
		}
		vec<double, C> beta = err / denom;
		ScaleAdd(p, *rnext, vec<T, C>(beta), p);
		std::swap(rcurrent, rnext);
	}
}
template<class T, int C> void SolveVecBICGStab(const Vector<T, C>& b,
		const SparseMatrix<T, C>& A, Vector<T, C>& x, int iters = 100,
		T tolerance = 1E-6f,
		const std::function<bool(int, double)>& iterationMonitor = nullptr) {
	const double ZERO_TOLERANCE = 1E-16;
	size_t N = b.size();
	Vector<T, C> p(N);
	Vector<T, C> Ap(N);

	Vector<T, C> r(N);
	Vector<T, C> rinit;
	Vector<T, C> delta(N);
	Vector<T, C> v(N);
	Vector<T, C> s(N);
	Vector<T, C> t(N);

	vec<double, C> err(T(0));
	v.set(vec<T, C>(T(0)));
	p.set(vec<T, C>(T(0)));

	vec<double, C> rhoNext;
	vec<double, C> rho(1);
	vec<T, C> alpha(1), beta;
	vec<T, C> omega(1);

	MultiplyVec(Ap, A, x);
	Subtract(r, b, Ap);
	rinit = r;
	err = lengthVecSqr(r);
	double e = lengthL1(err) / N;
	if (iterationMonitor) {
		if (!iterationMonitor(0, e))return;
	}
	for (int iter = 0; iter < iters; iter++) {
		rhoNext = dotVec(rinit, r);
		beta = vec<T, C>((rhoNext / rho)) * (alpha / omega);
		ScaleAdd(p, r, beta, p, -beta * omega, v);
		MultiplyVec(v, A, p);
		alpha = vec<T, C>(rhoNext / dotVec(rinit, v));
		ScaleSubtract(s, r, alpha, v);
		if (lengthL1(s) < N * ZERO_TOLERANCE) {
			ScaleAdd(x, x, alpha, p);
			break;
		}
		MultiplyVec(t, A, s);
		omega = vec<T, C>(dotVec(t, s) / dotVec(t, t));
		ScaleAdd(x, x, alpha, p, omega, s);

		ScaleSubtract(r, s, omega, t);
		rho = rhoNext;

		SubtractMultiplyVec(delta, b, A, x);
		vec<double, C> err = lengthVecSqr(delta);
		double e = lengthL1(err) / N;
		if (iterationMonitor) {
			if (!iterationMonitor(iter + 1, e))return;
		}
		if (e < tolerance)
			break;

	}
}
template<class T, int C> void SolveBICGStab(const Vector<T, C>& b,
		const SparseMatrix<T, 1>& A, Vector<T, C>& x, int iters = 100,
		T tolerance = 1E-6f,
		const std::function<bool(int, double)>& iterationMonitor = nullptr) {
	const double ZERO_TOLERANCE = 1E-16;

	size_t N = b.size();
	Vector<T, C> p(N);
	Vector<T, C> r(N);
	Vector<T, C> rinit;
	Vector<T, C> delta(N);
	Vector<T, C> v(N);
	Vector<T, C> s(N);
	Vector<T, C> t(N);

	vec<double, C> err(T(0));
	v.set(vec<T, C>(T(0)));
	p.set(vec<T, C>(T(0)));

	vec<double, C> rhoNext(1);
	vec<double, C> rho(1);
	vec<T, C> alpha(1), beta(1);
	vec<T, C> omega(1);

	SubtractMultiply(r, b, A, x);
	rinit = r;
	err = lengthVecSqr(r);
	double e = lengthL1(err) / N;
	if (iterationMonitor) {
		if (!iterationMonitor(0, e))return;
	}
	for (int iter = 0; iter < iters; iter++) {
		rhoNext = dotVec(rinit, r);
		beta = vec<T, C>((rhoNext / rho)) * (alpha / omega);
		ScaleAdd(p, r, beta, p, -beta * omega, v);
		Multiply(v, A, p);
		alpha = vec<T, C>(rhoNext / dotVec(rinit, v));
		ScaleSubtract(s, r, alpha, v);
		if (lengthL1(s) < N * ZERO_TOLERANCE) {
			ScaleAdd(x, alpha, p);
			break;
		}
		Multiply(t, A, s);
		omega = vec<T, C>(dotVec(t, s) / dotVec(t, t));
		ScaleAdd(x, x, alpha, p, omega, s);

		ScaleSubtract(r, s, omega, t);
		rho = rhoNext;

		SubtractMultiply(delta, b, A, x);
		vec<double, C> err = lengthVecSqr(delta);
		double e = lengthL1(err) / N;
		if (iterationMonitor) {
			if (!iterationMonitor(iter + 1, e))return;
		}
		if (e < tolerance)
			break;

	}
}
}
#endif
