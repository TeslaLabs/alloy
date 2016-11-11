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
#include "../../include/example/UnitsEx.h"
using namespace aly;
UnitsEx::UnitsEx() :
		Application(800, 600, "Units Example") {
}
bool UnitsEx::init(Composite& rootNode) {
	rootNode.add(MakeTextLabel("{10% from left edge and 10% from top edge}", CoordPercent(0.1f, 0.1f), CoordPX(400, 30), FontType::Normal, UnitPT(16.0f)));
	rootNode.add(MakeTextLabel("{5 pixels from left edge and 200 pixels from top edge}", CoordPX(5,200), CoordPX(400, 30), FontType::Normal, UnitPT(16.0f)));
	rootNode.add(MakeTextLabel("{100% from left edge minus 550 pixels and 250 pixels from top edge}", CoordPerPX(1.0f, 0.0f, -550, 250), CoordPX(550, 30), FontType::Normal, UnitPT(16.0f),COLOR_WHITE,HorizontalAlignment::Right,VerticalAlignment::Top));
	TextLabelPtr label1 =MakeTextLabel("{Centered and 300 pixels from top}", CoordPerPX(0.5,0,0,300), CoordPX(300, 30), FontType::Normal, UnitPT(16.0f), COLOR_WHITE, HorizontalAlignment::Center, VerticalAlignment::Top);
	label1->setOrigin(Origin::TopCenter);
	rootNode.add(label1);
	TextLabelPtr label2 = MakeTextLabel("{Centered and text aligned bottom}", CoordPercent(0.5, 1.0), CoordPX(300, 30), FontType::Normal, UnitPT(16.0f), COLOR_WHITE, HorizontalAlignment::Center, VerticalAlignment::Top);
	label2->setOrigin(Origin::BottomCenter);
	rootNode.add(label2);
	//getContext()->setDebug(true);
	return true;
}

