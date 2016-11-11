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

#ifndef COMMONSHADERS_H_
#define COMMONSHADERS_H_

#include "GLTexture.h"
#include "GLShader.h"
#include "GLFrameBuffer.h"
#include "AlloyImage.h"
#include "AlloyUnits.h"
#include <initializer_list>
namespace aly {
struct CameraParameters;
class DepthAndNormalShader: public GLShader {
public:
	DepthAndNormalShader(bool onScreen = true,
			const std::shared_ptr<AlloyContext>& context =
					AlloyDefaultContext());
	void draw(const std::initializer_list<Mesh*>& meshes,
			CameraParameters& camera, GLFrameBuffer& framebuffer,
			bool flatShading = false);
	void draw(
			const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, GLFrameBuffer& framebuffer,
			bool flatShading = false);
	void draw(const std::list<Mesh*>& meshes, CameraParameters& camera,
			GLFrameBuffer& framebuffer, bool flatShading = false);
	void draw(const std::vector<Mesh*>& meshes, CameraParameters& camera,
		GLFrameBuffer& framebuffer, bool flatShading = false);
	void draw(const std::list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, GLFrameBuffer& framebuffer,
			bool flatShading = false);
	void draw(const std::vector<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, GLFrameBuffer& framebuffer,
		bool flatShading = false);
	void draw(Mesh& mesh, CameraParameters& camera,
			GLFrameBuffer& framebuffer, bool flatShading = false) {
		draw( { &mesh }, camera, framebuffer, flatShading);
	}
};
class ColorVertexShader: public GLShader {
public:
	ColorVertexShader(bool onScreen = true,
			const std::shared_ptr<AlloyContext>& context =
					AlloyDefaultContext());
	void draw(const std::initializer_list<Mesh*>& meshes,
			CameraParameters& camera, GLFrameBuffer& framebuffer,
			bool flatShading = false);
	void draw(
			const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, GLFrameBuffer& framebuffer,
			bool flatShading = false);
	void draw(const std::list<Mesh*>& meshes, CameraParameters& camera,
			GLFrameBuffer& framebuffer, bool flatShading = false);
	void draw(const std::list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, GLFrameBuffer& framebuffer,
			bool flatShading = false);

	void draw(Mesh& mesh, CameraParameters& camera,
			GLFrameBuffer& framebuffer, bool flatShading = false) {
		draw( { &mesh }, camera, framebuffer, flatShading);
	}

};
class ParticleIdShader: public GLShader {
private:
	GLFrameBuffer framebuffer;
public:
	GLFrameBuffer& getFrameBuffer() {
		return framebuffer;
	}
	void read(Image2i& faceIdMap);
	ParticleIdShader(bool onScreen = true,
			const std::shared_ptr<AlloyContext>& context =
					AlloyDefaultContext());
	void initialize(int w, int h);
	void draw(const std::initializer_list<Mesh*>& meshes,
			CameraParameters& camera, int faceIdOffset = 0, int objectIdOffset =
					0, float radius = 1.0f);
	void draw(
			const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, int faceIdOffset = 0, int objectIdOffset =
					0, float radius = 1.0f);
	void draw(const std::list<Mesh*>& meshes, CameraParameters& camera,
			Image2i& faceIdMap, int faceIdOffset = 0, int objectIdOffset = 0,
			float radius = 1.0f);
	void draw(const std::list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, int faceIdOffset = 0, int objectIdOffset =
					0, float radius = 1.0f);
	void draw(Mesh& mesh, CameraParameters& camera, int faceIdOffset = 0,
			int objectIdOffset = 0, float radius = 1.0f) {
		draw( { &mesh }, camera, faceIdOffset, objectIdOffset, radius);
	}
};
class ParticleDepthShader: public GLShader {
public:
	ParticleDepthShader(bool onScreen = true,
			const std::shared_ptr<AlloyContext>& context =
					AlloyDefaultContext());
	void draw(const std::initializer_list<Mesh*>& meshes,
			CameraParameters& camera, GLFrameBuffer& framebuffer, float radius =
					1.0f);
	void draw(
			const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, GLFrameBuffer& framebuffer, float radius =
					1.0f);

