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
#include "AlloyDelaunay.h"
#include "../../include/example/LocatorEx.h"
using namespace aly;
LocatorEx::LocatorEx() :
	Application(800, 800, "Point Locator Example") {
}
bool LocatorEx::init(Composite& rootNode) {
	const int N = 300;
	const int K = 5;
	const float SEARCH_RADIUS = 0.2f;
	drawRegion = DrawPtr(new Draw("Draw Region", CoordPerPX(0.5f, 0.5f, -400, -400), CoordPX(800, 800)));
	rootNode.add(drawRegion);
	samples.resize(N);
	for (int n = 0; n < N; n++) {
		samples[n] = float2(RandomUniform(0.0f, 1.0f),
			RandomUniform(0.0f, 1.0f));
	}
	locator = std::unique_ptr<Locator2f>(new Locator2f(samples));
	matcher = std::unique_ptr<Matcher2f>(new Matcher2f(samples));
	MakeDelaunay(samples, delaunayTriangles);
	drawRegion->onDraw = [this, SEARCH_RADIUS](const AlloyContext* context, const box2px& bounds) {
		NVGcontext* nvg = context->nvgContext;

		nvgStrokeColor(nvg, Color(64, 64, 64));
		nvgStrokeWidth(nvg, 2.0f);
		for (int n = 0;n < (int)delaunayTriangles.size();n++) {
			uint3 tri = delaunayTriangles[n];
			float2 pt0 = samples[tri.x];
			float2 pt1 = samples[tri.y];
			float2 pt2 = samples[tri.z];
			pt0 = pt0*bounds.dimensions + bounds.position;
			pt1 = pt1*bounds.dimensions + bounds.position;
			pt2 = pt2*bounds.dimensions + bounds.position;
			nvgBeginPath(nvg);
			nvgMoveTo(nvg, pt0.x, pt0.y);
			nvgLineTo(nvg, pt1.x, pt1.y);
			nvgLineTo(nvg, pt2.x, pt2.y);
			nvgStroke(nvg);
		}
		if (cursor.x > 0 && cursor.y > 0) {
			nvgStrokeColor(nvg, Color(255, 128, 64));
			nvgStrokeWidth(nvg, 2.0f);
			for (int n = 0;n < (int)kNN.size();n++) {
				float2 pt = samples[kNN[n].first];
				nvgBeginPath(nvg);
				pt = pt*bounds.dimensions + bounds.position;
				nvgMoveTo(nvg, cursor.x, cursor.y);
				nvgLineTo(nvg, pt.x, pt.y);
				nvgStroke(nvg);
			}
			nvgBeginPath(nvg);
			nvgFillColor(nvg, Color(255, 128, 64));
			nvgCircle(nvg, cursor.x, cursor.y, 4.0f);
			nvgFill(nvg);
		}
		nvgFillColor(nvg, Color(128, 128, 128));
		for (float2 pt : samples.data) {
			nvgBeginPath(nvg);
			pt = pt*bounds.dimensions + bounds.position;
			nvgCircle(nvg, pt.x, pt.y, 2.0f);
			nvgFill(nvg);
		}
		for (float2i pt : nearest) {
			nvgFillColor(nvg, Color(128, 128, 255, 255));
			nvgBeginPath(nvg);
			float2 qt = pt*bounds.dimensions + bounds.position;
			nvgCircle(nvg, qt.x, qt.y, 4.0f);
			nvgFill(nvg);
		}
		if (closest.index >= 0) {
			nvgFillColor(nvg, Color(128, 255, 128));
			nvgBeginPath(nvg);
			float2 pt = closest*bounds.dimensions + bounds.position;
			nvgCircle(nvg, pt.x, pt.y, 6.0f);
			nvgFill(nvg);
		}
		if (cursor.x > 0 && cursor.y > 0) {
			nvgStrokeColor(nvg, Color(128, 128, 255, 128));
			nvgStrokeWidth(nvg, 2.0f);
			nvgBeginPath(nvg);
			nvgEllipse(nvg, cursor.x, cursor.y, SEARCH_RADIUS*bounds.dimensions.x, SEARCH_RADIUS*bounds.dimensions.y);
			nvgStroke(nvg);
		}
	};
	drawRegion->onMouseOver = [this, K, SEARCH_RADIUS](const AlloyContext* context, const InputEvent& event) {
		box2px bounds = drawRegion->getBounds();
		cursor = event.cursor;
		float2 pt = (cursor - bounds.position) / bounds.dimensions;
		closest = locator->closest(pt);
		locator->closest(pt, SEARCH_RADIUS, nearest);
		matcher->closest(pt, K, kNN);
		return true;
	};
	return true;
}

