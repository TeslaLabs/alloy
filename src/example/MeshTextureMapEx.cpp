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
#include "example/MeshTextureMapEx.h"
using namespace aly;
MeshTextureMapEx::MeshTextureMapEx() :
		Application(1400, 600, "Automatic Texture Mapping Example") {
}
bool MeshTextureMapEx::init(Composite& rootNode) {
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f),float3(1.0f, 1.0f, 1.0f));
	mesh.load(getFullPath("models/tanya.ply"));
	mesh.updateVertexNormals();
	texImage.load(mesh.textureImage);
	//Make region on screen to render 3d view
	renderRegion=MakeRegion("Render View",CoordPX(0.0f,0.0f),CoordPercent(1.0f,1.0f),COLOR_NONE,COLOR_WHITE,UnitPX(1.0f));
	//Initialize depth buffer to store the render
	depthFrameBuffer.initialize(600,600);
	//Set up camera
	camera.setNearFarPlanes(-2.0f, 2.0f);
	camera.setZoom(0.75f);
	camera.setCameraType(CameraType::Orthographic);
	camera.setDirty(true);
	//Map object geometry into unit bounding box for draw.
	camera.setPose(MakeTransform(mesh.getBoundingBox(), renderBBox));
	//Add listener to respond to mouse manipulations

	//Add render component to root node so it is relatively positioned.
	BorderCompositePtr layout = BorderCompositePtr(new BorderComposite("UI Layout", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f), false));
	int w = 512;
	int h = 512;
	
	ConvertImage(mesh.textureImage, img);
	resizeableRegion = AdjustableCompositePtr(new AdjustableComposite("Image", CoordPerPX(0.5, 0.5, -w* 0.5f, -h * 0.5f),CoordPX(w, h)));
	Application::addListener(resizeableRegion.get());
	ImageGlyphPtr imageGlyph = AlloyApplicationContext()->createImageGlyph(img, false);
	DrawPtr drawContour = DrawPtr(new Draw("Contour Draw", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f), [this](AlloyContext* context, const box2px& bounds) {
		NVGcontext* nvg = context->nvgContext;
		
		float scale = bounds.dimensions.x / (float)img.width;
		Vector2f& uvs = mesh.textureMap;
		float2 uv1,uv2,uv3;
		nvgLineCap(nvg, NVG_ROUND);
		nvgLineJoin(nvg, NVG_ROUND);
		if (0.05f*scale > 0.5f) {
			nvgStrokeColor(nvg, Color(0.4f, 0.4f, 0.4f, 0.5f));
			nvgStrokeWidth(nvg, 0.05f*scale);
			nvgBeginPath(nvg);
			for (int i = 0; i < img.width; i++) {
				float2 pt = float2(0.5f + i, 0.5f);
				pt.x = pt.x / (float)img.width;
				pt.y = pt.y / (float)img.height;
				pt = pt*bounds.dimensions + bounds.position;
				nvgMoveTo(nvg, pt.x, pt.y);
				pt = float2(0.5f + i, 0.5f + img.height - 1.0f);
				pt.x = pt.x / (float)img.width;
				pt.y = pt.y / (float)img.height;
				pt = pt*bounds.dimensions + bounds.position;
				nvgLineTo(nvg, pt.x, pt.y);
			}
			for (int j = 0; j < img.height; j++) {
				float2 pt = float2(0.5f, 0.5f + j);
				pt.x = pt.x / (float)img.width;
				pt.y = pt.y / (float)img.height;
				pt = pt*bounds.dimensions + bounds.position;
				nvgMoveTo(nvg, pt.x, pt.y);
				pt = float2(0.5f + img.width - 1.0f, 0.5f + j);
				pt.x = pt.x / (float)img.width;
				pt.y = pt.y / (float)img.height;
				pt = pt*bounds.dimensions + bounds.position;
				nvgLineTo(nvg, pt.x, pt.y);
			}
			nvgStroke(nvg);
		}
		float lineWidth = std::max(0.1f*scale, 1.0f);

		int N = (int)uvs.size();
		if (0.1f*scale > 1.0f) {
			nvgStrokeColor(nvg, Color(0.2f, 0.2f, 0.2f, 1.0f));
			nvgStrokeWidth(nvg, 2.0f*lineWidth);
			for (int i = 0; i < N; i += 3) {
				uv1 = uvs[i];
				uv1.y = 1.0f - uv1.y;
				uv1 = uv1*bounds.dimensions + bounds.position;

				uv2 = uvs[i + 1];
				uv2.y = 1.0f - uv2.y;
				uv2 = uv2*bounds.dimensions + bounds.position;

				uv3 = uvs[i + 2];
				uv3.y = 1.0f - uv3.y;
				uv3 = uv3*bounds.dimensions + bounds.position;

				float area = std::abs(crossMag(uv2 - uv1, uv3 - uv1));
				if (area > 5.0f) {
					nvgBeginPath(nvg);
					nvgMoveTo(nvg, uv1.x, uv1.y);
					nvgLineTo(nvg, uv2.x, uv2.y);
					nvgLineTo(nvg, uv3.x, uv3.y);
					nvgClosePath(nvg);
					nvgStroke(nvg);
				}
			}
		}
		nvgStrokeColor(nvg, Color(0.8f, 0.8f, 0.8f, 1.0f));
		nvgStrokeWidth(nvg, lineWidth);
		for (int i = 0; i < N; i += 3) {
			uv1 = uvs[i];
			uv1.y = 1.0f - uv1.y;
			uv1 = uv1*bounds.dimensions + bounds.position;
			
			uv2 = uvs[i + 1];
			uv2.y = 1.0f - uv2.y;
			uv2 = uv2*bounds.dimensions + bounds.position;

			uv3 = uvs[i + 2];
			uv3.y = 1.0f - uv3.y;
			uv3 = uv3*bounds.dimensions + bounds.position;

			float area = std::abs(crossMag(uv2 - uv1, uv3 - uv1));
			if (area > 5.0f) {
				nvgBeginPath(nvg);
				nvgMoveTo(nvg, uv1.x, uv1.y);
				nvgLineTo(nvg, uv2.x, uv2.y);
				nvgLineTo(nvg, uv3.x, uv3.y);
				nvgClosePath(nvg);
				nvgStroke(nvg);
			}
		}
	}));
	GlyphRegionPtr glyphRegion = GlyphRegionPtr(new GlyphRegion("Image Region", imageGlyph, CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f)));
	glyphRegion->setAspectRule(AspectRule::Unspecified);
	glyphRegion->foregroundColor = MakeColor(COLOR_NONE);
	glyphRegion->backgroundColor = MakeColor(COLOR_NONE);
	glyphRegion->borderColor = MakeColor(COLOR_NONE);
	drawContour->onScroll = [this](AlloyContext* context, const InputEvent& event)
	{
		box2px bounds = resizeableRegion->getBounds(false);
		pixel scaling = (pixel)(1 - 0.1f*event.scroll.y);
		pixel2 newBounds = bounds.dimensions*scaling;
		pixel2 cursor = context->cursorPosition;
		pixel2 relPos = (cursor - bounds.position) / bounds.dimensions;
		pixel2 newPos = cursor - relPos*newBounds;
		bounds.position = newPos;
		bounds.dimensions = newBounds;
		resizeableRegion->setDragOffset(pixel2(0, 0));
		resizeableRegion->position = CoordPX(bounds.position - resizeableRegion->parent->getBoundsPosition());
		resizeableRegion->dimensions = CoordPX(bounds.dimensions);

		float2 dims = float2(img.dimensions());
		cursor = aly::clamp(dims*(event.cursor - bounds.position) / bounds.dimensions, float2(0.0f), dims);

		context->requestPack();
		return true;
	};

	resizeableRegion->add(glyphRegion);
	resizeableRegion->add(drawContour);
	resizeableRegion->setAspectRatio(img.width / (float)img.height);
	resizeableRegion->setAspectRule(AspectRule::FixedHeight);
	resizeableRegion->setDragEnabled(true);
	resizeableRegion->setClampDragToParentBounds(false);
	resizeableRegion->borderWidth = UnitPX(2.0f);
	resizeableRegion->borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);

	glyphRegion->onMouseDown = [=](AlloyContext* context, const InputEvent& e) {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			resizeableRegion->borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHTEST);
		}
		return false;
	};
	glyphRegion->onMouseUp = [=](AlloyContext* context, const InputEvent& e) {
		resizeableRegion->borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
		return false;
	};
	CompositePtr viewRegion = CompositePtr(new Composite("View", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f)));

	viewRegion->backgroundColor = MakeColor(getContext()->theme.DARKER);
	viewRegion->borderColor = MakeColor(getContext()->theme.DARK);
	viewRegion->borderWidth = UnitPX(1.0f);
	viewRegion->add(resizeableRegion);
	layout->setWest(renderRegion, UnitPX(600.0f));
	layout->setCenter(viewRegion);
	addListener(&camera);
	camera.setActiveRegion(renderRegion.get(), false);
	rootNode.add(layout);
	return true;
}
void MeshTextureMapEx::draw(AlloyContext* context){
	if (camera.isDirty()) {
		//Compute depth and texture uvs only when camera view changes.
		depthAndTextureShader.draw(mesh, camera, depthFrameBuffer);
	}
	textureMeshShader.draw(depthFrameBuffer.getTexture(),texImage,camera,context->pixelRatio*renderRegion->getBounds(),context->getViewport());
	camera.setDirty(false);
}

