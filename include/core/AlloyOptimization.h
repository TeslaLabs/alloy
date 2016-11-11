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

#ifndef INCLUDE_CORE_ALLOYOPTIMIZATION_H_
#define INCLUDE_CORE_ALLOYOPTIMIZATION_H_
#include <AlloyOptimizationMath.h>
namespace aly {

	template<class T> struct SparseProblem {
		virtual size_t getInputSize() const=0;
		virtual size_t getOutputSize() const=0;
		virtual Vec<T> constraint() =0;
		virtual Vec<T> evaluate(const Vec<T>& input) =0;
		virtual void differentiate(const size_t& index, const Vec<T>& input, std::vector<std::pair<size_t, T>>& derivative) = 0;
		virtual void reset() {
		}

		void differentiate(const Vec<T>& input, SparseMat<T>& J) {
			J.resize(getOutputSize(), getInputSize());
#pragma omp parallel for
			for (int r = 0; r < (int)J.rows; r++) {
				std::vector<std::pair<size_t, T>> row;
				differentiate(r, input, row);
				for (auto pr : row) {
					J.set(r, pr.first, pr.second);
				}
			}
		}
		Vec<T> residual(const Vec<T>& input) {
			return constraint() - evaluate(input);
		}
		T errorL1(const Vec<T>& input) {
			return lengthL1(constraint() - evaluate(input));
		}
		T errorL2(const Vec<T>& input) {
			return length(constraint() - evaluate(input));
		}
		T errorSqr(const Vec<T>& input) {
			return lengthSqr(constraint() - evaluate(input));
		}
		T errorInf(const Vec<T>& input) {
			return lengthInf(constraint() - evaluate(input));
		}
		virtual ~SparseProblem() {

		}
	};
	void SolveCG(const Vec<float>& b, const SparseMat<float>& A, Vec<float>& x, int iters = 100, double tolerance = 1E-6,
			const std::function<bool(int, double)>& iterationMonitor = nullptr);
	void SolveBICGStab(const Vec<float>& b, const SparseMat<float>& A, Vec<float>& x, int iters = 100, double tolerance = 1E-6,
			const std::function<bool(int, double)>& iterationMonitor = nullptr);
	void SolveLevenbergMarquardt(SparseProblem<float>& problem, Vec<float>& p, int maxIterations = 100, double errorTolerance = 1E-9,
			const std::function<bool(int, double)>& monitor = nullptr);
	void SolveDogLeg(SparseProblem<float>& problem, Vec<float>& p, int maxIterations = 100, double errorTolerance = 1E-9, float trust = 1E3f,
			const std::function<bool(int, double)>& monitor = nullptr);

	void SolveCG(const Vec<double>& b, const SparseMat<double>& A, Vec<double>& x, int iters = 100, double tolerance = 1E-6,
			const std::function<bool(int, double)>& iterationMonitor = nullptr);
	void SolveBICGStab(const Vec<double>& b, const SparseMat<double>& A, Vec<double>& x, int iters = 100, double tolerance = 1E-6,
			const std::function<bool(int, double)>& iterationMonitor = nullptr);
	void SolveLevenbergMarquardt(SparseProblem<double>& problem, Vec<double>& p, int maxIterations = 100, double errorTolerance = 1E-9,
			const std::function<bool(int, double)>& monitor = nullptr);
	void SolveDogLeg(SparseProblem<double>& problem, Vec<double>& p, int maxIterations = 100, double errorTolerance = 1E-9, double trust = 1E3f,
			const std::function<bool(int, double)>& monitor = nullptr);

}
#endif /* INCLUDE_CORE_ALLOYOPTIMIZATION_H_ */
