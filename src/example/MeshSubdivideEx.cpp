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
#include "../../include/example/MeshSubdivideEx.h"
using namespace aly;
MeshSubdivideEx::MeshSubdivideEx() :
		Application(800, 600, "Mesh Sub-Division Example"), matcapShader(
				getFullPath("images/JG_Silver.png")), imageShader(
				ImageShader::Filter::SMALL_BLUR) {
}
bool MeshSubdivideEx::init(Composite& rootNode) {
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f),
			float3(1.0f, 1.0f, 1.0f));
	mesh.load(getFullPath("models/monkey.obj"));
	mesh.updateVertexNormals();
	//Initialize depth buffer to store the render
	int w = getContext()->getScreenWidth();
	int h = getContext()->getScreenHeight();
	depthFrameBuffer.initialize(w, h);
	wireframeFrameBuffer.initialize(w, h);
	//Set up camera
	camera.setNearFarPlanes(-5.0f, 5.0f);
	camera.setZoom(1.0f);
	camera.setCameraType(CameraType::Orthographic);
	//Map object geometry into unit bounding box for draw.
	camera.setPose(MakeTransform(mesh.getBoundingBox(), renderBBox));
	//Add listener to respond to mouse manipulations
	addListener(&camera);
	TextButtonPtr subCatmullClark = TextButtonPtr(
			new TextButton("Subdivide Catmull-Clark",
					CoordPerPX(0.25f, 0.0f, 0, 5), CoordPX(200, 30)));
	subCatmullClark->setOrigin(Origin::TopCenter);
	subCatmullClark->setAspectRule(AspectRule::Unspecified);

	TextButtonPtr resetButton = TextButtonPtr(
			new TextButton("Reset",
					CoordPerPX(0.5f, 0.0f, 0, 5), CoordPX(100, 30)));
	resetButton->setOrigin(Origin::TopCenter);
	resetButton->setAspectRule(AspectRule::Unspecified);

	TextButtonPtr subLoop = TextButtonPtr(
			new TextButton("Subdivide Loop", CoordPerPX(0.75f, 0.0f, 0, 5),
					CoordPX(200, 30)));
	subLoop->setOrigin(Origin::TopCenter);
	subLoop->setAspectRule(AspectRule::Unspecified);

	resetButton->onMouseDown =
			[=](AlloyContext* context,const InputEvent& e) {
				mesh.load(getFullPath("models/monkey.obj"));
				mesh.updateVertexNormals();
				mesh.setDirty(true);
				return true;
			};

	subCatmullClark->onMouseDown =
			[=](AlloyContext* context,const InputEvent& e) {
				Subdivide(mesh,SubDivisionScheme::CatmullClark);
				mesh.updateVertexNormals();
				mesh.setDirty(true);
				return true;
			};
	subLoop->onMouseDown = [=](AlloyContext* context,const InputEvent& e) {
		Subdivide(mesh,SubDivisionScheme::Loop);
		mesh.updateVertexNormals();
		mesh.setDirty(true);
		return true;
	};
	wireframeShader.setFaceColor(Color(0.1f, 0.1f, 1.0f, 0.5f));
	wireframeShader.setEdgeColor(Color(1.0f, 0.8f, 0.1f, 1.0f));
	wireframeShader.setLineWidth(1.5f);

	setOnResize([this](const int2& dims) {
		if(!getContext()->hasDeferredTasks()) {
			getContext()->addDeferredTask([this]() {
						int w=getContext()->getScreenWidth();
						int h=getContext()->getScreenHeight();
						depthFrameBuffer.initialize(w,h);
						wireframeFrameBuffer.initialize(w,h);
						camera.setDirty(true);
					});
		}
	});
	rootNode.add(subCatmullClark);
	rootNode.add(subLoop);
	rootNode.add(resetButton);
	return true;
}
void MeshSubdivideEx::draw(AlloyContext* context) {
	if (camera.isDirty()) {
		depthAndNormalShader.draw(mesh, camera, depthFrameBuffer);
		wireframeShader.draw(mesh, camera, wireframeFrameBuffer);
	}
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	matcapShader.draw(depthFrameBuffer.getTexture(), camera,
			context->getViewport(), context->getViewport(), RGBAf(1.0f));
	imageShader.draw(wireframeFrameBuffer.getTexture(), context->getViewport(),
			1.0f, false);
	camera.setDirty(false);
}

