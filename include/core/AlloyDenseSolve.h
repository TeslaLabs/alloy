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
#ifndef ALLOYDENSESOLVER_H_
#define ALLOYDENSESOLVER_H_
#include "AlloyMath.h"
#include "AlloyImage.h"
#include "AlloyDenseMatrix.h"
namespace aly {
	bool SANITY_CHECK_DENSE_SOLVE();
	bool SANITY_CHECK_ROBUST_SOLVE();
	void PoissonBlend(const Image4f& in, Image4f& out, int iterations, int levels,float lambda = 0.99f, const std::function<bool(int,int)>& iterationMonitor = nullptr);
	void PoissonBlend(const Image4f& in, Image4f& out, int iterations,float lambda = 0.99f, const std::function<bool(int)>& iterationMonitor = nullptr);
	void PoissonBlend(const Image2f& in, Image2f& out, int iterations, int levels,float lambda = 0.99f, const std::function<bool(int,int)>& iterationMonitor = nullptr);
	void PoissonBlend(const Image2f& in, Image2f& out, int iterations,float lambda = 0.99f, const std::function<bool(int)>& iterationMonitor = nullptr);
	void PoissonInpaint(const Image4f& source, const Image4f& target, Image4f& out,int iterations, int levels, float lambda = 0.99f , const std::function<bool(int, int)>& iterationMonitor = nullptr);
	void PoissonInpaint(const Image4f& source, const Image4f& target, Image4f& out,int iterations, float lambda = 0.99f, const std::function<bool(int)>& iterationMonitor = nullptr);
	void PoissonInpaint(const Image2f& source, const Image2f& target, Image2f& out,int iterations, int levels, float lambda = 0.99f, const std::function<bool(int, int)>& iterationMonitor = nullptr);
	void PoissonInpaint(const Image2f& source, const Image2f& target, Image2f& out,int iterations, float lambda = 0.99f, const std::function<bool(int)>& iterationMonitor = nullptr);
	void LaplaceFill(const Image4f& sourceImg, Image4f& targetImg, int iterations,int levels, float lambda = 0.99f, const std::function<bool(int, int)>& iterationMonitor=nullptr);
	void LaplaceFill(const Image4f& sourceImg, Image4f& targetImg, int iterations,float lambda = 0.99f , const std::function<bool(int)>& iterationMonitor=nullptr);
	void LaplaceFill(const Image2f& sourceImg, Image2f& targetImg, int iterations,float lambda = 0.99f, const std::function<bool(int)>& iterationMonitor = nullptr);
	void LaplaceFill(const Image2f& sourceImg, Image2f& targetImg, int iterations,int levels, float lambda = 0.99f, const std::function<bool(int, int)>& iterationMonitor = nullptr);

	/******************************************************************************
	 * XLISP-STAT 2.1 Copyright (c) 1990, by Luke Tierney
	 * XLISP version 2.1, Copyright (c) 1989, by David Betz.
	 *
	 * Permission to use, copy, modify, distribute, and sell this software and its
	 * documentation for any purpose is hereby granted without fee, provided that
	 * the above copyright notice appear in all copies and that both that
	 * copyright notice and this permission notice appear in supporting
	 * documentation, and that the name of Luke Tierney and David Betz not be
	 * used in advertising or publicity pertaining to distribution of the software
	 * without specific, written prior permission.  Luke Tierney and David Betz
	 * make no representations about the suitability of this software for any
	 * purpose. It is provided "as is" without express or implied warranty.
	 *
	 * LUKE TIERNEY AND DAVID BETZ DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
	 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
	 * IN NO EVENT SHALL LUKE TIERNEY NOR DAVID BETZ BE LIABLE FOR ANY SPECIAL,
	 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
	 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
	 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	 * PERFORMANCE OF THIS SOFTWARE.
	 *
	 * XLISP-STAT AUTHOR:
	 *               Luke Tierney
	 *               School of Statistics
	 *               University of Minnesota
	 *               Minneapolis, MN 55455
	 *               (612) 625-7843
	 *
	 *       Email Address:
	 *               internet: luke@umnstat.stat.umn.edu
	 *
	 * XLISP AUTHOR:
	 *              David Betz
	 *              P.O. Box 144
	 *              Peterborough, NH 03458
	 *              (603) 924-4145
	 ******************************************************************************
	 * XLISP-STAT 2.1 was ported to the Amiga by
	 *              J.K. Lindsey
	 *              Faculty of Economic, Business and Social Sciences,
	 *              University of Liege,
	 *              Sart Tilman B31,
	 *              4000 Liege,
	 *              Belgium
	 *              32-41-56.29.64
	 *
	 * The above permission and disclaimer also applies to all of the specifically
	 * Amiga portions of this software, with the restriction that the Amiga
	 * version not be used for any military-related applications.
	 ******************************************************************************

	 */

