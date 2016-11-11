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

#include <AlloyOptimization.h>
namespace aly {
	template<class T> void SolveCGInternal(const Vec<T>& b, const SparseMat<T>& A, Vec<T>& x, int iters, double tolerance,
			const std::function<bool(int, double)>& iterationMonitor) {
		const double ZERO_TOLERANCE = 1E-16;
		double err(0.0);
		size_t N = b.size();
		Vec<T> p(N);
		Vec<T> Ap(N);
		Multiply(Ap, A, x);
		Vec<T> tmp1(N), tmp2(N);
		Vec<T>* rcurrent = &tmp1;
		Vec<T>* rnext = &tmp2;
		Subtract(*rcurrent, b, Ap);
		p = *rcurrent;
		err = lengthSqr(*rcurrent);
		double e = err / N;
		if (iterationMonitor) {
			if (!iterationMonitor(0, e))
				return;
		}
		for (int iter = 0; iter < iters; iter++) {
			Multiply(Ap, A, p);
			double denom = dot(p, Ap);
			if (std::abs(denom) < ZERO_TOLERANCE) {
				denom = (denom < 0) ? -ZERO_TOLERANCE : ZERO_TOLERANCE;
			}
			double alpha = lengthSqr(*rcurrent) / denom;
			ScaleAdd(x, T(alpha), p);
			ScaleSubtract(*rnext, *rcurrent, T(alpha), Ap);
			double err = lengthSqr(*rnext);
			double e = err / N;
			if (iterationMonitor) {
				if (!iterationMonitor(iter + 1, e))
					return;
			}
			if (e < tolerance)
				break;
			denom = lengthSqr(*rcurrent);
			if (std::abs(denom) < ZERO_TOLERANCE) {
				denom = (denom < 0) ? -ZERO_TOLERANCE : ZERO_TOLERANCE;
			}
			double beta = err / denom;
			ScaleAdd(p, *rnext, T(beta), p);
			std::swap(rcurrent, rnext);
		}
	}
	template<class T> void SolveBICGStabInternal(const Vec<T>& b, const SparseMat<T>& A, Vec<T>& x, int iters, double tolerance,
			const std::function<bool(int, double)>& iterationMonitor) {
		const double ZERO_TOLERANCE = 1E-16;

		size_t N = b.size();
		Vec<T> p(N);
		Vec<T> r(N);
		Vec<T> rinit;
		Vec<T> delta(N);
		Vec<T> v(N);
		Vec<T> s(N);
		Vec<T> t(N);

		double err(T(0));
		v.set(T(0));
		p.set(T(0));

		double rhoNext(1);
		double rho(1);
		T alpha(1), beta(1);
		T omega(1);

		SubtractMultiply(r, b, A, x);
		rinit = r;
		err = lengthSqr(r);
		double e = err / N;
		if (iterationMonitor) {
			if (!iterationMonitor(0, e))
				return;
		}
		for (int iter = 0; iter < iters; iter++) {
			rhoNext = dot(rinit, r);
			beta = T((rhoNext / rho)) * (alpha / omega);
			ScaleAdd(p, r, beta, p, -beta * omega, v);
			Multiply(v, A, p);
			alpha = T(rhoNext / dot(rinit, v));
			ScaleSubtract(s, r, alpha, v);
			if (lengthL1(s) < N * ZERO_TOLERANCE) {
				ScaleAdd(x, alpha, p);
				break;
			}
			Multiply(t, A, s);
			omega = T(dot(t, s) / dot(t, t));
			ScaleAdd(x, x, alpha, p, omega, s);

			ScaleSubtract(r, s, omega, t);
			rho = rhoNext;

			SubtractMultiply(delta, b, A, x);
			double err = lengthSqr(delta);
			double e = err / N;
			if (iterationMonitor) {
				if (!iterationMonitor(iter + 1, e))
					return;
			}
			if (e < tolerance)
				break;

		}
	}
	template<class T> void SolveLevenbergMarquardtInternal(SparseProblem<T>& problem, Vec<T>& p, int maxIterations, double errorTolerance,
			const std::function<bool(int, double)>& monitor) {
		//Implementation not tested yet! Use at your own risk!
		T nu = T(2);
		const T tau = T(1E-3);
		problem.reset();
		Vec<T> delta, pnew;
		Vec<T> eps = problem.residual(p);
		SparseMat<T> J, Jt, JtJ, A, Astar;
		problem.differentiate(p, J);
		Jt = J.transpose();
		Vec<T> g = Jt * eps;
		A = Jt * J;
		T trace = T(0);
		for (size_t r = 0; r < A.rows; r++) {
			trace = std::max(A(r, r), trace);
		}
		T mu = tau * trace;
		double err = lengthInf(g);
		int iter = 0;
		bool stop = false;
		delta.resize(p.size(), T(0));
		while (err > errorTolerance && iter < maxIterations && !stop) {
			T rho = 0;
			do {
				Astar = A;
				for (size_t r = 0; r < A.rows; r++) {
					Astar(r, r) += mu;
				}
				if (monitor) {
					if (!monitor(iter, err)) {
						stop = true;
						break;
					}
				}
				SolveBICGStab(g, Astar, delta, maxIterations, errorTolerance);
				if (monitor) {
					if (!monitor(iter, err)) {
						stop = true;
						break;
					}
				}
				if (lengthSqr(delta) <= errorTolerance * errorTolerance * lengthSqr(p)) {
					stop = true;
				} else {
					pnew = p + delta;
					rho = T((lengthSqr(eps) - lengthSqr(problem.residual(pnew))) / dot(delta, mu * delta + g));
					if (rho > 0) {
						p = pnew;
						problem.differentiate(p, J);
						Jt = J.transpose();
						A = Jt * J;
						g = Jt * eps;
						eps = problem.residual(p);
						err = lengthInf(g);
						if (monitor) {
							if (!monitor(iter, err)) {
								stop = true;
								break;
							}
						}
						if (err <= errorTolerance)
							stop = true;
						T rp = T(2 * rho - 1);
						mu = mu * std::max(T(1.0 / 3.0), 1 - rp * rp * rp);
						nu = T(2);
					} else {
						mu *= nu;
						nu *= T(2);
					}
				}
			} while (rho <= 0 && !stop);
			iter++;
		}
	}

