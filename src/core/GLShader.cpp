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
#include "AlloyContext.h"
#include "AlloyMesh.h"
#include "GLShader.h"
#include <iostream>

namespace aly {

GLShader::GLShader(bool onScreen, const std::shared_ptr<AlloyContext>& context) :
		mVertexShaderHandle(0), mFragmentShaderHandle(0), mGeometryShaderHandle(
				0), mProgramHandle(0), context(context),onScreen(onScreen) {

}
GLShader& GLShader::begin() {
	context->begin(onScreen);
	glUseProgram(GetProgramHandle());
	shaderEnabled = true;
	return *this;
}
void GLShader::end() {
	shaderEnabled = false;
	glUseProgram((GLuint) NULL);
	context->end();
}
GLShader& GLShader::draw(const GLComponent& comp) {
	comp.draw();
	return *this;
}
GLShader& GLShader::draw(Mesh& mesh, const GLMesh::PrimitiveType& type,  bool forceVertexColor) {
	mesh.draw(type,onScreen,forceVertexColor);
	return *this;
}
GLShader& GLShader::draw(const std::list<const GLComponent*>& comps) {
	for (const GLComponent* comp : comps) {
		comp->draw();
	}
	return *this;
}
GLShader& GLShader::draw(const std::vector<const GLComponent*>& comps) {
	for (const GLComponent* comp : comps) {
		comp->draw();
	}
	return *this;
}
GLShader& GLShader::draw(const std::list<Mesh*>& meshes,
		const GLMesh::PrimitiveType& type, bool forceVertexColor) {
	for (Mesh* mesh : meshes) {
		mesh->draw(type,onScreen,forceVertexColor);
	}
	return *this;
}
GLShader& GLShader::draw(const std::vector<Mesh*>& meshes,
	const GLMesh::PrimitiveType& type, bool forceVertexColor) {
	for (Mesh* mesh : meshes) {
		mesh->draw(type, onScreen, forceVertexColor);
	}
	return *this;
}
GLShader& GLShader::draw(
		const std::initializer_list<const GLComponent*>& comps) {
	for (const GLComponent* comp : comps) {
		comp->draw();
	}
	return *this;
}
GLShader& GLShader::draw(const std::initializer_list<Mesh*>& meshes,
		const GLMesh::PrimitiveType& type, bool forceVertexColor) {
	for (Mesh* mesh : meshes) {
		mesh->draw(type,onScreen,forceVertexColor);
	}
	return *this;
}
void GLShader::initialize(
		const std::initializer_list<std::string>& pAttributeLocations,
		const std::string& pVertexShaderString,
		const std::string& pFragmentShaderString,
		const std::string& pGeometryShaderString) {
	if (pVertexShaderString.size() == 0 || pFragmentShaderString.size() == 0)
		return throw std::runtime_error("No shader program specified.");
	context->begin(onScreen);
	GLint lStatus;
	char message[4096] = "";
	mVertexShaderHandle = glCreateShader( GL_VERTEX_SHADER);
	const char* code = pVertexShaderString.c_str();
	glShaderSource(mVertexShaderHandle, 1, &code, 0);
	glCompileShader(mVertexShaderHandle);
	glGetShaderiv(mVertexShaderHandle, GL_COMPILE_STATUS, &lStatus);
	if (lStatus != GL_TRUE) {
		glGetInfoLogARB(mVertexShaderHandle, sizeof(message), NULL, message);
		context->end();
		std::cout << "--- Vertex Shader Code ---\n" <<pVertexShaderString << std::endl;
		throw std::runtime_error(
				MakeString() << "Unable to compile vertex shader ...\n"
						<< message);
	}
	mFragmentShaderHandle = glCreateShader( GL_FRAGMENT_SHADER);
	code = pFragmentShaderString.c_str();
	glShaderSource(mFragmentShaderHandle, 1, &code, 0);
	glCompileShader(mFragmentShaderHandle);
	glGetShaderiv(mFragmentShaderHandle, GL_COMPILE_STATUS, &lStatus);
	if (lStatus != GL_TRUE) {
		glGetInfoLogARB(mFragmentShaderHandle, sizeof(message), NULL, message);
		context->end();
		std::cout << "--- Fragment Shader Code ---\n" << pFragmentShaderString << std::endl;
		throw std::runtime_error(
				MakeString() << "Unable to compile fragment shader ...\n"
						<< message);
	}
	if (pGeometryShaderString.length() > 0) {
		mGeometryShaderHandle = glCreateShader(GL_GEOMETRY_SHADER);
		code = pGeometryShaderString.c_str();
		glShaderSource(mGeometryShaderHandle, 1, &code, 0);
		glCompileShader(mGeometryShaderHandle);
		glGetShaderiv(mGeometryShaderHandle, GL_COMPILE_STATUS, &lStatus);
		if (lStatus != GL_TRUE) {
			glGetInfoLogARB(mGeometryShaderHandle, sizeof(message), NULL,
					message);
			context->end();
			std::cout << "--- Geometry Shader Code ---\n" << pGeometryShaderString << std::endl;
			throw std::runtime_error(
					MakeString() << "Unable to compile geometry shader ...\n"
							<< message);
		}
	}
	mProgramHandle = glCreateProgram();
	glAttachShader(mProgramHandle, mVertexShaderHandle);
	glAttachShader(mProgramHandle, mFragmentShaderHandle);
	if (mGeometryShaderHandle > 0)
		glAttachShader(mProgramHandle, mGeometryShaderHandle);
	int lIndex = 0;
	for (std::string str : pAttributeLocations) {
		glBindAttribLocation(mProgramHandle, lIndex, str.c_str());
		++lIndex;
	}
	glLinkProgram(mProgramHandle);
	glGetProgramiv(mProgramHandle, GL_LINK_STATUS, &lStatus);
	if (lStatus != GL_TRUE) {
		glGetInfoLogARB(mProgramHandle, sizeof(message), NULL, message);
		context->end();
		throw std::runtime_error(
				MakeString() << "Unable to link shaders ...\n" << message);
	}
	context->end();
	CHECK_GL_ERROR();
}

GLShader::~GLShader() {
	if(context.get()!=nullptr){
		context->begin(onScreen);
		glDetachShader(mProgramHandle, mFragmentShaderHandle);
		glDeleteShader(mFragmentShaderHandle);
		glDetachShader(mProgramHandle, mVertexShaderHandle);
		glDeleteShader(mVertexShaderHandle);
		glDetachShader(mProgramHandle, mGeometryShaderHandle);
		glDeleteShader(mGeometryShaderHandle);
		glDeleteProgram(mProgramHandle);
		context->end();
	}
}
}