	void draw(const std::list<Mesh*>& meshes, CameraParameters& camera,
			GLFrameBuffer& framebuffer, float radius = 1.0f);
	void draw(const std::list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, GLFrameBuffer& framebuffer, float radius =
					1.0f);
	void draw(Mesh& mesh, CameraParameters& camera,
			GLFrameBuffer& framebuffer, float radius = 1.0f) {
		draw( { &mesh }, camera, framebuffer, radius);
	}
};
class ParticleMatcapShader: public GLShader {
private:
	GLTextureRGBA matcapTexture;
public:
	ParticleMatcapShader(const std::string& textureImage="", bool onScreen = true,
			const std::shared_ptr<AlloyContext>& context =
					AlloyDefaultContext());
	void setTextureImage(const std::string& textureImage);
	void setTextureImage(const ImageRGBA& textureImage);
	void draw(const std::initializer_list<Mesh*>& meshes,
			CameraParameters& camera, const box2px& bounds,
			const box2px& viewport, float radius = 1.0f);
	void draw(
			const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, const box2px& bounds,
			const box2px& viewport, float radius = 1.0f);
	void draw(const std::list<Mesh*>& meshes, CameraParameters& camera,
			const box2px& bounds, const box2px& viewport, float radius = 1.0f);
	void draw(const std::list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, const box2px& bounds,
			const box2px& viewport, float radius = 1.0f);
	void draw(Mesh& mesh, CameraParameters& camera, const box2px& bounds,
			const box2px& viewport, float radius = 1.0f) {
		draw( { &mesh }, camera, bounds, viewport, radius);
	}
};
class DepthAndTextureShader: public GLShader {
public:
	DepthAndTextureShader(bool onScreen = true,
			const std::shared_ptr<AlloyContext>& context =
					AlloyDefaultContext());
	void draw(const std::initializer_list<Mesh*>& meshes,
			CameraParameters& camera, GLFrameBuffer& framebuffer,
			bool flatShading = false);
	void draw(const std::list<Mesh*>& meshes, CameraParameters& camera,
			GLFrameBuffer& framebuffer, bool flatShading = false);

	void draw(
			const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, GLFrameBuffer& framebuffer,
			bool flatShading = false);
	void draw(const std::list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, GLFrameBuffer& framebuffer,
			bool flatShading = false);

