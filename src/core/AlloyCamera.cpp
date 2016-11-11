#include <AlloyCamera.h>
#include <cereal/archives/xml.hpp>
#include <cereal/archives/portable_binary.hpp>
#include "GLShader.h"
#include "AlloyMesh.h"
#include "AlloyUI.h"
#include <string>
#include <list>
#include <fstream>

namespace aly {
	const float Camera::sDeg2rad = ALY_PI / 180.0f;

	float3 CameraParameters::transformWorldToScreen(const float3& pt) const {
		float4 ptp(pt[0], pt[1], pt[2], 1.0f);
		float4 p = Projection * ViewModel * ptp;
		return float3(p[0] / p[3], p[1] / p[3], p[2] / p[3]);
	}

	float3 CameraParameters::transformWorldToNormalizedImage(const float3& pt) const {
		float4 ptp(pt[0], pt[1], pt[2], 1.0f);
		float4 p = Projection * ViewModel * ptp;
		return float3(0.5f * (p[0] / p[3] + 1.0f), 0.5f * (1.0f - p[1] / p[3]), p[2] / p[3]);
	}
	float3 CameraParameters::transformWorldToImage(const float3& pt, int w, int h, bool flip) const {
		float4 ptp(pt[0], pt[1], pt[2], 1.0f);
		float4 p = Projection * ViewModel * ptp;
		return float3(w * 0.5f * (p[0] / p[3] + 1.0f), (flip) ? (h * 0.5f * (1.0f - p[1] / p[3])) : h * 0.5f * (p[1] / p[3] + 1.0f), p[2] / p[3]);
	}
	float2x3 CameraParameters::differentiateWorldToImage(const float3& pt, int w, int h, bool flip) const {
		float4 p(pt[0], pt[1], pt[2], 1.0f);
		float4x4 Scale = float4x4::identity();
		Scale(0, 0) = w * 0.5f;
		Scale(0, 3) = w * 0.5f;
		Scale(1, 1) = (flip) ? -h * 0.5f : h * 0.5f;
		Scale(1, 3) = h * 0.5f;
		float4x4 M = Scale * Projection * ViewModel;
		float2x3 J;
		float4 Mx = M.row(0);
		float4 My = M.row(1);
		float4 Mw = M.row(3);
		float dw = dot(Mw, p);
		float dx = dot(Mx, p);
		float dy = dot(My, p);
		float denom = dw * dw;

		J(0, 0) = (Mx.x * dw - Mw.x * dx) / denom;
		J(0, 1) = (Mx.y * dw - Mw.y * dx) / denom;
		J(0, 2) = (Mx.z * dw - Mw.z * dx) / denom;

		J(1, 0) = (My.x * dw - Mw.x * dy) / denom;
		J(1, 1) = (My.y * dw - Mw.y * dy) / denom;
		J(1, 2) = (My.z * dw - Mw.z * dy) / denom;

		return J;
	}
	float2x3 CameraParameters::differentiateCameraToImage(const float3& pt, int w, int h, bool flip) const {
		float C = Projection(3, 2);
		float fx = Projection(0, 0);
		float fy = Projection(1, 1);
		float2x3 J = float2x3::zero();
		J(0, 0) = 0.5f * w * fx / (C * pt.z);
		J(1, 1) = (flip) ? -0.5f * h * fy / (C * pt.z) : 0.5f * h * fy / (C * pt.z);
		J(0, 2) = -0.5f * w * fx * pt.x / (C * pt.z * pt.z);
		J(1, 2) = -(flip) ? -0.5f * h * fy * pt.y / (C * pt.z * pt.z) : 0.5f * h * fy * pt.y / (C * pt.z * pt.z);
		return J;
	}
	float2x3 CameraParameters::differentiateWorldToScreen(const float3& pt) const {
		float4 p(pt[0], pt[1], pt[2], 1.0f);
		float4x4 M = Projection * ViewModel;
		float2x3 J;
		float4 Mx = M.row(0);
		float4 My = M.row(1);
		float4 Mw = M.row(3);
		float dw = dot(Mw, p);
		float dx = dot(Mx, p);
		float dy = dot(My, p);
		float denom = dw * dw;

		J(0, 0) = (Mx.x * dw - Mw.x * dx) / denom;
		J(0, 1) = (Mx.y * dw - Mw.y * dx) / denom;
		J(0, 2) = (Mx.z * dw - Mw.z * dx) / denom;

		J(1, 0) = (My.x * dw - Mw.x * dy) / denom;
		J(1, 1) = (My.y * dw - Mw.y * dy) / denom;
		J(1, 2) = (My.z * dw - Mw.z * dy) / denom;
		return J;
	}
	float3 CameraParameters::transformWorldToNormalizedDepth(const float3& pt, int w, int h, bool flip) const {
		float4 ptp(pt[0], pt[1], pt[2], 1.0f);
		float4 p = Projection * ViewModel * ptp;
		return float3(w * 0.5f * (p[0] / p[3] + 1.0f), (flip) ? h * 0.5f * (1.0f - p[1] / p[3]) : h * 0.5f * (p[1] / p[3] + 1.0f),
				(-p[2] / p[3] - nearPlane) / (farPlane - nearPlane));
	}
	float3 CameraParameters::transformNormalizedImageToWorld(const float3& pt, bool flip) const {
		float4 ptp;
		float A = Projection(2, 2);
		float B = Projection(2, 3);
		float C = Projection(3, 2);
		float fx = Projection(0, 0);
		float fy = Projection(1, 1);
		ptp.z = B / (C * pt.z - A);
		ptp.x = (2.0f * pt.x - 1.0f) * ptp.z * C / fx;
		ptp.y = (flip) ? (1.0f - 2.0f * pt.y) * ptp.z * C / fy : (2.0f * pt.y - 1.0f) * ptp.z * C / fy;
		ptp.w = 1.0f;
		ptp = ViewModelInverse * ptp;
		return ptp.xyz() / ptp.w;
	}
	float3 CameraParameters::transformImageToWorld(const float3& pt, int w, int h, bool flip) const {
		float4 ptp;
		float A = Projection(2, 2);
		float B = Projection(2, 3);
		float C = Projection(3, 2);
		float fx = Projection(0, 0);
		float fy = Projection(1, 1);
		ptp.z = B / (C * pt.z - A);
		ptp.x = (2.0f * pt.x / (float) w - 1.0f) * ptp.z * C / fx;
		ptp.y = (flip) ? (1.0f - 2.0f * pt.y / (float) h) * ptp.z * C / fy : (2.0f * pt.y / (float) h - 1.0f) * ptp.z * C / fy;
		ptp.w = 1.0f;
		ptp = ViewModelInverse * ptp;
		return ptp.xyz() / ptp.w;
	}
	float3 CameraParameters::transformScreenToWorld(const float3& pt) const {
		float4 ptp;
		float A = Projection(2, 2);
		float B = Projection(2, 3);
		float C = Projection(3, 2);
		float fx = Projection(0, 0);
		float fy = Projection(1, 1);
		ptp.z = B / (C * pt.z - A);
		ptp.x = pt.x * ptp.z * C / fx;
		ptp.y = pt.y * ptp.z * C / fy;
		ptp.w = 1.0f;
		ptp = ViewModelInverse * ptp;
		return ptp.xyz() / ptp.w;
	}
	float3 CameraParameters::transformScreenDepthToWorld(const float3& pt) const {
		float4 ptp;
		float C = Projection(3, 2);
		float fx = Projection(0, 0);
		float fy = Projection(1, 1);
		ptp.z = -(pt.z * (farPlane - nearPlane) + nearPlane);
		ptp.x = pt.x * ptp.z * C / fx;
		ptp.y = pt.y * ptp.z * C / fy;
		ptp.w = 1.0f;
		ptp = ViewModelInverse * ptp;
		return ptp.xyz() / ptp.w;
	}
	float3 CameraParameters::transformNormalizedImageDepthToWorld(const float3& pt, bool flip) const {
		float4 ptp;
		float C = Projection(3, 2);
		float fx = Projection(0, 0);
		float fy = Projection(1, 1);
		ptp.z = -(pt.z * (farPlane - nearPlane) + nearPlane);
		ptp.x = (2.0f * pt.x - 1.0f) * ptp.z * C / fx;
		ptp.y = (flip) ? (1.0f - 2.0f * pt.y) * ptp.z * C / fy : (2.0f * pt.y - 1.0f) * ptp.z * C / fy;
		ptp.w = 1.0f;
		ptp = ViewModelInverse * ptp;
		return ptp.xyz() / ptp.w;
	}
	float3 CameraParameters::transformImageDepthToWorld(const float3& pt, int w, int h, bool flip) const {
		float4 ptp;
		float C = Projection(3, 2);
		float fx = Projection(0, 0);
		float fy = Projection(1, 1);
		ptp.z = -(pt.z * (farPlane - nearPlane) + nearPlane);
		ptp.x = (2.0f * pt.x / (float) w - 1.0f) * ptp.z * C / fx;
		ptp.y = (flip) ? ((1.0f - 2.0f * pt.y / (float) h) * ptp.z * C / fy) : ((2.0f * pt.y / (float) h - 1.0f) * ptp.z * C / fy);
		ptp.w = 1.0f;
		ptp = ViewModelInverse * ptp;
		return ptp.xyz() / ptp.w;
	}

