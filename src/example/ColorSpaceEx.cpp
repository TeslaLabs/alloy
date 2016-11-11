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
#include "../../include/example/ColorSpaceEx.h"
using namespace aly;
ColorSpaceEx::ColorSpaceEx() :
	Application(800, 600, "Color Space Example"), particleMatcapShader(getFullPath("images/JG_Silver.png")) {
}
bool ColorSpaceEx::init(Composite& rootNode) {

	//Set up camera
	camera.setNearFarPlanes(-10.0f, 10.0f);
	camera.setZoom(1.0f);
	camera.setCameraType(CameraType::Orthographic);
	camera.setPose(float4x4::identity());

	//Map object geometry into unit bounding box for draw.
	//Add listener to respond to mouse manipulations
	addListener(&camera);
	setOnResize([this](const int2& dims) {
		camera.setDirty(true);
	});
	SelectionPtr selection = SelectionPtr(new Selection("Color Space", CoordPX(5, 5), CoordPX(200, 30), std::vector<std::string>{"RGB", "HSV", "XYZ", "CIELAB"}));
	selection->onSelect = [this](int index) {
		this->setColorSpace(index);

	};
	selection->setSelectedIndex(3);
	rootNode.add(selection);

	return true;
}
void ColorSpaceEx::setColorSpace(int index) {
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f), float3(1.0f, 1.0f, 1.0f));
	mesh.clear();
	for (int r = 0;r < 256;r += 4) {
		for (int g = 0;g < 256;g += 4) {
			for (int b = 0;b < 256;b += 4) {
				RGBf c = ToRGBf(ubyte3(r, g, b));
				float3 pos;
				switch (index) {
				case 0:
					pos = 255.0f*c;break;
				case 1:
					pos = 255.0f*RGBtoHSV(c);break;
				case 2:
					pos = 255.0f*RGBtoXYZ(c);
					break;

				case 3:
					pos = RGBtoLAB(c); 
					break;
				}
				mesh.vertexLocations.push_back(pos);
				mesh.vertexColors.push_back(float4(c, 1.0f));
			}
		}
	}

	mesh.setDirty(true);
	box3f box = mesh.updateBoundingBox();
	camera.setPose(MakeTransform(box, renderBBox));
	camera.setZoom(1.0f);
	camera.setDirty(true);
}
void ColorSpaceEx::draw(AlloyContext* context) {
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	static float scale=1.0f;
	if (camera.isDirty()) {
		scale = camera.getScale();
	}
	particleMatcapShader.draw(mesh, camera, getContext()->getViewport(), getContext()->getViewport(), 1.0f / std::sqrt(scale));
	camera.setDirty(false);
}