	void draw(Mesh& mesh, CameraParameters& camera,
			GLFrameBuffer& framebuffer, bool flatShading = false) {
		draw( { &mesh }, camera, framebuffer, flatShading);
	}

};
class FaceIdShader: public GLShader {
private:
	GLFrameBuffer framebuffer;
	GLShader particleIdShader;
public:
	GLFrameBuffer& getFrameBuffer() {
		return framebuffer;
	}
	const GLFrameBuffer& getFrameBuffer() const {
		return framebuffer;
	}
	FaceIdShader(bool onScreen = true,
			const std::shared_ptr<AlloyContext>& context =
					AlloyDefaultContext());
	void initialize(int w, int h);
	void read(Image2i& faceIdMap);
	int draw(const std::initializer_list<Mesh*>& meshes,
			CameraParameters& camera, int faceIdOffset = 0, int objectIdOffset =
					0, float radius = 1.0f);
	int draw(
			const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, int faceIdOffset = 0, int objectIdOffset =
					0, float radius = 1.0f);
	int draw(const std::list<Mesh*>& meshes, CameraParameters& camera,
			int faceIdOffset = 0, int objectIdOffset = 0, float radius = 1.0f);
	int draw(const std::list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, int faceIdOffset = 0, int objectIdOffset =
					0, float radius = 1.0f);
	int draw(const std::vector<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, int faceIdOffset = 0, int objectIdOffset =
		0, float radius = 1.0f);
	int draw(Mesh& mesh, CameraParameters& camera, int faceIdOffset = 0,
			int objectIdOffset = 0, float radius = 1.0f) {
		return draw( { &mesh }, camera, faceIdOffset, objectIdOffset, radius);
	}
};

class MatcapShader: public GLShader {
private:
	GLTextureRGBA matcapTexture;
public:
	MatcapShader(const std::string& textureImage="", bool onScreen = true,
			const std::shared_ptr<AlloyContext>& context =
					AlloyDefaultContext());
	void setTextureImage(const std::string& textureImage);
	void setTextureImage(const ImageRGBA& textureImage);

	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& imageTexture, CameraParameters& camera,
			const box2px& bounds, const box2px& viewport, const RGBAf& tint =
					RGBAf(1, 1, 1, 1)) {
		begin().set("matcapTexture", matcapTexture, 0).set("textureImage",
				imageTexture, 1).set("depthBufferSize",
				imageTexture.dimensions()).set("bounds", bounds).set("viewport",
				viewport).set("tint", tint).draw(imageTexture).end();
	}
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& imageTexture, CameraParameters& camera,
			const float2& location, const float2& dimensions,
			const box2px& viewport, const RGBAf& tint = RGBAf(1, 1, 1, 1)) {
		begin().set("matcapTexture", matcapTexture, 0).set("textureImage",
				imageTexture, 1).set("depthBufferSize",
				imageTexture.dimensions()).set("bounds",
				box2px(location, dimensions)).set("viewport", viewport).set(
				"tint", tint).draw(imageTexture).end();

	}
};
class TextureMeshShader: public GLShader {
public:
	TextureMeshShader(bool onScreen = true,
			const std::shared_ptr<AlloyContext>& context =
					AlloyDefaultContext());
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& depthBuffer,
			const GLTextureRGBAf& texture, CameraParameters& camera,
			const box2px& bounds, const box2px& viewport,
			const box2px& textureBounds = box2px(float2(0, 0), float2(1, 1))) {
		begin().set("textureImage", texture, 0).set("depthBuffer", depthBuffer,
				1).set("depthBufferSize", depthBuffer.dimensions()).set(
				"bounds", bounds).set("viewport", viewport).set("texBounds",
				textureBounds).draw(depthBuffer).end();
	}
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& depthBuffer,
			const GLTextureRGBAf& texture, CameraParameters& camera,
			const float2& location, const float2& dimensions,
			const box2px& viewport,
			const box2px& textureBounds = box2px(float2(0, 0), float2(1, 1))) {
		begin().set("textureImage", texture, 0).set("depthBuffer", depthBuffer,
				1).set("depthBufferSize", depthBuffer.dimensions()).set(
				"bounds", box2px(location, dimensions)).set("viewport",
				viewport).set("texBounds", textureBounds).draw(depthBuffer).end();

	}
};
class CompositeShader: public GLShader {
public:
	CompositeShader(bool onScreen = true,
			const std::shared_ptr<AlloyContext>& context =
					AlloyDefaultContext());
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& sourceImageTexture,
			const GLTexture<T, C, I>& sourceDepthTexture,
			const GLTexture<T, C, I>& targetImageTexture,
			const GLTexture<T, C, I>& targetDepthTexture, const box2px& bounds,
			float sourceAlpha = 1.0f, float targetAlpha = 1.0f) {
		begin().set("sourceImage", sourceImageTexture, 0).set("sourceDepth",
				sourceDepthTexture, 1).set("targetImage", targetImageTexture, 2).set(
				"targetDepth", targetDepthTexture, 3).set("bounds", bounds).set(
				"sourceAlpha", sourceAlpha).set("targetAlpha", targetAlpha).set(
				"viewport", context->getViewport())

		.draw(targetImageTexture).end();
	}
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& sourceImageTexture,
			const GLTexture<T, C, I>& sourceDepthTexture,
			const GLTexture<T, C, I>& targetImageTexture,
			const GLTexture<T, C, I>& targetDepthTexture,
			const float2& location, const float2& dimensions,
			float sourceAlpha = 1.0f, float targetAlpha = 1.0f) {
		begin().set("sourceImage", sourceImageTexture, 0).set("sourceDepth",
				sourceDepthTexture, 1).set("targetImage", targetImageTexture, 2).set(
				"targetDepth", targetDepthTexture, 3).set("bounds",
				box2px(location, dimensions)).set("sourceAlpha", sourceAlpha).set(
				"targetAlpha", targetAlpha).set("viewport",
				context->getViewport()).draw(targetImageTexture).end();
	}
};
class SelectionShader : public GLShader {
public:
	SelectionShader(bool onScreen = true,
		const std::shared_ptr<AlloyContext>& context =
		AlloyDefaultContext());
	template<class T, int C, ImageType I> void draw(
		const GLTexture<T, C, I>& depthTexture,
		const FaceIdShader& faceIdShader,
		const GLFrameBuffer& targetDepthBuffer,int2 faceId) {
		int vertId = faceId.x;
		int objectId = faceId.y;
		int4 mask;
		if (vertId < 0) {
			mask = int4(-1,-1,-1, (objectId>=0)?1 + objectId:-1);
		}
		else {
			mask = int4((vertId)& (0x00000FFF), (((vertId)& (0x00FFF000)) >> (12)), (((vertId)& (0xFF000000)) >> (24)), (objectId >= 0) ? 1 + objectId : -1);
		}
		targetDepthBuffer.begin();
		begin().set("faceId", mask).set("depthImage", depthTexture, 0).set("faceImage",
			faceIdShader.getFrameBuffer().getTexture(), 1).set("depthBufferSize",
				depthTexture.dimensions())
			.draw(depthTexture).end();
		targetDepthBuffer.end();
	}

};
class ImageShader: public GLShader {
public:
	enum class Filter {
		NONE, FXAA, SMALL_BLUR, MEDIUM_BLUR, LARGE_BLUR
	};
	ImageShader( const Filter& filter = Filter::NONE,bool onScreen = true,
			const std::shared_ptr<AlloyContext>& context =
					AlloyDefaultContext());
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& imageTexture, const box2px& bounds,
			float alpha = 1.0f, bool flip = false) {
		begin().set("flip", flip ? 1 : 0).set("textureImage", imageTexture, 0).set(
				"bounds", bounds).set("alpha", alpha).set("viewport",
				context->getViewport()).draw(imageTexture).end();
	}
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& imageTexture,
			float alpha = 1.0f, bool flip = false) {
		begin().set("flip", flip ? 1 : 0).set("textureImage", imageTexture, 0).set(
				"bounds", context->getViewport()).set("alpha", alpha).set("viewport",
				context->getViewport()).draw(imageTexture).end();
	}
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& imageTexture, const float2& location,
			const float2& dimensions, float alpha = 1.0f, bool flip = false) {
		begin().set("flip", flip ? 1 : 0).set("textureImage", imageTexture, 0).set(
				"bounds", box2px(location, dimensions)).set("alpha", alpha).set(
				"viewport", context->getViewport()).draw(imageTexture).end();

	}
};
class EdgeEffectsShader: public GLShader {
public:
	EdgeEffectsShader(bool onScreen = true,
			const std::shared_ptr<AlloyContext>& context =
					AlloyDefaultContext());
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& imageTexture, const box2px& bounds) {
		begin().set("KERNEL_SIZE", 4).set("depthBufferSize",
				imageTexture.dimensions()).set("textureImage", imageTexture, 0).set(
				"bounds", bounds).set("imageSize",
				imageTexture.bounds.dimensions).set("viewport",
				context->getViewport()).draw(imageTexture).end();
	}
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& imageTexture, const float2& location,
			const float2& dimensions) {
		draw(imageTexture, box2px(location, dimensions));
	}
};

