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
#include "../../include/example/MeshWireframeEx.h"
using namespace aly;
MeshWireframeEx::MeshWireframeEx() :
		Application(800, 600, "Mesh with Wireframe Example"), matcapShader(
				getFullPath("images/JG_Silver.png")),imageShader(ImageShader::Filter::SMALL_BLUR) {
}
bool MeshWireframeEx::init(Composite& rootNode) {
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f),
			float3(1.0f, 1.0f, 1.0f));
	mesh.load(getFullPath("models/monkey.obj"));
	Subdivide(mesh,SubDivisionScheme::CatmullClark);
	mesh.updateVertexNormals();
	//Make region on screen to render 3d view
	renderRegion = MakeRegion("Render View", CoordPerPX(0.5f, 0.5f, -256, -256),
			CoordPX(512, 512), COLOR_NONE, COLOR_WHITE, UnitPX(1.0f));
	//Initialize depth buffer to store the render
	depthFrameBuffer.initialize(512, 512);
	wireframeFrameBuffer.initialize(512, 512);
	//Set up camera
	camera.setNearFarPlanes(-2.0f, 2.0f);
	camera.setZoom(0.75f);
	camera.setCameraType(CameraType::Orthographic);
	//Map object geometry into unit bounding box for draw.
	camera.setPose(MakeTransform(mesh.getBoundingBox(), renderBBox));
	//Add listener to respond to mouse manipulations
	addListener(&camera);
	//Add render component to root node so it is relatively positioned.
	rootNode.add(renderRegion);
	setOnResize([=](const int2& dims) {
		camera.setDirty(true);
	});
	wireframeShader.setFaceColor(Color(0.1f,0.1f,1.0f,0.5f));
	wireframeShader.setEdgeColor(Color(1.0f,0.8f,0.1f,1.0f));
	wireframeShader.setLineWidth(1.5f);
	return true;
}
void MeshWireframeEx::draw(AlloyContext* context) {
	if (camera.isDirty()) {
		depthAndNormalShader.draw(mesh, camera, depthFrameBuffer);
		wireframeShader.draw(mesh, camera, wireframeFrameBuffer);
	}
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	matcapShader.draw(depthFrameBuffer.getTexture(), camera, context->pixelRatio*renderRegion->getBounds(), context->getViewport(), RGBAf(1.0f));
	imageShader.draw(wireframeFrameBuffer.getTexture(), context->pixelRatio*renderRegion->getBounds(), 1.0f, false);
	camera.setDirty(false);
}

