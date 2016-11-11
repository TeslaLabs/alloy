/*
 * Copyright(C) 2014, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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

#include "AlloyMath.h"
#include <random>
using namespace std;
namespace aly {

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

template<class T, int m, int n> void SVD_INTERNAL(const matrix<T, m, n>& M,
		matrix<T, m, m>& U, matrix<T, m, n>& D, matrix<T, n, n>& Vt) {
	double v[n][n];
	double u[m][m];
	double w[n];
	double rv1[n];
	int flag, i, its, j, jj, k, l, nm;
	double c, f, h, s, x, y, z;
	double anorm = 0.0, g = 0.0, scale = 0.0;
	if (m < n) {
		throw std::runtime_error(
				"SVD error, rows must be greater than or equal to cols.");
	}
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			u[i][j] = M(i, j);
		}
	}
	for (i = 0; i < n; i++) {
		l = i + 1;
		rv1[i] = scale * g;
		g = s = scale = 0.0;
		if (i < m) {
			for (k = i; k < m; k++)
				scale += std::abs((double) u[k][i]);
			if (scale) {
				for (k = i; k < m; k++) {
					u[k][i] = ((double) u[k][i] / scale);
					s += ((double) u[k][i] * (double) u[k][i]);
				}
				f = (double) u[i][i];
				g = -sign(std::sqrt(s), f);
				h = f * g - s;
				u[i][i] = (f - g);
				if (i != n - 1) {
					for (j = l; j < n; j++) {
						for (s = 0.0, k = i; k < m; k++)
							s += ((double) u[k][i] * (double) u[k][j]);
						f = s / h;
						for (k = i; k < m; k++)
							u[k][j] += (f * (double) u[k][i]);
					}
				}
				for (k = i; k < m; k++)
					u[k][i] = ((double) u[k][i] * scale);
			}
		}
		w[i] = (scale * g);
		g = s = scale = 0.0;
		if (i < m && i != n - 1) {
			for (k = l; k < n; k++)
				scale += std::abs((double) u[i][k]);
			if (scale) {
				for (k = l; k < n; k++) {
					u[i][k] = ((double) u[i][k] / scale);
					s += ((double) u[i][k] * (double) u[i][k]);
				}
				f = (double) u[i][l];
				g = -sign(std::sqrt(s), f);
				h = f * g - s;
				u[i][l] = (f - g);
				for (k = l; k < n; k++)
					rv1[k] = (double) u[i][k] / h;
				if (i != m - 1) {
					for (j = l; j < m; j++) {
						for (s = 0.0, k = l; k < n; k++)
							s += ((double) u[j][k] * (double) u[i][k]);
						for (k = l; k < n; k++)
							u[j][k] += (s * rv1[k]);
					}
				}
				for (k = l; k < n; k++)
					u[i][k] = ((double) u[i][k] * scale);
			}
		}
		anorm = aly::max(anorm, (std::abs((double) w[i]) + std::abs(rv1[i])));
	}
	for (i = n - 1; i >= 0; i--) {
		if (i < n - 1) {
			if (g) {
				for (j = l; j < n; j++)
					v[j][i] = (((double) u[i][j] / (double) u[i][l]) / g);
				for (j = l; j < n; j++) {
					for (s = 0.0, k = l; k < n; k++)
						s += ((double) u[i][k] * (double) v[k][j]);
					for (k = l; k < n; k++)
						v[k][j] += (s * (double) v[k][i]);
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
		g = (double) w[i];
		if (i < n - 1)
			for (j = l; j < n; j++)
				u[i][j] = 0.0;
		if (g) {
			g = 1.0 / g;
			if (i != n - 1) {
				for (j = l; j < n; j++) {
					for (s = 0.0, k = l; k < m; k++)
						s += ((double) u[k][i] * (double) u[k][j]);
					f = (s / (double) u[i][i]) * g;
					for (k = i; k < m; k++)
						u[k][j] += (f * (double) u[k][i]);
				}
			}
			for (j = i; j < m; j++)
				u[j][i] = ((double) u[j][i] * g);
		} else {
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
				if (nm>=0&&std::abs(w[nm]) + anorm == anorm)
					break;
			}
			if (flag) {
				c = 0.0;
				s = 1.0;
				for (i = l; i <= k; i++) {
					f = s * rv1[i];
					if (std::abs(f) + anorm != anorm) {
						g = (double) w[i];
						h = pythag(f, g);
						w[i] = h;
						h = 1.0 / h;
						c = g * h;
						s = (-f * h);
						for (j = 0; j < m; j++) {
							y = (double) u[j][nm];
							z = (double) u[j][i];
							u[j][nm] = (y * c + z * s);
							u[j][i] = (z * c - y * s);
						}
					}
				}
			}
			z = (double) w[k];
			if (l == k) {
				if (z < 0.0) {
					w[k] = (-z);
					for (j = 0; j < n; j++)
						v[j][k] = (-v[j][k]);
				}
				int iii, jjj;
				for (iii = k; (iii < n - 1) && (w[iii] < w[iii + 1]); iii++) {
					std::swap(w[iii], w[iii + 1]);
					for (jjj = 0; jjj < m; jjj++)
						std::swap(u[jjj][iii], u[jjj][iii + 1]);
					for (jjj = 0; jjj < n; jjj++)
						std::swap(v[jjj][iii], v[jjj][iii + 1]);
				}
				break;
			}
			if (its >= 30) {
				throw runtime_error("SVD did not converge.");
			}
			x = (double) w[l];
			nm = k - 1;
			y = (double) w[nm];
			g = rv1[nm];
			h = rv1[k];
			f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
			g = pythag(f, 1.0);
			f = ((x - z) * (x + z) + h * ((y / (f + sign(g, f))) - h)) / x;
			c = s = 1.0;
			for (j = l; j <= nm; j++) {
				i = j + 1;
				g = rv1[i];
				y = (double) w[i];
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
					x = (double) v[jj][j];
					z = (double) v[jj][i];
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
					y = (double) u[jj][j];
					z = (double) u[jj][i];
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
				D(i, j) = (T) w[j];
			} else {
				D(i, j) = T(0);
			}
		}
	}
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < m; j++) {
			U(i, j) = (T) u[i][j];
		}
	}
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			Vt(i, j) = (T) v[j][i];
		}
	}
}
void SVD(const matrix<float, 2, 2> &A, matrix<float, 2, 2>& U,
		matrix<float, 2, 2>& D, matrix<float, 2, 2>& Vt) {
	SVD_INTERNAL(A, U, D, Vt);
}
void SVD(const matrix<float, 3, 3> &A, matrix<float, 3, 3>& U,
		matrix<float, 3, 3>& D, matrix<float, 3, 3>& Vt) {
	SVD_INTERNAL(A, U, D, Vt);
}
void SVD(const matrix<float, 4, 4> &A, matrix<float, 4, 4>& U,
		matrix<float, 4, 4>& D, matrix<float, 4, 4>& Vt) {
	SVD_INTERNAL(A, U, D, Vt);
}
void SVD(const matrix<double, 2, 2> &A, matrix<double, 2, 2>& U,
		matrix<double, 2, 2>& D, matrix<double, 2, 2>& Vt) {
	SVD_INTERNAL(A, U, D, Vt);
}
void SVD(const matrix<double, 3, 3> &A, matrix<double, 3, 3>& U,
		matrix<double, 3, 3>& D, matrix<double, 3, 3>& Vt) {
	SVD_INTERNAL(A, U, D, Vt);
}
void SVD(const matrix<double, 4, 4> &A, matrix<double, 4, 4>& U,
		matrix<double, 4, 4>& D, matrix<double, 4, 4>& Vt) {
	SVD_INTERNAL(A, U, D, Vt);
}


float RandomUniform(float min, float max) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> noise(min,max);
	return noise(gen);
}
int RandomUniform(int min, int max) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<int> noise(min, max);
	return noise(gen);
}
double RandomUniform(double min, double max) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<double> noise(min, max);
	return noise(gen);
}
double RandomGaussian(double mean, double stddev) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::normal_distribution<double> noise(mean, stddev);
	return noise(gen);
}
float RandomGaussian(float mean, float stddev) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::normal_distribution<float> noise(mean, stddev);
	return noise(gen);
}

}