class NormalColorShader: public GLShader {
public:
	NormalColorShader(bool onScreen = true,
			const std::shared_ptr<AlloyContext>& context =
					AlloyDefaultContext());
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& imageTexture, const box2px& bounds) {
		begin().set("depthBufferSize", imageTexture.dimensions()).set(
				"textureImage", imageTexture, 0).set("bounds", bounds).set(
				"viewport", context->getViewport()).draw(imageTexture).end();
	}
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& imageTexture, const float2& location,
			const float2& dimensions) {
		draw(imageTexture, box2px(location, dimensions));
	}
};

class DepthColorShader: public GLShader {
public:
	DepthColorShader(bool onScreen = true,
			const std::shared_ptr<AlloyContext>& context =
					AlloyDefaultContext());

	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& imageTexture, float2 zRange,
			const box2px& bounds) {
		begin().set("depthBufferSize", imageTexture.dimensions()).set(
				"textureImage", imageTexture, 0).set("zMin", zRange.x), set(
				"zMax", zRange.y).set("bounds", bounds).set("viewport",
				context->getViewport()).draw(imageTexture).end();
	}
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& imageTexture, float2 zRange,
			const float2& location, const float2& dimensions) {
		draw(imageTexture, zRange, box2px(location, dimensions));
	}
};

