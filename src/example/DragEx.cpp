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
#include "../../include/example/DragEx.h"
using namespace aly;
DragEx::DragEx() :
		Application(800, 600, "Drag Example") {
}
bool DragEx::init(Composite& rootNode) {
	int N = 25;
	srand(817213);
	std::list<TextLabelPtr> labels;
	for (int i = 0;i <N;i++) {
		TextLabelPtr label = MakeTextLabel(MakeString() << "Drag (" << i<<")", CoordPX(rand()%700, rand()%500), CoordPX(100, 100), FontType::Bold, UnitPT(16.0f), COLOR_WHITE, HorizontalAlignment::Center, VerticalAlignment::Middle);
		label->backgroundColor = MakeColor(HSVAtoColor(HSVA(i/(float)N,0.7f,0.5f,1.0f)));
		label->borderColor = MakeColor(64,64,64);
		label->borderWidth = UnitPX(1.0f);
		label->onMouseDown = [=](AlloyContext* context, const InputEvent& e) {
			if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
				//Bring component to top by setting it to be drawn last.
				dynamic_cast<Composite*>(label->parent)->putLast(label);
				label->borderColor = MakeColor(200, 200, 200);
			}
			return false;
		};
		label->onMouseUp = [=](AlloyContext* context, const InputEvent& e) {
			label->borderColor = MakeColor(64,64,64);
			return false;
		};

		labels.push_back(label);
		label->setDragEnabled(true);
		rootNode.add(label);

	}
	//Force drag elements inside draw bounds.
	setOnResize([=](const int2& dims) {
		box2px bounds(pixel2(0, 0), pixel2((float)dims.x,(float)dims.y));
		for (TextLabelPtr label : labels) {
			//Get current location, including drag offset.
			box2px box = label->getBounds();
			box.position = aly::clamp(box.position, bounds.position,bounds.position + bounds.dimensions - box.dimensions);
			label->position=CoordPX(box.position);
			//Reset drag offset now that we changed position.
			label->setDragOffset(pixel2(0, 0));
			//Repack component.
			label->pack();
		}
	});
	//getContext()->setDebug(true);
	return true;
}