	void WriteCameraParametersToFile(const std::string& file, const CameraParameters& params) {
		std::string ext = GetFileExtension(file);
		if (ext == "json") {
			std::ofstream os(file);
			cereal::JSONOutputArchive archive(os);
			archive(cereal::make_nvp("camera", params));
		} else if (ext == "xml") {
			std::ofstream os(file);
			cereal::XMLOutputArchive archive(os);
			archive(cereal::make_nvp("camera", params));
		} else {
			std::ofstream os(file, std::ios::binary);
			cereal::PortableBinaryOutputArchive archive(os);
			archive(cereal::make_nvp("camera", params));
		}
	}
	void ReadCameraParametersFromFile(const std::string& file, CameraParameters& params) {
		std::string ext = GetFileExtension(file);
		if (ext == "json") {
			std::ifstream os(file);
			cereal::JSONInputArchive archive(os);
			archive(cereal::make_nvp("camera", params));
		} else if (ext == "xml") {
			std::ifstream os(file);
			cereal::XMLInputArchive archive(os);
			archive(cereal::make_nvp("camera", params));
		} else {
			std::ifstream os(file, std::ios::binary);
			cereal::PortableBinaryInputArchive archive(os);
			archive(cereal::make_nvp("camera", params));
		}
	}
	CameraParameters::CameraParameters() :
			changed(true), nearPlane(0.1f), farPlane(10000.0f), Projection(float4x4::identity()), View(float4x4::identity()), Model(float4x4::identity()), ViewModel(
					float4x4::identity()), NormalViewModel(float4x4::identity()), NormalView(float4x4::identity()), ViewInverse(float4x4::identity()), ViewModelInverse(
					float4x4::identity()) {
	}
	Camera::Camera() :
			CameraParameters(), Rw(float4x4::identity()), Rm(float4x4::identity()), cameraTrans(0, 0, 0), mouseXPos(0), mouseYPos(0), fov(60.0f), eye(
					float3(0.0f, 0.0f, -1.0f)), tumblingSpeed(0.5f), zoomSpeed(0.2f), strafeSpeed(0.001f), distanceToObject(1.0), mouseDown(false), startTumbling(
					false), zoomMode(false), needsDisplay(true), cameraType(CameraType::Perspective), activeRegion(nullptr), includeParentRegion(true) {
	}
	float CameraParameters::getScale() const {
		float3x3 U, D, Vt;
		float3x3 A = SubMatrix(View);
		SVD(A, U, D, Vt);
		return std::abs(D(0, 0));
	}
	void CameraParameters::setNearFarPlanes(float zNear, float zFar) {
		nearPlane = zNear;
		farPlane = zFar;
		float sz = -(zFar + zNear) / (zFar - zNear);
		float pz = -(2.0f * zFar * zNear) / (zFar - zNear);
		if (Projection(3, 2) != 0.0f) {
			Projection(2, 2) = sz;
			Projection(2, 3) = pz;
		} else {
			float pz = -(zFar + zNear) / (zFar - zNear);
			float sz = -(2.0f) / (zFar - zNear);
			Projection(2, 2) = sz;
			Projection(2, 3) = pz;
		}
		changed = true;
	}
	void Camera::lookAt(const float3& p, float dist) {
		lookAtPoint = p;
		distanceToObject = dist;
		changed = true;
		needsDisplay = true;
	}