class EdgeDepthAndNormalShader: public GLShader {
public:
	EdgeDepthAndNormalShader(bool onScreen = true,
			const std::shared_ptr<AlloyContext>& contex =
					AlloyDefaultContext());
	void draw(const std::initializer_list<Mesh*>& meshes,
			CameraParameters& camera, GLFrameBuffer& framebuffer);
	void draw(Mesh& mesh, CameraParameters& camera,
			GLFrameBuffer& framebuffer) {
		draw( { &mesh }, camera, framebuffer);
	}
	void draw(
			const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, GLFrameBuffer& framebuffer);
	void draw(const std::list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, GLFrameBuffer& framebuffer);
};
struct SimpleLight {
	Color ambientColor;
	Color diffuseColor;
	Color lambertianColor;
	Color specularColor;
	float specularPower;
	float3 position;
	float3 direction;
	float2 focalLength;
	bool moveWithCamera;
	bool castShadow;
	bool orthographic;
	bool attenuate;
	SimpleLight(const Color& ambientColor, const Color& diffuseColor,
			const Color& lambertianColor, const Color& specularColor,
			const float& specularPower, const float3& position,
			const float3& direction, const float2& focalLength = float2(0, 0),
			bool moveWithCamera = false, bool castShadow = false,
			bool orthographic = false, bool attenuate = false) :
			ambientColor(ambientColor), diffuseColor(diffuseColor), lambertianColor(
					lambertianColor), specularColor(specularColor), specularPower(
					specularPower), position(position), direction(direction), focalLength(
					focalLength), moveWithCamera(moveWithCamera), castShadow(
					castShadow), orthographic(orthographic), attenuate(
					attenuate) {

	}
	SimpleLight() :
			ambientColor(COLOR_NONE), diffuseColor(COLOR_NONE), lambertianColor(
					COLOR_NONE), specularColor(COLOR_NONE), specularPower(0.0f), position(
					0.0f), direction(0.0f), focalLength(0.0f), moveWithCamera(
					false), castShadow(false), orthographic(false), attenuate(
					false) {

	}
};
class PhongShader: public GLShader {

	std::vector<SimpleLight> lights;
public:
	inline SimpleLight& getLight(int i) {
		if (i >= (int) lights.size()) {
			throw std::runtime_error("Light index out of range.");
		}
		return lights[i];
	}
	const SimpleLight & operator [](int i) const {
		return lights[i];
	}
	SimpleLight & operator [](int i) {
		return lights[i];
	}
	inline int size() const {
		return (int) lights.size();
	}
	PhongShader(int numLights, bool onScreen=true,
			const std::shared_ptr<AlloyContext>& contex =
					AlloyDefaultContext());
	PhongShader(const SimpleLight& light, bool onScreen = true,
			const std::shared_ptr<AlloyContext>& context =
					AlloyDefaultContext()) :
			PhongShader(1, onScreen, context) {
		lights[0] = light;
	}
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& imageTexture, CameraParameters& camera,
			const box2px& bounds, const box2px& viewport) {
		begin();
		set("textureImage", imageTexture, 0).set("MIN_DEPTH",
				camera.getNearPlane()).set("MAX_DEPTH", camera.getFarPlane()).set(
				"focalLength", camera.getFocalLength()).set("bounds", bounds).set(
				"viewport", viewport);

		std::vector<Color> ambientColors;
		std::vector<Color> diffuseColors;
		std::vector<Color> lambertianColors;
		std::vector<Color> specularColors;

		std::vector<float> specularWeights;
		std::vector<float3> lightDirections;
		std::vector<float3> lightPositions;

		for (SimpleLight& light : lights) {
			ambientColors.push_back(light.ambientColor);
			diffuseColors.push_back(light.diffuseColor);
			specularColors.push_back(light.specularColor);
			lambertianColors.push_back(light.lambertianColor);
			specularWeights.push_back(light.specularPower);
			if (light.moveWithCamera) {
				float3 pt = (camera.View * light.position.xyzw()).xyz();
				lightPositions.push_back(pt);
				float3 norm = normalize(
						(camera.NormalView * float4(light.direction, 0)).xyz());
				lightDirections.push_back(norm);
			} else {
				lightPositions.push_back(light.position);
				lightDirections.push_back(light.direction);
			}
		}
		set("depthBufferSize", imageTexture.dimensions()).set("ambientColors",
				ambientColors).set("diffuseColors", diffuseColors).set(
				"lambertianColors", lambertianColors).set("specularColors",
				specularColors).set("specularWeights", specularWeights).set(
				"lightPositions", lightPositions).set("lightDirections",
				lightDirections).draw(imageTexture).end();
	}
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& imageTexture, CameraParameters& camera,
			const float2& location, const float2& dimensions,
			const box2px& viewport) {
		draw(imageTexture, camera, box2px(location, dimensions), viewport);

	}

};
class WireframeShader: public GLShader {
private:
	float lineWidth;
	Color edgeColor;
	Color faceColor;
	bool solid;
public:
	inline void setEdgeColor(const Color& c) {
		edgeColor = c;
	}
	inline void setFaceColor(const Color& c) {
		faceColor = c;
	}
	void setLineWidth(float w) {
		this->lineWidth = w;
	}
	void setSolid(bool s) {
		solid = s;
	}
	WireframeShader(bool onScreen = true,
			const std::shared_ptr<AlloyContext>& contex =
					AlloyDefaultContext());
	void draw(const std::initializer_list<Mesh*>& meshes,
			CameraParameters& camera, const box2px& bounds);
	void draw(Mesh& mesh, CameraParameters& camera,
			const box2px& bounds) {
		draw( { &mesh }, camera, bounds);
	}
	void draw(
			const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, const box2px& bounds);
	void draw(const std::list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, const box2px& bounds);

