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
#include "../../include/example/BodyPhysicsEx.h"
using namespace aly;
BodyPhysicsEx::BodyPhysicsEx() : Application(1200, 800, "Body Physics Example") {


}
bool BodyPhysicsEx::init(Composite& rootNode) {
	renderFrameBuffer.reset(new GLFrameBuffer());
	depthFrameBuffer.reset(new GLFrameBuffer());
	wireframeFrameBuffer.reset(new GLFrameBuffer());
	colorFrameBuffer.reset(new GLFrameBuffer());
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f), float3(1.0f, 1.0f, 1.0f));
	BorderCompositePtr layout = BorderCompositePtr(new BorderComposite("UI Layout", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f), false));
	ParameterPanePtr controls = ParameterPanePtr(new ParameterPane("Controls", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f)));
	CompositePtr buttons = CompositePtr(new Composite("Buttons", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f)));
	TextIconButtonPtr executeButton=TextIconButtonPtr(new TextIconButton("Execute",0xf085,CoordPerPX(0.5f,0.5f,-120.0f,-30.0f),CoordPX(240.0f,60.0f)));
	BorderCompositePtr controlLayout = BorderCompositePtr(new BorderComposite("Control Layout", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f), true));
	textLabel = TextLabelPtr(new TextLabel("", CoordPX(5, 5), CoordPerPX(1.0f, 0.0f,-10.0f,30.0f)));
	textLabel->fontSize = UnitPX(24.0f);
	textLabel->fontType = FontType::Bold;
	textLabel->fontStyle = FontStyle::Outline;
	buttons->add(executeButton);
	executeButton->setRoundCorners(true);
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
	camera.rotateModelX(ALY_PI);
	camera.rotateModelY(ALY_PI_2);
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
	controls->addNumberField("Fracture Rotation Tol.", fractureDistanceTolerance,Float(0.0f),Float(ALY_PI));
	controls->addNumberField("Damping", damping);
	controls->addCheckBox("Fracturing", fracturing);
	float4x4 MT = MakeTransform(objectBBox, renderBBox);
	camera.setPose(MT);
	addListener(&camera);
	frameBuffersDirty = true;
	renderRegion->onPack = [this]() {
		camera.setDirty(true);
		frameBuffersDirty = true;
	};
	camera.setActiveRegion(renderRegion.get(), false);
	wireframeShader.reset(new WireframeShader());
	wireframeShader->setFaceColor(Color(0.1f, 0.1f, 1.0f, 0.5f));
	wireframeShader->setEdgeColor(Color(1.0f, 0.8f, 0.1f, 1.0f));
	wireframeShader->setLineWidth(1.0f);
	return true;
}
void BodyPhysicsEx::initializeFrameBuffers(aly::AlloyContext* context) {
	float2 dims = renderRegion->getBounds().dimensions;
	int w = (int)dims.x;
	int h = (int)dims.y;
	renderFrameBuffer->initialize(w, h);
	colorFrameBuffer->initialize(w, h);
	depthFrameBuffer->initialize(w, h);
	wireframeFrameBuffer->initialize(w, h);
}
void BodyPhysicsEx::draw(AlloyContext* context) {
	const double MIN_ELAPSED_TIME = 0.25f;
	if (frameBuffersDirty) {
		initializeFrameBuffers(context);
		frameBuffersDirty = false;
	}
	
	camera.setDirty(false);
}

