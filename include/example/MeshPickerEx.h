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

#ifndef MESHPICKER_EX_H_
#define MESHPICKER_EX_H_

#include "AlloyApplication.h"
#include "CommonShaders.h"
class MeshPickerEx: public aly::Application {
protected:
	aly::RegionPtr renderRegion;
	aly::GLFrameBuffer depthFrameBuffer,selectedDepthBuffer;
	aly::DepthAndNormalShader depthAndNormalShader;
	aly::FaceIdShader faceIdShader;
	aly::MatcapShader matcapShader;
	aly::OutlineShader outlineShader;
	aly::SelectionShader selectionShader;
	aly::TextLabelPtr cursorText;
	std::vector<aly::Mesh> meshArray;
	std::vector<std::pair<aly::Mesh*, aly::float4x4>> drawList;
	aly::Camera camera;
	aly::Image2i faceIds;
	aly::int2 selectedFaceId;
public:
	MeshPickerEx();
	bool init(aly::Composite& rootNode);
	void draw(aly::AlloyContext* context);
};

#endif 
