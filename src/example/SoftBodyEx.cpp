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
#include "../../include/example/SoftBodyEx.h"
using namespace aly;
using namespace aly::softbody;
SoftBodyEx::SoftBodyEx() : Application(1200, 800, "Soft Body Physics Example"),phongShader(1) {


}
void SoftBodyEx::resetBodies() {
	bodies.clear();
}
void SoftBodyEx::addBody() {
	DrawBodyPtr dbody = DrawBodyPtr(new DrawBody(float3(1.0f,1.0f,1.0f)));
	Body& body = dbody->body;
	body.w = neighborSize.toInteger();
	body.alpha = alpha.toFloat();
	body.fracturing = fracturing;
	body.fractureDistanceTolerance = fractureDistanceTolerance.toFloat();
	body.fractureRotationTolerance = fractureRotationTolerance.toFloat();
	body.kRegionDamping = damping.toFloat();
	
	int Z = RandomUniform(3, 10);
	for (int z = 0; z < Z; z++) {
		int stx = 3 - RandomUniform(1, 3);
		int edx = 3 + RandomUniform(1, 3);

		int sty = 3 - RandomUniform(1, 3);
		int edy = 3 + RandomUniform(1, 3);
		
		for (int x = stx; x <= edx; x++){
			for (int y = sty; y <= edy; y++){
				body.addParticle(int3(x, y, z));
			}
		}
	}
	body.finalize();
	aly::Mesh& mesh = dbody->mesh;
	uint4 offset(0,0,0,0);
	for (CellPtr cell:body.cells) {
		for (int i = 0; i < 8; i++) {
			float3 pos = cell->vertices[i].position+float3(0.0f,0.0f,0.5f);
			pos.z = std::max(pos.z, 0.0f);
			mesh.vertexLocations.push_back(pos);
		}
		mesh.quadIndexes.push_back(offset + uint4(6, 2, 3, 7));
		mesh.quadIndexes.push_back(offset + uint4(1, 0, 4, 5));
		mesh.quadIndexes.push_back(offset + uint4(3, 1, 5, 7));
		mesh.quadIndexes.push_back(offset + uint4(2, 0, 1, 3));
		mesh.quadIndexes.push_back(offset + uint4(4, 0, 2, 6));
		mesh.quadIndexes.push_back(offset + uint4(5, 4, 6, 7));
		offset += uint4(8,8,8,8);
	}
	mesh.updateVertexNormals();
	mesh.updateBoundingBox();
	float theta = RandomUniform(0.0f, 2*ALY_PI);
	float3 randomVelocity = float3(std::cos(theta),std::sin(theta), 1.0f);
	randomVelocity = randomVelocity*maxVelocity.toFloat()*RandomUniform(0.0f, 1.0f);
	for(ParticlePtr particle : body.particles){
		particle->x.z += dropHeight.toFloat();
		particle->v += randomVelocity;
	}
	bodies.push_back(dbody);
}
bool SoftBodyEx::updatePhysics(){
	const int TARGET_FPS = 30;
	const float h = 1.0f / TARGET_FPS;
	const float lowX = -50.0f;
	const float hiX = 50.0f;
	const float lowY = -50.0f;
	const float hiY = 50.0f;
	const float lowZ = 0.0f;
	bool updated=false;
	const float3 gravity = float3(0.0f, 0.0f, -9.8f);
	for(DrawBodyPtr dbody : bodies)
	{
		Body* body = &dbody->body;
		// Do FastLSM simulation
		body->shapeMatch();
		body->calculateParticleVelocities(h);
		body->doRegionDamping();
		body->applyParticleVelocities(h);
		body->doFracturing();
		// Apply gravity and check floor

		for(ParticlePtr particle : body->particles){
			particle->f += gravity;
			if (particle->x.z < lowZ){
				// This particle has hit the floor
				particle->f.z -= particle->x.z- lowZ;
				particle->v = float3(0.0f);
				particle->x.z = lowZ;
			}
			if (particle->x.x < lowX) {
				// This particle has hit wall
				particle->f.x -= (particle->x.x - lowX);
				particle->v = float3(0.0f);
				particle->x.x = lowX;
			}
			if (particle->x.x > hiX) {
				// This particle has hit wall
				particle->f.x -= (particle->x.x - hiX);
				particle->v = float3(0.0f);
				particle->x.x = hiX;
			}
			if (particle->x.y < lowY) {
				// This particle has hit wall
				particle->f.y -= (particle->x.y - lowY);
				particle->v = float3(0.0f);
				particle->x.y = lowY;
			}
			if (particle->x.y > hiY) {
				// This particle has hit wall
				particle->f.y -= (particle->x.y - hiY);
				particle->v = float3(0.0f);
				particle->x.y = hiY;
			}
		}
		body->updateCellPositions();
		int index = 0;
		aly::Mesh& mesh = dbody->mesh;
		for (CellPtr cell : body->cells) {
			for (int i = 0; i < 8; i++) {
				float3 pos = cell->vertices[i].position + float3(0.0f, 0.0f, 0.5f);
				pos.z = std::max(pos.z, 0.0f);
				mesh.vertexLocations[index++]=pos;
			}
		}
		mesh.setDirty(true);
		updated = true;
	}
	return updated;
}
bool SoftBodyEx::init(Composite& rootNode) {
	grid.reset(new Grid(100.0f, 100.0f, 10, 10, getContext()));
	box3f objectBBox = box3f(float3(-50.0f,-50.0f,0.0f), float3(50.0f,50.0f, 50.0f));
	BorderCompositePtr layout = BorderCompositePtr(new BorderComposite("UI Layout", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f), false));
	ParameterPanePtr controls = ParameterPanePtr(new ParameterPane("Controls", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f)));
	CompositePtr buttons = CompositePtr(new Composite("Buttons", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f)));
	IconButtonPtr addButton = IconButtonPtr(new IconButton(0xF067, CoordPerPX(0.25f, 0.5f, -30.0f, -30.0f), CoordPX(60.0f, 60.0f),IconType::CIRCLE));
	IconButtonPtr resetButton = IconButtonPtr(new IconButton(0xF01E, CoordPerPX(0.75f, 0.5f, -30.0f, -30.0f), CoordPX(60.0f, 60.0f), IconType::CIRCLE));
	addButton->backgroundColor = MakeColor(COLOR_NONE);
	resetButton->backgroundColor = MakeColor(COLOR_NONE);
	
	addButton->onMouseDown = [this](AlloyContext* context, const InputEvent& e) {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			context->addDeferredTask([this]() {
				this->addBody();
			});
			return true;
		}
		return false;
	};
	resetButton->onMouseDown = [this](AlloyContext* context, const InputEvent& e) {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			context->addDeferredTask([this]() {
				this->resetBodies();
			});
			return true;
		}
		return false;
	};
	BorderCompositePtr controlLayout = BorderCompositePtr(new BorderComposite("Control Layout", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f), true));
	textLabel = TextLabelPtr(new TextLabel("", CoordPX(5, 5), CoordPerPX(1.0f, 0.0f,-10.0f,30.0f)));
	textLabel->fontSize = UnitPX(24.0f);
	textLabel->fontType = FontType::Bold;
	textLabel->fontStyle = FontStyle::Outline;
	addButton->borderWidth = UnitPX(0.0f);
	resetButton->borderWidth = UnitPX(0.0f);
	
	addButton->foregroundColor = MakeColor(getContext()->theme.LIGHTER);
	resetButton->foregroundColor = MakeColor(getContext()->theme.LIGHTER);
	addButton->borderColor = MakeColor(getContext()->theme.DARKER);
	resetButton->borderColor = MakeColor(getContext()->theme.DARKER);

	addButton->iconColor = MakeColor(getContext()->theme.DARK);
	resetButton->iconColor = MakeColor(getContext()->theme.DARK);

	addButton->setNudgePosition(pixel2(0.0f, 2.0f));
	resetButton->setNudgePosition(pixel2(0.0f, 2.0f));
	resetButton->setNudgeSize(-4.0f);
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
	camera.setNearFarPlanes(0.001f, 4.0f);
	camera.setZoom(1.75f);
	camera.setCameraType(CameraType::Perspective);
	camera.setPose(MakeScale(1.0f/200.0f));
	camera.rotateModelX(-ALY_PI * 60 / 180.0f);
	alpha=Float(1.0f);
	neighborSize=Integer(1);
	fractureDistanceTolerance=Float(999.0f);
	fractureRotationTolerance=Float(0.6f);
	damping=Float(0.5f);
	maxVelocity = Integer(20);
	dropHeight = Integer(10);
	fracturing = true;
	controls->addNumberField("Alpha",alpha,Float(0.0f),Float(1.0f));
	controls->addNumberField("Neighborhood", neighborSize, Integer(1), Integer(2));
	controls->addNumberField("Max Velocity", maxVelocity,Integer(0), Integer(40));
	controls->addNumberField("Drop Height", dropHeight, Integer(10), Integer(40));
	controls->addNumberField("Damping", damping);
	controls->addNumberField("Fracture Distance Tolerlance", fractureDistanceTolerance);
	controls->addNumberField("Fracture Rotation Tolerlance", fractureRotationTolerance);
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
	wireframeShader.setFaceColor(Color(0.1f, 0.1f, 0.1f, 0.0f));
	wireframeShader.setEdgeColor(Color(1.0f, 0.8f, 0.1f, 1.0f));
	wireframeShader.setLineWidth(1.25f);
	getContext()->addDeferredTask([this]() {
		frameBuffersDirty = true;
	});
	addBody();
	return true;
}
void SoftBodyEx::initializeFrameBuffers(aly::AlloyContext* context) {
	float2 dims = renderRegion->getBounds().dimensions;
	int w = (int)dims.x;
	int h = (int)dims.y;
	renderFrameBuffer.initialize(w, h);
	colorFrameBuffer.initialize(w, h);
	depthFrameBuffer.initialize(w, h);
	wireframeFrameBuffer.initialize(w, h);
}
void SoftBodyEx::draw(AlloyContext* context) {
	box2px rbbox = renderRegion->getBounds();
	if (frameBuffersDirty) {
		initializeFrameBuffers(context);
		frameBuffersDirty = false;
	}
	bool updated=true;
	
	try {
		updated = updatePhysics();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		std::getchar();
	}
	
	std::list<std::pair<Mesh*, float4x4>> drawList;
	drawList.push_back(std::pair<Mesh*, float4x4>(grid.get(), float4x4::identity()));
	for (DrawBodyPtr body : bodies) {
		drawList.push_back(std::pair<Mesh*, float4x4>(&body->mesh, float4x4::identity()));
	}
	if (camera.isDirty()||updated) {
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		depthAndNormalShader.draw(drawList, camera, depthFrameBuffer, true);
		glEnable(GL_BLEND);
		wireframeShader.draw(drawList, camera, wireframeFrameBuffer);
		colorFrameBuffer.begin();
		glEnable(GL_BLEND);
		phongShader.draw(depthFrameBuffer.getTexture(), camera, wireframeFrameBuffer.getViewport(), depthFrameBuffer.getViewport());		
		colorFrameBuffer.end();

	}
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	imageShader.draw(colorFrameBuffer.getTexture(), rbbox*context->pixelRatio, 1.0f, false);
	imageShader.draw(wireframeFrameBuffer.getTexture(), rbbox*context->pixelRatio, 1.0f, false);

	camera.setDirty(false);
}

