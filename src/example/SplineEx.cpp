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
#include "../../include/example/SplineEx.h"

using namespace aly;
SplineEx::SplineEx() :
	Application(600, 600, "Spline Example") {
}
void SplineEx::update() {
	bspline.build(controlPoints, SplineType::Clamp, 3);
	bspline.getKnots(knots);
	knotPoints.resize(knots.size());
	for (int n = 0;n < (int)knots.size();n++) {
		knotPoints[n] = bspline.evaluate(knots[n].x);
	}
	int N = 100;
	curvePoints.resize(N + 1);
	for (int n = 0;n <= N;n++) {
		curvePoints[n] = bspline.evaluate(n / (float)N);
	}

}
bool SplineEx::init(Composite& rootNode) {
	controlPoints.push_back(float2(-1.75, -1.0f));
	controlPoints.push_back(float2(-1.5, -0.5f));
	controlPoints.push_back(float2(-1.5, 0.0f));
	controlPoints.push_back(float2(-1.25, 0.5f));
	controlPoints.push_back(float2(-0.75, 0.75f));
	controlPoints.push_back(float2(0.0f, 0.5f));
	controlPoints.push_back(float2(0.5f, 0.0f));
	range = std::pair<float2, float2>(float2(-2, -2), float2(1, 1));
	update();
	draw = DrawPtr(new Draw("B-Spline", CoordPX(0.0f,0.0f), CoordPercent(1.0f, 1.0f)));
	draw->setAspectRule(AspectRule::FixedHeight);
	draw->setAspectRatio(1.0f);

	draw->onDraw = [this](const AlloyContext* context, const box2px& bounds) {
		NVGcontext* nvg = context->nvgContext;
		nvgStrokeWidth(nvg, 3.0f);
		nvgStrokeColor(nvg, Color(64, 128, 255));
		float2 pt = curvePoints[0];
		pt = (pt - range.first) / (range.second - range.first);
		pt.y = 1.0f - pt.y;
		pt = pt*bounds.dimensions + bounds.position;
		nvgLineCap(nvg, NVG_ROUND);
		nvgBeginPath(nvg);
		nvgMoveTo(nvg, pt.x, pt.y);
		for (int n = 1;n < (int)curvePoints.size();n++) {
			float2 pt = curvePoints[n];
			pt = (pt - range.first) / (range.second - range.first);
			pt.y = 1.0f - pt.y;
			pt = pt*bounds.dimensions + bounds.position;
			nvgLineTo(nvg, pt.x, pt.y);
			
		}
		nvgStroke(nvg);
		for (int n = 0;n <(int)knotPoints.size();n++) {
			nvgFillColor(nvg, Color(255, 128, 64));
			nvgBeginPath(nvg);
			float2 pt = knotPoints[n];
			pt = (pt - range.first) / (range.second - range.first);
			pt.y = 1.0f - pt.y;
			pt = pt*bounds.dimensions + bounds.position;
			nvgCircle(nvg, pt.x, pt.y, 4);
			nvgFill(nvg);
		}
		nvgStrokeWidth(nvg, 2.0f);
		float r;
		for (int n = 0;n <(int)controlPoints.size();n++) {
			if (n == selectedControlPoint) {
				nvgFillColor(nvg, Color(255,255,255));
				r = 8;
			} else {
				nvgFillColor(nvg, Color(192,192,192));
				r = 6;
			}
			nvgBeginPath(nvg);
			float2 pt = controlPoints[n];
			pt = (pt - range.first) / (range.second - range.first);
			pt.y = 1.0f - pt.y;
			pt = pt*bounds.dimensions + bounds.position;
			nvgCircle(nvg, pt.x, pt.y,r);
			nvgFill(nvg);
		}
	};
	draw->onMouseDown = [this](const AlloyContext* context, const InputEvent& e) {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			mouseDownControlPoint = selectedControlPoint;
		}
		return false;
	};
	draw->onMouseUp = [this](const AlloyContext* context, const InputEvent& e) {
		mouseDownControlPoint = -1;
		return false;
	};
	draw->onMouseOver=[this](const AlloyContext* context, const InputEvent& e) {
		box2px bounds = draw->getBounds();
		selectedControlPoint = -1;
		for (int n = 0;n <(int) controlPoints.size();n++) {
			float2 pt = (controlPoints[n] - range.first) / (range.second - range.first);
			pt.y = 1.0f - pt.y;
			pt = pt*bounds.dimensions + bounds.position;
			if (distanceSqr(pt, e.cursor) <36) {
				selectedControlPoint = n;
				break;
			}
		}

		if (context->isLeftMouseButtonDown()) {
			if (mouseDownControlPoint >= 0) {
				float2 pt = (e.cursor - bounds.position) / bounds.dimensions;
				pt.y = 1.0f - pt.y;
				pt = pt*(range.second - range.first) + range.first;
				controlPoints[mouseDownControlPoint] = pt;
				update();
			}
		}
		return false;
	};
	rootNode.add(draw);
	return true;
}

