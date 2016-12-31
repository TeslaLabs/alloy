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
void BodyPhysicsEx::resetBodies() {
	bodies.clear();
}
void BodyPhysicsEx::addBody() {
	DrawBodyPtr dbody = DrawBodyPtr(new DrawBody(float3(1.0f,1.0f,1.0f)));
	Body& body = dbody->body;
	body.w = std::rand() % 2 + 1;
	body.alpha = 0.75f;
	body.fracturing = true;
	body.fractureDistanceTolerance = 999.0f;
	body.fractureRotationTolerance = 0.6f;
	body.kRegionDamping = 0.25f;
	
	int width = std::rand() % 3 + 2;
	int height = std::rand() % 10 + 2;
	int depth = std::rand() % 3 + 2;
	std::cout << "Dimensions " << width << " " << height << " " << depth << std::endl;
	for (int x = 0; x < width; x++){
		for (int y = 0; y < height; y++){
			for (int z = 0; z < depth; z++){
				body.AddParticle(int3(x, y, z));
			}
		}
	}
	body.Finalize();
	aly::Mesh& mesh = dbody->mesh;
	std::cout << "Particles " << body.particles.size() << std::endl;
	std::cout << "Lattice Locations " << body.latticeLocations.size() << std::endl;
	std::cout << "Lattice " << body.lattice.size() << std::endl;
	std::cout << "Cells " << body.cells.size()<<std::endl;
	for (CellPtr cell:body.cells) {
		for (int i = 0; i < 8; i++) {
			float3 pos = cell->vertices[i].position;
			mesh.vertexLocations.push_back(pos);
			std::cout << i << ": " << pos << std::endl;
		}
		mesh.quadIndexes.push_back(uint4(7,3,2,6));
		mesh.quadIndexes.push_back(uint4(5,4,0,1));
		mesh.quadIndexes.push_back(uint4(7,5,1,3));
		mesh.quadIndexes.push_back(uint4(3,1,0,2));
		mesh.quadIndexes.push_back(uint4(6,2,0,4));
		mesh.quadIndexes.push_back(uint4(7,6,4,5));
	}
	mesh.updateVertexNormals();
	mesh.updateBoundingBox();
	std::cout << "Body " << mesh.getBoundingBox() << std::endl;
	// Add a position offset and a random velocity
	const float velocityMax = 25;
	const float velocityHalf = velocityMax / 2;
	float3 randomVelocity = float3((rand() % 1000 / 1000.0f) * velocityMax - velocityHalf, (rand() % 1000 / 1000.0f) * velocityMax - velocityHalf, (rand() % 1000 / 1000.0f) * velocityMax - velocityHalf);
	for(ParticlePtr particle : body.particles){
		particle->x.y += 5;
		particle->v += randomVelocity;
	}
	bodies.push_back(dbody);
}
void BodyPhysicsEx::updatePhysics(){
	const int MAX_BODIES = 50;
	const int SPAWN_RATE = 30;
	const int TARGET_FPS = 30;
	const float h = 1.0f / TARGET_FPS;
	for(DrawBodyPtr dbody : bodies)
	{
		Body* body = &dbody->body;
		// Do FastLSM simulation
		body->ShapeMatch();
		body->CalculateParticleVelocities(h);
		body->PerformRegionDamping();
		body->ApplyParticleVelocities(h);
		body->DoFracturing();
		// Apply gravity and check floor
		const float3 gravity = float3(0.0f, -9.8f, 0.0f);
		for(ParticlePtr particle : body->particles){
			particle->f += gravity;
			if (particle->x.y < 0){
				// This particle has hit the floor
				particle->f.y -= particle->x.y;
				particle->v = float3(0.0f);
				particle->x.y = 0;
			}
		}
		body->UpdateCellPositions();
	}
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
	camera.setNearFarPlanes(-2.0f, 2.0f);
	camera.setZoom(0.75f);
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
	box2px rbbox = renderRegion->getBounds();
	if (frameBuffersDirty) {
		initializeFrameBuffers(context);
		frameBuffersDirty = false;
	}
	/*
	try {
		updatePhysics();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		std::getchar();
	}
	*/
	std::list<std::pair<Mesh*, float4x4>> drawList;
	drawList.push_back(std::pair<Mesh*, float4x4>(grid.get(), float4x4::identity()));
	for (DrawBodyPtr body : bodies) {
		drawList.push_back(std::pair<Mesh*, float4x4>(&body->mesh, float4x4::identity()));
	}
	if (camera.isDirty()) {
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		depthAndNormalShader.draw(drawList, camera, depthFrameBuffer, true);	
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

