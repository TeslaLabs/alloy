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

#ifndef LOCATOR_EX_H_
#define LOCATOR_EX_H_

#include <AlloyLocator.h>
#include "AlloyApplication.h"
#include "AlloyVector.h"
class LocatorEx: public aly::Application {
protected:
	std::vector<std::pair<size_t, float>> kNN;
	std::vector < aly::float2i> nearest;
	aly::float2i closest;
	aly::float2 cursor;
	aly::Vector2f samples;
	aly::DrawPtr drawRegion;
	std::vector<aly::uint3> delaunayTriangles;
	std::unique_ptr<aly::Locator2f> locator;
	std::unique_ptr<aly::Matcher2f> matcher;
public:
	LocatorEx();
	bool init(aly::Composite& rootNode);
};

#endif 
