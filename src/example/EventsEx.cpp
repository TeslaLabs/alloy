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
#include "../../include/example/EventsEx.h"
using namespace aly;
EventsEx::EventsEx() :
		Application(800, 600, "Events Example") {
}
bool EventsEx::init(Composite& rootNode) {

	TextLabelPtr labelA = MakeTextLabel("Region A",
			CoordPercent(0.1f, 0.333333f), CoordPercent(0.3f, 0.333333f),
			FontType::Bold, UnitPT(16.0f), COLOR_WHITE,
			HorizontalAlignment::Center, VerticalAlignment::Middle);
	TextLabelPtr labelB = MakeTextLabel("Region B",
			CoordPercent(0.6f, 0.333333f), CoordPercent(0.3f, 0.333333f),
			FontType::Bold, UnitPT(16.0f), COLOR_WHITE,
			HorizontalAlignment::Center, VerticalAlignment::Middle);
	TextLabelPtr labelC = MakeTextLabel("Resize Region", CoordPerPX(0.5f, 0.5f,-200,-20),
			CoordPX(400,40), FontType::Bold, UnitPT(16.0f),
			COLOR_WHITE, HorizontalAlignment::Left, VerticalAlignment::Top);
	labelC->setAlignment(HorizontalAlignment::Center,VerticalAlignment::Middle);
	AdjustableCompositePtr resizeRegion=AdjustableCompositePtr(new AdjustableComposite("Resize Region",CoordPerPX(0.5f,0.0f,-250.0f,40.0f), CoordPX(500, 100)));
	resizeRegion->setDragEnabled(true);
	resizeRegion->backgroundColor=MakeColor(Color(64,192,64));
	resizeRegion->setAspectRule(AspectRule::FixedWidth);
	resizeRegion->add(labelC);

	labelA->backgroundColor = MakeColor(255, 128, 64);
	labelB->backgroundColor = MakeColor(64, 128, 255);
	labelA->onMouseDown = [=](AlloyContext* context, const InputEvent& event) {
		labelA->setLabel((event.clicks == 2) ? "Double Click A" : "Mouse Down A");
		labelA->backgroundColor = MakeColor(128, 64, 32);
		//Return false so event is not consumed. If event is consumed here, then onEvent() will not be called.
			return false;
		};
	labelA->onMouseUp = [=](AlloyContext* context, const InputEvent& event) {
		labelA->setLabel("Mouse Up A");
		labelA->backgroundColor = MakeColor(255, 128, 64);
		//Return false so event is not consumed. If event is consumed here, then onEvent() will not be called.
			return false;
		};
	labelA->onMouseOver = [=](AlloyContext* context, const InputEvent& event) {
		labelA->setLabel("Mouse Over A");
		//Return false so event is not consumed. If event is consumed here, then onEvent() will not be called.
			return false;
		};
	labelB->onMouseDown = [=](AlloyContext* context, const InputEvent& event) {
		labelB->setLabel((event.clicks==2)?"Double Click B":"Mouse Down B");
		labelB->backgroundColor = MakeColor(32,64,128);
		//Return false so event is not consumed. If event is consumed here, then onEvent() will not be called.
			return false;
		};
	labelB->onMouseUp = [=](AlloyContext* context, const InputEvent& event) {
		labelB->setLabel("Mouse Up B");
		labelB->backgroundColor = MakeColor(64,128,255);
		//Return false so event is not consumed. If event is consumed here, then onEvent() will not be called.
			return false;
		};
	labelB->onMouseOver = [=](AlloyContext* context, const InputEvent& event) {
		labelB->setLabel("Mouse Over B");
		//Return false so event is not consumed. If event is consumed here, then onEvent() will not be called.
			return false;
		};
	rootNode.onEvent = [=](AlloyContext* context, const InputEvent& event) {
		labelC->setLabel(MakeString()<<"["<<event.type<<"] "<<event.cursor);
		if (event.type == InputType::Cursor) {
			if (!context->isMouseOver(labelA.get())) {
				labelA->setLabel("Region A");
				labelA->backgroundColor = MakeColor(255, 128, 64);
			}
			if (!context->isMouseOver(labelB.get())) {
				labelB->setLabel( "Region B");
				labelB->backgroundColor = MakeColor(64, 128, 255);
			}
		}
		return false;
	};
	//Don't forget to do this if you decide to use the onEvent() method! onEvent() catches all events, regardless of the selected component.
	addListener(&rootNode);
	rootNode.add(labelA);
	rootNode.add(labelB);
	rootNode.add(resizeRegion);
	return true;
}

