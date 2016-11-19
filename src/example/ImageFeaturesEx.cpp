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
#include "AlloyDenseSolve.h"
#include "AlloyImageFeatures.h"
#include "../../include/example/ImageFeaturesEx.h"
using namespace aly;
using namespace aly::daisy;
ImageFeaturesEx::ImageFeaturesEx() :
		Application(800, 600, "Image Features Example") {
}
bool ImageFeaturesEx::init(Composite& rootNode) {

	ImageRGBA leftImg, rightImg,tmp;
	ReadImageFromFile(getFullPath("images/stereo_left.png"), leftImg);
	ReadImageFromFile(getFullPath("images/stereo_right.png"), rightImg);
	DownSample(leftImg, tmp);
	leftImg = tmp;
	//DownSample(tmp,leftImg);
	ConvertImage(leftImg, left);
	DownSample(rightImg, tmp);
	rightImg = tmp;
	//DownSample(tmp, rightImg);
	ConvertImage(rightImg, right);

	tmp.resize(left.width, left.height);
	for (RGBA& c : tmp.data) {
		c = RGBA(64, 64, 64, 255);
	}
	ImageGlyphPtr srcGlyph = createImageGlyph(leftImg, false);
	ImageGlyphPtr tarGlyph = createImageGlyph(rightImg, false);
	ImageGlyphPtr resultGlyph = createImageGlyph(tmp, false);

	GlyphRegionPtr srcRegion = MakeGlyphRegion(srcGlyph,
			CoordPercent(0.05f, 0.0f), CoordPercent(0.4f, 0.3f),
			AspectRule::FixedWidth, COLOR_NONE, COLOR_NONE,
			Color(200, 200, 200, 255), UnitPX(1.0f));

	GlyphRegionPtr tarRegion = MakeGlyphRegion(tarGlyph,
			CoordPercent(0.55f, 0.0f), CoordPercent(0.4f, 0.3f),
			AspectRule::FixedWidth, COLOR_NONE, COLOR_NONE,
			Color(200, 200, 200, 255), UnitPX(1.0f));

	TextLabelPtr textLabel = TextLabelPtr(
			new TextLabel("Stereo Matching ...",
					CoordPercent(0.2f, 0.4f), CoordPercent(0.8f, 0.5f)));
	textLabel->fontSize = UnitPX(20.0f);
	textLabel->fontType = FontType::Bold;
	textLabel->fontStyle = FontStyle::Outline;

	GlyphRegionPtr resultRegion = MakeGlyphRegion(resultGlyph,
			CoordPercent(0.5f, 0.4f), CoordPercent(0.8f, 0.55f),
			AspectRule::FixedHeight, COLOR_NONE, COLOR_NONE,
			Color(200, 200, 200, 255), UnitPX(1.0f));
	resultRegion->setOrigin(Origin::TopCenter);
	rootNode.add(srcRegion);
	rootNode.add(tarRegion);
	rootNode.add(resultRegion);
	rootNode.add(textLabel);

	workerTask = WorkerTaskPtr(new WorkerTask([=] {
		ImageRGBA resultImg(leftImg.width,rightImg.height);
		Daisy daisy;
		DescriptorField leftDescriptors;
		DescriptorField rightDescriptors;
		textLabel->setLabel("Computing left image descriptors ...");
		daisy.initialize(left);
		daisy.getDescriptors(leftDescriptors, Normalization::Sift,false,false);
		textLabel->setLabel("Computing right image descriptors ...");
		daisy.initialize(right);
		daisy.getDescriptors(rightDescriptors, Normalization::Sift, false, false);
		const int shiftBound = 64;
		const float minScore = 0.8f;
		textLabel->setLabel("Stereo matching ...");
#pragma omp parallel for
		for (int j = 0; j < leftDescriptors.height; j++) {
			for (int i = 0; i < leftDescriptors.width; i++) {
				double bestScore = 0.0;
				int bestOffset = 0;
				for (int ii = std::max(i - shiftBound,0); ii <= i; ii++) {
					double score = dot(leftDescriptors(i, j), rightDescriptors(ii, j));
					if (score > bestScore) {
						bestOffset = i - ii;
						bestScore = score;
					}
				}
				if (bestScore > minScore) {
					resultImg(i, j) = ToRGBA(HSVAtoRGBAf(HSVA((bestOffset) / (float)(shiftBound), clamp(((float)bestScore - minScore) / (1.0f - minScore), 0.0f, 1.0f), 0.8f, 1.0f)));
				}
				else {
					resultImg(i, j) = RGBA(64, 64, 64, 255);
				}
			}
		}
		getContext()->addDeferredTask([=]() {
					resultGlyph->set(resultImg,getContext().get());
					textLabel->setLabel("Finished!");
				});
	}));
	workerTask->execute(isForcedClose());
	
	return true;
}
