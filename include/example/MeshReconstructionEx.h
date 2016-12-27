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

#ifndef MESHRECONSTRUCTION_EX_H_
#define MESHRECONSTRUCTION_EX_H_

#include "AlloyApplication.h"
#include "CommonShaders.h"
class MeshReconstructionEx : public aly::Application {
protected:
	aly::Mesh mesh;
	aly::Mesh pointCloud;
	int displayIndex;
	int cameraType;
	aly::ImageGlyphPtr imageGlyph;
	aly::CompositePtr resizeableRegion;
	aly::CompositePtr renderRegion;
	aly::Number lineWidth;
	aly::Number particleSize;
	aly::Color faceColor;
	aly::Color lineColor;
	aly::Color pointColor;
	std::string matcapImageFile;
	bool frameBuffersDirty;
	bool parametersDirty;
	bool showPointCloud;
	bool showReconstruction;
	bool colorPointCloud;
	bool colorReconstruction;

	aly::Number treeDepth;
	aly::Number trimPercent;
	aly::Number bsplineDegree;
	aly::Number pointWeight;
	aly::Number samplesPerNode;
	aly::Number islandRatio;
	bool linearFitSurface;

	aly::CheckBoxPtr showReconstructionField;
	aly::CheckBoxPtr showPointCloudField;
	aly::ToggleBoxPtr colorReconstructionField;
	aly::ToggleBoxPtr colorPointCloudField;
	aly::SelectionPtr displayIndexField;
	aly::ModifiableNumberPtr lineWidthField;
	aly::ModifiableNumberPtr particleSizeField;
	aly::ColorSelectorPtr surfaceColorField;
	aly::ColorSelectorPtr pointColorField;
	aly::ColorSelectorPtr faceColorField;
	aly::ColorSelectorPtr lineColorField;
	aly::IconButtonPtr captureScreenshotButton;
	std::unique_ptr<aly::GLFrameBuffer> pointColorFrameBuffer;
	std::unique_ptr<aly::GLFrameBuffer> renderFrameBuffer;
	std::unique_ptr<aly::GLFrameBuffer> colorFrameBuffer;
	std::unique_ptr<aly::GLFrameBuffer> depthFrameBuffer;
	std::unique_ptr<aly::GLFrameBuffer> wireframeFrameBuffer;

	std::unique_ptr<aly::GLFrameBuffer> pointCloudDepthBuffer;
	std::unique_ptr<aly::CompositeShader> compositeShader;
	std::unique_ptr<aly::DepthAndNormalShader> depthAndNormalShader;
	std::unique_ptr<aly::ColorVertexShader> colorVertexShader;
	std::unique_ptr<aly::ParticleDepthShader> particleDepthShader;
	std::unique_ptr<aly::ParticleMatcapShader> particleMatcapShader;
	std::unique_ptr<aly::WireframeShader> wireframeShader;
	std::unique_ptr<aly::MatcapShader> matcapShader;
	std::unique_ptr<aly::ImageShader> imageShader;
	aly::Camera camera;
	aly::WorkerTaskPtr worker;
	aly::box3f objectBBox;
	void initializeFrameBuffers(aly::AlloyContext* context);
public:
	MeshReconstructionEx();
	void solve();
	bool init(aly::Composite& rootNode);
	void draw(aly::AlloyContext* context);
};

#endif 