	template<class T, int C> void SVD(const DenseMatrix<T, C>& M,
		DenseMatrix<T, C>& U, DenseMatrix<T, C>& D, DenseMatrix<T, C>& Vt,
		double zeroTolerance = 0) {
		const int m = M.rows;
		const int n = M.cols;
		std::vector<std::vector<double>> v(n, std::vector<double>(n));
		std::vector<std::vector<double>> u(m, std::vector<double>(m));
		std::vector<double> w(n);
		std::vector<double> rv1(n);

		int flag, i, its, j, jj, k, l, nm;
		double c, f, h, s, x, y, z;
		double anorm, g, scale;
		U.resize(m, m);
		Vt.resize(n, n);
		D.resize(m, n);
		for (int cc = 0; cc < C; cc++) {
			anorm = 0.0, g = 0.0, scale = 0.0;
			if (m < n) {
				throw std::runtime_error(
					"SVD error, rows must be greater than or equal to cols.");
			}
			for (int i = 0; i < m; i++) {
				for (int j = 0; j < n; j++) {
					u[i][j] = (double)M[i][j][cc];
				}
			}
			for (i = 0; i < n; i++) {
				l = i + 1;
				rv1[i] = scale * g;
				g = s = scale = 0.0;
				if (i < m) {
					for (k = i; k < m; k++)
						scale += std::abs((double)u[k][i]);
					if (scale > zeroTolerance) {
						for (k = i; k < m; k++) {
							u[k][i] = ((double)u[k][i] / scale);
							s += ((double)u[k][i] * (double)u[k][i]);
						}
						f = (double)u[i][i];
						g = -sign(std::sqrt(s), f);
						h = f * g - s;
						u[i][i] = (f - g);
						if (i != n - 1) {
							for (j = l; j < n; j++) {
								for (s = 0.0, k = i; k < m; k++)
									s += ((double)u[k][i] * (double)u[k][j]);
								f = s / h;
								for (k = i; k < m; k++)
									u[k][j] += (f * (double)u[k][i]);
							}
						}
						for (k = i; k < m; k++)
							u[k][i] = ((double)u[k][i] * scale);
					}
				}
				w[i] = (scale * g);
				g = s = scale = 0.0;
				if (i < m && i != n - 1) {
					for (k = l; k < n; k++)
						scale += std::abs((double)u[i][k]);
					if (scale > zeroTolerance) {
						for (k = l; k < n; k++) {
							u[i][k] = ((double)u[i][k] / scale);
							s += ((double)u[i][k] * (double)u[i][k]);
						}
						f = (double)u[i][l];
						g = -sign(std::sqrt(s), f);
						h = f * g - s;
						u[i][l] = (f - g);
						for (k = l; k < n; k++)
							rv1[k] = (double)u[i][k] / h;
						if (i != m - 1) {
							for (j = l; j < m; j++) {
								for (s = 0.0, k = l; k < n; k++)
									s += ((double)u[j][k] * (double)u[i][k]);
								for (k = l; k < n; k++)
									u[j][k] += (s * rv1[k]);
							}
						}
						for (k = l; k < n; k++)
							u[i][k] = ((double)u[i][k] * scale);
					}
				}
				anorm = aly::max(anorm,
					(std::abs((double)w[i]) + std::abs(rv1[i])));
			}
			for (i = n - 1; i >= 0; i--) {
				if (i < n - 1) {
					if (std::abs(g) > zeroTolerance) {
						for (j = l; j < n; j++)
							v[j][i] = (((double)u[i][j] / (double)u[i][l]) / g);
						for (j = l; j < n; j++) {
							for (s = 0.0, k = l; k < n; k++)
								s += ((double)u[i][k] * (double)v[k][j]);
							for (k = l; k < n; k++)
								v[k][j] += (s * (double)v[k][i]);
						}
					}
					for (j = l; j < n; j++)
						v[i][j] = v[j][i] = 0.0;
				}
				v[i][i] = 1.0;
				g = rv1[i];
				l = i;
			}
			for (i = n - 1; i >= 0; i--) {
				l = i + 1;
				g = (double)w[i];
				if (i < n - 1)
					for (j = l; j < n; j++)
						u[i][j] = 0.0;
				if (std::abs(g) > zeroTolerance) {
					g = 1.0 / g;
					if (i != n - 1) {
						for (j = l; j < n; j++) {
							for (s = 0.0, k = l; k < m; k++)
								s += ((double)u[k][i] * (double)u[k][j]);
							f = (s / (double)u[i][i]) * g;
							for (k = i; k < m; k++)
								u[k][j] += (f * (double)u[k][i]);
						}
					}
					for (j = i; j < m; j++)
						u[j][i] = ((double)u[j][i] * g);
				}
				else {
					for (j = i; j < m; j++)
						u[j][i] = 0.0;
				}
				++u[i][i];
			}
			for (k = n - 1; k >= 0; k--) {
				for (its = 0; its < 30; its++) {
					flag = 1;
					for (l = k; l >= 0; l--) {
						nm = l - 1;
						if (std::abs(rv1[l]) + anorm == anorm) {
							flag = 0;
							break;
						}
						if (nm >= 0 && std::abs((double)w[nm]) + anorm == anorm) {
							break;
						}
					}
					if (flag) {
						c = 0.0;
						s = 1.0;
						for (i = l; i <= k; i++) {
							f = s * rv1[i];
							if (std::abs(f) + anorm != anorm) {
								g = (double)w[i];
								h = pythag(f, g);
								w[i] = h;
								h = 1.0 / h;
								c = g * h;
								s = (-f * h);
								for (j = 0; j < m; j++) {
									y = (double)u[j][nm];
									z = (double)u[j][i];
									u[j][nm] = (y * c + z * s);
									u[j][i] = (z * c - y * s);
								}
							}
						}
					}
					z = (double)w[k];
					if (l == k) {
						if (z < 0.0) {
							w[k] = (-z);
							for (j = 0; j < n; j++)
								v[j][k] = (-v[j][k]);
						}
						int iii, jjj;
						for (iii = k; (iii < n - 1) && (w[iii] < w[iii + 1]);
						iii++) {
							std::swap(w[iii], w[iii + 1]);
							for (jjj = 0; jjj < m; jjj++)
								std::swap(u[jjj][iii], u[jjj][iii + 1]);
							for (jjj = 0; jjj < n; jjj++)
								std::swap(v[jjj][iii], v[jjj][iii + 1]);
						}
						break;
					}
					if (its >= 30) {
						throw std::runtime_error("SVD did not converge.");
					}
					x = (double)w[l];
					nm = k - 1;
					y = (double)w[nm];
					g = rv1[nm];
					h = rv1[k];
					f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
					g = pythag(f, 1.0);
					f = ((x - z) * (x + z) + h * ((y / (f + sign(g, f))) - h)) / x;
					c = s = 1.0;
					for (j = l; j <= nm; j++) {
						i = j + 1;
						g = rv1[i];
						y = (double)w[i];
						h = s * g;
						g = c * g;
						z = pythag(f, h);
						rv1[j] = z;
						c = f / z;
						s = h / z;
						f = x * c + g * s;
						g = g * c - x * s;
						h = y * s;
						y = y * c;
						for (jj = 0; jj < n; jj++) {
							x = (double)v[jj][j];
							z = (double)v[jj][i];
							v[jj][j] = (x * c + z * s);
							v[jj][i] = (z * c - x * s);
						}
						z = pythag(f, h);
						w[j] = z;
						if (z) {
							z = 1.0 / z;
							c = f * z;
							s = h * z;
						}
						f = (c * g) + (s * y);
						x = (c * y) - (s * g);
						for (jj = 0; jj < m; jj++) {
							y = (double)u[jj][j];
							z = (double)u[jj][i];
							u[jj][j] = (y * c + z * s);
							u[jj][i] = (z * c - y * s);
						}
					}
					rv1[l] = 0.0;
					rv1[k] = f;
					w[k] = x;
				}
			}
			for (int i = 0; i < m; i++) {
				for (int j = 0; j < n; j++) {
					if (i == j) {
						D[i][j][cc] = (T)w[j];
					}
					else {
						D[i][j][cc] = T(0);
					}
				}
			}
			for (int i = 0; i < m; i++) {
				for (int j = 0; j < m; j++) {
					U[i][j][cc] = (T)u[i][j];
				}
			}
			for (int i = 0; i < n; i++) {
				for (int j = 0; j < n; j++) {
					Vt[i][j][cc] = (T)v[j][i];
				}
			}
		}
	}
	template<class T, int C> DenseMatrix<T, C> inverse(const DenseMatrix<T, C>& M,
		double zeroTolerance = 0.0) {
		if (M.rows != M.cols) {
			throw std::runtime_error(MakeString()<<"Could not invert matrix.  Rows and columns must agree: [" << M.rows << ", " << M.cols<<"]");
		}
		DenseMatrix<T, C> U, D, Vt;
		SVD(M, U, D, Vt);
		int K = aly::min(D.rows, D.cols);
		for (int k = 0; k < K; k++) {
			vec<double, C> d = vec<double, C>(D[k][k]);
			for (int c = 0; c < C; c++) {
				if (std::abs(d[c]) > zeroTolerance) {
					d[c] = 1.0 / d[c];
				}
			}
			D[k][k] = vec<T, C>(d);
		}
		return (U * D * Vt).transpose();
	}
	template<class T, int C> Vector<T, C> SolveSVD(const DenseMatrix<T, C>& A,
		const Vector<T, C>& b) {
		if (A.rows != (int)b.size()) {
			throw std::runtime_error(
				MakeString()
				<< "Matrix row dimensions and vector length must agree. A=["
				<< A.rows << "," << A.cols << "] b=[" << b.size()
				<< "]");
		}
		if (A.rows != A.cols) {
			DenseMatrix<T, C> At = A.transpose();
			DenseMatrix<T, C> AtA = At * A;
			Vector<T, C> Atb = At * b;
			return inverse(AtA) * Atb;
		}
		else {
			return inverse(A) * b;
		}
	}
	//Back port of NIST's Java Implementation of LINPACK called JAMA. Code is licensed for free use in the public domain. http://math.nist.gov/javanumerics/jama/
	/** LU Decomposition.
	 <P>
	 For an m-by-n matrix A with m >= n, the LU decomposition is an m-by-n
	 unit lower triangular matrix L, an n-by-n upper triangular matrix U,
	 and a permutation vector piv of length m so that A(piv,:) = L*U.
	 If m < n, then L is m-by-m and U is m-by-n.
	 <P>
	 The LU decompostion with pivoting always exists, even if the matrix is
	 singular, so the constructor will never fail.  The primary use of the
	 LU decomposition is in the solution of square systems of simultaneous
	 linear equations.  This will fail if isNonsingular() returns false.
	 */

