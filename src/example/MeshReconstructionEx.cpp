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
#include "../../include/example/MeshReconstructionEx.h"
using namespace aly;
MeshReconstructionEx::MeshReconstructionEx() : Application(800, 600, "Mesh Reconstruction Example") {


}
void MeshReconstructionEx::initializeMesh() {
	shadingType = 0;
	displayIndex = 0;
	lineWidth.setValue(1.0f);
	particleSize.setValue(0.02f);
	static int offsetIncrement = 0;
	if (mesh.triIndexes.size() == 0 && mesh.quadIndexes.size() == 0) {
		displayIndex = 4;
	}
	if (mesh.vertexColors.size() > 0) {
		shadingType = 1;
	}
	if (mesh.textureImage.size() > 0) {
		shadingType = 2;
	}
	lineColor = AlloyApplicationContext()->theme.DARK.toSemiTransparent(0.5f);
	faceColor = AlloyApplicationContext()->theme.LIGHT;
	surfaceColor = Color(255, 255, 255, 255);
	pointColor = Color(255, 255, 255, 255);
	texImage.reset(new GLTextureRGBAf());
	if (mesh.textureImage.size() > 0) {
		texImage->load(mesh.textureImage);
	}
}
bool MeshReconstructionEx::init(Composite& rootNode) {
	mesh.load(getFullPath("models/armadillo.ply"));
	pointCloud.load(getFullPath("models/eagle.ply"));
	std::cout << "Point Cloud " << pointCloud << std::endl;
	pointCloud.updateBoundingBox();
	objectBBox = pointCloud.getBoundingBox();
	displayIndex = 0;
	parametersDirty = true;
	frameBuffersDirty = true;
	uiDirty = true;
	this->app = app;
	renderFrameBuffer.reset(new GLFrameBuffer());
	pointColorFrameBuffer.reset(new GLFrameBuffer());
	bundleDepthFrameBuffer.reset(new GLFrameBuffer());
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
	textureMeshShader.reset(new TextureMeshShader());
	depthAndTextureShader.reset(new DepthAndTextureShader());
	matcapImageFile = app->getFullPath("images/JG_Silver.png");
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f), float3(1.0f, 1.0f, 1.0f));
	BorderCompositePtr layout = BorderCompositePtr(new BorderComposite("UI Layout", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f), false));
	ParameterPanePtr controls = ParameterPanePtr(new ParameterPane("Controls", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f)));
	BorderCompositePtr controlLayout = BorderCompositePtr(new BorderComposite("Control Layout", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f), true));

	controls->onChange = [this](const std::string& label, const AnyInterface& value) {
		parametersDirty = true;
	};
	float aspect = 6.0f;
	lineWidth = Float(1.0f);
	particleSize = Float(0.2f);

	lineColor = AlloyApplicationContext()->theme.DARK.toSemiTransparent(0.5f);
	faceColor = AlloyApplicationContext()->theme.LIGHT;
	surfaceColor = Color(255, 255, 255, 255);
	pointColor = Color(255, 255, 255, 255);

	controls->setAlwaysShowVerticalScrollBar(false);
	controls->setScrollEnabled(false);
	controls->backgroundColor = MakeColor(app->getContext()->theme.DARKER);
	controlLayout->backgroundColor = MakeColor(app->getContext()->theme.DARKER);
	controlLayout->borderWidth = UnitPX(1.0f);
	controlLayout->borderColor = MakeColor(app->getContext()->theme.LIGHT);
	renderRegion = CompositePtr(new Composite("Render View", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f)));
	layout->setWest(controlLayout, UnitPX(300.0f));
	controlLayout->setNorth(controls, UnitPX(500.0f));
	layout->setCenter(renderRegion);
	rootNode.add(layout);

	matcapShader->setTextureImage(getFullPath("images/JG_Silver.png"));
	particleMatcapShader->setTextureImage(getFullPath("images/JG_Silver.png"));
	camera.setNearFarPlanes(-2.0f, 2.0f);
	camera.setZoom(0.75f);
	camera.setCameraType(CameraType::Orthographic);
	controls->addGroup("Visualization",true);
	displayIndexField = controls->addSelectionField("Display", displayIndex, std::vector<std::string> { "Solid", "Solid & Wireframe", "Points & Wireframe","Wireframe", "Points" }, aspect);
	shadingStyleField = controls->addSelectionField("Shading", shadingType, std::vector<std::string> { "Matcap", "Vertex Color", "Texture" }, aspect);
	controls->addSelectionField("Camera", cameraType, std::vector<std::string> { "Orthographic", "Perspective" }, aspect);
	lineWidthField = controls->addNumberField("Line Width", lineWidth, Float(0.5f), Float(5.0f), 5.5f);
	particleSizeField = controls->addNumberField("Particle Size", particleSize, Float(0.0f), Float(1.0f), 5.5f);
	surfaceColorField = controls->addColorField("Surface", surfaceColor);
	pointColorField = controls->addColorField("Point", pointColor);
	faceColorField = controls->addColorField("Face", faceColor);
	lineColorField = controls->addColorField("Line", lineColor);
	float4x4 MT = MakeTransform(objectBBox, renderBBox);
	camera.setPose(MT);
	//Add listener to respond to mouse manipulations
	app->addListener(&camera);
	//Add render component to root node so it is relatively positioned.
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
void MeshReconstructionEx::initializeFrameBuffers(aly::AlloyContext* context) {
	float2 dims = renderRegion->getBounds().dimensions;
	int w = (int)dims.x;
	int h = (int)dims.y;
	renderFrameBuffer->initialize(w, h);
	pointColorFrameBuffer->initialize(w, h);
	colorFrameBuffer->initialize(w, h);
	depthFrameBuffer->initialize(w, h);
	bundleDepthFrameBuffer->initialize(w, h);
	wireframeFrameBuffer->initialize(w, h);
}
void MeshReconstructionEx::draw(AlloyContext* context) {
	const double MIN_ELAPSED_TIME = 0.25f;
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
		case 0:
			if (shadingType == 1) {
				colorVertexShader->draw(drawList, camera, *colorFrameBuffer);
			}
			else if (shadingType == 2) {
				depthAndTextureShader->draw(drawList, camera, *depthFrameBuffer);
			}
			else {
				depthAndNormalShader->draw(drawList, camera, *depthFrameBuffer);
			}
			wireframeShader->setSolid(false);
			break;
		case 1:
			if (shadingType == 1) {
				colorVertexShader->draw(drawList, camera, *colorFrameBuffer);
			}
			else if (shadingType == 2) {
				depthAndTextureShader->draw(drawList, camera, *depthFrameBuffer);
			}
			else {
				depthAndNormalShader->draw(drawList, camera, *depthFrameBuffer);
			}
			wireframeShader->setSolid(true);
			wireframeShader->setFaceColor(Color(0, 0, 0, 0));
			wireframeShader->setEdgeColor(lineColor);
			wireframeShader->draw(drawList, camera, *wireframeFrameBuffer);
			break;
		case 2:
			if (shadingType == 1) {
				colorFrameBuffer->begin();
				particleMatcapShader->draw(drawList, camera, colorFrameBuffer->getViewport(), colorFrameBuffer->getViewport(), psize);
				colorFrameBuffer->end();
			}
			else {
				particleDepthShader->draw(drawList, camera, *depthFrameBuffer, psize);
			}
			wireframeShader->setSolid(false);
			wireframeShader->setFaceColor(Color(0, 0, 0, 0));
			wireframeShader->setEdgeColor(lineColor);
			wireframeShader->draw(drawList, camera, *wireframeFrameBuffer);
			break;
		case 3:
			depthAndNormalShader->draw(drawList, camera, *depthFrameBuffer);
			colorVertexShader->draw(drawList, camera, *depthFrameBuffer);
			wireframeShader->setSolid(false);
			wireframeShader->setFaceColor(Color(0, 0, 0, 0));
			wireframeShader->setEdgeColor(lineColor);
			wireframeShader->draw(drawList, camera, *wireframeFrameBuffer);
			break;
		case 4:
			if (shadingType == 1) {
				colorFrameBuffer->begin();
				particleMatcapShader->draw(drawList, camera, colorFrameBuffer->getViewport(), colorFrameBuffer->getViewport(), psize);
				colorFrameBuffer->end();
			}
			else {
				particleDepthShader->draw(drawList, camera, *depthFrameBuffer, psize);
			}
			break;
		default:
			break;
		}
		if (pointCloud.vertexLocations.size()>0) {
			particleDepthShader->draw(pointCloud, camera, *bundleDepthFrameBuffer, psize);
		}
	}
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	const RGBAf bgColor = context->theme.DARKEST.toRGBAf();
	glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	//int2 dims = renderFrameBuffer->getDimensions();
	//box2px rbbox(float2(0.0f, 0.0f), float2((float)dims.x, (float)dims.y));
	box2px rbbox=renderRegion->getBounds();
	renderFrameBuffer->begin();
	switch (displayIndex) {
	case 0:
		if (shadingType == 1) {
			imageShader->draw(colorFrameBuffer->getTexture(), context->pixelRatio * rbbox, 1.0f, false);
		}
		else if (shadingType == 2) {
			textureMeshShader->draw(depthFrameBuffer->getTexture(), *texImage, camera, context->getViewport(), context->getViewport());
		}
		else {
			matcapShader->draw(depthFrameBuffer->getTexture(), camera, context->getViewport(), context->getViewport(), surfaceColor.toRGBAf());
		}
		break;
	case 1:
		if (shadingType == 1) {
			imageShader->draw(colorFrameBuffer->getTexture(), context->pixelRatio * rbbox, 1.0f, false);
		}
		else if (shadingType == 2) {
			textureMeshShader->draw(depthFrameBuffer->getTexture(), *texImage, camera, context->getViewport(), context->getViewport());
		}
		else {
			matcapShader->draw(depthFrameBuffer->getTexture(), camera, context->getViewport(), context->getViewport(), faceColor.toRGBAf());
		}
		imageShader->draw(wireframeFrameBuffer->getTexture(), context->getViewport(), 1.0f, false);
		break;
	case 2:
		imageShader->draw(wireframeFrameBuffer->getTexture(), context->getViewport(), 1.0f, false);
		if (shadingType == 1) {
			imageShader->draw(colorFrameBuffer->getTexture(), context->getViewport(), 1.0f, false);
		}
		else {
			matcapShader->draw(depthFrameBuffer->getTexture(), camera, context->getViewport(), context->getViewport(), pointColor.toRGBAf());
		}
		break;
	case 3:
		imageShader->draw(wireframeFrameBuffer->getTexture(), context->getViewport(), 1.0f, false);
		break;
	case 4:
		if (shadingType == 1) {
			imageShader->draw(colorFrameBuffer->getTexture(), context->getViewport(), 1.0f, false);
		}
		else {
			matcapShader->draw(depthFrameBuffer->getTexture(), camera, context->getViewport(), context->getViewport(), pointColor.toRGBAf());
		}
		break;
	default:
		break;
	}
	renderFrameBuffer->end();
	if (pointCloud.vertexLocations.size()>0 && displayIndex != 2 && displayIndex != 3) {

		
		pointColorFrameBuffer->begin();
		if (pointCloud.vertexColors.size() > 0) {
			glEnable(GL_DEPTH_TEST);
			particleMatcapShader->draw(pointCloud, camera, context->getViewport(), context->getViewport(), psize);
			glDisable(GL_DEPTH_TEST);
		} else {
			matcapShader->draw(bundleDepthFrameBuffer->getTexture(), camera, context->getViewport(), context->getViewport(), pointColor.toRGBAf());
		}
		pointColorFrameBuffer->end();
		
		compositeShader->draw(
			pointColorFrameBuffer->getTexture(),
			bundleDepthFrameBuffer->getTexture(),
			renderFrameBuffer->getTexture(),
			depthFrameBuffer->getTexture(),
			context->pixelRatio * rbbox);
	}
	else {
		imageShader->draw(renderFrameBuffer->getTexture(), context->pixelRatio * rbbox, 1.0f, false);
	}
	camera.setDirty(false);
	parametersDirty = false;
}