	void draw(const std::initializer_list<Mesh*>& meshes,
			CameraParameters& camera, const GLFrameBuffer& frameBuffer);
	void draw(Mesh& mesh, CameraParameters& camera,
			const GLFrameBuffer& frameBuffer);
	void draw(
			const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, const GLFrameBuffer& frameBuffer);
	void draw(const std::list<std::pair<Mesh*, float4x4>>& meshes,
			CameraParameters& camera, const GLFrameBuffer& frameBuffer);
};
class LineDistanceShader : public GLShader {
private:
	float lineWidth;
	bool solid;
public:
	void setLineWidth(float w) {
		this->lineWidth = w;
	}
	void setSolid(bool s) {
		solid = s;
	}
	LineDistanceShader(bool onScreen = true,
		const std::shared_ptr<AlloyContext>& contex =
		AlloyDefaultContext());
	void draw(const std::initializer_list<Mesh*>& meshes,
		CameraParameters& camera, const box2px& bounds);
	void draw(Mesh& mesh, CameraParameters& camera,
		const box2px& bounds) {
		draw({ &mesh }, camera, bounds);
	}
	void draw(
		const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, const box2px& bounds);
	void draw(const std::list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, const box2px& bounds);

	void draw(const std::initializer_list<Mesh*>& meshes,
		CameraParameters& camera, const GLFrameBuffer& frameBuffer);
	void draw(Mesh& mesh, CameraParameters& camera,
		const GLFrameBuffer& frameBuffer);
	void draw(
		const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, const GLFrameBuffer& frameBuffer);
	void draw(const std::list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, const GLFrameBuffer& frameBuffer);
};
class AmbientOcclusionShader: public GLShader {
private:
	float sampleRadius;
	std::vector<float3> sampleNormals;
public:
	void setSampleRadius(float r){
		sampleRadius=r;
	}
	AmbientOcclusionShader(bool onScreen = true,
			const std::shared_ptr<AlloyContext>& contex =
					AlloyDefaultContext());
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& imageTexture, const box2px& bounds,
			const box2px viewport, CameraParameters& camera) {

		begin().set("depthBufferSize", imageTexture.dimensions()).set(
				"textureImage", imageTexture, 0).set("MIN_DEPTH",
				camera.getNearPlane()).set("MAX_DEPTH", camera.getFarPlane()).set(
				"focalLength", camera.getFocalLength()).set("bounds", bounds).set(
				"viewport", viewport).set("u_radius", sampleRadius).set(
				"u_kernel", sampleNormals).draw(imageTexture).end();
	}
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& imageTexture, const float2& location,
			const float2& dimensions, const box2px viewport,
			CameraParameters& camera) {
		draw(imageTexture, box2px(location, dimensions), viewport, camera);
	}
	template<class T, int C, ImageType I> void draw(
		const GLTexture<T, C, I>& imageTexture, CameraParameters& camera,const aly::GLFrameBuffer& frameBuffer) {
		frameBuffer.begin();
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		draw(imageTexture, frameBuffer.getViewport(), frameBuffer.getViewport(), camera);
		glEnable(GL_BLEND);
		frameBuffer.end();
	}
};
class DistanceFieldShader: public GLShader {
private:
	int kernelSize;
	aly::Color innerGlowColor;
	aly::Color outerGlowColor;
	aly::Color edgeColor;
public:
	inline void setInnerGlowColor(const Color& c) {
		innerGlowColor = c;
	}
	inline void setOuterGlowColor(const Color& c) {
		outerGlowColor = c;
	}
	inline void setEdgeColor(const Color& c) {
		edgeColor = c;
	}
	inline void setExtent(int distance) {
		kernelSize = distance;
	}
		
