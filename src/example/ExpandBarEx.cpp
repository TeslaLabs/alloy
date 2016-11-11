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

#include "../../include/example/ExpandBarEx.h"

#include "Alloy.h"
#include "AlloyExpandBar.h"
using namespace aly;
ExpandBarEx::ExpandBarEx() :
		Application(800, 600, "Expand Bar Example") {
}
bool ExpandBarEx::init(Composite& rootNode) {
	ExpandBarPtr expandBar = ExpandBarPtr(
			new ExpandBar("exapander", CoordPercent(0.7f, 0.0f),
					CoordPercent(0.3f, 1.0f)));

	CompositePtr geomRegion = CompositePtr(
			new aly::Composite("Geometry", CoordPX(0, 0),
					CoordPerPX(1.0f, 0.0f, 0.0f, 400.0f)));
	geomRegion->setScrollEnabled(true);
	geomRegion->setOrientation(Orientation::Vertical);
	geomRegion->add(
			MakeRegion("Region 1", CoordPX(0, 0),
					CoordPerPX(1.0f, 0.0f, 0.0f, 300.0f), Color(255, 0, 0),COLOR_NONE));
	geomRegion->add(
			MakeRegion("Region 2", CoordPX(0, 0),
					CoordPerPX(1.0f, 0.0f, 0.0f, 300.0f), Color(0, 255, 0),COLOR_NONE));
	expandBar->addRegion(geomRegion, 400, false);
	std::string exampleFile = getContext()->getFullPath(
			"models" + ALY_PATH_SEPARATOR+"monkey.ply");
	RegionPtr apprRegion = RegionPtr(
			new aly::Region("Appearance", CoordPX(0, 0),
					CoordPerPX(1.0f, 0.0f, 0.0f, 3000.0f)));
	expandBar->addRegion(apprRegion, 400, false);
	apprRegion->backgroundColor = MakeColor(128, 64, 255);
	CompositePtr lightRegion = CompositePtr(
			new aly::Composite("Lighting", CoordPX(0, 0),
					CoordPerPX(1.0f, 0.0f, 0.0f, 300.0f)));
	expandBar->addRegion(lightRegion, 300, false);
	RegionPtr renderingRegion = RegionPtr(
			new aly::Region("Rendering", CoordPX(0, 0),
					CoordPerPX(1.0f, 0.0f, 0.0f, 300.0f)));
	expandBar->addRegion(renderingRegion, 300, true);
	ExpandBarPtr childBar = ExpandBarPtr(
			new ExpandBar("Filtering", CoordPercent(0.0f, 0.0f),
					CoordPerPX(1.0f, 0.0f,0.0f,400.0f)));
	CompositePtr motionBlurRegion = CompositePtr(
			new aly::Composite("Motion Blur", CoordPX(0, 0),
					CoordPerPX(1.0f, 0.0f, 0.0f,200.0f)));
	childBar->addRegion(motionBlurRegion,200, false);
	CompositePtr lensBlurRegion = CompositePtr(
			new aly::Composite("Lens Blur", CoordPX(0, 0),
					CoordPerPX(1.0f, 0.0f, 0.0f, 200.0f)));
	childBar->addRegion(lensBlurRegion, 200, false);
	CompositePtr gaussianBlurRegion = CompositePtr(
			new aly::Composite("Gaussian Blur", CoordPX(0, 0),
					CoordPerPX(1.0f, 0.0f, 0.0f, 200.0f)));
	childBar->addRegion(gaussianBlurRegion, 200, false);
	CompositePtr sharpenRegion = CompositePtr(
			new aly::Composite("Sharpen", CoordPX(0, 0),
					CoordPerPX(1.0f, 0.0f, 0.0f, 200.0f)));
	childBar->addRegion(sharpenRegion, 200, false);

	motionBlurRegion->setOrientation(Orientation::Vertical,pixel2(2.0f,2.0f),pixel2(2.0f,2.0f));
	for(int i=0;i<8;i++){
		motionBlurRegion->add(ToggleBoxPtr(new ToggleBox(MakeString()<<"Option "<<i,CoordPX(2.0f,0.0f),CoordPerPX(1.0f,0.0f,-Composite::scrollBarSize-2.0f,30.0f),true,true)));
	}

	lensBlurRegion->setOrientation(Orientation::Vertical,pixel2(2.0f,2.0f),pixel2(2.0f,2.0f));
	for(int i=0;i<8;i++){
		lensBlurRegion->add(CheckBoxPtr(new CheckBox(MakeString()<<"Check "<<i,CoordPX(2.0f,0.0f),CoordPerPX(1.0f,0.0f,-Composite::scrollBarSize-2.0f,30.0f),false,true)));
	}

	gaussianBlurRegion->setOrientation(Orientation::Vertical,pixel2(2.0f,2.0f),pixel2(2.0f,2.0f));
	for(int i=0;i<8;i++){
		TextButtonPtr button=TextButtonPtr(new TextButton(MakeString()<<"Button "<<i,CoordPX(2.0f,0.0f),CoordPerPX(1.0f,0.0f,-Composite::scrollBarSize-2.0f,30.0f)));
		button->setAspectRule(AspectRule::Unspecified);
		gaussianBlurRegion->add(button);
	}

	expandBar->addRegion(childBar, 200, false);
	expandBar->backgroundColor=MakeColor(getContext()->theme.DARKER);
	rootNode.backgroundColor = MakeColor(getContext()->theme.LIGHT);
	rootNode.add(expandBar);
	return true;
}

