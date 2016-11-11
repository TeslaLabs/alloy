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
#include "../../include/example/ImageEx.h"
using namespace aly;
ImageEx::ImageEx() :
		Application(800, 600, "Image Example"){
}
bool ImageEx::init(Composite& rootNode) {

	rootNode.add(MakeTextLabel("Drag Images", CoordPerPX(1.0f, 0.0f,-120.0f,0.0f), CoordPX(120, 30), FontType::Normal, UnitPT(20.0f), RGBA(255, 255, 255, 255), HorizontalAlignment::Left, VerticalAlignment::Top));
	//Use OpenGL directly to draw and position image underneath region.
	//This is not recommended for moving regions because there 
	//will be a one frame delay between the opengl and nanovg draw pass.
	//The texture image will also be drawn beneath everything else, 
	//so region z-ordering has no effect.
	ImageRGBAf img;
	ReadImageFromFile(getFullPath("images/sfsunset.png"),img);
	texImage.load(img,true);
	drawRegion=MakeRegion("sfsunset", CoordPX(5.0f, 260.0f), CoordPX(250, 250), COLOR_NONE, Color(200, 200, 200, 255),UnitPX(1.0f));
	drawRegion->setAspectRatio(img.width / (float)img.height);
	drawRegion->setAspectRule(AspectRule::FixedHeight);
	drawRegion->setDragEnabled(true);
	rootNode.add(drawRegion);
	//Use nanovg to draw
	GlyphRegionPtr imageRegion = MakeGlyphRegion(createImageGlyph(getFullPath("images/sfmarket.png")), CoordPX(0.0f, 0.0f), CoordPX(250, 250), AspectRule::FixedHeight,COLOR_NONE, COLOR_NONE, Color(200, 200, 200, 255), UnitPX(1.0f));
	imageRegion->setDragEnabled(true);
	rootNode.add(imageRegion);
	//getContext()->setDebug(true);
	return true;
}
void ImageEx::draw(aly::AlloyContext* context) {
	//Use glsl shader to draw image
	imageShader.draw(texImage, context->pixelRatio*drawRegion->getBounds(),1.0f,true);
}