	template<class T,int C, ImageType I> void draw(
			const GLTexture<T, C, I>& imageTexture, const box2px& bounds,
			const box2px& viewport) {

		begin().set("KERNEL_SIZE", kernelSize).set("innerColor", innerGlowColor).set(
				"outerColor", outerGlowColor).set("edgeColor", edgeColor).set(
				"textureImage", imageTexture, 0).set("bounds", bounds).set(
				"imageSize", imageTexture.bounds.dimensions).set("viewport",
				viewport).draw(imageTexture).end();
	}
	template<class T, int C, ImageType I> void draw(
		const GLTexture<T, C, I>& imageTexture, const GLFrameBuffer& frameBuffer) {
		frameBuffer.begin();
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		draw(imageTexture, frameBuffer.getViewport(),frameBuffer.getViewport());
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		frameBuffer.end();
	}
	template<class T, int C, ImageType I> void draw(
			const GLTexture<T, C, I>& imageTexture, const float2& location,
			const float2& dimensions, const box2px& viewport) {
		draw(imageTexture, box2px(location, dimensions), viewport);
	}
	DistanceFieldShader(bool onScreen=true,const std::shared_ptr<AlloyContext>& contex =
			AlloyDefaultContext());

	
};

class OutlineShader : public GLShader {
private:
	int kernelSize;
	aly::Color innerGlowColor;
	aly::Color outerGlowColor;
	aly::Color edgeColor;
	float lineWidth;
public:
	inline void setInnerGlowColor(const Color& c) {
		innerGlowColor = c;
	}
	inline void setOuterGlowColor(const Color& c) {
		outerGlowColor = c;
	}
	inline void setEdgeColor(const Color& c) {
		edgeColor = c;
	}
	inline void setLineWidth(float w) {
		lineWidth = w;
		kernelSize =(int)std::ceil(w)+1;
	}
	OutlineShader(bool onScreen = true, const std::shared_ptr<AlloyContext>& contex =
		AlloyDefaultContext());
	template<class T, int C, ImageType I> void draw(
		const GLTexture<T, C, I>& imageTexture, const box2px& bounds,
		const box2px& viewport) {
		begin().set("LINE_WIDTH",lineWidth).set("KERNEL_SIZE", kernelSize).set("innerColor", innerGlowColor).set(
			"outerColor", outerGlowColor).set("edgeColor", edgeColor).set(
				"textureImage", imageTexture, 0).set("bounds", bounds).set(
					"imageSize", imageTexture.bounds.dimensions).set("viewport",
						viewport).draw(imageTexture).end();
	}
	template<class T, int C, ImageType I> void draw(
		const GLTexture<T, C, I>& imageTexture, const GLFrameBuffer& frameBuffer) {

		frameBuffer.begin();
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		draw(imageTexture, frameBuffer.getViewport(), frameBuffer.getViewport());
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		frameBuffer.end();
	}
	template<class T, int C, ImageType I> void draw(
		const GLTexture<T, C, I>& imageTexture, const float2& location,
		const float2& dimensions, const box2px& viewport) {
		draw(imageTexture, box2px(location, dimensions), viewport);
	}
};
}

#endif /* COMMONSHADERS_H_ */
