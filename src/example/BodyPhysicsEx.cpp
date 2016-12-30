/*
* Copyright(C) 2016, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
#include "AlloyReconstruction.h"
#include "AlloyMeshPrimitives.h"
#include "../../include/example/BodyPhysicsEx.h"
using namespace aly;
BodyPhysicsEx::BodyPhysicsEx() : Application(1200, 800, "Body Physics Example"),phongShader(1) {


}
bool BodyPhysicsEx::init(Composite& rootNode) {
	grid.reset(new Grid(1.0f,1.0f, 5, 5,getContext()));
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f), float3(1.0f, 1.0f, 1.0f));
	BorderCompositePtr layout = BorderCompositePtr(new BorderComposite("UI Layout", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f), false));
	ParameterPanePtr controls = ParameterPanePtr(new ParameterPane("Controls", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f)));
	CompositePtr buttons = CompositePtr(new Composite("Buttons", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f)));
	IconButtonPtr addButton = IconButtonPtr(new IconButton(0xF067, CoordPerPX(0.25f, 0.5f, -30.0f, -30.0f), CoordPX(60.0f, 60.0f),IconType::CIRCLE));
	IconButtonPtr resetButton = IconButtonPtr(new IconButton(0xF01E, CoordPerPX(0.75f, 0.5f, -30.0f, -30.0f), CoordPX(60.0f, 60.0f), IconType::CIRCLE));
	addButton->backgroundColor = MakeColor(COLOR_NONE);
	resetButton->backgroundColor = MakeColor(COLOR_NONE);
	BorderCompositePtr controlLayout = BorderCompositePtr(new BorderComposite("Control Layout", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f), true));
	textLabel = TextLabelPtr(new TextLabel("", CoordPX(5, 5), CoordPerPX(1.0f, 0.0f,-10.0f,30.0f)));
	textLabel->fontSize = UnitPX(24.0f);
	textLabel->fontType = FontType::Bold;
	textLabel->fontStyle = FontStyle::Outline;
	addButton->borderWidth = UnitPX(0.0f);
	resetButton->borderWidth = UnitPX(0.0f);
	
	addButton->foregroundColor = MakeColor(getContext()->theme.LIGHTER);
	resetButton->foregroundColor = MakeColor(getContext()->theme.LIGHTER);
	
	addButton->iconColor = MakeColor(getContext()->theme.DARK);
	resetButton->iconColor = MakeColor(getContext()->theme.DARK);

	addButton->setNudgePosition(pixel2(0.0f, 2.0f));
	resetButton->setNudgePosition(pixel2(0.0f, 2.0f));
	resetButton->setNudgeSize(-2.0f);
	buttons->add(addButton);
	buttons->add(resetButton);
	float aspect = 6.0f;

	controls->setAlwaysShowVerticalScrollBar(false);
	controls->setScrollEnabled(false);
	controls->backgroundColor = MakeColor(getContext()->theme.DARKER);
	controlLayout->backgroundColor = MakeColor(getContext()->theme.DARKER);
	controlLayout->borderWidth = UnitPX(1.0f);
	controlLayout->borderColor = MakeColor(getContext()->theme.LIGHT);
	renderRegion = CompositePtr(new Composite("Render View", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f)));
	renderRegion->add(textLabel);
	layout->setWest(controlLayout, UnitPX(400.0f));
	controlLayout->setCenter(controls);
	controlLayout->setSouth(buttons, UnitPX(80.0f));
	layout->setCenter(renderRegion);
	buttons->borderWidth = UnitPX(1.0f);
	buttons->borderColor = MakeColor(getContext()->theme.DARK);
	buttons->backgroundColor = MakeColor(getContext()->theme.DARKER);
	rootNode.add(layout);
	camera.setNearFarPlanes(-2.0f, 2.0f);
	camera.setZoom(0.75f);
	std::cout << "Size " << grid->getBoundingBox() << std::endl;
	camera.setCameraType(CameraType::Orthographic);

	alpha=Float(1.0f);
	neighborSize=Integer(1);
	fractureDistanceTolerance=Float(999.0f);
	fractureRotationTolerance=Float(0.6f);
	aly::Number damping=Float(0.5f);
	bool fracturing;

	controls->addNumberField("Alpha",alpha,Float(0.0f),Float(1.0f));
	controls->addNumberField("Neighborhood", neighborSize, Integer(1), Integer(4));
	controls->addNumberField("Fracture Distance Tol.", fractureDistanceTolerance);
	controls->addNumberField("Fracture Rotation Tol.", fractureRotationTolerance);
	controls->addNumberField("Damping", damping);
	controls->addCheckBox("Fracturing", fracturing);

	addListener(&camera);

	renderRegion->onPack = [this]() {
		camera.setDirty(true);
		frameBuffersDirty = true;
	};
	camera.setDirty(true);
	camera.setActiveRegion(renderRegion.get(), false);

	phongShader[0] = SimpleLight(Color(0.3f, 0.3f, 0.3f, 0.25f),
		Color(0.0f, 0.0f, 0.0f, 0.0f), Color(0.0f, 0.0f, 0.8f, 0.5f),
		Color(0.0f, 0.0f, 0.0f, 0.0f), 16.0f, float3(0, 0.0, 2.0),
		float3(0, 1, 0));
	phongShader[0].moveWithCamera = false;

	wireframeShader.setFaceColor(Color(0.1f, 0.1f, 1.0f, 0.0f));
	wireframeShader.setEdgeColor(Color(1.0f, 0.8f, 0.1f, 1.0f));
	wireframeShader.setLineWidth(2.0f);
	getContext()->addDeferredTask([this]() {
		frameBuffersDirty = true;;
	});
	return true;
}
void BodyPhysicsEx::initializeFrameBuffers(aly::AlloyContext* context) {
	float2 dims = renderRegion->getBounds().dimensions;
	int w = (int)dims.x;
	int h = (int)dims.y;
	renderFrameBuffer.initialize(w, h);
	colorFrameBuffer.initialize(w, h);
	depthFrameBuffer.initialize(w, h);
	wireframeFrameBuffer.initialize(w, h);
}
void BodyPhysicsEx::draw(AlloyContext* context) {
	const double MIN_ELAPSED_TIME = 0.25f;
	box2px rbbox = renderRegion->getBounds();
	if (frameBuffersDirty) {
		initializeFrameBuffers(context);
		frameBuffersDirty = false;
	}
	if (camera.isDirty()) {
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		depthAndNormalShader.draw(*grid, camera, depthFrameBuffer, true);	
		wireframeFrameBuffer.begin();
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		phongShader.draw(depthFrameBuffer.getTexture(), camera,wireframeFrameBuffer.getViewport(), wireframeFrameBuffer.getViewport());
		wireframeShader.draw(*grid, camera, wireframeFrameBuffer.getViewport());
		wireframeFrameBuffer.end();
	}
	imageShader.draw(wireframeFrameBuffer.getTexture(), rbbox*context->pixelRatio, 1.0f, false);
	camera.setDirty(false);
}

