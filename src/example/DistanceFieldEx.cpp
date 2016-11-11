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
#include "../../include/example/DistanceFieldEx.h"
#include "AlloyDistanceField.h"
#include "AlloyIsoContour.h"
using namespace aly;
DistanceFieldEx::DistanceFieldEx() :
		Application(800, 400, "Distance Field Example"),currentIso(0.0f) {
}
bool DistanceFieldEx::init(Composite& rootNode) {
	//SANITY_CHECK_DISTANCE_FIELD();
	currentIso = maxDistance;
	int w = getContext()->getScreenWidth()/2;
	int h = getContext()->getScreenHeight()/2;
	GLFrameBuffer renderBuffer;
	//Render text to image
	NVGcontext* nvg = getContext()->nvgContext;
	renderBuffer.initialize(w, h);
	renderBuffer.begin(RGBAf(1.0f, 1.0f, 1.0f, 1.0f));
		nvgBeginFrame(nvg, w, h, 1.0f);
			nvgFontFaceId(nvg, getContext()->getFont(FontType::Bold)->handle);
			nvgFillColor(nvg, Color(0, 0, 0));
			nvgFontSize(nvg, 150.0f);
			nvgTextAlign(nvg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
			nvgText(nvg, w * 0.5f, h * 0.5f, "Alloy", nullptr);
		nvgEndFrame(nvg);
	renderBuffer.end();
	ImageRGBAf img = renderBuffer.getTexture().read();
	FlipVertical(img);
	Image1f gray;
	ConvertImage(img, gray);
	//Make boundary == 0, outside == 0.5 inside == -0.5
	gray -= float1(0.5f);
	DistanceField2f df;
	//Solve distance field out to +/- 40 pixels
	df.solve(gray, distField, maxDistance);
	IsoContour isoContour;
	isoContour.solve(distField,curvePoints,curveIndexes, 0.0f, TopologyRule2D::Unconstrained);
	//Normalize distance field range so it can be rendered as gray scale image.
	distField = (distField + float1(maxDistance)) / float1(2.0f * maxDistance);
	GlyphRegionPtr imageRegion = MakeGlyphRegion(createImageGlyph(distField),
			CoordPX(0.0f, 0.0f), CoordPercent(1.0f,1.0f), AspectRule::FixedHeight,
			COLOR_NONE, COLOR_NONE, Color(200, 200, 200, 255), UnitPX(1.0f));
	rootNode.add(imageRegion);
	draw = DrawPtr(new Draw("Iso-Contour", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f)));
	draw->setAspectRule(AspectRule::FixedHeight);
	draw->onMouseOver = [this](const AlloyContext* context, const InputEvent& e) {
		box2px bounds = draw->getBounds();
		float2 pt=(e.cursor-bounds.position)/bounds.dimensions;
		float level = distField(pt.x*distField.width, pt.y*distField.height).x;
		if (std::abs(level-currentIso)>1.0f/maxDistance&&level<0.999f) {
			IsoContour traceContour;
			traceContour.solve(distField, tracePoints, traceIndexes, level, TopologyRule2D::Unconstrained);
			currentIso = level;
		}
		return false;
	};
	draw->onDraw = [this](const AlloyContext* context, const box2px& bounds) {
		NVGcontext* nvg = context->nvgContext;
		nvgStrokeWidth(nvg, 3.0f);
		nvgStrokeColor(nvg, Color(128, 128, 255));
		nvgLineCap(nvg, NVG_ROUND);
		nvgBeginPath(nvg);
		for (int n = 0;n < (int)traceIndexes.size();n++) {
			uint2 ln = traceIndexes[n];
			float2 pt = tracePoints[ln.x];
			pt.x = pt.x / (float)distField.width;
			pt.y = pt.y / (float)distField.height;
			pt = pt*bounds.dimensions + bounds.position;
			nvgMoveTo(nvg, pt.x, pt.y);
			pt = tracePoints[ln.y];
			pt.x = pt.x / (float)distField.width;
			pt.y = pt.y / (float)distField.height;
			pt = pt*bounds.dimensions + bounds.position;
			nvgLineTo(nvg, pt.x, pt.y);
		}
		nvgStroke(nvg);
		nvgStrokeWidth(nvg, 4.0f);
		nvgStrokeColor(nvg, Color( 255,128,64));
		nvgLineCap(nvg, NVG_ROUND);
		nvgBeginPath(nvg);
		for (int n = 0;n < (int)curveIndexes.size();n++) {
			std::list<uint32_t> curve = curveIndexes[n];
			bool firstTime = true;
			for (uint32_t idx : curve) {
				float2 pt = curvePoints[idx];
				pt.x = pt.x / (float)distField.width;
				pt.y = pt.y / (float)distField.height;
				pt = pt*bounds.dimensions + bounds.position;
				if (firstTime) {
					nvgMoveTo(nvg, pt.x, pt.y);
				} else {
					nvgLineTo(nvg, pt.x, pt.y);
				}
				firstTime = false;
			}
		}
		nvgStroke(nvg);


	};
	rootNode.add(draw);
	return true;
}