	void Camera::setSpeed(float zoomSpeed, float strafeSpeed, float tumblingSpeed) {
		zoomSpeed = std::max(0.0001f, zoomSpeed);
		strafeSpeed = std::max(0.0001f, strafeSpeed);
		tumblingSpeed = std::max(0.01f, tumblingSpeed);
	}

	void Camera::aim(const box2px& bounds) {
		float aspectRatio = bounds.dimensions.x / (float) bounds.dimensions.y;
		if (changed) {
			changed = false;
			float4x4 Tinv = float4x4::identity();
			float4x4 Teye = float4x4::identity();
			float4x4 S = float4x4::identity();

			Tinv(0, 3) = -lookAtPoint[0];
			Tinv(1, 3) = -lookAtPoint[1];
			Tinv(2, 3) = -lookAtPoint[2];

			float4x4 T = float4x4::identity();
			T(0, 3) = lookAtPoint[0];
			T(1, 3) = lookAtPoint[1];
			T(2, 3) = lookAtPoint[2];

			Teye(0, 3) = eye[0];
			Teye(1, 3) = eye[1];
			Teye(2, 3) = eye[2];

			S(0, 0) = distanceToObject;
			S(1, 1) = distanceToObject;
			S(2, 2) = distanceToObject;

			float4x4 Tcamera = float4x4::identity();
			Tcamera(0, 3) = cameraTrans[0];
			Tcamera(1, 3) = cameraTrans[1];
			Tcamera(2, 3) = cameraTrans[2];

			if (cameraType == CameraType::Perspective) {
				Projection = Tcamera * MakePerspectiveMatrix(fov, aspectRatio, nearPlane, farPlane);
			} else if (cameraType == CameraType::Orthographic) {
				Projection = Tcamera * MakeOrthographicMatrix(1.0f / aspectRatio, 1.0f, nearPlane, farPlane);
			}
			View = Teye * S * Rw * T * Rm;
			ViewModel = View * Model;
			ViewModelInverse = inverse(ViewModel);
			NormalViewModel = transpose(ViewModelInverse);
			ViewInverse = inverse(View);
			NormalView = transpose(ViewInverse);
			needsDisplay = true;
		}
	}
	void CameraParameters::aim(const box2px& bounds) {
		if (changed) {
			changed = false;
			ViewModel = View * Model;
			ViewModelInverse = inverse(ViewModel);
			NormalViewModel = transpose(ViewModelInverse);
			ViewInverse = inverse(View);
			NormalView = transpose(ViewInverse);
		}
	}
	void Camera::reset() {
		Rm = float4x4::identity();
		Rw = float4x4::identity();
		distanceToObject = 1.0;
		lookAtPoint = float3(0, 0, 0);
		cameraTrans = float3(0, 0, 0);
	}
	float2 Camera::computeNormalizedDepthRange(const Mesh& mesh) {
		box3f bbox = mesh.getBoundingBox();
		float4 center = bbox.center().xyzw();
		float4 origin = inverse(ViewModel) * float4(0, 0, 0, 1);
		float3 ray = normalize(center.xyz() - origin.xyz() / origin.w);
		float zMin = getNormalizedDepth(center - 0.5f * bbox.dimensions.z * float4(ray, 0));
		float zMax = getNormalizedDepth(center + 0.5f * bbox.dimensions.z * float4(ray, 0));
		return float2(zMin, zMax);
	}
	void Camera::handleKeyEvent(GLFWwindow* win, int key, int action) {
		if ((char) key == 'A') {
			Rm = MakeRotationY((float) (2 * sDeg2rad)) * Rm;
			changed = true;
		} else if ((char) key == 'D') {
			Rm = MakeRotationY((float) (-2 * sDeg2rad)) * Rm;
			changed = true;
		} else if ((char) key == 'S') {
			Rm = MakeRotationX((float) (2 * sDeg2rad)) * Rm;
			changed = true;
		} else if ((char) key == 'W') {
			Rm = MakeRotationX((float) (-2 * sDeg2rad)) * Rm;
			changed = true;
		} else if ((char) key == 'R') {
			reset();
			changed = true;
		} else if (key == GLFW_KEY_UP) {
			cameraTrans[1] += 0.025f;
			changed = true;
		} else if (key == GLFW_KEY_DOWN) {
			cameraTrans[1] -= 0.025f;
			changed = true;
		} else if (key == GLFW_KEY_LEFT) {
			cameraTrans[0] -= 0.025f;
			changed = true;
		} else if (key == GLFW_KEY_RIGHT) {
			cameraTrans[0] += 0.025f;
			changed = true;
		} else if (key == GLFW_KEY_PAGE_UP) {
			distanceToObject = std::max(1E-3f, (1 + zoomSpeed) * distanceToObject);
			changed = true;
		} else if (key == GLFW_KEY_PAGE_DOWN) {
			distanceToObject = std::max(1E-3f, (1 - zoomSpeed) * distanceToObject);
			changed = true;
		} else {
			if (glfwGetKey(win, key) == GLFW_PRESS) {
				switch (key) {
				case GLFW_KEY_SPACE:
					zoomMode = true;
					break;
				}
			} else if (glfwGetKey(win, key) == GLFW_RELEASE) {
				switch (key) {
				case GLFW_KEY_SPACE:
					zoomMode = false;
					break;
				}
			}
		}
		changed = true;
	}