	template<class T, int C> bool LU(const DenseMatrix<T, C>& A,
		DenseMatrix<T, 1>& L, DenseMatrix<T, 1>& U, std::vector<int>& piv,
		int cc = 0, const double zeroTolerance = 0.0) {
		const int m = A.rows;
		const int n = A.cols;
		std::vector<std::vector<double>> LU(m, std::vector<double>(n, 0.0));
		piv.resize(m);
		std::vector<double> LUcolj(m, 0.0);
		int pivsign;
		double* LUrowi;
		L.resize(m, n);
		U.resize(n, n);
		bool nonSingular = true;
		for (int i = 0; i < m; i++) {
			for (int j = 0; j < n; j++) {
				LU[i][j] = (double)A[i][j][cc];
			}
		}
		for (int i = 0; i < m; i++) {
			piv[i] = i;
		}
		pivsign = 1;
		for (int j = 0; j < n; j++) {
			for (int i = 0; i < m; i++) {
				LUcolj[i] = LU[i][j];
			}
			for (int i = 0; i < m; i++) {
				LUrowi = &LU[i][0];
				int kmax = aly::min(i, j);
				double s = 0.0;
				for (int k = 0; k < kmax; k++) {
					s += LUrowi[k] * LUcolj[k];
				}
				LUrowi[j] = LUcolj[i] -= s;
			}
			int p = j;
			for (int i = j + 1; i < m; i++) {
				if (std::abs(LUcolj[i]) > std::abs(LUcolj[p])) {
					p = i;
				}
			}
			if (p != j) {
				for (int k = 0; k < n; k++) {
					std::swap(LU[p][k], LU[j][k]);
				}
				std::swap(piv[p], piv[j]);
				pivsign = -pivsign;
			}
			if (j < m && std::abs(LU[j][j]) > zeroTolerance) {
				for (int i = j + 1; i < m; i++) {
					LU[i][j] /= LU[j][j];
				}
			}
		}
		for (int j = 0; j < n; j++) {
			if (std::abs(LU[j][j]) <= zeroTolerance) {
				nonSingular = false;
				break;
			}
		}
		for (int i = 0; i < m; i++) {
			for (int j = 0; j < n; j++) {
				if (i > j) {
					L[i][j].x = (T)LU[i][j];
				}
				else if (i == j) {
					L[i][j].x = T(1.0);
				}
				else {
					L[i][j].x = T(0.0);
				}
			}
		}
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				if (i <= j) {
					U[i][j].x = T(LU[i][j]);
				}
				else {
					U[i][j].x = T(0.0);
				}
			}
		}
		return nonSingular;
	}

	template<class T, int C> Vector<T, C> SolveLU(const DenseMatrix<T, C>& A,
		const Vector<T, C>& b) {

		if (A.rows != (int)b.size()) {
			throw std::runtime_error(
				MakeString()
				<< "Matrix row dimensions and vector length must agree. A=["
				<< A.rows << "," << A.cols << "] b=[" << b.size()
				<< "]");
		}
		if (A.rows != A.cols) {
			DenseMatrix<T, C> At = A.transpose();
			DenseMatrix<T, C> AtA = At * A;
			Vector<T, C> Atb = At * b;
			int n = AtA.cols;
			Vector<T, C> x(A.cols);
			Vector<T, C> y(A.cols);
			DenseMatrix<T, 1> L, U;
			std::vector<int> piv;
			for (int cc = 0; cc < C; cc++) {
				bool nonSingular = LU(AtA, L, U, piv, cc);
				if (!nonSingular) {
					throw std::runtime_error("Matrix is singular.");
				}
				// Forward solve Ly = b
				for (int i = 0; i < n; i++) {
					y[i][cc] = Atb[piv[i]][cc];
					for (int j = 0; j < i; j++) {
						y[i][cc] -= L[i][j].x * y[j][cc];
					}
					y[i][cc] /= L[i][i].x;
				}
				// Backward solve Ux = y
				for (int i = n - 1; i >= 0; i--) {
					x[i][cc] = y[i][cc];
					for (int j = i + 1; j < n; j++) {
						x[i][cc] -= U[i][j].x * x[j][cc];
					}
					x[i][cc] /= U[i][i].x;
				}
			}
			return x;
		}
		else {
			int n = A.cols;

			Vector<T, C> x(A.cols);
			Vector<T, C> y(A.cols);
			DenseMatrix<T, 1> L, U;
			std::vector<int> piv;
			for (int cc = 0; cc < C; cc++) {
				bool nonSingular = LU(A, L, U, piv, cc);
				if (!nonSingular) {
					throw std::runtime_error("Matrix is singular.");
				}
				// Forward solve Ly = b
				for (int i = 0; i < n; i++) {
					y[i][cc] = b[piv[i]][cc];
					for (int j = 0; j < i; j++) {
						y[i][cc] -= L[i][j].x * y[j][cc];
					}
					y[i][cc] /= L[i][i].x;
				}
				// Backward solve Ux = y
				for (int i = n - 1; i >= 0; i--) {
					x[i][cc] = y[i][cc];
					for (int j = i + 1; j < n; j++) {
						x[i][cc] -= U[i][j].x * x[j][cc];
					}
					x[i][cc] /= U[i][i].x;
				}
			}
			return x;
		}
	}

	/** QR Decomposition.
	 <P>
	 For an m-by-n matrix A with m >= n, the QR decomposition is an m-by-n
	 orthogonal matrix Q and an n-by-n upper triangular matrix R so that
	 A = Q*R.
	 <P>
	 The QR decompostion always exists, even if the matrix does not have
	 full rank, so the constructor will never fail.  The primary use of the
	 QR decomposition is in the least squares solution of nonsquare systems
	 of simultaneous linear equations.  This will fail if isFullRank()
	 returns false.
	 */
	template<class T, int C> bool QR(const DenseMatrix<T, C>& A,
		DenseMatrix<T, C>& Q, DenseMatrix<T, C>& R) {
		const int m = A.rows;
		const int n = A.cols;
		std::vector<std::vector<double>> QR(m, std::vector<double>(n));
		std::vector<double> Rdiag(m);
		R.resize(n, n);
		Q.resize(m, n);
		bool nonSingular = true;
		for (int cc = 0; cc < C; cc++) {
			for (int i = 0; i < m; i++) {
				for (int j = 0; j < n; j++) {
					QR[i][j] = (double)A[i][j][cc];
				}
			}
			for (int k = 0; k < n; k++) {
				double nrm = 0;
				for (int i = k; i < m; i++) {
					nrm = pythag(nrm, QR[i][k]);
				}
				if (nrm != 0.0) {
					if (QR[k][k] < 0) {
						nrm = -nrm;
					}
					for (int i = k; i < m; i++) {
						QR[i][k] /= nrm;
					}
					QR[k][k] += 1.0;
					for (int j = k + 1; j < n; j++) {
						double s = 0.0;
						for (int i = k; i < m; i++) {
							s += QR[i][k] * QR[i][j];
						}
						s = -s / QR[k][k];
						for (int i = k; i < m; i++) {
							QR[i][j] += s * QR[i][k];
						}
					}
				}
				Rdiag[k] = -nrm;
			}

			for (int i = 0; i < n; i++) {
				for (int j = 0; j < n; j++) {
					if (i < j) {
						R[i][j][cc] = T(QR[i][j]);
					}
					else if (i == j) {
						R[i][j][cc] = T(Rdiag[i]);
					}
					else {
						R[i][j][cc] = T(0.0);
					}
				}
			}
			for (int j = 0; j < n; j++) {
				if (Rdiag[j] == 0) {
					nonSingular = false;
					break;
				}
			}
			for (int k = n - 1; k >= 0; k--) {
				for (int i = 0; i < m; i++) {
					Q[i][k][cc] = T(0.0);
				}
				Q[k][k][cc] = T(1.0);
				for (int j = k; j < n; j++) {
					if (QR[k][k] != 0) {
						double s = 0.0;
						for (int i = k; i < m; i++) {
							s += QR[i][k] * Q[i][j][cc];
						}
						s = -s / QR[k][k];
						for (int i = k; i < m; i++) {
							Q[i][j][cc] += T(s * QR[i][k]);
						}
					}
				}
			}
		}
		return nonSingular;
	}

	template<class T, int C> Vector<T, C> SolveQR(const DenseMatrix<T, C>& A,
		const Vector<T, C>& b) {

		if (A.rows != (int)b.size()) {
			throw std::runtime_error(
				MakeString()
				<< "Matrix row dimensions and vector length must agree. A=["
				<< A.rows << "," << A.cols << "] b=[" << b.size()
				<< "]");
		}
		if (A.rows != A.cols) {
			DenseMatrix<T, C> At = A.transpose();
			DenseMatrix<T, C> AtA = At * A;
			Vector<T, C> Atb = At * b;
			int n = AtA.cols;
			Vector<T, C> x(A.cols);
			DenseMatrix<T, C> Q, R;
			bool nonSingular = QR(AtA, Q, R);
			if (!nonSingular) {
				throw std::runtime_error("Matrix is singular.");
			}
			// Compute Y = transpose(Q)*B
			x = Q.transpose() * Atb;
			// Solve R*X = Y;
			for (int k = n - 1; k >= 0; k--) {
				x[k] /= R[k][k];
				for (int i = 0; i < k; i++) {
					x[i] -= x[k] * R[i][k];
				}
			}
			return x;
		}
		else {
			int n = A.cols;

			Vector<T, C> x(A.cols);
			DenseMatrix<T, C> Q, R;
			bool nonSingular = QR(A, Q, R);
			if (!nonSingular) {
				throw std::runtime_error("Matrix is singular.");
			}
			// Compute Y = transpose(Q)*B
			x = Q.transpose() * b;
			// Solve R*X = Y;
			for (int k = n - 1; k >= 0; k--) {
				x[k] /= R[k][k];
				for (int i = 0; i < k; i++) {
					x[i] -= x[k] * R[i][k];
				}
			}
			return x;
		}
	}
	enum class MatrixFactorization {
		SVD, QR, LU
	};
	template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const MatrixFactorization& type) {
		switch (type) {
		case MatrixFactorization::SVD:
			return ss << "SVD";
		case MatrixFactorization::QR:
			return ss << "QR";
		case MatrixFactorization::LU:
			return ss << "LU";
		}
		return ss;
	}
	template<class T, int C> Vector<T, C> Solve(const DenseMatrix<T, C>& A,
		const Vector<T, C>& b, MatrixFactorization factor =
		MatrixFactorization::SVD) {
		switch (factor) {
		case MatrixFactorization::SVD:
			return SolveSVD(A, b);
		case MatrixFactorization::QR:
			return SolveQR(A, b);
		case MatrixFactorization::LU:
			return SolveLU(A, b);
		}
		return Vector<T, C>();
	}
	template<class T, int C> Vector<T, C> SolveRobust(const DenseMatrix<T, C>& A, const Vector<T, C>& b,
		int p = 1, int iterations = 100, double errorTolerance = 1E-6f,
		double zeroTolerance = 1E-16, MatrixFactorization factor = MatrixFactorization::SVD) {
		int N = (int)b.size();
		Vector<T, C> W(N);
		W.set(vec<T, C>(T(1)));
		Vector<T, C> X;
		double lastError = std::numeric_limits<double>::max();
		for (int iter = 0;iter < iterations;iter++) {
			X = SolveQR(W*A, W*b);
			Vector<T, C> R = b - A*X;
			vec<double, C> err = lengthVecSqr(R);
			double e = lengthL1(err) / N;
			if (std::abs(e - lastError) < errorTolerance) {
				break;
			}
			lastError = e;
#pragma omp parallel for
			for (int n = 0;n < N;n++) {
				vec<T, C> w;
				vec<T, C> r = R[n];
				for (int c = 0;c < C;c++) {
					w[c] = std::pow(std::max(std::abs(r[c]), T(zeroTolerance)), -p);
				}
				W[n] = w;
			}
		}
		return X;
	}
	template<class T, int C> Vector<T, C> SolveRansac(const DenseMatrix<T, C>& A, const Vector<T, C>& b,
		int sampleSize, double inlierTolerance = 0.01f, int iterations = 100) {
		int N = (int)b.size();
		DenseMatrix<T, C> As;
		Vector<T, C> bs;

		std::vector<int> order(N);
		std::random_device rd;
		std::mt19937 g(rd());
		for (int n = 0;n < N;n++) {
			order[n] = n;
		}
		Vector<T, C> X;
		if (N <= sampleSize) {
			return SolveQR(A, b);
		}
		std::shuffle(order.begin(), order.end(), g);
		Vector<T, C> W(N);
		int bestInliner = 0;

		Vector<T, C> BestX;
		int offset = 0;
		for (int iter = 0;iter < iterations;iter++) {
			W.set(vec<T, C>(T(0.0)));
			As.resize(sampleSize, A.cols);
			bs.resize(sampleSize);
			for (int i = 0;i < sampleSize;i++) {
				int idx = order[(i + offset)%N];
				As[i] = A[idx];
				bs[i] = b[idx];
			}
			X = SolveQR(As, bs);
			Vector<T, C> R = b - A*X;
			int count = 0;
			for (int n = 0;n < N;n++) {
				if (lengthL1(R[n]) < inlierTolerance) {
					count++;
				}
			}
			if (count > bestInliner) {
				BestX = X;
				bestInliner = count;
			}
			if (count > std::max(N / 2, sampleSize))break;//Good enough set of inliers, break
			offset += sampleSize;
			if (offset >= N) {
				std::shuffle(order.begin(), order.end(), g);
				offset = 0;
			}
		}
		if (bestInliner<A.cols) {
			return SolveQR(A, b);
		}
		order.clear();
		Vector<T, C> R = b - A*BestX;
		for (int n = 0;n < N;n++) {
			if (lengthL1(R[n]) < inlierTolerance) {
				order.push_back(n);
			}
		}
		if ((int)order.size() < sampleSize) {
			return BestX;
		}
		As.resize((int)order.size(), A.cols);
		bs.resize((int)order.size());
		for (int i = 0;i < (int)order.size();i++) {
			int idx = order[i];
			As[i] = A[idx];
			bs[i] = b[idx];
		}
		X = SolveQR(As, bs);
		return X;
	}
}
#endif
