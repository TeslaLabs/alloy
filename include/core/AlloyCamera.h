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

#ifndef ALLOYCAMERA_H_
#define ALLOYCAMERA_H_
#include <cereal/cereal.hpp>
#include "AlloyMath.h"
#include "AlloyContext.h"
#include <fstream>
namespace aly {
class Mesh;
class Region;
enum class CameraType {
	Perspective, Orthographic
};
template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const CameraType& type) {
	switch (type) {
	case CameraType::Perspective:
		return ss << "Perspective";
	case CameraType::Orthographic:
		return ss << "Orthographic";
	}
	return ss;
}
struct CameraParameters {
	bool changed;
	float nearPlane, farPlane;
	aly::int2 dimensions;
	float4x4 Projection, View, Model;
	float4x4 ViewModel, NormalViewModel, NormalView;
	float4x4 ViewInverse, ViewModelInverse;
	CameraParameters();
	template<class Archive>
	void save(Archive & archive) const {
		archive(CEREAL_NVP(Projection), CEREAL_NVP(View), CEREAL_NVP(Model), CEREAL_NVP(nearPlane), CEREAL_NVP(farPlane), CEREAL_NVP(dimensions));
	}

	template<class Archive>
	void load(Archive & archive) {
		archive(CEREAL_NVP(Projection), CEREAL_NVP(View), CEREAL_NVP(Model), CEREAL_NVP(nearPlane), CEREAL_NVP(farPlane), CEREAL_NVP(dimensions));
		ViewModel = View * Model;
		ViewModelInverse = inverse(ViewModel);
		NormalViewModel = transpose(ViewModelInverse);
		ViewInverse = inverse(View);
		NormalView = transpose(ViewInverse);
	}
	float3 transformWorldToScreen(const float3& pt) const;

	float3 transformWorldToNormalizedImage(const float3& pt) const;
	float3 transformWorldToImage(const float3& pt, int w, int h, bool flip = true) const;
	float2x3 differentiateWorldToImage(const float3& pt, int w, int h, bool flip = true) const;
	float2x3 differentiateCameraToImage(const float3& pt, int w, int h, bool flip = true) const;
	float2x3 differentiateWorldToScreen(const float3& pt) const;
	float3 transformWorldToNormalizedDepth(const float3& pt, int w, int h, bool flip = true) const;
	float3 transformNormalizedImageToWorld(const float3& pt, bool flip = true) const;
	float3 transformImageToWorld(const float3& pt, int w, int h, bool flip = true) const;
	float3 transformScreenToWorld(const float3& pt) const;
	float3 transformScreenDepthToWorld(const float3& pt) const;
	float3 transformNormalizedImageDepthToWorld(const float3& pt, bool flip = true) const;
	float3 transformImageDepthToWorld(const float3& pt, int w, int h, bool flip = true) const;

	virtual void aim(const aly::box2px& bounds = box2px(pixel2(0.0f, 0.0f), pixel2(1.0f, 1.0f)));
	virtual ~CameraParameters() {
	}
	inline float2 getFocalLength() const {
		return float2(Projection(0, 0), Projection(1, 1));
	}
	virtual void setNearFarPlanes(float n, float f);
	float getNearPlane() const {
		return nearPlane;
	}
	float getFarPlane() const {
		return farPlane;
	}
	float2 getZRange() const {
		return float2(nearPlane, farPlane);
	}
	virtual void setDirty(bool d) {
		changed = true;
	}
	virtual float getScale() const;
};
class Camera: public CameraParameters, public EventHandler {
protected:
	// Camera parameters
	float4x4 Rw, Rm;
	float3 cameraTrans;
	float mouseXPos;
	float mouseYPos;
	float fov;
	float3 lookAtPoint, eye;
	float tumblingSpeed, zoomSpeed, strafeSpeed;
	float distanceToObject;
	bool mouseDown, startTumbling, zoomMode, needsDisplay;
	CameraType cameraType;
	Region* activeRegion;
	bool includeParentRegion;
	void handleKeyEvent(GLFWwindow* win, int key, int action);
	void handleButtonEvent(int button, int action);
	void handleCursorEvent(float x, float y);
	void handleScrollEvent(int pos);
public:
	Camera();
	void reset();
	std::string getName() const override {
		return "Camera";
	}
	void setActiveRegion(Region* region, bool includeParent = true) {
		activeRegion = region;
		includeParentRegion = includeParent;
	}
	void aim(const aly::box2px& bounds) override;
	void setPose(const float4x4& m) {
		Model = m;
	}
	void setCameraType(const CameraType& type) {
		cameraType = type;
		changed = true;
	}
	inline void rotateModelX(float angle) {
		Rm = MakeRotationX(angle) * Rm;
		changed = true;
	}
	inline void rotateModelY(float angle) {
		Rm = MakeRotationY(angle) * Rm;
		changed = true;
	}
	inline void rotateModelZ(float angle) {
		Rm = MakeRotationZ(angle) * Rm;
		changed = true;
	}
	inline void rotateWorldX(float angle) {
		Rw = MakeRotationX(angle) * Rw;
		changed = true;
	}
	inline void rotateWorldY(float angle) {
		Rw = MakeRotationY(angle) * Rw;
		changed = true;
	}
	inline void rotateWorldZ(float angle) {
		Rw = MakeRotationZ(angle) * Rw;
		changed = true;
	}
	inline void setModelRotation(const float4x4& M) {
		Rm = M;
		changed = true;
	}
	inline void setWorldRotation(const float4x4& M) {
		Rw = M;
		changed = true;
	}
	inline void setTranslation(const float3& t) {
		cameraTrans = t;
	}
	float4x4 getWorldRotation() const {
		return Rw;
	}
	float4x4 getModelRotation() const {
		return Rm;
	}
	float3 getTranslation() const {
		return cameraTrans;
	}
	virtual float getScale() const override {
		return distanceToObject;
	}
	float4x4 getPose() const {
		return Model;
	}
	bool isDirty() const {
		return needsDisplay || changed;
	}
	virtual void setDirty(bool d) override {
		needsDisplay = d;
		if (d)
			changed = true;
	}

	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event) override;

	void setFieldOfView(float degrees) {
		fov = degrees;
		changed = true;
	}
	void setSpeed(float zoomSpeed, float strafeSpeed, float tumblingSpeed);
	void lookAt(const float3& p, float dist);
	void lookAt(const float3& p) {
		lookAtPoint = p;
		changed = true;
	}

	float getNormalizedDepth(const float4& pt) const {
		float4 out = ViewModel * pt;
		return (-out.z - nearPlane) / (farPlane - nearPlane);
	}
	float2 computeNormalizedDepthRange(const Mesh& mesh);

	void resetTranslation() {
		cameraTrans = float3(0, 0, 0);
		lookAtPoint = float3(0, 0, 0);
		changed = true;
	}

	void setZoom(float z) {
		distanceToObject = z;
		changed = true;
	}
	static const float sDeg2rad;
};
void WriteCameraParametersToFile(const std::string& file, const CameraParameters& params);
void ReadCameraParametersFromFile(const std::string& file, CameraParameters& params);

// class Camera
}
#endif /* ALLOYCAMERA_H_ */