	template<class T> void SolveDogLegInternal(SparseProblem<T>& problem, Vec<T>& p, int maxIterations , double errorTolerance, T trust,
			const std::function<bool(int, double)>& monitor) {
		//Implementation not tested yet! Use at your own risk!
		SparseMat<T> J, Jt, JtJ, A;
		problem.differentiate(p, J);
		Jt = J.transpose();
		A = Jt * J;
		Vec<T> eps = problem.residual(p);
		Vec<T> g = Jt * eps;
		Vec<T> pnew;
		Vec<T> delta_sd, delta_dl, delta_gn;
		double err = lengthInf(g);
		bool stop = false;
		int iter = 0;
		T beta;
		while (err > errorTolerance && iter < maxIterations && !stop) {
			T rho = T(0);
			Vec<T> delta_sd = T(lengthSqr(g) / lengthSqr(J * g)) * g;
			bool GNcomputed = false;
			do {
				T mag = T(length(delta_sd));
				if (mag >= trust) {
					delta_dl = (trust / mag) * delta_sd;
				} else {
					if (monitor) {
						if (!monitor(iter, err)) {
							stop = true;
							break;
						}
					}
					if (!GNcomputed) {
						SolveBICGStab(g, A, delta_gn);
						GNcomputed = true;
					}
					if (monitor) {
						if (!monitor(iter, err)) {
							stop = true;
							break;
						}
					}
					if (length(delta_gn) <= trust) {
						delta_dl = delta_gn;
					} else {
						//choose beta such that delta_dl is length trust
						Vec<T> v = delta_gn - delta_sd;
						T l2 = T(lengthSqr(v));
						T neg_c = -T(dot(v, delta_sd));
						T discriminant = T(neg_c * neg_c - l2 * (lengthSqr(delta_sd) - trust * trust));
						beta = (neg_c + std::sqrt(std::max(discriminant, T(0)))) / l2;
						delta_dl = delta_sd + beta * v;
					}
				}

				if (length(delta_dl) <= errorTolerance * length(p)) {
					stop = true;
				} else {
					pnew = p + delta_dl;
					rho = T((lengthSqr(eps) - lengthSqr(problem.residual(pnew))) / (2 * dot(g, delta_dl) - dot(delta_dl, A * delta_dl)));
					if (rho > 0) {
						p = pnew;
						problem.differentiate(p, J);
						Jt = J.transpose();
						A = Jt * J;
						g = Jt * eps;
						eps = problem.residual(p);
						err = lengthInf(g);
						if (monitor) {
							if (!monitor(iter, err)) {
								stop = true;
								break;
							}
						}
						if (err <= errorTolerance)
							stop = true;
					}
					if (rho < 0.25f) {
						trust *= 0.1f;
					} else if (rho > 0.75) {
						trust *= 2.0f;
					}
					if (trust < errorTolerance*length(p)) {
						stop = true;
					}
				}
				if (monitor) {
					if (!monitor(iter, err)) {
						stop = true;
						break;
					}
				}
			} while (rho <= 0 && !stop);
			iter++;
		}
	}
	void SolveCG(const Vec<float>& b, const SparseMat<float>& A, Vec<float>& x, int iters, double tolerance,
			const std::function<bool(int, double)>& iterationMonitor) {
		SolveCGInternal(b, A, x, iters, tolerance, iterationMonitor);
	}
	void SolveBICGStab(const Vec<float>& b, const SparseMat<float>& A, Vec<float>& x, int iters, double tolerance,
			const std::function<bool(int, double)>& iterationMonitor) {
		SolveBICGStabInternal(b, A, x, iters, tolerance, iterationMonitor);
	}
	void SolveLevenbergMarquardt(SparseProblem<float>& problem, Vec<float>& p, int maxIterations, double errorTolerance,
			const std::function<bool(int, double)>& monitor) {
		SolveLevenbergMarquardtInternal(problem, p, maxIterations, errorTolerance, monitor);
	}
	void SolveDogLeg(SparseProblem<float>& problem, Vec<float>& p, int maxIterations, double errorTolerance, float trust,
			const std::function<bool(int, double)>& monitor) {
		SolveDogLegInternal(problem, p, maxIterations, errorTolerance, trust, monitor);
	}

	void SolveCG(const Vec<double>& b, const SparseMat<double>& A, Vec<double>& x, int iters, double tolerance,
			const std::function<bool(int, double)>& iterationMonitor) {
		SolveCGInternal(b, A, x, iters, tolerance, iterationMonitor);
	}
	void SolveBICGStab(const Vec<double>& b, const SparseMat<double>& A, Vec<double>& x, int iters, double tolerance,
			const std::function<bool(int, double)>& iterationMonitor) {
		SolveBICGStabInternal(b, A, x, iters, tolerance, iterationMonitor);
	}
	void SolveLevenbergMarquardt(SparseProblem<double>& problem, Vec<double>& p, int maxIterations, double errorTolerance,
			const std::function<bool(int, double)>& monitor) {
		SolveLevenbergMarquardtInternal(problem, p, maxIterations, errorTolerance, monitor);
	}
	void SolveDogLeg(SparseProblem<double>& problem, Vec<double>& p, int maxIterations, double errorTolerance, double trust,
			const std::function<bool(int, double)>& monitor) {
		SolveDogLegInternal(problem, p, maxIterations, errorTolerance, trust, monitor);
	}
}
