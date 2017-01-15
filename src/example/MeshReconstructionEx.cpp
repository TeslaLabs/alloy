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
#include "../../include/example/MeshReconstructionEx.h"
using namespace aly;
MeshReconstructionEx::MeshReconstructionEx() : Application(1200, 800, "Mesh Reconstruction Example") {


}
bool MeshReconstructionEx::init(Composite& rootNode) {
	srand((unsigned int)time(nullptr));
	pointCloud.load(getFullPath("models/eagle.ply"));
	objectBBox = pointCloud.getBoundingBox();
	displayIndex = 0;
	parametersDirty = true;
	frameBuffersDirty = true;
	renderFrameBuffer.reset(new GLFrameBuffer());
	pointColorFrameBuffer.reset(new GLFrameBuffer());
	pointCloudDepthBuffer.reset(new GLFrameBuffer());
	depthFrameBuffer.reset(new GLFrameBuffer());
	wireframeFrameBuffer.reset(new GLFrameBuffer());
	colorFrameBuffer.reset(new GLFrameBuffer());
	compositeShader.reset(new CompositeShader());
	depthAndNormalShader.reset(new DepthAndNormalShader());
	wireframeShader.reset(new WireframeShader());
	particleDepthShader.reset(new ParticleDepthShader());
	particleMatcapShader.reset(new ParticleMatcapShader());
	matcapShader.reset(new MatcapShader());
	imageShader.reset(new ImageShader());
	colorVertexShader.reset(new ColorVertexShader());
	matcapImageFile = getFullPath("images/JG_Silver.png");
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
	executeButton->onMouseDown = [this](aly::AlloyContext* context, const InputEvent& e) {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			context->addDeferredTask([this]() {
				solve();
			});
			return true;
		}
		return false;
	};
	controls->onChange = [this](const std::string& label, const AnyInterface& value) {
		parametersDirty = true;
	};
	float aspect = 6.0f;

	lineColor = AlloyApplicationContext()->theme.DARK.toSemiTransparent(0.5f);
	faceColor = AlloyApplicationContext()->theme.LIGHT;

	pointColor = Color(255, 255, 255, 255);

	displayIndex = 0;
	lineWidth=Float(1.0f);
	particleSize=Float(0.04f);
	colorPointCloud = (pointCloud.vertexColors.size() > 0);
	showPointCloud = (pointCloud.vertexLocations.size() > 0);
	showReconstruction = (mesh.triIndexes.size() + mesh.triIndexes.size() > 0);
	colorReconstruction = (mesh.vertexColors.size()>0);

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

	matcapShader->setTextureImage(getFullPath("images/JG_Silver.png"));
	particleMatcapShader->setTextureImage(getFullPath("images/JG_Silver.png"));
	camera.setNearFarPlanes(-2.0f, 2.0f);
	camera.setZoom(0.75f);
	camera.rotateModelX(ALY_PI);
	camera.rotateModelY(ALY_PI_2);
	camera.setCameraType(CameraType::Orthographic);
	controls->addGroup("Visualization",true);
	

	displayIndexField = controls->addSelectionField("Display", displayIndex, std::vector<std::string> { "Solid", "Solid & Wireframe", "Wireframe"}, aspect);
	showPointCloudField = controls->addCheckBox("Show Points", showPointCloud);
	showReconstructionField = controls->addCheckBox("Show Mesh", showReconstruction);
	colorReconstructionField = controls->addToggleBox("Mesh Color", colorReconstruction);
	colorPointCloudField = controls->addToggleBox("Point Color",colorPointCloud);
	controls->addSelectionField("Camera", cameraType, std::vector<std::string> { "Orthographic", "Perspective" }, aspect);
	lineWidthField = controls->addNumberField("Line Width", lineWidth, Float(0.5f), Float(5.0f), 5.5f);
	particleSizeField = controls->addNumberField("Particle Size", particleSize, Float(0.0f), Float(1.0f), 5.5f);
	pointColorField = controls->addColorField("Point", pointColor);
	faceColorField = controls->addColorField("Face", faceColor);
	lineColorField = controls->addColorField("Line", lineColor);
	ReconstructionParameters params;
	treeDepth = Integer(params.Depth.value);
	treeFullDepth = Integer(params.FullDepth.value);
	treeMaxSolveDepth = Integer(params.MaxSolveDepth.value);
	trimPercent = Float(params.Trim.value);
	bsplineDegree = Integer(params.Degree.value);
	pointWeight = Float(params.PointWeight.value);
	samplesPerNode = Float(params.SamplesPerNode.value);
	islandRatio = Float(params.IslandAreaRatio.value);
	smoothingIterations = Integer(params.Smooth.value);
	solverIterations = Integer(params.Iters.value);
	linearFitSurface = params.LinearFit.set;
	boundaryType = params.BType.value;
	nonManifoldSurface = params.NonManifold.set;
	controls->addGroup("Surface Reconstruction",true);
	controls->addSelectionField("Boundary", boundaryType, std::vector<std::string>{"Free","Dirichlet","Neumann","Count"});
	controls->addNumberField("Tree Depth",treeDepth,Integer(1),Integer(12));
	controls->addNumberField("Full Depth", treeFullDepth, Integer(1), Integer(12));
	controls->addNumberField("Solve Depth", treeMaxSolveDepth, Integer(1), Integer(12));
	controls->addNumberField("Trim Percent", trimPercent,Float(0.0f),Float(1.0f));
	controls->addNumberField("B-Spline", bsplineDegree,Integer(1),Integer(4));
	controls->addNumberField("Point Weight", pointWeight,Float(0.0f),Float(8.0f));
	controls->addNumberField("Node Samples", samplesPerNode);
	controls->addNumberField("Solver Iterations", solverIterations);
	controls->addNumberField("Smooth Iterations", smoothingIterations);
	controls->addNumberField("Island Ratio", islandRatio);
	controls->addCheckBox("Non-Manifold", nonManifoldSurface);
	controls->addCheckBox("Linear Fit", linearFitSurface);

	float4x4 MT = MakeTransform(objectBBox, renderBBox);
	camera.setPose(MT);
	addListener(&camera);
	renderRegion->onPack = [this]() {
		camera.setDirty(true);
		frameBuffersDirty = true;
	};
	camera.setActiveRegion(renderRegion.get(), false);
	wireframeShader->setFaceColor(Color(0.1f, 0.1f, 1.0f, 0.5f));
	wireframeShader->setEdgeColor(Color(1.0f, 0.8f, 0.1f, 1.0f));
	wireframeShader->setLineWidth(lineWidth.toFloat() * 8.0f);
	return true;
}
void MeshReconstructionEx::solve() {
	
	showReconstructionField->setValue(false);
	showPointCloudField->setValue(true);
	showReconstruction = showReconstructionField->getValue();
	showPointCloud = showPointCloudField->getValue();
	mesh.clear();
	parametersDirty = true;
	if (worker.get() != nullptr) {
		worker->cancel();
	}
	else {
		worker.reset(new WorkerTask([this]() {
			ReconstructionParameters params;
			params.BType.value = boundaryType;
			params.Smooth.value = smoothingIterations.toInteger();
			params.Depth.value = treeDepth.toInteger();
			params.FullDepth.value = treeFullDepth.toInteger();
			params.MaxSolveDepth.value = treeMaxSolveDepth.toInteger();
			params.Trim.value = trimPercent.toFloat();
			params.Degree.value = bsplineDegree.toInteger();
			params.PointWeight.value = pointWeight.toFloat();
			params.SamplesPerNode.value = samplesPerNode.toFloat();
			params.IslandAreaRatio.value = islandRatio.toFloat();
			params.Iters.value = solverIterations.toInteger();
			params.LinearFit.set = linearFitSurface;
			params.NonManifold.set = nonManifoldSurface;
			SurfaceReconstruct(params, pointCloud, mesh, [this](const std::string& status, float progress) {
				textLabel->setLabel(MakeString()<<status<<" ...");
				AlloyApplicationContext()->requestPack();
				return !worker->isCanceled();
			});
		}, [this]() {
			colorReconstructionField->setValue(mesh.vertexColors.size() > 0);
			showReconstructionField->setValue(true);
			showPointCloudField->setValue(false);
			showReconstruction = showReconstructionField->getValue();
			showPointCloud = showPointCloudField->getValue();
			colorReconstruction = colorReconstructionField->getValue();
			textLabel->setLabel("");
			mesh.setDirty(true);
			parametersDirty = true;
			AlloyApplicationContext()->requestPack();
		}));
	}
	worker->execute();
}
void MeshReconstructionEx::initializeFrameBuffers(aly::AlloyContext* context) {
	float2 dims = renderRegion->getBounds().dimensions;
	int w = (int)dims.x;
	int h = (int)dims.y;
	renderFrameBuffer->initialize(w, h);
	pointColorFrameBuffer->initialize(w, h);
	colorFrameBuffer->initialize(w, h);
	depthFrameBuffer->initialize(w, h);
	pointCloudDepthBuffer->initialize(w, h);
	wireframeFrameBuffer->initialize(w, h);
}
void MeshReconstructionEx::draw(AlloyContext* context) {
	const float MIN_ELAPSED_TIME = 0.25f;
	if (frameBuffersDirty) {
		initializeFrameBuffers(context);
		frameBuffersDirty = false;
	}
	if (parametersDirty) {
		if (cameraType == 0) {
			camera.setNearFarPlanes(-2.0f, 2.0f);
			camera.setCameraType(CameraType::Orthographic);
		} else {
			camera.setNearFarPlanes(0.01f, 10.0f);
			camera.setCameraType(CameraType::Perspective);
		}
	}
	std::list<std::pair<aly::Mesh*, aly::float4x4>> drawList;
	drawList.push_back(std::pair<aly::Mesh*, aly::float4x4>(&mesh, mesh.pose));
	float psize = 0.001f + particleSize.toFloat() * 0.09f;
	if (camera.isDirty() || parametersDirty) {
		wireframeShader->setLineWidth(lineWidth.toFloat());
		switch (displayIndex) {
			case 0: //Solid
				if (colorReconstruction) {
					colorVertexShader->draw(drawList, camera, *colorFrameBuffer);
				}
				depthAndNormalShader->draw(drawList, camera, *depthFrameBuffer);
				wireframeShader->setSolid(false);
				break;
			case 1: //Solid & Wireframe
				if (colorReconstruction) {
					colorVertexShader->draw(drawList, camera, *colorFrameBuffer);
				}
				depthAndNormalShader->draw(drawList, camera, *depthFrameBuffer);
				wireframeShader->setSolid(true);
				wireframeShader->setFaceColor(Color(0, 0, 0, 0));
				wireframeShader->setEdgeColor(lineColor);
				wireframeShader->draw(drawList, camera, *wireframeFrameBuffer);
				break;
			case 2: // Wireframe
				wireframeShader->setSolid(false);
				wireframeShader->setFaceColor(Color(0, 0, 0, 0));
				wireframeShader->setEdgeColor(lineColor);
				wireframeShader->draw(drawList, camera, *wireframeFrameBuffer);
				break;
		}
		if (showPointCloud) {
			if (colorPointCloud) {
				pointColorFrameBuffer->begin();
				particleMatcapShader->draw(pointCloud, camera, context->getViewport(), context->getViewport(), psize);
				pointColorFrameBuffer->end();
			}
			particleDepthShader->draw(pointCloud, camera, *pointCloudDepthBuffer, psize);			
		}
	}
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	const RGBAf bgColor = context->theme.DARKEST.toRGBAf();
	glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	box2px rbbox=renderRegion->getBounds();
	renderFrameBuffer->begin();
	switch (displayIndex) {
		case 0: //Solid
			if (colorReconstruction) {
				imageShader->draw(colorFrameBuffer->getTexture(), context->getViewport(), 1.0f, false);
			} else {
				matcapShader->draw(depthFrameBuffer->getTexture(), camera, context->getViewport(), context->getViewport(), faceColor.toRGBAf());
			}
			break;
		case 1: ///Solid & Wireframe
			if (colorReconstruction) {
				imageShader->draw(colorFrameBuffer->getTexture(), context->getViewport(), 1.0f, false);
			} else {
				matcapShader->draw(depthFrameBuffer->getTexture(), camera, context->getViewport(), context->getViewport(), faceColor.toRGBAf());
			}
			imageShader->draw(wireframeFrameBuffer->getTexture(), context->getViewport(), 1.0f, false);
			break;
		case 2: //Wireframe
			imageShader->draw(wireframeFrameBuffer->getTexture(), context->getViewport(), 1.0f, false);
			break;
		default:
			break;
	}
	renderFrameBuffer->end();
	if (showPointCloud) {
		if (!colorPointCloud) {
			pointColorFrameBuffer->begin();
			matcapShader->draw(pointCloudDepthBuffer->getTexture(), camera, context->getViewport(), context->getViewport(), pointColor.toRGBAf());
			pointColorFrameBuffer->end();
		}
		if (showReconstruction) {
			compositeShader->draw(
				pointColorFrameBuffer->getTexture(),
				pointCloudDepthBuffer->getTexture(),
				renderFrameBuffer->getTexture(),
				depthFrameBuffer->getTexture(),
				context->pixelRatio * rbbox);
		} else {
			imageShader->draw(pointColorFrameBuffer->getTexture(), context->pixelRatio * rbbox, 1.0f, false);
		}
	} else if(showReconstruction){
		imageShader->draw(renderFrameBuffer->getTexture(), context->pixelRatio * rbbox, 1.0f, false);
	}
	camera.setDirty(false);
	parametersDirty = false;
}

