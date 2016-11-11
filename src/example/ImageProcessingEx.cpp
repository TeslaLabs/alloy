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
#include "AlloyImageProcessing.h"
#include "../../include/example/ImageProcessingEx.h"
using namespace aly;
ImageProcessingEx::ImageProcessingEx() :
		Application(800, 800, "Image Processing Example") {
}
bool ImageProcessingEx::init(Composite& rootNode) {
	float4 min, max;
	ImageRGBAf tmp,initImg,gx,gy,laplace,smooth;
	ReadImageFromFile(getFullPath("images/sfmarket.png"), initImg);
	Crop(initImg, tmp, int2(0, initImg.height / 2), int2(initImg.height / 2, initImg.height / 2));
	DownSample(tmp,initImg);
	Gradient5x5(initImg, gx, gy);
	Laplacian5x5(initImg, laplace);
	
	min = gx.min();
	max = gx.max();
	gx = (gx - min) / (max - min);
	
	min = gy.min();
	max = gy.max();
	gy = (gy - min) / (max - min);
	
	min = laplace.min();
	max = laplace.max();
	laplace = (laplace - min) / (max - min);

	Smooth11x11(initImg, smooth);
	
	for (float4& val : gx.data) {
		val.w = 1.0f;
	}
	for (float4& val : gy.data) {
		val.w = 1.0f;
	}
	for (float4& val : laplace.data) {
		val.w = 1.0f;
	}
	GlyphRegionPtr initRegion = MakeGlyphRegion(createImageGlyph(initImg),
			CoordPercent(0.33333f, 0.33333f), CoordPercent(0.33333f, 0.33333f),
			AspectRule::FixedWidth, COLOR_NONE, COLOR_NONE,
			Color(200, 200, 200, 255), UnitPX(1.0f));

	GlyphRegionPtr gxRegion = MakeGlyphRegion(createImageGlyph(gx),
		CoordPercent(0.0f, 0.0f), CoordPercent(0.5f, 0.5f),
		AspectRule::FixedWidth, COLOR_NONE, COLOR_NONE,
		Color(200, 200, 200, 255), UnitPX(1.0f));

	GlyphRegionPtr gyRegion = MakeGlyphRegion(createImageGlyph(gy),
		CoordPercent(0.5f, 0.0f), CoordPercent(0.5f, 0.5f),
		AspectRule::FixedWidth, COLOR_NONE, COLOR_NONE,
		Color(200, 200, 200, 255), UnitPX(1.0f));

	GlyphRegionPtr laplaceRegion = MakeGlyphRegion(createImageGlyph(laplace),
		CoordPercent(0.0f, 0.5f), CoordPercent(0.5f, 0.5f),
		AspectRule::FixedWidth, COLOR_NONE, COLOR_NONE,
		Color(200, 200, 200, 255), UnitPX(1.0f));

	GlyphRegionPtr smoothRegion = MakeGlyphRegion(createImageGlyph(smooth),
		CoordPercent(0.5f, 0.5f), CoordPercent(0.5f, 0.5f),
		AspectRule::FixedWidth, COLOR_NONE, COLOR_NONE,
		Color(200, 200, 200, 255), UnitPX(1.0f));

	rootNode.add(gxRegion);
	rootNode.add(gyRegion);
	rootNode.add(laplaceRegion);
	rootNode.add(smoothRegion);
	rootNode.add(initRegion);
	return true;
}
