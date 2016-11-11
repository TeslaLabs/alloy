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
#include "../../include/example/MeshDepthEx.h"
using namespace aly;
const int RENDER_WIDTH = 256;
const int RENDER_HEIGHT = 256;
MeshDepthEx::MeshDepthEx() :
		Application(RENDER_WIDTH*5, RENDER_HEIGHT*2, "Deferred Depth Render Example"),imageShader(ImageShader::Filter::SMALL_BLUR){
}
bool MeshDepthEx::init(Composite& rootNode) {
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f), float3(1.0f, 1.0f, 1.0f));

	mesh.load(getFullPath("models/icosahedron.ply"));
	Subdivide(mesh, SubDivisionScheme::Loop);
	mesh.updateVertexNormals();
	//Initialize depth buffer to store the render
	depthFrameBuffer.initialize(2*RENDER_WIDTH,2*RENDER_HEIGHT);
	edgeFrameBuffer.initialize(RENDER_WIDTH, RENDER_HEIGHT);
	distanceFieldFrameBuffer.initialize(RENDER_WIDTH, RENDER_HEIGHT);
	outlineFrameBuffer.initialize(RENDER_WIDTH, RENDER_HEIGHT);
	particleDepthFrameBuffer.initialize(RENDER_WIDTH, RENDER_HEIGHT);
	lineFrameBuffer.initialize(RENDER_WIDTH, RENDER_HEIGHT);
	//occlusionFrameBuffer.initialize(RENDER_WIDTH, RENDER_HEIGHT);
	//Set up camera
	camera.setNearFarPlanes(-2.0f, 2.0f);
	camera.setZoom(1.0f);
	camera.setCameraType(CameraType::Orthographic);
	camera.setDirty(true);
	//Map object geometry into unit bounding box for draw.
	camera.setPose(MakeTransform(mesh.getBoundingBox(), renderBBox));
	//Add listener to respond to mouse manipulations
	addListener(&camera);
	distanceFieldShader.setExtent(16);
	lineDistanceShader.setLineWidth(3.0f);
	//Renders as line width 3 because depth frame buffer is twice the size. Yields nice anti-aliased lines.
	outlineShader.setLineWidth(6.0f);
	//ambientOcclusionShader.setSampleRadius(0.01f);
	return true;
}
void MeshDepthEx::draw(AlloyContext* context){
	static float2 dRange = float2(0, 0);
	if (camera.isDirty()) {
		//Compute depth and normals only when camera view changes.
		edgeDepthAndNormalShader.draw(mesh, camera, edgeFrameBuffer);
		lineDistanceShader.draw(mesh, camera, lineFrameBuffer);
		particleDepthShader.draw(mesh, camera, particleDepthFrameBuffer, 0.1f);
		dRange = camera.computeNormalizedDepthRange(mesh);
		//Use flat normals for ambient occlusion
		depthAndNormalShader.draw(mesh, camera, depthFrameBuffer, true);
		//ambientOcclusionShader.draw(depthFrameBuffer.getTexture(),camera, occlusionFrameBuffer);
		depthAndNormalShader.draw(mesh, camera, depthFrameBuffer, false);
		distanceFieldShader.draw(depthFrameBuffer.getTexture(), distanceFieldFrameBuffer);
		outlineShader.draw(depthFrameBuffer.getTexture(), outlineFrameBuffer);
	}
	//Recompute lighting at every draw pass.
	depthColorShader.draw(depthFrameBuffer.getTexture(), dRange, context->pixelRatio*float2(0.0f, 0.0f), context->pixelRatio*float2((float)RENDER_WIDTH, (float)RENDER_HEIGHT));
	normalColorShader.draw(depthFrameBuffer.getTexture(), context->pixelRatio*float2(0.0f, (float)RENDER_HEIGHT), context->pixelRatio*float2((float)RENDER_WIDTH, (float)RENDER_HEIGHT));
	
	depthColorShader.draw(edgeFrameBuffer.getTexture(), float2(0.0f,0.1f), context->pixelRatio*float2((float)RENDER_WIDTH,0.0f), context->pixelRatio*float2((float)RENDER_WIDTH, (float)RENDER_HEIGHT));
	normalColorShader.draw(edgeFrameBuffer.getTexture(), context->pixelRatio*float2((float)RENDER_WIDTH, (float)RENDER_HEIGHT), context->pixelRatio*float2((float)RENDER_WIDTH, (float)RENDER_HEIGHT));
	
	depthColorShader.draw(particleDepthFrameBuffer.getTexture(), dRange, context->pixelRatio*float2(2.0f*(float)RENDER_WIDTH, 0.0f), context->pixelRatio*float2((float)RENDER_WIDTH, (float)RENDER_HEIGHT));
	normalColorShader.draw(particleDepthFrameBuffer.getTexture(), context->pixelRatio*float2(2.0f*(float)RENDER_WIDTH,(float)RENDER_HEIGHT), context->pixelRatio*float2((float)RENDER_WIDTH, (float)RENDER_HEIGHT));

	depthColorShader.draw(lineFrameBuffer.getTexture(), dRange, context->pixelRatio*float2(3.0f*(float)RENDER_WIDTH,0.0f), context->pixelRatio*float2((float)RENDER_WIDTH, (float)RENDER_HEIGHT));
	normalColorShader.draw(lineFrameBuffer.getTexture(), context->pixelRatio*float2(3.0f*(float)RENDER_WIDTH,(float)RENDER_HEIGHT), context->pixelRatio*float2((float)RENDER_WIDTH, (float)RENDER_HEIGHT));

	imageShader.draw(distanceFieldFrameBuffer.getTexture(), context->pixelRatio*float2(4 * (float)RENDER_WIDTH, 0.0f), context->pixelRatio*float2((float)RENDER_WIDTH, (float)RENDER_HEIGHT), 1.0f, false);
	imageShader.draw(outlineFrameBuffer.getTexture(), context->pixelRatio*float2(4 * (float)RENDER_WIDTH, (float)RENDER_HEIGHT), context->pixelRatio*float2((float)RENDER_WIDTH, (float)RENDER_HEIGHT), 1.0f, false);

	camera.setDirty(false);
}

