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
#include "AlloyDenseSolve.h"
#include "AlloyFileUtil.h"
namespace aly {
void LaplaceFill(const Image4f& sourceImg, Image4f& targetImg, int iterations,
		int levels, float lambda, const std::function<bool(int, int)>& iterationMonitor ) {
	if (sourceImg.dimensions() != targetImg.dimensions())
		throw std::runtime_error(
				MakeString() << "Cannot solve. Image dimensions do not match "
						<< sourceImg.dimensions() << " "
						<< targetImg.dimensions());
	if (levels <= 1) {
		LaplaceFill(sourceImg, targetImg, iterations, lambda, [=](int iter) {
			if (iterationMonitor) {
				return iterationMonitor(0, iter);
			}
			else {
				return true;
			}
		});
	} else {
		std::vector<Image4f> srcPyramid(levels);
		std::vector<Image4f> tarPyramid(levels);
		srcPyramid[0] = sourceImg;
		tarPyramid[0] = targetImg;
		for (int l = 1; l < levels; l++) {
			srcPyramid[l - 1].downSample(srcPyramid[l]);
			tarPyramid[l - 1].downSample(tarPyramid[l]);
		}
		for (int l = levels - 1; l >= 1; l--) {
			if (iterationMonitor) {
				if (!iterationMonitor(l, 0))return;
			}
			LaplaceFill(srcPyramid[l], tarPyramid[l], iterations, lambda, [=](int iter) {
				if (iterationMonitor) {
					return iterationMonitor(l, iter);
				}
				else {
					return true;
				}
			});
			tarPyramid[l].upSample(tarPyramid[l - 1]);
		}
		targetImg = tarPyramid[0];
		LaplaceFill(sourceImg, targetImg, iterations, lambda, [=](int iter) {
			if (iterationMonitor) {
				return iterationMonitor(0, iter);
			}
			else {
				return true;
			}
		});
	}
}
void LaplaceFill(const Image2f& sourceImg, Image2f& targetImg, int iterations,
		int levels, float lambda, const std::function<bool(int, int)>& iterationMonitor) {
	if (sourceImg.dimensions() != targetImg.dimensions())
		throw std::runtime_error(
				MakeString() << "Cannot solve. Image dimensions do not match "
						<< sourceImg.dimensions() << " "
						<< targetImg.dimensions());
	if (levels <= 1) {
		LaplaceFill(sourceImg, targetImg, iterations, lambda, [=](int iter) {
			return iterationMonitor(0, iter);
		});
	} else {
		std::vector<Image2f> srcPyramid(levels);
		std::vector<Image2f> tarPyramid(levels);
		srcPyramid[0] = sourceImg;
		tarPyramid[0] = targetImg;
		for (int l = 1; l < levels; l++) {
			srcPyramid[l - 1].downSample(srcPyramid[l]);
			tarPyramid[l - 1].downSample(tarPyramid[l]);
		}
		for (int l = levels - 1; l >= 1; l--) {
			if (iterationMonitor) {
				if (!iterationMonitor(l, 0))return;
			}
			LaplaceFill(srcPyramid[l], tarPyramid[l], iterations, lambda, [=](int iter) {
				if (iterationMonitor) {
					return iterationMonitor(l, iter);
				}
				else {
					return true;
				}
			});
			tarPyramid[l].upSample(tarPyramid[l - 1]);
		}
		targetImg = tarPyramid[0];
		LaplaceFill(sourceImg, targetImg, iterations, lambda, [=](int iter) {
			if (iterationMonitor) {
				return iterationMonitor(0, iter);
			}
			else {
				return true;
			}
		});
	}
}
void LaplaceFill(const Image2f& sourceImg, Image2f& targetImg, int iterations,
		float lambda,const std::function<bool(int)>& iterationMonitor) {
	if (sourceImg.dimensions() != targetImg.dimensions())
		throw std::runtime_error(
				MakeString() << "Cannot solve. Image dimensions do not match "
						<< sourceImg.dimensions() << " "
						<< targetImg.dimensions());
	Image2f divergence(sourceImg.width, sourceImg.height);
	divergence.set(float2(0.0f));
#pragma omp parallel for
	for (int j = 1; j < sourceImg.height - 1; j++) {
		for (int i = 1; i < sourceImg.width - 1; i++) {
			float2 src = sourceImg(i, j);
			float2 tar = targetImg(i, j);
			float alpha = src.y;
			src.y = 1.0f;
			float2 val1 = sourceImg(i, j);
			float2 val2 = sourceImg(i, j + 1);
			float2 val3 = sourceImg(i, j - 1);
			float2 val4 = sourceImg(i + 1, j);
			float2 val5 = sourceImg(i - 1, j);
			float2 div(0.0f);
			if (val1.y > 0 && val2.y > 0 && val3.y > 0 && val4.y > 0
					&& val5.y > 0) {
				div = val1 - 0.25f * (val2 + val3 + val4 + val5);
				div.y = 0.0f;
			}
			divergence(i, j) = alpha * div;
			targetImg(i, j) = mix(tar, src, alpha);
		}
	}
	const int xShift[] = { 0, 0, 1, 1 };
	const int yShift[] = { 0, 1, 0, 1 };
	for (int iter = 0; iter < iterations; iter++) {
		if (iterationMonitor) {
			if (!iterationMonitor(iter))break;
		}
		for (int k = 0; k < 4; k++) {
			//Assumes color at boundary of target image is fixed!
#pragma omp parallel for
			for (int j = yShift[k] + 1; j < sourceImg.height - 1; j += 2) {
				for (int i = xShift[k] + 1; i < sourceImg.width - 1; i += 2) {
					float2 div = targetImg(i, j)
							- 0.25f
									* (targetImg(i, j - 1) + targetImg(i, j + 1)
											+ targetImg(i - 1, j)
											+ targetImg(i + 1, j));
					div = (div - divergence(i, j));
					targetImg(i, j) -= lambda * div;
				}
			}
		}
	}
}
void LaplaceFill(const Image4f& sourceImg, Image4f& targetImg, int iterations,
		float lambda, const std::function<bool(int)>& iterationMonitor) {
	if (sourceImg.dimensions() != targetImg.dimensions())
		throw std::runtime_error(
				MakeString() << "Cannot solve. Image dimensions do not match "
						<< sourceImg.dimensions() << " "
						<< targetImg.dimensions());
	Image4f divergence(sourceImg.width, sourceImg.height);
	divergence.set(float4(0.0f));
#pragma omp parallel for
	for (int j = 1; j < sourceImg.height - 1; j++) {
		for (int i = 1; i < sourceImg.width - 1; i++) {
			float4 src = sourceImg(i, j);
			float4 tar = targetImg(i, j);
			float alpha = src.w;
			src.w = 1.0f;
			float4 val1 = sourceImg(i, j);
			float4 val2 = sourceImg(i, j + 1);
			float4 val3 = sourceImg(i, j - 1);
			float4 val4 = sourceImg(i + 1, j);
			float4 val5 = sourceImg(i - 1, j);
			float4 div(0.0f);
			if (val1.w > 0 && val2.w > 0 && val3.w > 0 && val4.w > 0
					&& val5.w > 0) {
				div = val1 - 0.25f * (val2 + val3 + val4 + val5);
				div.w = 0.0f;
			}
			divergence(i, j) = alpha * div;
			targetImg(i, j) = mix(tar, src, alpha);
		}
	}
	const int xShift[] = { 0, 0, 1, 1 };
	const int yShift[] = { 0, 1, 0, 1 };
	for (int iter = 0; iter < iterations; iter++) {
		if (iterationMonitor) {
			if (!iterationMonitor(iter))break;
		}
		for (int k = 0; k < 4; k++) {
			//Assumes color at boundary of target image is fixed!
#pragma omp parallel for
			for (int j = yShift[k] + 1; j < sourceImg.height - 1; j += 2) {
				for (int i = xShift[k] + 1; i < sourceImg.width - 1; i += 2) {
					float4 div = targetImg(i, j)
							- 0.25f
									* (targetImg(i, j - 1) + targetImg(i, j + 1)
											+ targetImg(i - 1, j)
											+ targetImg(i + 1, j));
					div = (div - divergence(i, j));
					targetImg(i, j) -= lambda * div;
				}
			}
		}
	}
}
void PoissonInpaint(const Image4f& sourceImg, const Image4f& targetImg,
		Image4f& outImg, int iterations, int levels, float lambda, const std::function<bool(int,int)>& iterationMonitor) {
	//Assumes mask is encoded in the W channel of the source image.
	if (sourceImg.dimensions() != targetImg.dimensions()
			|| sourceImg.dimensions() != outImg.dimensions())
		throw std::runtime_error(
				MakeString() << "Cannot solve. Image dimensions do not match "
						<< sourceImg.dimensions() << " "
						<< targetImg.dimensions());
	if (levels <= 1) {
		PoissonInpaint(sourceImg, targetImg, outImg, iterations, lambda, [=](int iter) {
			if (iterationMonitor) {
				return iterationMonitor(0, iter);
			}
			else {
				return true;
			}
		});
	} else {
		std::vector<Image4f> srcPyramid(levels);
		std::vector<Image4f> tarPyramid(levels);
		std::vector<Image4f> outPyramid(levels);
		srcPyramid[0] = sourceImg;
		tarPyramid[0] = targetImg;
		outPyramid[0] = outImg;
		for (int l = 1; l < levels; l++) {
			srcPyramid[l - 1].downSample(srcPyramid[l]);
			tarPyramid[l - 1].downSample(tarPyramid[l]);
			outPyramid[l - 1].downSample(outPyramid[l]);
		}
		for (int l = levels - 1; l >= 1; l--) {
			if (iterationMonitor) {
				if (!iterationMonitor(l, 0))return;
			}
			PoissonInpaint(srcPyramid[l], tarPyramid[l], outPyramid[l],
					iterations, lambda, [=](int iter) {
				if (iterationMonitor) {
					return iterationMonitor(l, iter);
				}
				else {
					return true;
				}
			});
			outPyramid[l].upSample(outPyramid[l - 1]);
		}
		outImg = outPyramid[0];
		PoissonInpaint(sourceImg, targetImg, outImg, iterations, lambda, [=](int iter) {
			if (iterationMonitor) {
				return iterationMonitor(0, iter);
			}
			else {
				return true;
			}
		});
	}
}
void PoissonInpaint(const Image4f& sourceImg, const Image4f& targetImg,
		Image4f& outImg, int iterations, float lambda, const std::function<bool(int)>& iterationMonitor) {
	//Assumes mask is encoded in the W channel of the source image.
	if (sourceImg.dimensions() != targetImg.dimensions()
			|| sourceImg.dimensions() != outImg.dimensions())
		throw std::runtime_error(
				MakeString() << "Cannot solve. Image dimensions do not match "
						<< sourceImg.dimensions() << " "
						<< targetImg.dimensions());
	Image4f divergence(sourceImg.width, sourceImg.height);
	divergence.set(float4(0.0f));
#pragma omp parallel for
	for (int j = 1; j < sourceImg.height - 1; j++) {
		for (int i = 1; i < sourceImg.width - 1; i++) {
			float4 src = sourceImg(i, j);
			float alpha = src.w;
			src.w = 1.0f;
			float4 val1 = sourceImg(i, j);
			float4 val2 = sourceImg(i, j + 1);
			float4 val3 = sourceImg(i, j - 1);
			float4 val4 = sourceImg(i + 1, j);
			float4 val5 = sourceImg(i - 1, j);
			float4 divSrc(0.0f);
			if (val1.w > 0 && val2.w > 0 && val3.w > 0 && val4.w > 0
					&& val5.w > 0) {
				divSrc = val1 - 0.25f * (val2 + val3 + val4 + val5);
				divSrc.w = 0.0f;
			}

			val1 = targetImg(i, j);
			val2 = targetImg(i, j + 1);
			val3 = targetImg(i, j - 1);
			val4 = targetImg(i + 1, j);
			val5 = targetImg(i - 1, j);
			float4 divTar(0.0f);
			if (val1.w > 0 && val2.w > 0 && val3.w > 0 && val4.w > 0
					&& val5.w > 0) {
				divTar = val1 - 0.25f * (val2 + val3 + val4 + val5);
				divTar.w = 0.0f;
			}
			divergence(i, j) = mix(divTar, divSrc, alpha);
		}
	}
	const int xShift[] = { 0, 0, 1, 1 };
	const int yShift[] = { 0, 1, 0, 1 };
	for (int iter = 0; iter < iterations; iter++) {
		if (iterationMonitor) {
			if (!iterationMonitor(iter))break;
		}
		for (int k = 0; k < 4; k++) {
			//Assumes color at boundary of target image is fixed!
#pragma omp parallel for
			for (int j = yShift[k] + 1; j < sourceImg.height - 1; j += 2) {
				for (int i = xShift[k] + 1; i < sourceImg.width - 1; i += 2) {
					float4 div = outImg(i, j)
							- 0.25f
									* (outImg(i, j - 1) + outImg(i, j + 1)
											+ outImg(i - 1, j)
											+ outImg(i + 1, j));
					div = (div - divergence(i, j));
					outImg(i, j) -= lambda * div;
				}
			}
		}
	}
}
void PoissonInpaint(const Image2f& sourceImg, const Image2f& targetImg,
		Image2f& outImg, int iterations, int levels, float lambda, const std::function<bool(int, int)>& iterationMonitor) {
	//Assumes mask is encoded in the W channel of the source image.
	if (sourceImg.dimensions() != targetImg.dimensions()
			|| sourceImg.dimensions() != outImg.dimensions())
		throw std::runtime_error(
				MakeString() << "Cannot solve. Image dimensions do not match "
						<< sourceImg.dimensions() << " "
						<< targetImg.dimensions());
	if (levels <= 1) {
		PoissonInpaint(sourceImg, targetImg, outImg, iterations, lambda, [=](int iter) {
			if (iterationMonitor) {
				return iterationMonitor(0, iter);
			}
			else {
				return true;
			}
		});
	} else {
		std::vector<Image2f> srcPyramid(levels);
		std::vector<Image2f> tarPyramid(levels);
		std::vector<Image2f> outPyramid(levels);
		srcPyramid[0] = sourceImg;
		tarPyramid[0] = targetImg;
		outPyramid[0] = outImg;
		for (int l = 1; l < levels; l++) {
			srcPyramid[l - 1].downSample(srcPyramid[l]);
			tarPyramid[l - 1].downSample(tarPyramid[l]);
			outPyramid[l - 1].downSample(outPyramid[l]);
		}
		for (int l = levels - 1; l >= 1; l--) {
			if (iterationMonitor) {
				if (!iterationMonitor(l, 0))return;
			}
			PoissonInpaint(srcPyramid[l], tarPyramid[l], outPyramid[l],
					iterations, lambda, [=](int iter) {
				if (iterationMonitor) {
					return iterationMonitor(l, iter);
				}
				else {
					return true;
				}
			});
			outPyramid[l].upSample(outPyramid[l - 1]);
		}
		outImg = outPyramid[0];
		PoissonInpaint(sourceImg, targetImg, outImg, iterations, lambda, [=](int iter) {
			if (iterationMonitor) {
				return iterationMonitor(0, iter);
			}
			else {
				return true;
			}
		});
	}
}
void PoissonInpaint(const Image2f& sourceImg, const Image2f& targetImg,
		Image2f& outImg, int iterations, float lambda, const std::function<bool(int)>& iterationMonitor) {
	//Assumes mask is encoded in the W channel of the source image.
	if (sourceImg.dimensions() != targetImg.dimensions()
			|| sourceImg.dimensions() != outImg.dimensions())
		throw std::runtime_error(
				MakeString() << "Cannot solve. Image dimensions do not match "
						<< sourceImg.dimensions() << " "
						<< targetImg.dimensions());
	Image2f divergence(sourceImg.width, sourceImg.height);
	divergence.set(float2(0.0f));
#pragma omp parallel for
	for (int j = 1; j < sourceImg.height - 1; j++) {
		for (int i = 1; i < sourceImg.width - 1; i++) {
			float2 src = sourceImg(i, j);
			float alpha = src.y;
			src.y = 1.0f;
			float2 val1 = sourceImg(i, j);
			float2 val2 = sourceImg(i, j + 1);
			float2 val3 = sourceImg(i, j - 1);
			float2 val4 = sourceImg(i + 1, j);
			float2 val5 = sourceImg(i - 1, j);
			float2 divSrc(0.0f);
			if (val1.y > 0 && val2.y > 0 && val3.y > 0 && val4.y > 0
					&& val5.y > 0) {
				divSrc = val1 - 0.25f * (val2 + val3 + val4 + val5);
				divSrc.y = 0.0f;
			}

			val1 = targetImg(i, j);
			val2 = targetImg(i, j + 1);
			val3 = targetImg(i, j - 1);
			val4 = targetImg(i + 1, j);
			val5 = targetImg(i - 1, j);
			float2 divTar(0.0f);
			if (val1.y > 0 && val2.y > 0 && val3.y > 0 && val4.y > 0
					&& val5.y > 0) {
				divTar = val1 - 0.25f * (val2 + val3 + val4 + val5);
				divTar.y = 0.0f;
			}
			divergence(i, j) = mix(divTar, divSrc, alpha);
		}
	}
	const int xShift[] = { 0, 0, 1, 1 };
	const int yShift[] = { 0, 1, 0, 1 };
	for (int iter = 0; iter < iterations; iter++) {
		if (iterationMonitor) {
			if (!iterationMonitor(iter))break;
		}
		for (int k = 0; k < 4; k++) {
			//Assumes color at boundary of target image is fixed!
#pragma omp parallel for
			for (int j = yShift[k] + 1; j < sourceImg.height - 1; j += 2) {
				for (int i = xShift[k] + 1; i < sourceImg.width - 1; i += 2) {
					float2 div = outImg(i, j)
							- 0.25f
									* (outImg(i, j - 1) + outImg(i, j + 1)
											+ outImg(i - 1, j)
											+ outImg(i + 1, j));
					div = (div - divergence(i, j));
					outImg(i, j) -= lambda * div;
				}
			}
		}
	}
}
void PoissonBlend(const Image4f& sourceImg, Image4f& targetImg, int iterations,
		int levels, float lambda, const std::function<bool(int, int)>& iterationMonitor) {
	if (sourceImg.dimensions() != targetImg.dimensions())
		throw std::runtime_error(
				MakeString() << "Cannot solve. Image dimensions do not match "
						<< sourceImg.dimensions() << " "
						<< targetImg.dimensions());
	if (levels <= 1) {
		PoissonBlend(sourceImg, targetImg, iterations, lambda, [=](int iter) {
			if (iterationMonitor) {
				return iterationMonitor(0, iter);
			}
			else {
				return true;
			}
		});
	} else {
		std::vector<Image4f> srcPyramid(levels);
		std::vector<Image4f> tarPyramid(levels);
		srcPyramid[0] = sourceImg;
		tarPyramid[0] = targetImg;
		for (int l = 1; l < levels; l++) {
			srcPyramid[l - 1].downSample(srcPyramid[l]);
			tarPyramid[l - 1].downSample(tarPyramid[l]);
		}
		for (int l = levels - 1; l >= 1; l--) {
			if (iterationMonitor) {
				if (!iterationMonitor(l, 0))return;
			}
			PoissonBlend(srcPyramid[l], tarPyramid[l], iterations, lambda, [=](int iter) {
				if (iterationMonitor) {
					return iterationMonitor(l, iter);
				}
				else {
					return true;
				}
			});
			tarPyramid[l].upSample(tarPyramid[l - 1]);
		}
		targetImg = tarPyramid[0];
		PoissonBlend(sourceImg, targetImg, iterations, lambda, [=](int iter) {
			if (iterationMonitor) {
				return iterationMonitor(0, iter);
			}
			else {
				return true;
			}
		});
	}
}
void PoissonBlend(const Image2f& sourceImg, Image2f& targetImg, int iterations,
		int levels, float lambda, const std::function<bool(int, int)>& iterationMonitor) {
	if (sourceImg.dimensions() != targetImg.dimensions())
		throw std::runtime_error(
				MakeString() << "Cannot solve. Image dimensions do not match "
						<< sourceImg.dimensions() << " "
						<< targetImg.dimensions());
	if (levels <= 1) {
		PoissonBlend(sourceImg, targetImg, iterations, lambda, [=](int iter) {
			if (iterationMonitor) {
				return iterationMonitor(0, iter);
			}
			else {
				return true;
			}
		});
	} else {
		std::vector<Image2f> srcPyramid(levels);
		std::vector<Image2f> tarPyramid(levels);
		srcPyramid[0] = sourceImg;
		tarPyramid[0] = targetImg;
		for (int l = 1; l < levels; l++) {
			srcPyramid[l - 1].downSample(srcPyramid[l]);
			tarPyramid[l - 1].downSample(tarPyramid[l]);
		}
		for (int l = levels - 1; l >= 1; l--) {
			if (iterationMonitor) {
				if (!iterationMonitor(l, 0))return;
			}
			PoissonBlend(srcPyramid[l], tarPyramid[l], iterations, lambda, [=](int iter) {
				if (iterationMonitor) {
					return iterationMonitor(l, iter);
				}
				else {
					return true;
				}
			});
			tarPyramid[l].upSample(tarPyramid[l - 1]);
		}
		targetImg = tarPyramid[0];
		PoissonBlend(sourceImg, targetImg, iterations, lambda, [=](int iter) {
			if (iterationMonitor) {
				return iterationMonitor(0, iter);
			}
			else {
				return true;
			}
		});
	}
}
void PoissonBlend(const Image4f& sourceImg, Image4f& targetImg, int iterations,
		float lambda, const std::function<bool(int)>& iterationMonitor) {
	if (sourceImg.dimensions() != targetImg.dimensions())
		throw std::runtime_error(
				MakeString() << "Cannot solve. Image dimensions do not match "
						<< sourceImg.dimensions() << " "
						<< targetImg.dimensions());
	Image4f divergence(sourceImg.width, sourceImg.height);
#pragma omp parallel for
	for (int j = 1; j < sourceImg.height - 1; j++) {
		for (int i = 1; i < sourceImg.width - 1; i++) {
			float4 val1 = sourceImg(i, j);
			float4 val2 = sourceImg(i, j + 1);
			float4 val3 = sourceImg(i, j - 1);
			float4 val4 = sourceImg(i + 1, j);
			float4 val5 = sourceImg(i - 1, j);
			float4 div(0.0f);
			if (val1.w > 0 && val2.w > 0 && val3.w > 0 && val4.w > 0
					&& val5.w > 0) {
				div = val1 - 0.25f * (val2 + val3 + val4 + val5);
				div.w = 0.0f;
			}
			divergence(i, j) = div;

		}
	}
	const int xShift[] = { 0, 0, 1, 1 };
	const int yShift[] = { 0, 1, 0, 1 };
	const float THRESHOLD = 0.5;
	for (int iter = 0; iter < iterations; iter++) {
		if (iterationMonitor) {
			if (!iterationMonitor(iter))break;
		}
		for (int k = 0; k < 4; k++) {
			//Assumes color at boundary of target image is fixed!
#pragma omp parallel for
			for (int j = yShift[k] + 1; j < sourceImg.height - 1; j += 2) {
				for (int i = xShift[k] + 1; i < sourceImg.width - 1; i += 2) {
					float4 val1 = targetImg(i, j);
					float4 val2 = targetImg(i, j + 1);
					float4 val3 = targetImg(i, j - 1);
					float4 val4 = targetImg(i + 1, j);
					float4 val5 = targetImg(i - 1, j);
					float4 div(0.0f);
					if (val1.w >= THRESHOLD && val2.w >= THRESHOLD
							&& val3.w >= THRESHOLD && val4.w >= THRESHOLD
							&& val5.w >= THRESHOLD) {
						div = val1 - 0.25f * (val2 + val3 + val4 + val5);
						div = (div - divergence(i, j));
						div.w = 0;
						targetImg(i, j) -= lambda * div;
					}
				}
			}
		}
	}
}

void PoissonBlend(const Image2f& sourceImg, Image2f& targetImg, int iterations,
		float lambda, const std::function<bool(int)>& iterationMonitor) {
	if (sourceImg.dimensions() != targetImg.dimensions())
		throw std::runtime_error(
				MakeString() << "Cannot solve. Image dimensions do not match "
						<< sourceImg.dimensions() << " "
						<< targetImg.dimensions());
	Image2f divergence(sourceImg.width, sourceImg.height);
#pragma omp parallel for
	for (int j = 1; j < sourceImg.height - 1; j++) {
		for (int i = 1; i < sourceImg.width - 1; i++) {
			float2 val1 = sourceImg(i, j);
			float2 val2 = sourceImg(i, j + 1);
			float2 val3 = sourceImg(i, j - 1);
			float2 val4 = sourceImg(i + 1, j);
			float2 val5 = sourceImg(i - 1, j);
			float2 div(0.0f);
			if (val1.y > 0 && val2.y > 0 && val3.y > 0 && val4.y > 0
					&& val5.y > 0) {
				div = val1 - 0.25f * (val2 + val3 + val4 + val5);
				div.y = 0.0f;
			}
			divergence(i, j) = div;

		}
	}
	const int xShift[] = { 0, 0, 1, 1 };
	const int yShift[] = { 0, 1, 0, 1 };
	const float THRESHOLD = 0.5;
	for (int iter = 0; iter < iterations; iter++) {
		if (iterationMonitor) {
			if (!iterationMonitor(iter))break;
		}
		for (int k = 0; k < 4; k++) {
			//Assumes color at boundary of target image is fixed!
#pragma omp parallel for
			for (int j = yShift[k] + 1; j < sourceImg.height - 1; j += 2) {
				for (int i = xShift[k] + 1; i < sourceImg.width - 1; i += 2) {
					float2 val1 = targetImg(i, j);
					float2 val2 = targetImg(i, j + 1);
					float2 val3 = targetImg(i, j - 1);
					float2 val4 = targetImg(i + 1, j);
					float2 val5 = targetImg(i - 1, j);
					float2 div(0.0f);
					if (val1.y >= THRESHOLD && val2.y >= THRESHOLD
							&& val3.y >= THRESHOLD && val4.y >= THRESHOLD
							&& val5.y >= THRESHOLD) {
						div = val1 - 0.25f * (val2 + val3 + val4 + val5);
						div = (div - divergence(i, j));
						div.y = 0;
						targetImg(i, j) -= lambda * div;
					}
				}
			}
		}
	}
}
}