	void Camera::handleButtonEvent(int button, int action) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS)
				mouseDown = true;
			else if (action == GLFW_RELEASE)
				mouseDown = false;
		} else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS) {
				mouseDown = true;
				zoomMode = true;
			} else if (action == GLFW_RELEASE) {
				mouseDown = false;
				zoomMode = false;
			}
		}
		if (action == GLFW_RELEASE)
			mouseDown = false;
		startTumbling = true;
		changed = true;
	}

	bool Camera::onEventHandler(AlloyContext* context, const InputEvent& event) {
		if (context->getGlassPane()->isVisible())
			return false;
		if (!event.isAltDown() && !event.isShiftDown() && !event.isControlDown()) {
			switch (event.type) {
			case InputType::Cursor:
				//if(event.action == GLFW_PRESS&&activeRegion!=nullptr&&!context->isMouseOver(activeRegion,includeParentRegion))return false;
				handleCursorEvent(event.cursor.x, event.cursor.y);
				return true;
			case InputType::MouseButton:
				if (event.action == GLFW_PRESS && activeRegion != nullptr && !context->isMouseOver(activeRegion, includeParentRegion))
					return false;
				handleButtonEvent(event.button, event.action);
				return true;
			case InputType::Scroll:
				if (activeRegion != nullptr && !context->isMouseOver(activeRegion, includeParentRegion))
					return false;
				handleScrollEvent((int) event.scroll.y);
				return true;
			case InputType::Key:
				if (activeRegion != nullptr && !context->isMouseOver(activeRegion, includeParentRegion))
					return false;
				handleKeyEvent(context->window, event.key, event.action);
				return false;
			default:
				return false;
			}
		} else {
			return false;
		}
	}
	void Camera::handleCursorEvent(float x, float y) {
		if (startTumbling) {
			mouseXPos = x;
			mouseYPos = y;
			startTumbling = false;
		}
		float dx, dy;
		dx = x - mouseXPos;
		dy = y - mouseYPos;
		if (mouseDown && !zoomMode) {
			Rw = MakeRotationY((float) (dx * tumblingSpeed * sDeg2rad)) * MakeRotationX((float) (dy * tumblingSpeed * sDeg2rad)) * Rw;
			changed = true;
		} else if (mouseDown && zoomMode) {
			float3 mUp = Rw.row(1).xyz();
			float3 mRight = Rw.row(0).xyz();
			lookAtPoint += (mRight * dx - dy * mUp) * (float) (strafeSpeed);
			changed = true;
		}
		mouseXPos = x;
		mouseYPos = y;
	}
	void Camera::handleScrollEvent(int pos) {
		distanceToObject = std::max(1E-3f, (1 - pos * zoomSpeed) * distanceToObject);
		changed = true;
	}
}
