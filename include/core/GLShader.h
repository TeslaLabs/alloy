/*
 * Copyright(C) 2014, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
#ifndef GLSHADER_H_
#define GLSHADER_H_
#include <AlloyCamera.h>
#include "AlloyContext.h"
#include "GLTexture.h"
#include "AlloyMesh.h"
#include <vector>
#include <string>
namespace aly {
class GLShader {
protected:
	GLuint mVertexShaderHandle;
	GLuint mFragmentShaderHandle;
	GLuint mGeometryShaderHandle;
	GLuint mProgramHandle;
	bool shaderEnabled = false;
	std::shared_ptr<AlloyContext> context;
	bool onScreen=true;
	void enableCheck() {
		if (!shaderEnabled)
			throw std::runtime_error(
					"Could not set shader parameter because shader is not enabled. Call shader.begin() first.");
	}
public:
	bool isOnScreenRender() const {
		return onScreen;
	}
	std::shared_ptr<AlloyContext>& getContext() {
		return context;
	}
	// Default constructor.
	GLShader(bool onScreen = true,const std::shared_ptr<AlloyContext>& context =
			AlloyDefaultContext());
	virtual ~GLShader();
	// Initialization function to compile the shader.
	void initialize(const std::initializer_list<std::string>& attributes,
			const std::string& pVertexShaderString,
			const std::string& pFragmentShaderString,
			const std::string& pGeomShaderString);
	void initialize(const std::initializer_list<std::string>& attributes,
			const std::string& pVertexShaderString,
			const std::string& pFragmentShaderString) {
		initialize(attributes, pVertexShaderString, pFragmentShaderString, "");
	}
	void initialize(const std::string& pVertexShaderString,
			const std::string& pFragmentShaderString) {
		initialize( { }, pVertexShaderString, pFragmentShaderString, "");
	}
	GLShader(bool  onScreen, std::shared_ptr<AlloyContext>& context,
			const std::initializer_list<std::string>& attributes,
			const std::string& pVertexShaderString,
			const std::string& pFragmentShaderString,
			const std::string& pGeomShaderString) :
			GLShader(onScreen, context) {
		initialize(attributes, pVertexShaderString, pFragmentShaderString,
				pGeomShaderString);
	}
	inline GLint getUniformLocation(const std::string& variable) const {
		GLint index = glGetUniformLocation(mProgramHandle, variable.c_str());
		return index;
	}
	inline GLShader& set(const std::string& variable, float value) {
		enableCheck();
		glUniform1f(getUniformLocation(variable), value);
		return *this;
	}
	inline GLShader& set(const std::string& variable, int value) {
		enableCheck();
		glUniform1i(getUniformLocation(variable), value);
		return *this;
	}
	inline GLShader& set(const std::string& variable, uint32_t value) {
		enableCheck();
		glUniform1ui(getUniformLocation(variable), value);
		return *this;
	}
	inline GLShader& set(const std::string& variable, float1 value) {
		enableCheck();
		glUniform1f(getUniformLocation(variable), value.x);
		return *this;
	}
	inline GLShader& set(const std::string& variable, int1 value) {
		enableCheck();
		glUniform1i(getUniformLocation(variable), value.x);
		return *this;
	}
	inline GLShader& set(const std::string& variable, uint1 value) {
		enableCheck();
		glUniform1ui(getUniformLocation(variable), value.x);
		return *this;
	}
	inline GLShader& set(const std::string& variable, float2 value) {
		enableCheck();
		glUniform2f(getUniformLocation(variable), value.x, value.y);
		return *this;
	}
	inline GLShader& set(const std::string& variable, int2 value) {
		enableCheck();
		glUniform2i(getUniformLocation(variable), value.x, value.y);
		return *this;
	}
	inline GLShader& set(const std::string& variable, uint2 value) {
		enableCheck();
		glUniform2ui(getUniformLocation(variable), value.x, value.y);
		return *this;
	}

	inline GLShader& set(const std::string& variable, float3 value) {
		enableCheck();
		glUniform3f(getUniformLocation(variable), value.x, value.y, value.z);
		return *this;
	}
	inline GLShader& set(const std::string& variable,
			const std::vector<float4>& value) {
		enableCheck();
		glUniform4fv(getUniformLocation(variable),
				static_cast<GLsizei>(value.size()),
				(const float*) value.data());
		return *this;
	}
	inline GLShader& set(const std::string& variable,
			const std::vector<float3>& value) {
		enableCheck();
		glUniform3fv(getUniformLocation(variable),
				static_cast<GLsizei>(value.size()),
				(const float*) value.data());
		return *this;
	}
	inline GLShader& set(const std::string& variable,
			const std::vector<float2>& value) {
		enableCheck();
		glUniform2fv(getUniformLocation(variable),
				static_cast<GLsizei>(value.size()),
				(const float*) value.data());
		return *this;
	}
	inline GLShader& set(const std::string& variable,
			const std::vector<float1>& value) {
		enableCheck();
		glUniform1fv(getUniformLocation(variable),
				static_cast<GLsizei>(value.size()),
				(const float*) value.data());
		return *this;
	}
	inline GLShader& set(const std::string& variable,
			const std::vector<float>& value) {
		enableCheck();
		glUniform1fv(getUniformLocation(variable),
				static_cast<GLsizei>(value.size()), value.data());
		return *this;
	}
	inline GLShader& set(const std::string& variable,
			const std::vector<int4>& value) {
		enableCheck();
		glUniform4iv(getUniformLocation(variable),
				static_cast<GLsizei>(value.size()), (const int*) value.data());
		return *this;
	}
	inline GLShader& set(const std::string& variable,
			const std::vector<int3>& value) {
		enableCheck();
		glUniform3iv(getUniformLocation(variable),
				static_cast<GLsizei>(value.size()), (const int*) value.data());
		return *this;
	}
	inline GLShader& set(const std::string& variable,
			const std::vector<int2>& value) {
		enableCheck();
		glUniform2iv(getUniformLocation(variable),
				static_cast<GLsizei>(value.size()), (const int*) value.data());
		return *this;
	}
	inline GLShader& set(const std::string& variable,
			const std::vector<int1>& value) {
		enableCheck();
		glUniform1iv(getUniformLocation(variable),
				static_cast<GLsizei>(value.size()), (const int*) value.data());
		return *this;
	}
	inline GLShader& set(const std::string& variable,
			const std::vector<int>& value) {
		enableCheck();
		glUniform1iv(getUniformLocation(variable),
				static_cast<GLsizei>(value.size()), value.data());
		return *this;
	}
	inline GLShader& set(const std::string& variable,
			const std::vector<uint4>& value) {
		enableCheck();
		glUniform4uiv(getUniformLocation(variable),
				static_cast<GLsizei>(value.size()), (const uint*) value.data());
		return *this;
	}
	inline GLShader& set(const std::string& variable,
			const std::vector<uint3>& value) {
		enableCheck();
		glUniform3uiv(getUniformLocation(variable),
				static_cast<GLsizei>(value.size()), (const uint*) value.data());
		return *this;
	}
	inline GLShader& set(const std::string& variable,
			const std::vector<uint2>& value) {
		enableCheck();
		glUniform2uiv(getUniformLocation(variable),
				static_cast<GLsizei>(value.size()), (const uint*) value.data());
		return *this;
	}
	inline GLShader& set(const std::string& variable,
			const std::vector<uint1>& value) {
		enableCheck();
		glUniform1uiv(getUniformLocation(variable),
				static_cast<GLsizei>(value.size()), (const uint*) value.data());
		return *this;
	}
	inline GLShader& set(const std::string& variable,
			const std::vector<uint>& value) {
		enableCheck();
		glUniform1uiv(getUniformLocation(variable),
				static_cast<GLsizei>(value.size()), value.data());
		return *this;
	}
	inline GLShader& set(const std::string& variable,
			const std::vector<Color>& value) {
		enableCheck();
		glUniform4fv(getUniformLocation(variable),
				static_cast<GLsizei>(value.size()),
				(const float*) value.data());
		return *this;
	}
	inline GLShader& set(const std::string& variable, int3 value) {
		enableCheck();
		glUniform3i(getUniformLocation(variable), value.x, value.y, value.z);
		return *this;
	}
	inline GLShader& set(const std::string& variable, uint3 value) {
		enableCheck();
		glUniform3ui(getUniformLocation(variable), value.x, value.y, value.z);
		return *this;
	}

	inline GLShader& set(const std::string& variable, float4 value) {
		enableCheck();
		glUniform4f(getUniformLocation(variable), value.x, value.y, value.z,
				value.w);
		return *this;
	}
	inline GLShader& set(const std::string& variable, int4 value) {
		enableCheck();
		glUniform4i(getUniformLocation(variable), value.x, value.y, value.z,
				value.w);
		return *this;
	}
	inline GLShader& set(const std::string& variable, uint4 value) {
		enableCheck();
		glUniform4ui(getUniformLocation(variable), value.x, value.y, value.z,
				value.w);
		return *this;
	}

	inline GLShader& set(const std::string& variable, const float4x4& value) {
		enableCheck();
		glUniformMatrix4fv(getUniformLocation(variable), 1, false, value.ptr());
		return *this;
	}
	inline GLShader& set(const std::string& variable, const float3x3& value) {
		enableCheck();
		glUniformMatrix3fv(getUniformLocation(variable), 1, false, value.ptr());
		return *this;
	}
	inline GLShader& set(const std::string& variable, const float2x2& value) {
		enableCheck();
		glUniformMatrix2fv(getUniformLocation(variable), 1, false, value.ptr());
		return *this;
	}

	inline GLShader& set(const std::string& variable, const aly::Color& value) {
		enableCheck();
		glUniform4f(getUniformLocation(variable), value.r, value.g, value.b,
				value.a);
		return *this;
	}
	inline GLShader& set(CameraParameters& camera, const box2px& bounds) {
		enableCheck();
		camera.aim(bounds);
		set("ProjMat", camera.Projection);
		set("ViewMat", camera.View);
		set("ModelMat", camera.Model);
		set("ViewModelMat", camera.ViewModel);
		set("NormalMat", camera.NormalViewModel);
		return *this;
	}
	inline GLShader& set(const std::string& variable, const box2f& value) {
		enableCheck();
		glUniform4f(getUniformLocation(variable), value.position.x,
				value.position.y, value.dimensions.x, value.dimensions.y);
		return *this;
	}
	inline GLShader& set(const std::string& variable, const box2i& value) {
		enableCheck();
		glUniform4i(getUniformLocation(variable), value.position.x,
				value.position.y, value.dimensions.x, value.dimensions.y);
		return *this;
	}
	template<class T, int C, ImageType I> GLShader& set(
			const std::string& variable, const GLTexture<T, C, I>& value,
			int id) {
		enableCheck();
		glUniform1i(getUniformLocation(variable), id);
		glActiveTexture(GL_TEXTURE0 + id);
		value.bind();
		return *this;
	}

	GLShader& begin();
	GLShader& draw(const std::initializer_list<const GLComponent*>& comps);
	GLShader& draw(const std::initializer_list<Mesh*>& meshes,
			const GLMesh::PrimitiveType& type,bool froceVertexColor = false);
	GLShader& draw(const std::list<const GLComponent*>& comps);
	GLShader& draw(const std::vector<const GLComponent*>& comps);
	GLShader& draw(const std::vector<Mesh*>& meshes,
		const GLMesh::PrimitiveType& type, bool froceVertexColor = false);
	GLShader& draw(const std::list<Mesh*>& meshes,
			const GLMesh::PrimitiveType& type, bool froceVertexColor=false );

	GLShader& draw(const GLComponent& comps);
	GLShader& draw(Mesh& meshes, const GLMesh::PrimitiveType& type,bool froceVertexColor = false);

	void end();
	inline GLuint GetProgramHandle() const {
		return mProgramHandle;
	}
};

}
#endif /* SHADER_H_ */
