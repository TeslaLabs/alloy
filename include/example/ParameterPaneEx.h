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

#ifndef PARAMETERPANE_EX_H_
#define PARAMETERPANE_EX_H_
#include "AlloyApplication.h"
class ParameterPaneEx: public aly::Application {
protected:
	aly::Number paramFloat1;
	aly::Number paramInt1;
	aly::Number paramFloat2;
	aly::Number paramLow;
	aly::Number paramHi;
	aly::Number paramTweenLo;
	aly::Number paramTweenHi;
	std::vector<aly::Number> paramIntegerList;
	std::vector<aly::Number> paramFloatList;
	std::vector<aly::Number> paramDoubleList;
	int paramSelect;
	std::string paramString;
	aly::Color paramColor;
	std::string paramFile;
	bool paramBool1;
	bool paramBool2;
	std::vector<std::string> paramMultiFile;
public:
	ParameterPaneEx();
	bool init(aly::Composite& rootNode);
};

#endif 
