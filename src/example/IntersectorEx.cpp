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

#include "../../include/example/IntersectorEx.h"

#include "Alloy.h"
using namespace aly;
IntersectorEx::IntersectorEx() :
		Application(768, 512, "Mesh Intersector Example"),matcapShader(getFullPath("images/JG_Gold.png")) {
}
bool IntersectorEx::init(Composite& rootNode) {
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f),float3(1.0f, 1.0f, 1.0f));

	mesh.load(getFullPath("models/monkey.obj"));
	Subdivide(mesh, SubDivisionScheme::CatmullClark);
	mesh.updateVertexNormals();
	//Make region on screen to render 3d view
	renderRegion=MakeRegion("Render View",CoordPX(0,0),CoordPX(512,512),COLOR_NONE,COLOR_WHITE,UnitPX(1.0f));
	//Initialize depth buffer to store the render
	depthFrameBuffer.initialize(512,512);
	//Set up camera
	camera.setNearFarPlanes(0.1f, 2.0f);
	camera.setZoom(1.1f);
	camera.setCameraType(CameraType::Perspective);
	camera.setDirty(true);
	//Map object geometry into unit bounding box for draw.
	camera.setPose(MakeTransform(mesh.getBoundingBox(), renderBBox));
	//Add listener to respond to mouse manipulations
	addListener(&camera);
	//Add render component to root node so it is relatively positioned.

	ImageRGBA tarImg(256,256);
	tarImg.set(RGBA(0,64,0,255));
	ImageGlyphPtr depthGlyph = createImageGlyph(tarImg, false);
	GlyphRegionPtr depthRegion = MakeGlyphRegion(depthGlyph,
		CoordPX(512,0), CoordPX(256,256),
		AspectRule::FixedWidth, COLOR_NONE, COLOR_NONE,
		Color(200, 200, 200, 255), UnitPX(1.0f));
	ImageGlyphPtr distGlyph = createImageGlyph(tarImg, false);
	GlyphRegionPtr distRegion = MakeGlyphRegion(distGlyph,
		CoordPX(512, 256), CoordPX(256, 256),
		AspectRule::FixedWidth, COLOR_NONE, COLOR_NONE,
		Color(200, 200, 200, 255), UnitPX(1.0f));

	TextLabelPtr textLabel = TextLabelPtr(
		new TextLabel("",
			CoordPX(5,5), CoordPercent(0.5f, 0.5f)));
	textLabel->fontSize = UnitPX(20.0f);
	textLabel->fontType = FontType::Bold;
	textLabel->fontStyle = FontStyle::Outline;

	rootNode.add(renderRegion);
	rootNode.add(depthRegion);
	rootNode.add(distRegion);
	rootNode.add(textLabel);
	
	workerTask = WorkerTaskPtr(new WorkerTask([=] {
		ImageRGBA depthRGBA(tarImg.width,tarImg.height);
		ImageRGBA distRGBA(tarImg.width,tarImg.height);
		Image1f distImg(tarImg.width, tarImg.height);
		Image1f depthImg(tarImg.width, tarImg.height);
		camera.aim(depthFrameBuffer.getViewport());
		textLabel->setLabel( "Building Kd-Tree ...");
		kdTree.build(mesh,6);
		textLabel->setLabel( "Computing Depth Field ...");
		float minD = 1E30f;
		float maxD = 0;
#pragma omp parallel for
		for (int i = 0; i < depthRGBA.width; i++) {
			for (int j = 0; j < depthRGBA.height; j++) {
				float3 pt1 = camera.transformImageToWorld(
					float3((float)(i+0.5f), (float)(j+0.5f), 0.0f), depthRGBA.width,
					depthRGBA.height);
				float3 pt2 = camera.transformImageToWorld(
					float3((float)(i+0.5f), (float)(j+0.5f), 1.0f), depthRGBA.width,
					depthRGBA.height);
				float3 v = normalize(pt2 - pt1);
				float3 lastPoint(0.0f);
				double d = kdTree.intersectRayDistance(pt1, v, lastPoint);
				if (d != NO_HIT_DISTANCE) {
					depthImg(i, j) = (float)d;
					//Not OpenMP safe, but gives a reasonable estimate of min/max
					minD = std::min((float)d, minD);
					maxD = std::max((float)d, maxD);
				}
				else {
					depthImg(i, j) = 0.0f;
				}
			}
		}
		
		for (int n = 0;n < (int)depthImg.size();n++) {
			float d = depthImg[n].x;
			if (d > 0) {
				depthRGBA[n] = HSVAtoColor(HSVA(0.9f*(d - minD) / (maxD - minD), 1.0f, 1.0f, 1.0f)).toRGBA();
			}
			else {
				depthRGBA[n] = RGBA(0,0,0,0);
			}
		}

		getContext()->addDeferredTask([=]() {
			depthGlyph->set(depthRGBA, getContext().get());
		});
		textLabel->setLabel("Computing Distance Field ...");
		box3f bbox = mesh.getBoundingBox();
		minD = 1E30f;
		maxD = -1E30f;
		//Make sure bounds are square.
		float dim = std::max(bbox.dimensions.x, bbox.dimensions.z);
		float3 center = bbox.center();
		float3 newDims(dim, bbox.dimensions.y, dim);
		bbox = box3f(center - newDims*0.5f, newDims);
#pragma omp parallel for
		for (int i = 0; i < distRGBA.width; i++) {
			for (int j = 0; j < distRGBA.height; j++) {
				float3 pt1 = bbox.position
					+ bbox.dimensions
					* float3(i / (float)distRGBA.width, 0.5f,
						j / (float)distRGBA.height);
				float3 lastPoint(0.0f);
				double d = kdTree.closestPoint(pt1, lastPoint);
				if (d != NO_HIT_DISTANCE) {
					distImg(i, j) = (float)d;
					//Not OpenMP safe, but gives a reasonable estimate of min/max
					minD = std::min((float)d, minD);
					maxD = std::max((float)d, maxD);
				}
				else {
					distImg(i, j) = 0;
				}
			}
		}
		for (int n = 0;n < (int)distImg.size();n++) {
			float d = distImg[n].x;
			if (d > 0) {
				distRGBA[n] = HSVAtoColor(HSVA(0.9f*(d - minD) / (maxD - minD), 1.0f, 1.0f, 1.0f)).toRGBA();
			}
			else {
				distRGBA[n] = RGBA(0, 0, 0, 0);
			}
		}
		getContext()->addDeferredTask([=]() {
			distGlyph->set(distRGBA, getContext().get());
		});
		textLabel->setLabel("Finished!");
	}));
	workerTask->execute(isForcedClose());
	return true;
}
void IntersectorEx::draw(AlloyContext* context){
	if (camera.isDirty()) {
		//Compute depth and normals only when camera view changes.
		depthAndNormalShader.draw(mesh, camera, depthFrameBuffer);
	}
	//Recompute lighting at every draw pass.
	matcapShader.draw(depthFrameBuffer.getTexture(),camera, context->pixelRatio*renderRegion->getBounds(),context->getViewport(),RGBAf(1.0f));
	camera.setDirty(false);
}

