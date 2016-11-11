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
#include "../../include/example/MeshPickerEx.h"
using namespace aly;
MeshPickerEx::MeshPickerEx() :
		Application(800, 600, "Mesh Picker Example"),matcapShader(getFullPath("images/JG_Silver.png")) {
}
bool MeshPickerEx::init(Composite& rootNode) {
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f),float3(1.0f, 1.0f, 1.0f));
	meshArray.resize(3);
	meshArray[0].load(getFullPath("models/monkey.obj"));
	meshArray[1].load(getFullPath("models/torus.ply"));
	meshArray[2].load(getFullPath("models/icosahedron.ply"));
	Subdivide(meshArray[2], SubDivisionScheme::Loop);
	Subdivide(meshArray[2], SubDivisionScheme::Loop);

	drawList.push_back( { &meshArray[0],float4x4::identity()} );
	drawList.push_back( { &meshArray[1],MakeScale(1.2f)*MakeTranslation(float3(-1.0f,1.0f,-1.0f))} );
	drawList.push_back( { &meshArray[2],MakeTranslation(float3(-1.0f,1.25f,0.0f))*MakeScale(0.2f)} );
	drawList.push_back({ &meshArray[2],MakeTranslation(float3(1.0f,-1.25f,0.0f))*MakeScale(0.2f) });
	//Initialize depth buffer to store the render
	int w = getContext()->getScreenWidth();
	int h = getContext()->getScreenHeight();
	depthFrameBuffer.initialize(w, h);
	selectedDepthBuffer.initialize(w, h);
	faceIdShader.initialize(w, h);
	//Make region on screen to render 3d view
	renderRegion=MakeRegion("Render View",CoordPerPX(0.5f,0.5f,-w/2,-h/2),CoordPX(w,h),COLOR_NONE,COLOR_WHITE,UnitPX(1.0f));

	//Set up camera
	camera.setNearFarPlanes(-2.0f, 2.0f);
	camera.setZoom(0.75f);
	camera.setCameraType(CameraType::Orthographic);
	camera.setDirty(true);
	//Map object geometry into unit bounding box for draw.
	camera.setPose(MakeTransform(meshArray[0].getBoundingBox(), renderBBox));
	//Add listener to respond to mouse manipulations
	addListener(&camera);
	//Add render component to root node so it is relatively positioned.
	rootNode.add(renderRegion);
	cursorText = MakeTextLabel("Cursor", CoordPX(0, 0), CoordPX(100, 30),FontType::Bold);
	cursorText->fontStyle = FontStyle::Outline;
	cursorText->setOrigin(Origin::BottomCenter);
	cursorText->horizontalAlignment = HorizontalAlignment::Center;
	rootNode.add(cursorText);
	selectedFaceId = int2(-1, -1);
	rootNode.onMouseOver = [this](AlloyContext* context, const InputEvent& e) {
		cursorText->setLabel("");
		selectedFaceId = int2(-1, -1);
		return false;
	};
	renderRegion->onMouseOver=[this](AlloyContext* context, const InputEvent& e) {
		if (faceIds.size() > 0) {
			box2px bounds = renderRegion->getBounds();
			float2 pix = pixel2(faceIds.dimensions())*(e.cursor - bounds.position) / bounds.dimensions;
			pix.y = faceIds.height - 1 - pix.y;
			selectedFaceId=faceIds((int)pix.x,(int)pix.y);
			if (selectedFaceId.y >= 0) {
				cursorText->setLabel(MakeString()<< "Face Id: " << selectedFaceId.x);
				cursorText->position=CoordPX(e.cursor);
				cursorText->pack(context);
			}
			else {
				selectedFaceId = int2(-1,-1);
				cursorText->setLabel( "");
			}
		}
		return false;
	};
	outlineShader.setLineWidth(2.0f);
	outlineShader.setInnerGlowColor(RGBAf(0.5f, 0.0f, 0.0f, 0.2f));
	outlineShader.setEdgeColor(RGBAf(1.0f, 1.0f, 1.0f, 0.2f));
	outlineShader.setOuterGlowColor(RGBAf(0.0f, 0.0f, 0.0f, 0.0f));
	return true;
}
void MeshPickerEx::draw(AlloyContext* context){
	
	if (camera.isDirty()) {
		//Compute depth and normals only when camera view changes.
		depthAndNormalShader.draw(drawList, camera, depthFrameBuffer);
		faceIdShader.draw(drawList, camera);
		faceIdShader.read(faceIds);
	}
	int oid = selectedFaceId.y;
	//Recompute lighting at every draw pass.
	matcapShader.draw(depthFrameBuffer.getTexture(), camera, context->pixelRatio*renderRegion->getBounds(), context->getViewport(), RGBAf(1.0f));
	if (oid >=0) {
		//Mask out selected object from depth buffer. Set faceId to -1 to un-specify face.
		selectionShader.draw(depthFrameBuffer.getTexture(),faceIdShader,selectedDepthBuffer,int2(-1,selectedFaceId.y));
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		outlineShader.setInnerGlowColor(RGBAf(0.5f, 0.0f, 0.0f, 0.2f));
		outlineShader.setEdgeColor(RGBAf(0.5f, 0.5f, 0.5f, 0.2f));

		//Draw outline around masked region.
		outlineShader.draw(selectedDepthBuffer.getTexture(), context->pixelRatio*renderRegion->getBounds(),context->getViewport());
		
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		//Mask out selected face from depth buffer
		selectionShader.draw(depthFrameBuffer.getTexture(), faceIdShader, selectedDepthBuffer, selectedFaceId);
		
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		outlineShader.setInnerGlowColor(RGBAf(0.0f, 0.0f, 1.0f, 0.2f));
		outlineShader.setEdgeColor(RGBAf(1.0f, 1.0f, 1.0f, 0.2f));

		//Draw outline around masked region.
		outlineShader.draw(selectedDepthBuffer.getTexture(), context->pixelRatio*renderRegion->getBounds(), context->getViewport());
	}

	camera.setDirty(false);
}

