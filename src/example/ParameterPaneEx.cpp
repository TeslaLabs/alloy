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

#include "Alloy.h"
#include "AlloyParameterPane.h"
#include "../../include/example/ParameterPaneEx.h"
using namespace aly;
ParameterPaneEx::ParameterPaneEx() :
		Application(800, 600, "Parameter Pane Example") {
	paramFloat1 = aly::Float(5.0f);
	paramInt1 = aly::Integer(3);
	paramFloat2 = aly::Float(0.5f);
	paramSelect = 2;
	paramColor = aly::Color(1.0f, 0.0f, 0.0f, 1.0f);
	paramBool1 = false;
	paramBool2 = false;
	paramLow = Integer(1001);
	paramHi = Integer(1100);
	paramTweenLo = Integer(1025);
	paramTweenHi = Integer(1075);
	paramIntegerList.push_back(Integer(1001));
	paramIntegerList.push_back(Integer(1070));

	paramFloatList.push_back(Float(12.34f));
	paramFloatList.push_back(Float(56.78f));

	paramDoubleList.push_back(Double((double)ALY_PI));
	paramDoubleList.push_back(Double((double)std::exp(1.0)));

	paramString = "Hello";
}
bool ParameterPaneEx::init(Composite& rootNode) {
	paramFile = getContext()->getFullPath("images/sfsunset.png");
	paramMultiFile.push_back(getContext()->getFullPath("images/sfsunset.png"));
	paramMultiFile.push_back(getContext()->getFullPath("images/sfmarket.png"));
	BorderCompositePtr borderComposite = BorderCompositePtr(new BorderComposite("Layout",CoordPX(0.0f,0.0f),CoordPercent(1.0f,1.0f),true));
	CompositePtr centerPane = CompositePtr(new Composite("Center", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f)));
	ParameterPanePtr paramPane = ParameterPanePtr(new ParameterPane("Parameter Pane",CoordPX(0.0f,0.0f),CoordPercent(1.0f,1.0f),26.0f));

	paramPane->addGroup("Group 1",true);
		paramPane->addFileField("File", paramFile);
		paramPane->addSelectionField("Selection", paramSelect, std::vector<std::string>{"Cool", "Neat", "Awesome", "Boring"});
		paramPane->addNumberField("Float", paramFloat1);
		paramPane->addNumberField("Integer", paramInt1);
		paramPane->addRangeField("Range", paramLow,paramHi);
		paramPane->addRangeField("Range Tween", paramTweenLo, paramTweenHi, Integer(1001), Integer(1100));
		paramPane->addTextField("String", paramString);

		
	paramPane->addGroup("Group 2",true);
		paramPane->addMultiFileSelector("Multi-File", paramMultiFile);
		paramPane->addColorField("Color", paramColor);
		paramPane->addNumberField("Tween", paramFloat2, Float(0.0f),Float(1.0f));
		paramPane->addToggleBox("Toggle", paramBool1);
		paramPane->addCheckBox("Check", paramBool2);


	paramPane->addGroup("Group 3",true);
		paramPane->addNumberVectorField("Integers",paramIntegerList,NumberType::Integer,8.0f);
		paramPane->addNumberVectorField("Floats",paramFloatList,NumberType::Float,8.0f);
		paramPane->addNumberVectorField("Doubles",paramDoubleList,NumberType::Double,8.0f);

	centerPane->backgroundColor = MakeColor(getContext()->theme.LIGHT);
	paramPane->backgroundColor=MakeColor(getContext()->theme.DARKER);
	borderComposite->setCenter(centerPane);
	borderComposite->setEast(paramPane, UnitPX(400.0f));
	rootNode.add(borderComposite);
	return true;
}

