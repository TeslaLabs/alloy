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
#include "../../include/example/OneEuroFilterEx.h"
using namespace aly;
OneEuroFilterEx::OneEuroFilterEx() :
	Application(800, 800, "One Euro Filter Example",false) {
	for(int i=0;i<(int)dataBuffer.size();i++){
		dataBuffer[i]=pixel2(-10.0f);
		filterBuffer[i]=pixel2(-10.0f);
	}
	filter.setBeta(0.007f);
	filter.setMinCutoff(1.0f);
	filter.setDerivativeCutoff(1.0f);
	filter.reset();
}
bool OneEuroFilterEx::init(Composite& rootNode) {
	drawRegion = DrawPtr(new Draw("Cursor Draw", CoordPX(0.0f, 0.0f), CoordPX(800, 800), [this](AlloyContext* context, const box2px& bounds) {
		drawCursor(context, bounds);
	}));
	rootNode.add(drawRegion);

	CompositePtr controlRegion = MakeComposite("Controls", CoordPerPX(1.0f,1.0f,-200.0f, -185.0f), CoordPX(200,200));
	controlRegion->setOrientation(Orientation::Vertical,pixel2(5.0f),pixel2(5.0f));
	rootNode.add(controlRegion);
	{
		HorizontalSliderPtr noiseSlider = HorizontalSliderPtr(new HorizontalSlider("Noise", CoordPX(5.0f, 0.0f), CoordPerPX(1.0f, 0.0f, -10.0f, 40.0f),Float(0.0f),Float(50.0f),Float(noise)));
		noiseSlider->setOnChangeEvent([this](const Number& value) {
			this->noise = value.toFloat();
		});
		controlRegion->add(noiseSlider);
	}
	{
		HorizontalSliderPtr betaSlider = HorizontalSliderPtr(new HorizontalSlider("Beta", CoordPX(5.0f, 0.0f), CoordPerPX(1.0f, 0.0f, -10.0f, 40.0f),Float(0.0f),Float(0.05f),Float(0.007f)));
		betaSlider->setOnChangeEvent([this](const Number& value) {
			filter.setBeta(std::max(value.toFloat(), 1E-5f));
		});
		controlRegion->add(betaSlider);
	}
	{
		HorizontalSliderPtr cutoffSlider = HorizontalSliderPtr(new HorizontalSlider("Frequency Cutoff", CoordPX(5.0f, 0.0f), CoordPerPX(1.0f, 0.0f, -10.0f, 40.0f), Float(0.0f), Float(10.0f), Float(1.0f)));
		cutoffSlider->setOnChangeEvent([this](const Number& value) {
			filter.setMinCutoff(std::max(value.toFloat(), 1E-5f));
		});
		controlRegion->add(cutoffSlider);
	}
	{
		HorizontalSliderPtr derivSlider = HorizontalSliderPtr(new HorizontalSlider("Derivative Cutoff", CoordPX(5.0f, 0.0f), CoordPerPX(1.0f, 0.0f, -10.0f, 40.0f), Float(0.0f), Float(10.0f), Float(1.0f)));
		derivSlider->setOnChangeEvent([this](const Number& value) {
			filter.setDerivativeCutoff(std::max(value.toFloat(), 1E-5f));
		});
		controlRegion->add(derivSlider);
	}
	return true;
}
void OneEuroFilterEx::drawCursor(AlloyContext* context, const box2px& bounds) {
	NVGcontext* nvg = context->nvgContext;

	const float w = 120;
	const float h = 120;
	pixel2 offset(w, h);

	pixel2 pt,fpt;
	pixel2 noiseCursor;
	pixel2 filterCursor;
	pixel2 cursor =context->getCursorPosition();
	noiseCursor=cursor;
	filterCursor=cursor;
	if (bounds.contains(cursor)) {
		noiseCursor =aly::clamp(float2(RandomGaussian(cursor.x, noise), RandomGaussian(cursor.y, noise)),offset,bounds.position+bounds.dimensions);
		pt = (noiseCursor - offset)*(bounds.dimensions) / (bounds.dimensions - offset);
		dataBuffer[index] =pt;
		fpt = filter.evaluate(pt);
		filterBuffer[index] =fpt;
		fpt = aly::clamp(fpt*((bounds.dimensions - offset) / (bounds.dimensions))+offset,offset,bounds.position+bounds.dimensions);
		index = (index + 1) % BUFFER_SIZE;
		nvgStrokeColor(nvg, Color(1.0f, 0.6f, 0.6f));
		nvgStrokeWidth(nvg, 3.0f);
		nvgBeginPath(nvg);
		nvgCircle(nvg, noiseCursor.x, noiseCursor.y, 12.0f);
		nvgStroke(nvg);
		nvgStrokeColor(nvg, Color(0.6f, 1.0f, 0.6f));
		nvgStrokeWidth(nvg, 3.0f);
		nvgBeginPath(nvg);
		nvgCircle(nvg, fpt.x, fpt.y, 8.0f);
		nvgStroke(nvg);
		context->setCursor(&aly::Cursor::CrossHairs);
	}

	nvgBeginPath(nvg);
	nvgFillColor(nvg, Color(0.3f, 0.3f, 0.3f));
	nvgRect(nvg, 0, 0, w, h);
	nvgFill(nvg);

	pt = (cursor - offset)*(bounds.dimensions) / (bounds.dimensions - offset);
	nvgBeginPath(nvg);
	nvgStrokeWidth(nvg, 2.0f);
	nvgStrokeColor(nvg, Color(0.6f,0.6f,0.6f));
	nvgMoveTo(nvg, 0, h*pt.y / bounds.dimensions.y);
	nvgLineTo(nvg, w, h*pt.y / bounds.dimensions.y);
	nvgMoveTo(nvg, w*pt.x / bounds.dimensions.x, 0);
	nvgLineTo(nvg, w*pt.x / bounds.dimensions.x, h);
	nvgStroke(nvg);

	nvgFillColor(nvg, Color(0.6f, 0.6f, 0.6f));
	nvgBeginPath(nvg);
	nvgCircle(nvg, w*pt.x / bounds.dimensions.x, h*pt.y / bounds.dimensions.y, 4.0f);
	nvgFill(nvg);

	pt = (fpt - offset)*(bounds.dimensions) / (bounds.dimensions - offset);
	nvgBeginPath(nvg);
	nvgStrokeColor(nvg, Color(0.6f, 1.0f, 0.6f));
	nvgCircle(nvg, w*pt.x / bounds.dimensions.x, h*pt.y / bounds.dimensions.y, 4.0f);
	nvgStroke(nvg);

	nvgBeginPath(nvg);
	nvgFillColor(nvg, Color(0.3f, 0.3f, 0.3f));
	nvgRect(nvg, 0, h, w, bounds.dimensions.y - h);
	nvgFill(nvg);

	nvgBeginPath(nvg);
	nvgRect(nvg, w, 0, bounds.dimensions.x - w, h);
	nvgFill(nvg);

	nvgStrokeColor(nvg, Color(1.0f, 0.6f, 0.6f));
	nvgStrokeWidth(nvg, 2.0f);


	pushScissor(nvg,box2px(pixel2(0,h),pixel2(w, bounds.dimensions.y - h)));
	int count = 0;
	nvgBeginPath(nvg);
	for (int i = BUFFER_SIZE - 1; i >= 0; i--) {
		pt = dataBuffer[(i + index) % BUFFER_SIZE];
		if (pt.x >-1) {
			if (count == 0) {
				nvgMoveTo(nvg, w*pt.x / bounds.dimensions.x, (bounds.dimensions.y - h) *count / BUFFER_SIZE + h);
			}
			else {
				nvgLineTo(nvg, w*pt.x / bounds.dimensions.x, (bounds.dimensions.y - h) *count / BUFFER_SIZE + h);
			}
			count++;
		}
	}
	nvgStroke(nvg);
	popScissor(nvg);

	pushScissor(nvg,box2px(pixel2(w,0),pixel2(bounds.dimensions.x - w, h)));
	count = 0;
	nvgBeginPath(nvg);
	for (int i = BUFFER_SIZE - 1; i >= 0; i--) {
		pt = dataBuffer[(i + index) % BUFFER_SIZE];
		if (pt.x >-1) {
			if (count == 0) {
				nvgMoveTo(nvg, (bounds.dimensions.x - w) *count / BUFFER_SIZE + w, h*pt.y / bounds.dimensions.y);
			}
			else {
				nvgLineTo(nvg, (bounds.dimensions.x - w) *count / BUFFER_SIZE + w, h*pt.y / bounds.dimensions.y);
			}
			count++;
		}
	}
	nvgStroke(nvg);
	popScissor(nvg);

	pushScissor(nvg,box2px(pixel2(0,h),pixel2(w, bounds.dimensions.y - h)));
	nvgStrokeColor(nvg, Color(0.6f, 1.0f, 0.6f));
	count = 0;
	nvgBeginPath(nvg);
	for (int i = BUFFER_SIZE - 1; i >= 0; i--) {
		pt = filterBuffer[(i + index) % BUFFER_SIZE];
		if (pt.x >-1) {
			if (count == 0) {
				nvgMoveTo(nvg, w*pt.x / bounds.dimensions.x, (bounds.dimensions.y - h) *count / BUFFER_SIZE + h);
			}
			else {
				nvgLineTo(nvg, w*pt.x / bounds.dimensions.x, (bounds.dimensions.y - h) *count / BUFFER_SIZE + h);
			}
			count++;
		}
	}
	nvgStroke(nvg);
	popScissor(nvg);
	pushScissor(nvg,box2px(pixel2(w,0),pixel2(bounds.dimensions.x - w, h)));
	count = 0;
	nvgBeginPath(nvg);
	for (int i = BUFFER_SIZE - 1; i >= 0; i--) {
		pt = filterBuffer[(i + index) % BUFFER_SIZE];
		if (pt.x >-1) {
			if (count == 0) {
				nvgMoveTo(nvg, (bounds.dimensions.x - w) *count / BUFFER_SIZE + w, h*pt.y / bounds.dimensions.y);
			}
			else {
				nvgLineTo(nvg, (bounds.dimensions.x - w) *count / BUFFER_SIZE + w, h*pt.y / bounds.dimensions.y);
			}
			count++;
		}
	}
	nvgStroke(nvg);
	popScissor(nvg);
	nvgBeginPath(nvg);
	nvgStrokeWidth(nvg, 2.0f);
	nvgStrokeColor(nvg, Color(0.8f, 0.8f, 0.8f));
	nvgRect(nvg, 1, 1, w - 2, h - 2);
	nvgStroke(nvg);
}

