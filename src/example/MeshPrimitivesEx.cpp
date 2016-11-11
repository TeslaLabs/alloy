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
#include "AlloyMeshPrimitives.h"
#include "../../include/example/MeshPrimitivesEx.h"
using namespace aly;
MeshPrimitivesEx::MeshPrimitivesEx() :
		Application(800, 600, "Mesh Primitives Example"), phongShader(1), imageShader(
				ImageShader::Filter::FXAA),mesh(nullptr) {
}
bool MeshPrimitivesEx::init(Composite& rootNode) {

	phongShader[0] = SimpleLight(Color(0.3f, 0.3f, 0.3f, 0.25f),
		Color(0.0f, 0.0f, 0.0f, 0.0f), Color(0.0f, 0.0f, 0.8f, 0.5f),
		Color(0.0f, 0.0f, 0.0f, 0.0f), 16.0f, float3(0, 0.0, 2.0),
		float3(0, 1, 0));
	phongShader[0].moveWithCamera = false;

	//Initialize depth buffer to store the render
	int w = getContext()->getScreenWidth();
	int h = getContext()->getScreenHeight();
	depthFrameBuffer.initialize(w, h);
	wireframeFrameBuffer.initialize(w, h);
	//Set up camera
	camera.setNearFarPlanes(0.25f, 2.0f);

	camera.setCameraType(CameraType::Perspective);
	//Map object geometry into unit bounding box for draw.
	addListener(&camera);
	SelectionPtr selection = SelectionPtr(new Selection("Primitive", CoordPX(5, 5), CoordPX(250, 30),
		std::vector<std::string>{
		"Cube", "Icosahedron", "Sphere",
		"Cylinder","Torus", "Plane", "Cone", "Pyramid",
		"Frustum", "Capsule",
		"Tessellated Sphere (Quads)","Tessellated Sphere (Triangles)",
		"Grid","Asteroid"}));
	selection->setMaxDisplayEntries(14);
	camera.aim(getContext()->getViewport());
	meshPrimitives.resize(14);
	meshPrimitives[0] = std::shared_ptr<aly::Mesh>(new Box(box3f(float3(0, 0, 0), float3(10, 10, 10))));
	meshPrimitives[1] = std::shared_ptr<aly::Mesh>(new Icosahedron(10.0f));
	meshPrimitives[2] = std::shared_ptr<aly::Mesh>(new Sphere(5.0f, 20, 20));
	meshPrimitives[3] = std::shared_ptr<aly::Mesh>(new Cylinder(2.0f, 4.0f, 20));
	meshPrimitives[4] = std::shared_ptr<aly::Mesh>(new Torus(3.0f, 4.0f,60, 12));
	meshPrimitives[5] = std::shared_ptr<aly::Mesh>(new Plane(5.0f, 7.0f));
	meshPrimitives[6] = std::shared_ptr<aly::Mesh>(new Cone(2.0f, 4.0f, 20));
	meshPrimitives[7] = std::shared_ptr<aly::Mesh>(new Pyramid(5.0f, 7.0f, 10.0f));
	meshPrimitives[8] = std::shared_ptr<aly::Mesh>(new Frustum(camera));
	meshPrimitives[9] = std::shared_ptr<aly::Mesh>(new Capsule(2.0f, 10.0f,20, 20));
	meshPrimitives[10] = std::shared_ptr<aly::Mesh>(new TessellatedSphere(3.0f,3,SubDivisionScheme::CatmullClark));
	meshPrimitives[11] = std::shared_ptr<aly::Mesh>(new TessellatedSphere(3.0f,3, SubDivisionScheme::Loop));
	meshPrimitives[12] = std::shared_ptr<aly::Mesh>(new Grid(5.0f,7.0f,5,7));
	meshPrimitives[13] = std::shared_ptr<aly::Mesh>(new Asteroid(3));
	selection->onSelect = [this](int index) {
		box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f),float3(1.0f, 1.0f, 1.0f));
		mesh = meshPrimitives[index].get();
		camera.setPose(MakeRotationX(ALY_PI*0.66666f)*MakeTransform(mesh->getBoundingBox(), renderBBox));
		camera.reset();
		camera.setZoom(0.6f);
		camera.setDirty(true);
	};
	selection->setSelectedIndex(2);
	setOnResize([=](const int2& dims) {
		camera.setDirty(true);
	});
	wireframeShader.setFaceColor(Color(1.0f, 1.0f, 1.0f, 0.0f));
	wireframeShader.setEdgeColor(Color(1.0f, 0.8f, 0.1f, 1.0f));
	wireframeShader.setLineWidth(2.0f);
	wireframeShader.setSolid(false);
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
	rootNode.add(selection);

	return true;
}
void MeshPrimitivesEx::draw(AlloyContext* context) {
	if (camera.isDirty()) {
		if (mesh != nullptr) {
			depthAndNormalShader.draw(*mesh, camera, depthFrameBuffer, true);
			wireframeFrameBuffer.begin();
			phongShader.draw(depthFrameBuffer.getTexture(), camera, context->getViewport(), context->getViewport());
			wireframeShader.draw(*mesh, camera, context->getViewport());
			wireframeFrameBuffer.end();
		}
	}
	imageShader.draw(wireframeFrameBuffer.getTexture(), context->getViewport(),1.0f, false);
	camera.setDirty(false);
}

