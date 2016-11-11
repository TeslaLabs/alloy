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
#include "../../include/example/TweenEx.h"
using namespace aly;
TweenEx::TweenEx() :
		Application(800, 600, "Tween Example") {
}
bool TweenEx::init(Composite& rootNode) {
	TextLabelPtr label = MakeTextLabel("Hello There!", CoordPercent(0.5f, 0.9f), CoordPX(100, 30), FontType::Normal, UnitPT(16.0f),RGBA(255, 255, 255, 255), HorizontalAlignment::Center,VerticalAlignment::Bottom);
	label->setOrigin(Origin::MiddleCenter);
	label->setTruncate(false);
	std::shared_ptr<ImageGlyph> img = createImageGlyph(getFullPath("images/sfmarket.png"));
	GlyphRegionPtr imgr = MakeGlyphRegion(img, CoordPercent(0.5f, 0.45f),CoordPX(100,300), AspectRule::FixedHeight, Color(32, 64, 128, 255),Color(128, 128, 128, 128), Color(200, 200, 200, 255), UnitPX(1.0f));
	GlyphRegionPtr iconr = MakeGlyphRegion(createAwesomeGlyph(0xf1b3),CoordPX(20, 20), CoordPX(50, 50), Color(32, 64, 128, 255),Color(255, 255, 255, 255));
	imgr->setOrigin(Origin::MiddleCenter);
	addTween(imgr->foregroundColor, Color(128, 128, 128, 255),Color(128, 128, 128, 0), 3.0, SineOut());
	addTween(imgr->dimensions, CoordPX(50, 50), CoordPX(300, 300), 1.0, SineOut());
	addTween(iconr->backgroundColor, Color(255, 64, 32, 255),Color(32, 64, 255, 255), 3.0, SineIn());
	addTween(iconr->foregroundColor, Color(0, 0, 0, 255),Color(255, 255, 255, 255), 3.0, SineIn());
	addTween(iconr->position, CoordPX(100, 10), CoordPerPX(0.5f,0.0f,-25.0f, 10.0f), 3.0, ExponentialOut());
	addTween(label->position, CoordPercent(0.5f, 0.7f), CoordPercent(0.5f, 0.9f), 1.0f, ExponentialOut())->addCompleteEvent([=](Tweenable* tween) {
		addTween(label->fontSize, UnitPT(16.0f), UnitPT(36.0f), 1.0f, ExponentialIn())->addCompleteEvent(
			[=](Tweenable* object) {
			label->setLabel( "Did you like that tween?");
		});
	});
	
	rootNode.add(label);
	rootNode.add(iconr);
	rootNode.add(imgr);
	return true;
}

