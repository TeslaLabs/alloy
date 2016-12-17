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
#include "AlloyMath.h"
#include "AlloyMesh.h"
#include "AlloyFileUtil.h"
#include <vector>
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <set>
#include "AlloyPLY.h"
#include "tiny_obj_loader.h"
#ifndef ALY_WINDOWS
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
namespace aly {
using namespace std;
using namespace ply;

void GLMesh::draw() const {
	draw(PrimitiveType::ALL, onScreen);
}
void GLMesh::draw(const PrimitiveType& type, bool forceVertexColor) const {
	if (mesh.isDirty(onScreen)) {
		mesh.update(onScreen);
		mesh.setDirty(onScreen, false);
	}
	context->begin(onScreen);
	if (vao > 0)
		glBindVertexArray(vao);
	if ((type == GLMesh::PrimitiveType::ALL || type == GLMesh::PrimitiveType::POINTS) && vertexCount > 0) {
		if (vertexBuffer > 0) {
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		}
		if (normalBuffer > 0) {
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		}
		if (colorBuffer > 0) {
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
			glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);
		}
		glDrawArrays(GL_POINTS, 0, vertexCount);
	}
	if ((type == GLMesh::PrimitiveType::ALL || type == GLMesh::PrimitiveType::QUADS) && quadIndexCount > 0) {
		for (int n = 0; n < 4; n++) {
			if (quadVertexBuffer[n] > 0) {
				glEnableVertexAttribArray(3 + n);
				glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer[n]);
				glVertexAttribPointer(3 + n, 3, GL_FLOAT, GL_FALSE, 0, 0);
			}

		}
		for (int n = 0; n < 4; n++) {
			if (quadNormalBuffer[n] > 0) {
				glEnableVertexAttribArray(7 + n);
				glBindBuffer(GL_ARRAY_BUFFER, quadNormalBuffer[n]);
				glVertexAttribPointer(7 + n, 3, GL_FLOAT, GL_FALSE, 0, 0);
			}
		}

		for (int n = 0; n < 4; n++) {
			if (quadColorBuffer[n] > 0) {
				glEnableVertexAttribArray(11 + n);
				glBindBuffer(GL_ARRAY_BUFFER, quadColorBuffer[n]);
				glVertexAttribPointer(11 + n, 4, GL_FLOAT, GL_FALSE, 0, 0);
			}
		}

		if (!forceVertexColor) {
			for (int n = 0; n < 4; n++) {
				if (quadTextureBuffer[n] > 0) {
					glEnableVertexAttribArray(11 + n);
					glBindBuffer(GL_ARRAY_BUFFER, quadTextureBuffer[n]);
					glVertexAttribPointer(11 + n, 2, GL_FLOAT, GL_FALSE, 0, 0);
				}
			}
		}
		glDrawArrays(GL_POINTS, 0, quadIndexCount);
	}
	CHECK_GL_ERROR();
	if ((type == GLMesh::PrimitiveType::ALL || type == GLMesh::PrimitiveType::TRIANGLES) && triIndexCount > 0) {
		for (int n = 0; n < 3; n++) {
			if (triVertexBuffer[n] > 0) {
				glEnableVertexAttribArray(3 + n);
				glBindBuffer(GL_ARRAY_BUFFER, triVertexBuffer[n]);
				glVertexAttribPointer(3 + n, 3, GL_FLOAT, GL_FALSE, 0, 0);
			}
		}
		for (int n = 0; n < 3; n++) {
			if (triNormalBuffer[n] > 0) {
				glEnableVertexAttribArray(7 + n);
				glBindBuffer(GL_ARRAY_BUFFER, triNormalBuffer[n]);
				glVertexAttribPointer(7 + n, 3, GL_FLOAT, GL_FALSE, 0, 0);
			}
		}

		for (int n = 0; n < 3; n++) {
			if (triColorBuffer[n] > 0) {
				glEnableVertexAttribArray(11 + n);
				glBindBuffer(GL_ARRAY_BUFFER, triColorBuffer[n]);
				glVertexAttribPointer(11 + n, 4, GL_FLOAT, GL_FALSE, 0, 0);
			}
		}

		if (!forceVertexColor) {
			for (int n = 0; n < 3; n++) {
				if (triTextureBuffer[n] > 0) {
					glEnableVertexAttribArray(11 + n);
					glBindBuffer(GL_ARRAY_BUFFER, triTextureBuffer[n]);
					glVertexAttribPointer(11 + n, 2, GL_FLOAT, GL_FALSE, 0, 0);
				}
			}
		}
		glDrawArrays(GL_POINTS, 0, triIndexCount);
	}
	for (int i = 0; i <= 14; i++) {
		glDisableVertexAttribArray(i);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	CHECK_GL_ERROR();

	context->end();
}
GLMesh::GLMesh(Mesh& mesh, bool onScreen, const std::shared_ptr<AlloyContext>& context) :
		GLComponent(onScreen, context), mesh(mesh), vao(0), vertexBuffer(0), normalBuffer(0), colorBuffer(0), triIndexBuffer(0), quadIndexBuffer(0), triCount(
				0), quadCount(0), vertexCount(0), triIndexCount(0), quadIndexCount(0) {

	for (int n = 0; n < 4; n++)
		quadColorBuffer[n] = 0;
	for (int n = 0; n < 4; n++)
		quadVertexBuffer[n] = 0;
	for (int n = 0; n < 3; n++)
		triVertexBuffer[n] = 0;
	for (int n = 0; n < 4; n++)
		quadNormalBuffer[n] = 0;
	for (int n = 0; n < 3; n++)
		triNormalBuffer[n] = 0;
	for (int n = 0; n < 4; n++)
		quadTextureBuffer[n] = 0;
	for (int n = 0; n < 3; n++)
		triTextureBuffer[n] = 0;
	for (int n = 0; n < 3; n++)
		triColorBuffer[n] = 0;
}
GLMesh::~GLMesh() {
	if (context.get() == nullptr)
		return;
	context->begin(onScreen);
	if (glIsBuffer(vertexBuffer) == GL_TRUE)
		glDeleteBuffers(1, &vertexBuffer);
	if (glIsBuffer(normalBuffer) == GL_TRUE)
		glDeleteBuffers(1, &normalBuffer);
	if (glIsBuffer(colorBuffer) == GL_TRUE)
		glDeleteBuffers(1, &colorBuffer);
	if (glIsBuffer(triIndexBuffer) == GL_TRUE)
		glDeleteBuffers(1, &triIndexBuffer);
	if (glIsBuffer(quadIndexBuffer) == GL_TRUE)
		glDeleteBuffers(1, &quadIndexBuffer);

	for (int n = 0; n < 4; n++)
		if (glIsBuffer(quadVertexBuffer[n]) == GL_TRUE)
			glDeleteBuffers(1, &quadVertexBuffer[n]);
	for (int n = 0; n < 3; n++)
		if (glIsBuffer(triVertexBuffer[n]) == GL_TRUE)
			glDeleteBuffers(1, &triVertexBuffer[n]);
	for (int n = 0; n < 4; n++)
		if (glIsBuffer(quadColorBuffer[n]) == GL_TRUE)
			glDeleteBuffers(1, &quadColorBuffer[n]);
	for (int n = 0; n < 3; n++)
		if (glIsBuffer(triColorBuffer[n]) == GL_TRUE)
			glDeleteBuffers(1, &triColorBuffer[n]);
	for (int n = 0; n < 4; n++)
		if (glIsBuffer(quadNormalBuffer[n]) == GL_TRUE)
			glDeleteBuffers(1, &quadNormalBuffer[n]);
	for (int n = 0; n < 3; n++)
		if (glIsBuffer(triNormalBuffer[n]) == GL_TRUE)
			glDeleteBuffers(1, &triNormalBuffer[n]);

	for (int n = 0; n < 4; n++)
		if (glIsBuffer(quadTextureBuffer[n]) == GL_TRUE)
			glDeleteBuffers(1, &quadTextureBuffer[n]);
	for (int n = 0; n < 3; n++)
		if (glIsBuffer(triTextureBuffer[n]) == GL_TRUE)
			glDeleteBuffers(1, &triTextureBuffer[n]);

	if (vao != 0)
		glDeleteVertexArrays(1, &vao);
	context->end();
}
void GLMesh::update() {
	if (context.get() == nullptr)
		return;
	context->begin(onScreen);
	quadCount = 0;
	triCount = 0;
	triIndexCount = 0;
	quadIndexCount = 0;
	vertexCount = 0;
	if (vao == 0)
		glGenVertexArrays(1, &vao);

	if (mesh.vertexLocations.size() > 0) {

		if (glIsBuffer(vertexBuffer) == GL_TRUE)
			glDeleteBuffers(1, &vertexBuffer);
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		if (glIsBuffer(vertexBuffer) == GL_FALSE)
			throw std::runtime_error("Error: Unable to create vertex buffer");
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * mesh.vertexLocations.size(), mesh.vertexLocations.ptr(), GL_STATIC_DRAW);
		vertexCount = (uint32_t) mesh.vertexLocations.size();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	if (mesh.vertexNormals.size() > 0) {
		if (glIsBuffer(normalBuffer) == GL_TRUE)
			glDeleteBuffers(1, &normalBuffer);
		glGenBuffers(1, &normalBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		if (glIsBuffer(normalBuffer) == GL_FALSE)
			throw std::runtime_error("Error: Unable to create normal buffer");
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * mesh.vertexNormals.size(), mesh.vertexNormals.ptr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	if (mesh.vertexColors.size() > 0) {
		if (glIsBuffer(colorBuffer) == GL_TRUE)
			glDeleteBuffers(1, &colorBuffer);
		glGenBuffers(1, &colorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
		if (glIsBuffer(colorBuffer) == GL_FALSE)
			throw std::runtime_error("Error: Unable to create color buffer");
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * mesh.vertexColors.size(), mesh.vertexColors.ptr(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	if (mesh.triIndexes.size() > 0) {
		int offset = 0;
		std::vector<float3> tris[3];
		for (int n = 0; n < 3; n++) {
			tris[n].resize(mesh.triIndexes.size());
			if (glIsBuffer(triVertexBuffer[n]) == GL_TRUE)
				glDeleteBuffers(1, &triVertexBuffer[n]);
			glGenBuffers(1, &triVertexBuffer[n]);
		}
		for (uint3 face : mesh.triIndexes.data) {
			for (int n = 0; n < 3; n++) {
				tris[n][offset] = mesh.vertexLocations[face[n]];
			}
			offset++;
		}
		for (int n = 0; n < 3; n++) {
			if (glIsBuffer(triVertexBuffer[n]) == GL_TRUE)
				glDeleteBuffers(1, &triVertexBuffer[n]);
			glGenBuffers(1, &triVertexBuffer[n]);
			glBindBuffer(GL_ARRAY_BUFFER, triVertexBuffer[n]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * tris[n].size(), tris[n].data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		CHECK_GL_ERROR();

		triIndexCount = (GLuint) mesh.triIndexes.size();

	}
	if (mesh.quadIndexes.size() > 0) {
		int offset = 0;
		std::vector<float3> quads[4];
		for (int n = 0; n < 4; n++) {
			quads[n].resize(mesh.quadIndexes.size());
			if (glIsBuffer(quadVertexBuffer[n]) == GL_TRUE)
				glDeleteBuffers(1, &quadVertexBuffer[n]);
			glGenBuffers(1, &quadVertexBuffer[n]);
		}
		for (uint4 face : mesh.quadIndexes.data) {
			for (int n = 0; n < 4; n++) {
				quads[n][offset] = mesh.vertexLocations[face[n]];
			}
			offset++;
		}
		for (int n = 0; n < 4; n++) {
			if (glIsBuffer(quadVertexBuffer[n]) == GL_TRUE)
				glDeleteBuffers(1, &quadVertexBuffer[n]);
			glGenBuffers(1, &quadVertexBuffer[n]);
			glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer[n]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * quads[n].size(), quads[n].data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		CHECK_GL_ERROR();

		quadIndexCount = (GLuint) mesh.quadIndexes.size();
	}
	if (mesh.vertexNormals.size() > 0) {
		if (mesh.quadIndexes.size() > 0) {
			int offset = 0;
			std::vector<float3> quads[4];
			for (int n = 0; n < 4; n++) {
				quads[n].resize(mesh.quadIndexes.size());
				if (glIsBuffer(quadNormalBuffer[n]) == GL_TRUE)
					glDeleteBuffers(1, &quadNormalBuffer[n]);
				glGenBuffers(1, &quadNormalBuffer[n]);
			}
			for (uint4 face : mesh.quadIndexes.data) {
				for (int n = 0; n < 4; n++) {
					quads[n][offset] = mesh.vertexNormals[face[n]];
				}
				offset++;
			}
			for (int n = 0; n < 4; n++) {
				if (glIsBuffer(quadNormalBuffer[n]) == GL_TRUE)
					glDeleteBuffers(1, &quadNormalBuffer[n]);
				glGenBuffers(1, &quadNormalBuffer[n]);
				glBindBuffer(GL_ARRAY_BUFFER, quadNormalBuffer[n]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * quads[n].size(), quads[n].data(),
				GL_STATIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
			CHECK_GL_ERROR();
		}
		if (mesh.triIndexes.size() > 0) {
			int offset = 0;
			std::vector<float3> tris[3];
			for (int n = 0; n < 3; n++) {
				tris[n].resize(mesh.triIndexes.size());
				if (glIsBuffer(triNormalBuffer[n]) == GL_TRUE)
					glDeleteBuffers(1, &triNormalBuffer[n]);
				glGenBuffers(1, &triNormalBuffer[n]);
			}
			for (uint3 face : mesh.triIndexes.data) {
				for (int n = 0; n < 3; n++) {
					tris[n][offset] = mesh.vertexNormals[face[n]];
				}
				offset++;
			}
			for (int n = 0; n < 3; n++) {
				if (glIsBuffer(triNormalBuffer[n]) == GL_TRUE)
					glDeleteBuffers(1, &triNormalBuffer[n]);
				glGenBuffers(1, &triNormalBuffer[n]);
				glBindBuffer(GL_ARRAY_BUFFER, triNormalBuffer[n]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * tris[n].size(), tris[n].data(),
				GL_STATIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
			CHECK_GL_ERROR();
		}
	}
	if (mesh.textureMap.size() > 0) {
		if (mesh.quadIndexes.size() > 0) {
			int offset = 0;
			size_t idx = 0;
			std::vector<float2> quads[4];
			for (int n = 0; n < 4; n++) {
				quads[n].resize(mesh.quadIndexes.size());
				if (glIsBuffer(quadTextureBuffer[n]) == GL_TRUE)
					glDeleteBuffers(1, &quadTextureBuffer[n]);
				glGenBuffers(1, &quadTextureBuffer[n]);
			}
			for (offset = 0; offset < (int) mesh.quadIndexes.size(); offset++) {
				for (int n = 0; n < 4; n++) {
					quads[n][offset] = mesh.textureMap[idx++];
				}
			}
			for (int n = 0; n < 4; n++) {
				if (glIsBuffer(quadTextureBuffer[n]) == GL_TRUE)
					glDeleteBuffers(1, &quadTextureBuffer[n]);
				glGenBuffers(1, &quadTextureBuffer[n]);
				glBindBuffer(GL_ARRAY_BUFFER, quadTextureBuffer[n]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * quads[n].size(), quads[n].data(),
				GL_STATIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
			CHECK_GL_ERROR();
		}
		if (mesh.triIndexes.size() > 0) {
			int offset = 0;
			size_t idx = 0;
			std::vector<float2> tris[3];
			for (int n = 0; n < 3; n++) {
				tris[n].resize(mesh.triIndexes.size());
				if (glIsBuffer(triTextureBuffer[n]) == GL_TRUE)
					glDeleteBuffers(1, &triTextureBuffer[n]);
				glGenBuffers(1, &triTextureBuffer[n]);
			}
			for (offset = 0; offset < (int) mesh.triIndexes.size(); offset++) {
				for (int n = 0; n < 3; n++) {
					tris[n][offset] = mesh.textureMap[idx++];
				}
			}
			for (int n = 0; n < 3; n++) {
				if (glIsBuffer(triTextureBuffer[n]) == GL_TRUE)
					glDeleteBuffers(1, &triTextureBuffer[n]);
				glGenBuffers(1, &triTextureBuffer[n]);
				glBindBuffer(GL_ARRAY_BUFFER, triTextureBuffer[n]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * tris[n].size(), tris[n].data(),
				GL_STATIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
			CHECK_GL_ERROR();
		}
	}

	if (mesh.vertexColors.size() > 0) {
		if (mesh.quadIndexes.size() > 0) {
			int offset = 0;
			std::vector<float4> quads[4];
			for (int n = 0; n < 4; n++) {
				quads[n].resize(mesh.quadIndexes.size());
				if (glIsBuffer(quadColorBuffer[n]) == GL_TRUE)
					glDeleteBuffers(1, &quadColorBuffer[n]);
				glGenBuffers(1, &quadColorBuffer[n]);
			}
			for (uint4 face : mesh.quadIndexes.data) {
				for (int n = 0; n < 4; n++) {
					quads[n][offset] = mesh.vertexColors[face[n]];
				}
				offset++;
			}
			for (int n = 0; n < 4; n++) {
				if (glIsBuffer(quadColorBuffer[n]) == GL_TRUE)
					glDeleteBuffers(1, &quadColorBuffer[n]);
				glGenBuffers(1, &quadColorBuffer[n]);
				glBindBuffer(GL_ARRAY_BUFFER, quadColorBuffer[n]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * quads[n].size(), quads[n].data(),
				GL_STATIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
			CHECK_GL_ERROR();
		}
		if (mesh.triIndexes.size() > 0) {
			int offset = 0;
			std::vector<float4> tris[3];
			for (int n = 0; n < 3; n++) {
				tris[n].resize(mesh.triIndexes.size());
				if (glIsBuffer(triColorBuffer[n]) == GL_TRUE)
					glDeleteBuffers(1, &triColorBuffer[n]);
				glGenBuffers(1, &triColorBuffer[n]);
			}
			if (mesh.vertexColors.size() == mesh.vertexLocations.size()) {
				for (uint3 face : mesh.triIndexes.data) {
					for (int n = 0; n < 3; n++) {
						tris[n][offset] = mesh.vertexColors[face[n]];
					}
					offset++;
				}
			}
			else {
				for (uint3 face : mesh.triIndexes.data) {
					for (int n = 0; n < 3; n++) {
						tris[n][offset] = mesh.vertexColors[offset*3+n];
					}
					offset++;
				}
			}
			for (int n = 0; n < 3; n++) {
				if (glIsBuffer(triColorBuffer[n]) == GL_TRUE)
					glDeleteBuffers(1, &triColorBuffer[n]);
				glGenBuffers(1, &triColorBuffer[n]);
				glBindBuffer(GL_ARRAY_BUFFER, triColorBuffer[n]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * tris[n].size(), tris[n].data(),
				GL_STATIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
			CHECK_GL_ERROR();
		}
	}

	context->end();
}
void Mesh::setContext(const std::shared_ptr<AlloyContext>& context) {
	glOnScreen.reset(new GLMesh(*this, true, context));
	glOffScreen.reset(new GLMesh(*this, false, context));
}
Mesh::Mesh(const std::shared_ptr<AlloyContext>& context) :
		glOnScreen(new GLMesh(*this, true, context)), glOffScreen(new GLMesh(*this, false, context)), pose(float4x4::identity()) {
}
Mesh::Mesh() :
		pose(float4x4::identity()) {
}

bool Mesh::load(const std::string& file) {
	try {
		ReadMeshFromFile(file, *this);
		return true;
	} catch (const exception& e) {
		std::cout << e.what() << std::endl;
		return false;
	}
}
void Mesh::draw(const GLMesh::PrimitiveType& type, bool onScreen, bool forceVertexColor) {
	if (onScreen) {
		if (glOnScreen.get() == nullptr) {
			glOnScreen.reset(new GLMesh(*this, true, AlloyDefaultContext()));
		}
		glOnScreen->draw(type, forceVertexColor);
	} else {
		if (glOffScreen.get() == nullptr) {
			glOffScreen.reset(new GLMesh(*this, false, AlloyDefaultContext()));
		}
		glOffScreen->draw(type, forceVertexColor);
	}
}
void Mesh::clear() {
	vertexLocations.clear();
	vertexNormals.clear();
	vertexColors.clear();
	quadIndexes.clear();
	triIndexes.clear();
	textureMap.clear();
	textureImage.clear();
	setDirty(true);
}
bool Mesh::save(const std::string& file) {
	try {
		WriteMeshToFile(file, *this);
		return true;
	} catch (exception&) {
		return false;
	}
}
void WriteMeshToFile(const std::string& file, const Mesh& mesh) {
	std::string ext = GetFileExtension(file);
	if (ext == "ply") {
		WritePlyMeshToFile(file, mesh, true);
	} else if (ext == "obj") {
		WriteObjMeshToFile(file, mesh);
	} else
		throw std::runtime_error(MakeString() << "Could not write mesh file " << file);
}
void WriteObjMeshToFile(const std::string& file, const Mesh& mesh) {
	uint32_t i;
	std::ofstream out(file.c_str());
	out << setprecision(8);
	out << "####							 \n";
	out << "#								 \n";
	out << "# OBJ File Created by Alloy\n";
	out << "#								 \n";
	out << "# Vertices: " << mesh.vertexLocations.size() << "			 \n";
	out << "# Faces : " << mesh.triIndexes.size() + mesh.quadIndexes.size() << "				 \n";
	out << "#								 \n";
	out << "####							 \n";
	string mtlFile = GetFileNameWithoutExtension(file) + ".mtl";
	if (mesh.textureMap.size() > 0 && mesh.textureImage.size() > 0) {
		out << "mtllib " << mtlFile << "\n";
		std::ofstream mtl(GetFileWithoutExtension(file) + ".mtl");
		mtl << "#								   \n";
		mtl << "# Wavefront material file		   \n";
		mtl << "#								   \n";
		mtl << "newmtl material_0				   \n";
		mtl << "Ka 0.200000 0.200000 0.200000  \n";
		mtl << "Kd 0.500000 0.500000 0.500000  \n";
		mtl << "Ks 0.000000 0.000000 0.000000  \n";
		mtl << "Ni 1.000000					   \n";
		mtl << "d 1.000000					   \n";
		mtl << "illum 2						   \n";
		mtl << "Ns 0.000000					   \n";
		std::string fileName = GetFileNameWithoutExtension(file);
		fileName += string(".png");
		std::string filePath = GetFileWithoutExtension(file);
		filePath += string(".png");
		std::string comment = "TextureFile " + fileName;
		WriteImageToFile(filePath, mesh.textureImage);
		mtl << "map_Kd " << fileName << "\n";
		mtl.close();
	}
	if (mesh.vertexNormals.size() > 0) {
		for (i = 0; i < mesh.vertexNormals.size(); i++) {
			float3 n = mesh.vertexNormals[i];
			out << "vn " << n.x << " " << n.y << " " << n.z << "\n";
			float3 p = mesh.vertexLocations[i];
			if (mesh.vertexColors.size() > 0) {
				RGBAf c = mesh.vertexColors[i];
				out << "v " << p.x << " " << p.y << " " << p.z << " " << c.x << " " << c.y << " " << c.z << "\n";
			} else {
				out << "v " << p.x << " " << p.y << " " << p.z << "\n";
			}
		}
	} else {
		for (i = 0; i < mesh.vertexLocations.size(); i++) {
			float3 p = mesh.vertexLocations[i];
			if (mesh.vertexColors.size() > 0) {
				RGBAf c = mesh.vertexColors[i];
				out << "v " << p.x << " " << p.y << " " << p.z << " " << c.x << " " << c.y << " " << c.z << "\n";
			} else {
				out << "v " << p.x << " " << p.y << " " << p.z << "\n";
			}
		}
	}

	for (i = 0; i < mesh.textureMap.size(); i++) {
		float2 vt = mesh.textureMap[i];
		out << "vt " << vt.x << " " << vt.y << "\n";
	}
	if (mesh.textureMap.size() > 0 && mesh.textureImage.size() > 0)
		out << "usemtl material_0\n";
	i = 0;
	for (uint3 tri : mesh.triIndexes.data) {
		out << "f ";
		if (mesh.vertexNormals.size() > 0 && mesh.textureMap.size() == 0) {
			out << (tri.x + 1) << "/" << (tri.x + 1) << " ";
			out << (tri.y + 1) << "/" << (tri.y + 1) << " ";
			out << (tri.z + 1) << "/" << (tri.z + 1) << "\n";
		} else if (mesh.vertexNormals.size() == 0 && mesh.textureMap.size() > 0) {
			out << (tri.x + 1) << "/" << (i + 1) << " ";
			out << (tri.y + 1) << "/" << (i + 2) << " ";
			out << (tri.z + 1) << "/" << (i + 3) << "\n";
		} else if (mesh.vertexNormals.size() > 0 && mesh.textureMap.size() > 0) {
			out << (tri.x + 1) << "/" << (i + 1) << "/" << (tri.x + 1) << " ";
			out << (tri.y + 1) << "/" << (i + 2) << "/" << (tri.y + 1) << " ";
			out << (tri.z + 1) << "/" << (i + 3) << "/" << (tri.z + 1) << "\n";
		} else {
			out << (tri.x + 1) << " ";
			out << (tri.y + 1) << " ";
			out << (tri.z + 1) << "\n";
		}
		i += 3;
	}
	i = 0;
	for (uint4 quad : mesh.quadIndexes.data) {
		out << "f ";
		if (mesh.vertexNormals.size() > 0 && mesh.textureMap.size() == 0) {
			out << (quad.x + 1) << "/" << (quad.x + 1) << " ";
			out << (quad.y + 1) << "/" << (quad.y + 1) << " ";
			out << (quad.z + 1) << "/" << (quad.z + 1) << " ";
			out << (quad.w + 1) << "/" << (quad.w + 1) << "\n";
		} else if (mesh.vertexNormals.size() == 0 && mesh.textureMap.size() > 0) {
			out << (quad.x + 1) << "/" << (i + 1) << " ";
			out << (quad.y + 1) << "/" << (i + 2) << " ";
			out << (quad.z + 1) << "/" << (i + 3) << " ";
			out << (quad.w + 1) << "/" << (i + 4) << "\n";
		} else if (mesh.vertexNormals.size() > 0 && mesh.textureMap.size() > 0) {
			out << (quad.x + 1) << "/" << (i + 1) << "/" << (quad.x + 1) << " ";
			out << (quad.y + 1) << "/" << (i + 2) << "/" << (quad.y + 1) << " ";
			out << (quad.z + 1) << "/" << (i + 3) << "/" << (quad.z + 1) << " ";
			out << (quad.w + 1) << "/" << (i + 4) << "/" << (quad.w + 1) << "\n";
		} else {
			out << (quad.x + 1) << " ";
			out << (quad.y + 1) << " ";
			out << (quad.z + 1) << " ";
			out << (quad.w + 1) << "\n";
		}
		i += 4;
	}
	out.close();
}
void WritePlyMeshToFile(const std::string& file, const Mesh& mesh, bool binary) {
	std::vector<std::string> elemNames = { "vertex", "face" };
	int i, j, idx;
	bool hasTexture = (mesh.textureMap.size() > 0);
	// Get input and check data
	PLYReaderWriter ply;
	ply.openForWriting(file, elemNames, (binary) ? FileFormat::BINARY_LE : FileFormat::ASCII); //

// compute colors, if any
	int numPts = (int) (mesh.vertexLocations.size());

	int numPolys = (int) (mesh.quadIndexes.size() + mesh.triIndexes.size());
	std::vector<unsigned char> pointColors;

	if (mesh.vertexColors.size() > 0) {
		size_t inc = 0;
		pointColors.resize(3 * mesh.vertexColors.size());
		for (i = 0; i < numPts; i++) {
			float4 d = mesh.vertexColors[i];
			pointColors[inc++] = (unsigned char) clamp(d[0] * 255.0f, 0.0f, 255.0f);
			pointColors[inc++] = (unsigned char) clamp(d[1] * 255.0f, 0.0f, 255.0f);
			pointColors[inc++] = (unsigned char) clamp(d[2] * 255.0f, 0.0f, 255.0f);
		}
	}
	// describe what properties go into the vertex and face elements
	ply.elementCount("vertex", numPts);
	ply.describeProperty("vertex", &MeshVertProps[0]);
	ply.describeProperty("vertex", &MeshVertProps[1]);
	ply.describeProperty("vertex", &MeshVertProps[2]);

	if (mesh.vertexNormals.size() > 0) {
		ply.describeProperty("vertex", &MeshVertProps[3]);
		ply.describeProperty("vertex", &MeshVertProps[4]);
		ply.describeProperty("vertex", &MeshVertProps[5]);
	}
	if (mesh.vertexColors.size() > 0) {
		ply.describeProperty("vertex", &MeshVertProps[9]);
		ply.describeProperty("vertex", &MeshVertProps[10]);
		ply.describeProperty("vertex", &MeshVertProps[11]);
	}
	ply.elementCount("face", numPolys);

	if (hasTexture) {
		ply.describeProperty("face", &MeshFaceProps[2]);
		ply.describeProperty("face", &MeshFaceProps[3]);
	} else {
		ply.describeProperty("face", &MeshFaceProps[0]);
	}

	// write a comment and an object information field
	ply.appendComment("PLY File");
	if (mesh.textureImage.size() > 0) {
		std::string textureFile = GetFileNameWithoutExtension(file) + ".png";
		std::string comment = "TextureFile " + textureFile;
		ply.appendComment(comment);
		std::string root = RemoveTrailingSlash(GetParentDirectory(file));
		if (root.size() > 0) {
			WriteImageToFile(root + ALY_PATH_SEPARATOR+ textureFile, mesh.textureImage);
		}
		else {
			WriteImageToFile(textureFile, mesh.textureImage);
		}
	}
	ply.appendObjInfo("ImageSci");
	// complete the header
	ply.headerComplete();

	// set up and write the vertex elements
	plyVertex vert;
	ply.putElementSetup("vertex");

	for (i = 0; i < numPts; i++) {
		float3 pt = mesh.vertexLocations[i];
		vert.x[0] = pt[0];
		vert.x[1] = pt[1];
		vert.x[2] = pt[2];
		if (mesh.vertexNormals.size() > 0) {
			float3 n = mesh.vertexNormals[i];
			vert.n[0] = n[0];
			vert.n[1] = n[1];
			vert.n[2] = n[2];
		}
		if (pointColors.size() > 0) {
			idx = 3 * i;
			vert.red = pointColors[idx];
			vert.green = pointColors[idx + 1];
			vert.blue = pointColors[idx + 2];
		}
		ply.putElement(&vert);
	}
	// set up and write the face elements
	plyFace face;
	plyFaceTexture faceT;
	int verts[256];
	float2 uvs[4];
	float velocity[3];
	face.verts = verts;
	faceT.verts = verts;
	faceT.velocity = velocity;
	faceT.uvs = (float*) uvs;
	ply.putElementSetup("face");
	if (hasTexture) {
		int sz = (int) (mesh.quadIndexes.size());
		for (int i = 0; i < sz; i++) {
			faceT.nverts = 4;
			faceT.uvcount = 8;
			for (j = 0; j < 4; j++) {
				faceT.verts[j] = mesh.quadIndexes[i][j];
				uvs[j] = mesh.textureMap[4 * i + j];
			}
			ply.putElement(&faceT);
		}
		sz = (int) (mesh.triIndexes.size());
		for (int i = 0; i < sz; i++) {
			faceT.nverts = 3;
			faceT.uvcount = 6;
			for (j = 0; j < 3; j++) {
				faceT.verts[j] = mesh.triIndexes[i][j];
				uvs[j] = mesh.textureMap[3 * i + j];
			}
			ply.putElement(&faceT);
		}
	} else {
		int sz = (int) (mesh.quadIndexes.size());
		for (int i = 0; i < sz; i++) {
			for (j = 0; j < 4; j++) {
				face.nverts = 4;
				face.verts[j] = mesh.quadIndexes[i][j];
			}
			ply.putElement(&face);
		}
		sz = (int) (mesh.triIndexes.size());
		for (int i = 0; i < sz; i++) {
			for (j = 0; j < 3; j++) {
				face.nverts = 3;
				face.verts[j] = mesh.triIndexes[i][j];
			}
			ply.putElement(&face);
		}
	}
}
void Mesh::updateVertexNormals(int SMOOTH_ITERATIONS, float DOT_TOLERANCE) {
	uint32_t sz = (uint32_t) triIndexes.size();
	float3 pt;
	vertexNormals.clear();
	vertexNormals.resize(vertexLocations.size(), float3(0.0f));
	for (uint32_t i = 0; i < sz; i++) {
		uint3 verts = triIndexes[i];
		float3 v1 = vertexLocations[verts.x];
		float3 v2 = vertexLocations[verts.y];
		float3 v3 = vertexLocations[verts.z];
		float3 norm = cross((v3 - v1), (v2 - v1));
		vertexNormals[verts.x] += norm;
		vertexNormals[verts.y] += norm;
		vertexNormals[verts.z] += norm;
	}
	sz = (uint32_t) quadIndexes.size();
	for (uint32_t i = 0; i < sz; i++) {
		uint4 verts = quadIndexes[i];
		float3 v1 = vertexLocations[verts.x];
		float3 v2 = vertexLocations[verts.y];
		float3 v3 = vertexLocations[verts.z];
		float3 v4 = vertexLocations[verts.w];

		vertexNormals[verts.x] += cross((v4 - v1), (v2 - v1));
		vertexNormals[verts.y] += cross((v1 - v2), (v3 - v2));
		vertexNormals[verts.z] += cross((v2 - v3), (v4 - v3));
		vertexNormals[verts.w] += cross((v3 - v4), (v1 - v4));
	}
#pragma omp parallel for
	for (int n = 0; n < (int) vertexNormals.size(); n++) {
		vertexNormals[n] = normalize(vertexNormals[n]);
	}
	if (SMOOTH_ITERATIONS > 0) {
		int vertCount = (int) vertexLocations.size();
		std::vector<float3> tmp(vertCount);
		std::vector<std::list<int>> vertNbrs(vertCount);
		int indexCount = (int) quadIndexes.size();
		for (int i = 0; i < indexCount; i++) {
			uint4 verts = quadIndexes[i];
			int v1 = verts.x;
			int v2 = verts.y;
			int v3 = verts.z;
			int v4 = verts.w;

			vertNbrs[v1].push_back(v2);
			vertNbrs[v2].push_back(v3);
			vertNbrs[v3].push_back(v1);

			vertNbrs[v3].push_back(v4);
			vertNbrs[v4].push_back(v1);
			vertNbrs[v1].push_back(v3);
		}
		indexCount = (int) triIndexes.size();
		for (int i = 0; i < indexCount; i++) {
			uint3 verts = triIndexes[i];
			int v1 = verts.x;
			int v2 = verts.y;
			int v3 = verts.z;
			vertNbrs[v1].push_back(v2);
			vertNbrs[v2].push_back(v3);
			vertNbrs[v3].push_back(v1);
		}
		for (int iter = 0; iter < SMOOTH_ITERATIONS; iter++) {
#pragma omp for
			for (int i = 0; i < vertCount; i++) {
				float3 norm = vertexNormals[i];
				float3 avg = float3(0.0f);
				for (int nbr : vertNbrs[i]) {
					float3 nnorm = vertexNormals[nbr];
					;
					if (dot(norm, nnorm) > DOT_TOLERANCE) {
						avg += nnorm;
					} else {
						avg += norm;
					}
				}
				tmp[i] = normalize(avg);
			}
			vertexNormals = tmp;
		}
	}
	setDirty(true);
}
float Mesh::estimateVoxelSize(int stride) {
	int count = 0;
	//float maxLength = 0.0f;
	int sz = (int) triIndexes.size();
	float mEstimatedVoxelSize = 0.0f;
	for (int i = 0; i < sz; i += stride) {
		uint3 verts = triIndexes[i];
		float3 v1 = vertexLocations[verts.x];
		float3 v2 = vertexLocations[verts.y];
		float3 v3 = vertexLocations[verts.z];
		float e1 = length(v1 - v2);
		float e2 = length(v1 - v3);
		float e3 = length(v2 - v3);
		//maxLength = std::max(std::max(e1, e2), std::max(maxLength, e3));
		mEstimatedVoxelSize += e1 + e2 + e3;
	}
	count = sz / stride;
	sz = (int) (quadIndexes.size());
	for (int i = 0; i < sz; i += stride) {
		uint4 verts = quadIndexes[i];
		float3 v1 = vertexLocations[verts.x];
		float3 v2 = vertexLocations[verts.y];
		float3 v3 = vertexLocations[verts.z];
		float3 v4 = vertexLocations[verts.w];
		float e1 = length(v1 - v2);
		float e2 = length(v2 - v3);
		float e3 = length(v3 - v4);
		float e4 = length(v4 - v1);
		//maxLength = std::max(maxLength,std::max(std::max(e1, e2), std::max(e3, e4)));
		mEstimatedVoxelSize += e1 + e2 + e3 + e4;
	}
	count += sz / stride;
	mEstimatedVoxelSize /= count;
	return mEstimatedVoxelSize;
}
box3f Mesh::updateBoundingBox() {
	const int BATCHES = 32;
	float3 minPt(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	std::vector<float3> minPtBatch(BATCHES, float3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()));
	float3 maxPt(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
	std::vector<float3> maxPtBatch(BATCHES, float3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()));
	int SZ = (int) vertexLocations.size();
	int batchSize = (SZ % BATCHES == 0) ? SZ / BATCHES : SZ / BATCHES + 1;
#pragma omp for
	for (int b = 0; b < BATCHES; b++) {
		int sz = std::min(SZ, batchSize * (b + 1));
		for (uint32_t idx = b * batchSize; idx < (uint32_t) sz; idx++) {
			float3& pt = vertexLocations[idx];
			minPtBatch[b][0] = std::min(minPtBatch[b][0], pt[0]);
			minPtBatch[b][1] = std::min(minPtBatch[b][1], pt[1]);
			minPtBatch[b][2] = std::min(minPtBatch[b][2], pt[2]);

			maxPtBatch[b][0] = std::max(maxPtBatch[b][0], pt[0]);
			maxPtBatch[b][1] = std::max(maxPtBatch[b][1], pt[1]);
			maxPtBatch[b][2] = std::max(maxPtBatch[b][2], pt[2]);
		}
	}

	for (int b = 0; b < BATCHES; b++) {
		minPt[0] = std::min(minPtBatch[b][0], minPt[0]);
		minPt[1] = std::min(minPtBatch[b][1], minPt[1]);
		minPt[2] = std::min(minPtBatch[b][2], minPt[2]);

		maxPt[0] = std::max(maxPtBatch[b][0], maxPt[0]);
		maxPt[1] = std::max(maxPtBatch[b][1], maxPt[1]);
		maxPt[2] = std::max(maxPtBatch[b][2], maxPt[2]);
	}
	boundingBox = box3f(minPt, maxPt - minPt);
	return boundingBox;
}
void Mesh::scale(float sc) {
#pragma omp parallel for
	for (int i = 0; i < (int) vertexLocations.size(); i++) {
		vertexLocations[i] *= sc;
	}
	boundingBox.dimensions = sc * boundingBox.dimensions;
	boundingBox.position = sc * boundingBox.position;
	setDirty(true);
}
void Mesh::transform(const float4x4& M) {
#pragma omp parallel for
	for (int i = 0; i < (int) vertexLocations.size(); i++) {
		float4 pt = M * vertexLocations[i].xyzw();
		vertexLocations[i] = pt.xyz() / pt.w;
	}

	if (vertexNormals.size() > 0) {
		float3x3 NM = transpose(inverse(SubMatrix(M)));
#pragma omp parallel for
		for (int i = 0; i < (int) vertexLocations.size(); i++) {
			vertexNormals[i] = normalize(NM * vertexNormals[i]);
		}
	}
	updateBoundingBox();
	setDirty(true);
}
void Mesh::mapIntoBoundingBox(float voxelSize) {
	float3 minPt = boundingBox.min();
#pragma omp parallel for
	for (int i = 0; i < (int) vertexLocations.size(); i++) {
		float3& pt = vertexLocations[i];
		pt = (pt - minPt) / voxelSize;
	}
	setDirty(true);
}
void Mesh::mapOutOfBoundingBox(float voxelSize) {
	float3 minPt = boundingBox.min();
#pragma omp for
	for (int i = 0; i < (int) vertexLocations.size(); i++) {
		float3& pt = vertexLocations[i];
		pt = pt * voxelSize + minPt;
	}
	setDirty(true);
}

void Mesh::update(bool onScreen) {
	if (onScreen) {
		if (glOnScreen.get() == nullptr) {
			glOnScreen.reset(new GLMesh(*this, true, AlloyDefaultContext()));
		}
		glOnScreen->update();
	} else {
		if (glOnScreen.get() == nullptr) {
			glOnScreen.reset(new GLMesh(*this, false, AlloyDefaultContext()));
		}
		glOffScreen->update();
	}
}
Mesh::~Mesh() {
	// TODO Auto-generated destructor stub
}
void ReadObjMeshFromFile(const std::string& file, std::vector<Mesh>& meshList) {
	using namespace tinyobj;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err = tinyobj::LoadObj(shapes, materials, file.c_str(), GetParentDirectory(file).c_str());
	if (err.size() > 0)
		throw std::runtime_error(err);
	meshList.resize(shapes.size());
	for (int n = 0; n < (int) shapes.size(); n++) {
		Mesh& mesh = meshList[n];
		tinyobj::shape_t& shape = shapes[n];
		if (shape.mesh.positions.size() > 0) {
			mesh.vertexLocations.resize(shape.mesh.positions.size() / 3);
			mesh.vertexLocations.set(shape.mesh.positions.data());
		}
		if (shape.mesh.normals.size() > 0) {
			mesh.vertexNormals.resize(shape.mesh.normals.size() / 3);
			mesh.vertexNormals.set(shape.mesh.normals.data());
		}
		if (shape.mesh.triIndices.size() > 0) {
			mesh.triIndexes.resize(shape.mesh.triIndices.size() / 3);
			mesh.triIndexes.set(shape.mesh.triIndices.data());
		}
		if (shape.mesh.quadIndices.size() > 0) {
			mesh.quadIndexes.resize(shape.mesh.quadIndices.size() / 4);
			mesh.quadIndexes.set(shape.mesh.quadIndices.data());
		}
		for (size_t i = 0; i < shape.mesh.triIndices.size(); i += 3) {
			uint32_t vid1 = shape.mesh.triIndices[i];
			uint32_t vid2 = shape.mesh.triIndices[i + 1];
			uint32_t vid3 = shape.mesh.triIndices[i + 2];
			if (shape.mesh.texcoords.size() > 0) {
				mesh.textureMap.push_back(float2(shape.mesh.texcoords[2 * vid1], shape.mesh.texcoords[2 * vid1 + 1]));
				mesh.textureMap.push_back(float2(shape.mesh.texcoords[2 * vid2], shape.mesh.texcoords[2 * vid2 + 1]));
				mesh.textureMap.push_back(float2(shape.mesh.texcoords[2 * vid3], shape.mesh.texcoords[2 * vid3 + 1]));
			}
		}
		for (size_t i = 0; i < shape.mesh.quadIndices.size(); i += 4) {
			uint32_t vid1 = shape.mesh.quadIndices[i];
			uint32_t vid2 = shape.mesh.quadIndices[i + 1];
			uint32_t vid3 = shape.mesh.quadIndices[i + 2];
			uint32_t vid4 = shape.mesh.quadIndices[i + 3];
			if (shape.mesh.texcoords.size() > 0) {
				mesh.textureMap.push_back(float2(shape.mesh.texcoords[2 * vid1], shape.mesh.texcoords[2 * vid1 + 1]));
				mesh.textureMap.push_back(float2(shape.mesh.texcoords[2 * vid2], shape.mesh.texcoords[2 * vid2 + 1]));
				mesh.textureMap.push_back(float2(shape.mesh.texcoords[2 * vid3], shape.mesh.texcoords[2 * vid3 + 1]));
				mesh.textureMap.push_back(float2(shape.mesh.texcoords[2 * vid4], shape.mesh.texcoords[2 * vid4 + 1]));
			}
		}
		std::string texName = materials[n].diffuse_texname;
		if (texName.size() > 0) {
			aly::ReadImageFromFile(GetParentDirectory(file) + ALY_PATH_SEPARATOR+ texName, mesh.textureImage);
		}
		mesh.updateBoundingBox();
	}

}
void ReadObjMeshFromFile(const std::string& file, Mesh& mesh) {
	using namespace tinyobj;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err = tinyobj::LoadObj(shapes, materials, file.c_str(), GetParentDirectory(file).c_str());
	if (err.size() > 0)
		throw std::runtime_error(err);

	uint32_t positionCount = 0;
	uint32_t colorCount = 0;
	uint32_t normalCount = 0;
	uint32_t texCount = 0;
	uint32_t triIndexCount = 0;
	uint32_t quadIndexCount = 0;
	for (int n = 0; n < (int) shapes.size(); n++) {
		shape_t& shape = shapes[n];
		positionCount += (uint32_t) shape.mesh.positions.size();
		colorCount += (uint32_t) shape.mesh.colors.size();
		normalCount += (uint32_t) shape.mesh.normals.size();
		texCount += (uint32_t) shape.mesh.texcoords.size();
		triIndexCount += (uint32_t) shape.mesh.triIndices.size();
		quadIndexCount += (uint32_t) shape.mesh.quadIndices.size();
	}
	mesh.vertexColors.clear();
	mesh.vertexColors.resize(colorCount / 3);
	mesh.vertexLocations.resize(positionCount / 3);
	mesh.vertexNormals.resize(normalCount / 3);
	mesh.triIndexes.resize(triIndexCount / 3);
	mesh.quadIndexes.resize(quadIndexCount / 4);
	positionCount = 0;
	colorCount=0;
	normalCount = 0;
	texCount = 0;
	triIndexCount = 0;
	quadIndexCount = 0;
	mesh.textureMap.clear();
	for (int n = 0; n < (int) shapes.size(); n++) {
		shape_t& shape = shapes[n];
		for (size_t i = 0; i < shape.mesh.triIndices.size(); i += 3) {
			mesh.triIndexes[triIndexCount++] = uint3(positionCount + shape.mesh.triIndices[i], positionCount + shape.mesh.triIndices[i + 1],
					positionCount + shape.mesh.triIndices[i + 2]);
		}
		for (size_t i = 0; i < shape.mesh.quadIndices.size(); i += 4) {
			mesh.quadIndexes[quadIndexCount++] = uint4(positionCount + shape.mesh.quadIndices[i], positionCount + shape.mesh.quadIndices[i + 1],
					positionCount + shape.mesh.quadIndices[i + 2], positionCount + shape.mesh.quadIndices[i + 3]);
		}
		for (size_t i = 0; i < shape.mesh.positions.size(); i += 3) {
			mesh.vertexLocations[positionCount++] = float3(shape.mesh.positions[i], shape.mesh.positions[i + 1], shape.mesh.positions[i + 2]);
		}
		for (size_t i = 0; i < shape.mesh.colors.size(); i += 3) {
			mesh.vertexColors[colorCount++] = float4(shape.mesh.colors[i], shape.mesh.colors[i + 1], shape.mesh.colors[i + 2],1.0f);
		}
		for (size_t i = 0; i < shape.mesh.normals.size(); i += 3) {
			mesh.vertexNormals[normalCount++] = float3(shape.mesh.normals[i], shape.mesh.normals[i + 1], shape.mesh.normals[i + 2]);
		}
		for (size_t i = 0; i < shape.mesh.triIndices.size(); i += 3) {
			uint32_t vid1 = shape.mesh.triIndices[i];
			uint32_t vid2 = shape.mesh.triIndices[i + 1];
			uint32_t vid3 = shape.mesh.triIndices[i + 2];
			if (shape.mesh.texcoords.size() > 0) {
				mesh.textureMap.push_back(float2(shape.mesh.texcoords[2 * vid1], shape.mesh.texcoords[2 * vid1 + 1]));
				mesh.textureMap.push_back(float2(shape.mesh.texcoords[2 * vid2], shape.mesh.texcoords[2 * vid2 + 1]));
				mesh.textureMap.push_back(float2(shape.mesh.texcoords[2 * vid3], shape.mesh.texcoords[2 * vid3 + 1]));
			}
		}
		for (size_t i = 0; i < shape.mesh.quadIndices.size(); i += 4) {
			uint32_t vid1 = shape.mesh.quadIndices[i];
			uint32_t vid2 = shape.mesh.quadIndices[i + 1];
			uint32_t vid3 = shape.mesh.quadIndices[i + 2];
			uint32_t vid4 = shape.mesh.quadIndices[i + 3];
			if (shape.mesh.texcoords.size() > 0) {
				mesh.textureMap.push_back(float2(shape.mesh.texcoords[2 * vid1], shape.mesh.texcoords[2 * vid1 + 1]));
				mesh.textureMap.push_back(float2(shape.mesh.texcoords[2 * vid2], shape.mesh.texcoords[2 * vid2 + 1]));
				mesh.textureMap.push_back(float2(shape.mesh.texcoords[2 * vid3], shape.mesh.texcoords[2 * vid3 + 1]));
				mesh.textureMap.push_back(float2(shape.mesh.texcoords[2 * vid4], shape.mesh.texcoords[2 * vid4 + 1]));
			}
		}
		if(n<(int)materials.size()){
			std::string texName = materials[n].diffuse_texname;
			if (texName.size() > 0) {
				aly::ReadImageFromFile(GetParentDirectory(file) + ALY_PATH_SEPARATOR+ texName, mesh.textureImage);
			}
		}
	}
	if (mesh.vertexNormals.size() == 0) {
		mesh.updateVertexNormals();
	}
	mesh.updateBoundingBox();
}
void ReadMeshFromFile(const std::string& file, Mesh &mesh) {
	std::string ext = GetFileExtension(file);
	if (ext == "ply") {
		ReadPlyMeshFromFile(file, mesh);
	} else if (ext == "obj") {
		ReadObjMeshFromFile(file, mesh);
	} else
		throw std::runtime_error(MakeString() << "Could not read file " << file);
}
void ReadPlyMeshFromFile(const std::string& file, Mesh &mesh) {
	int i, j;
	int numPts = 0, numPolys = 0;
	PLYReaderWriter ply;
	ply.openForReading(file);
	// Check to make sure that we can read geometry
	PlyElement *elem;
	int index;
	if ((elem = ply.findElement("vertex")) == nullptr || ply.findProperty(elem, "x", &index) == nullptr || ply.findProperty(elem, "y", &index) == nullptr
			|| ply.findProperty(elem, "z", &index) == nullptr || (elem = ply.findElement("face")) == nullptr
			|| ply.findProperty(elem, "vertex_indices", &index) == nullptr) {
		throw std::runtime_error(MakeString() << "Could not read geometry [" << file << "]");
	}

	// Check for optional attribute data. We can handle intensity; and the
	// triplet red, green, blue.
	bool RGBPointsAvailable = false;
	bool hasNormals = false;

	mesh.triIndexes.clear();
	mesh.quadIndexes.clear();
	mesh.vertexLocations.clear();
	mesh.vertexNormals.clear();
	mesh.vertexColors.clear();
	mesh.textureMap.clear();
	mesh.textureImage.clear();
	if ((elem = ply.findElement("vertex")) != nullptr && ply.findProperty(elem, "red", &index) != nullptr && ply.findProperty(elem, "green", &index) != nullptr
			&& ply.findProperty(elem, "blue", &index) != nullptr) {
		RGBPointsAvailable = true;
	}
	if ((elem = ply.findElement("vertex")) != nullptr && ply.findProperty(elem, "nx", &index) != nullptr && ply.findProperty(elem, "ny", &index) != nullptr
			&& ply.findProperty(elem, "nz", &index) != nullptr) {
		hasNormals = true;
	}
	bool hasTexture = false;

	if ((elem = ply.findElement("face")) != nullptr && ply.findProperty(elem, "texcoord", &index) != nullptr) {
		hasTexture = true;
		std::string textureFile;
		for (string comment : ply.getComments()) {
			const string keyName("TextureFile");
			int offset = (int) comment.find(keyName, 0);
			if (offset >= 0) {
				textureFile = comment.substr(offset + keyName.size() + 1);
				break;
			}

		}
		std::string texturePath = RemoveTrailingSlash(GetParentDirectory(file)) + ALY_PATH_SEPARATOR+ GetFileName(textureFile);
		if (textureFile.size() > 0 && FileExists(texturePath)) {
			ReadImageFromFile(texturePath, mesh.textureImage);
		}
	}

	int verts[256];
	float velocity[3];
	float uvs[8];

	plyVertex vertex;
	plyFace face;
	face.verts = verts;
	face.velocity = velocity;

	plyFaceTexture faceTex;
	faceTex.uvs = uvs;
	faceTex.verts = verts;
	faceTex.velocity = velocity;

	memset(verts, 0, sizeof(verts));
	std::vector<std::string> elist = ply.getElementNames();
	std::string elemName;
	int numElems, nprops;
	// Okay, now we can grab the data
	for (i = 0; i < ply.getNumberOfElements(); i++) {
		//get the description of the first element */
		elemName = elist[i];
		ply.getElementDescription(elemName, &numElems, &nprops);
		// if we're on vertex elements, read them in
		if (elemName == "vertex") {
			// Create a list of points
			numPts = numElems;
			mesh.vertexLocations.resize(numPts, float3(0.0f));
			// Setup to read the PLY elements
			ply.getProperty(elemName, &MeshVertProps[0]);
			ply.getProperty(elemName, &MeshVertProps[1]);
			ply.getProperty(elemName, &MeshVertProps[2]);
			if (hasNormals) {
				mesh.vertexNormals.resize(numPts);
				ply.getProperty(elemName, &MeshVertProps[3]);
				ply.getProperty(elemName, &MeshVertProps[4]);
				ply.getProperty(elemName, &MeshVertProps[5]);
			}
			if (RGBPointsAvailable) {
				mesh.vertexColors.resize(numPts);
				ply.getProperty(elemName, &MeshVertProps[9]);
				ply.getProperty(elemName, &MeshVertProps[10]);
				ply.getProperty(elemName, &MeshVertProps[11]);
			}
			for (j = 0; j < numPts; j++) {
				ply.getElement(&vertex);
				mesh.vertexLocations[j] = float3(vertex.x[0], vertex.x[1], vertex.x[2]);

				if (RGBPointsAvailable) {
					mesh.vertexColors[j] = float4(vertex.red / 255.0f, vertex.green / 255.0f, vertex.blue / 255.0f, 1.0f);
				}
				if (hasNormals) {
					mesh.vertexNormals[j] = float3(vertex.n[0], vertex.n[1], vertex.n[2]);
				}
			}
		}			//if vertex
		else if (elemName == "face") {
			// Create a polygonal array
			numPolys = numElems;
			// Get the face properties
			if (hasTexture) {
				ply.getProperty(elemName, &MeshFaceProps[2]);
				ply.getProperty(elemName, &MeshFaceProps[3]);
				for (j = 0; j < numPolys; j++) {
					ply.getElement(&faceTex);
					if (faceTex.nverts == 4) {
						mesh.quadIndexes.append(uint4(faceTex.verts[0], faceTex.verts[1], faceTex.verts[2], faceTex.verts[3]));
					} else if (faceTex.nverts == 3) {
						mesh.triIndexes.append(uint3(faceTex.verts[0], faceTex.verts[1], faceTex.verts[2]));
					}
					for (int i = 0; i < faceTex.nverts; i++) {
						mesh.textureMap.append(float2(faceTex.uvs[2 * i], faceTex.uvs[2 * i + 1]));
					}
				}
			} else {
				ply.getProperty(elemName, &MeshFaceProps[0]);
				for (j = 0; j < numPolys; j++) {
					ply.getElement(&face);
					if (face.nverts == 4) {
						mesh.quadIndexes.append(uint4(face.verts[0], face.verts[1], face.verts[2], face.verts[3]));
					} else if (face.nverts == 3) {
						mesh.triIndexes.append(uint3(face.verts[0], face.verts[1], face.verts[2]));
					}
				}
			}
		}							//if face
	} //for all elements of the PLY file
	if (mesh.vertexLocations.size() > 0) {
		mesh.updateBoundingBox();
	}
	if (mesh.vertexNormals.size() == 0) {
		mesh.updateVertexNormals();
	}
	mesh.setDirty(true);
}

void CreateVertexNeighborTable(const Mesh& mesh, std::vector<std::set<uint32_t>>& vertNbrs) {
	vertNbrs.resize(mesh.vertexLocations.size());
	for (const uint3& face : mesh.triIndexes.data) {
		vertNbrs[face.x].insert(face.y);
		vertNbrs[face.y].insert(face.z);
		vertNbrs[face.z].insert(face.x);
		vertNbrs[face.z].insert(face.y);
		vertNbrs[face.y].insert(face.x);
		vertNbrs[face.x].insert(face.z);
	}
	for (const uint4& face : mesh.quadIndexes.data) {
		vertNbrs[face.x].insert(face.y);
		vertNbrs[face.y].insert(face.z);
		vertNbrs[face.z].insert(face.w);
		vertNbrs[face.w].insert(face.x);
		vertNbrs[face.w].insert(face.z);
		vertNbrs[face.z].insert(face.y);
		vertNbrs[face.y].insert(face.x);
		vertNbrs[face.x].insert(face.w);
	}
}
inline uint64_t faceHashCode(const uint2& val) {
	return ((uint64_t) val.y) << 32 | ((uint64_t) val.x);
}
void CreateOrderedVertexNeighborTable(const Mesh& mesh, std::vector<std::list<uint32_t>>& vertNbrsOut, bool leaveTail) {
	//Leave tail means to not remove the duplicate vertex neighbor at the end of the neighbor list.
	//Non-manifold vertexes will not have a tail, so the tail can be used to detect them in simple (common) cases.
	vertNbrsOut.resize(mesh.vertexLocations.size());
	std::vector<std::vector<uint32_t>> vertNbrs(mesh.vertexLocations.size());
	for (const uint3& face : mesh.triIndexes.data) {
		vertNbrs[face.x].push_back(face.z);
		vertNbrs[face.x].push_back(face.y);

		vertNbrs[face.y].push_back(face.x);
		vertNbrs[face.y].push_back(face.z);

		vertNbrs[face.z].push_back(face.y);
		vertNbrs[face.z].push_back(face.x);
	}
	for (const uint4& face : mesh.quadIndexes.data) {
		vertNbrs[face.x].push_back(face.w);
		vertNbrs[face.x].push_back(face.y);

		vertNbrs[face.y].push_back(face.x);
		vertNbrs[face.y].push_back(face.z);

		vertNbrs[face.z].push_back(face.y);
		vertNbrs[face.z].push_back(face.w);

		vertNbrs[face.w].push_back(face.z);
		vertNbrs[face.w].push_back(face.x);
	}
	int N = (int) vertNbrs.size();
	for (int n = 0; n < N; n++) {
		std::vector<uint32_t>& nbrs = vertNbrs[n];
		if (nbrs.size() > 0) {
			vertNbrsOut[n].clear();
			bool found;
			std::list<uint32_t> chain;
			chain.push_back(nbrs[0]);
			chain.push_back(nbrs[1]);
			nbrs[0] = -1;
			nbrs[1] = -1;
			do {
				uint32_t front = chain.front();
				uint32_t back = chain.back();
				found = false;
				for (int i = 0; i < (int) nbrs.size(); i += 2) {
					if (nbrs[i] == back) {
						chain.push_back(nbrs[i + 1]);
						nbrs[i + 1] = -1;
						found = true;
						break;
					}
				}
				if (!found) {
					for (int i = 1; i < (int) nbrs.size(); i += 2) {
						if (nbrs[i] == front) {
							chain.push_front(nbrs[i - 1]);
							nbrs[i - 1] = -1;
							found = true;
							break;
						}
					}
				}
				if (!found) {
					if (chain.size() > 0) {
						if (!leaveTail && chain.front() == chain.back())
							chain.pop_back();
						vertNbrsOut[n].insert(vertNbrsOut[n].end(), chain.begin(), chain.end());
						chain.clear();
					}
					for (int i = 0; i < (int) nbrs.size(); i += 2) {
						if (nbrs[i] != (uint32_t) -1 && nbrs[i + 1] != (uint32_t) -1) {
							chain.push_back(nbrs[i]);
							chain.push_back(nbrs[i + 1]);
							found = true;
							break;
						}
					}
				}
			} while (found);

			if (chain.size() > 0) {
				if (!leaveTail && chain.front() == chain.back())
					chain.pop_back();
				vertNbrsOut[n].insert(vertNbrsOut[n].end(), chain.begin(), chain.end());
				/*
				 nbrs=vertNbrs[n];
				 std::cout << "in " << n << ": ";
				 for (int i = 0; i < nbrs.size(); i += 2) {
				 std::cout << "(" << nbrs[i] << "," << nbrs[i + 1] << ") ";
				 }
				 std::cout << std::endl;

				 std::cout << "out " << n << ": ";
				 for (uint32_t elem : vertNbrsOut[n]) {
				 std::cout << elem << " ";
				 }

				 std::cout << std::endl;
				 */
			}

		}
	}
}

bool Mesh::convertQuadsToTriangles() {
	bool ret=(quadIndexes.size()>0);
	if (textureMap.size() > 0 && quadIndexes.size() > 0) {
		Vector2f newTextureMap;
		uint32_t index = 0;
		for (int n = 0; n < (int) triIndexes.size(); n++) {
			float2 tx = textureMap[index++];
			float2 ty = textureMap[index++];
			float2 tz = textureMap[index++];
			newTextureMap.push_back(tx);
			newTextureMap.push_back(ty);
			newTextureMap.push_back(tz);
		}
		for (const uint4& face : quadIndexes.data) {
			float3 pt1 = vertexLocations[face.x];
			float3 pt2 = vertexLocations[face.y];
			float3 pt3 = vertexLocations[face.z];
			float3 pt4 = vertexLocations[face.w];

			float2 tx = textureMap[index++];
			float2 ty = textureMap[index++];
			float2 tz = textureMap[index++];
			float2 tw = textureMap[index++];
			if (distanceSqr(pt1, pt3) < distanceSqr(pt2, pt4)) {
				triIndexes.push_back(uint3(face.x, face.y, face.z));
				newTextureMap.push_back(tx);
				newTextureMap.push_back(ty);
				newTextureMap.push_back(tz);
				triIndexes.push_back(uint3(face.z, face.w, face.x));
				newTextureMap.push_back(tz);
				newTextureMap.push_back(tw);
				newTextureMap.push_back(tx);
			} else {
				triIndexes.push_back(uint3(face.x, face.y, face.w));
				newTextureMap.push_back(tx);
				newTextureMap.push_back(ty);
				newTextureMap.push_back(tw);
				triIndexes.push_back(uint3(face.w, face.y, face.z));
				newTextureMap.push_back(tw);
				newTextureMap.push_back(ty);
				newTextureMap.push_back(tz);
			}
		}

		textureMap = newTextureMap;
	} else {
		for (const uint4& face : quadIndexes.data) {
			float3 pt1 = vertexLocations[face.x];
			float3 pt2 = vertexLocations[face.y];
			float3 pt3 = vertexLocations[face.z];
			float3 pt4 = vertexLocations[face.w];
			if (distanceSqr(pt1, pt3) < distanceSqr(pt2, pt4)) {
				triIndexes.push_back(uint3(face.x, face.y, face.z));
				triIndexes.push_back(uint3(face.z, face.w, face.x));
			} else {
				triIndexes.push_back(uint3(face.x, face.y, face.w));
				triIndexes.push_back(uint3(face.w, face.y, face.z));
			}
		}
	}
	if (quadIndexes.size() > 0)
		setDirty(true);
	quadIndexes.clear();
	if (vertexNormals.size() > 0) {
		updateVertexNormals();
	}
	return ret;
}
void CreateFaceNeighborTable(const Mesh& mesh, std::vector<std::list<uint32_t>>& faceNbrs) {
	faceNbrs.resize(mesh.triIndexes.size() + mesh.quadIndexes.size());
	std::map<uint64_t, std::list<uint>> edgeTable;
	uint2 edge;
	uint fid = 0;
	for (const uint3& face : mesh.triIndexes.data) {
		edge = (face.x < face.y) ? uint2(face.x, face.y) : uint2(face.y, face.x);
		edgeTable[faceHashCode(edge)].push_back(fid);
		edge = (face.y < face.z) ? uint2(face.y, face.z) : uint2(face.z, face.y);
		edgeTable[faceHashCode(edge)].push_back(fid);
		edge = (face.z < face.x) ? uint2(face.z, face.x) : uint2(face.x, face.z);
		edgeTable[faceHashCode(edge)].push_back(fid);
		fid++;
	}
	for (const uint4& face : mesh.quadIndexes.data) {
		edge = (face.x < face.y) ? uint2(face.x, face.y) : uint2(face.y, face.x);
		edgeTable[faceHashCode(edge)].push_back(fid);
		edge = (face.y < face.z) ? uint2(face.y, face.z) : uint2(face.z, face.y);
		edgeTable[faceHashCode(edge)].push_back(fid);
		edge = (face.z < face.w) ? uint2(face.z, face.w) : uint2(face.w, face.z);
		edgeTable[faceHashCode(edge)].push_back(fid);
		edge = (face.w < face.x) ? uint2(face.w, face.x) : uint2(face.x, face.w);
		edgeTable[faceHashCode(edge)].push_back(fid);
		fid++;
	}
	for (std::pair<const uint64_t, std::list<uint>>& edgeList : edgeTable) {
		if (edgeList.second.size() == 2) {
			uint fid1 = edgeList.second.front();
			uint fid2 = edgeList.second.back();
			if (fid1 != fid2) {
				faceNbrs[fid1].push_back(fid2);
				faceNbrs[fid2].push_back(fid1);
			}
		}
	}
}
struct FaceEdge {
	std::list<uint> faces;
	size_t edgePointIndex;
};
void SubdivideCatmullClark(Mesh& mesh) {
	std::vector<std::list<uint32_t>> faceFaceNbrs;
	faceFaceNbrs.resize(mesh.triIndexes.size() + mesh.quadIndexes.size());
	std::map<uint64_t, FaceEdge> edgeTable;
	std::set<uint2> edges;
	bool hasUVs = mesh.textureMap.size() > 0;
	bool hasColor = mesh.vertexColors.size() > 0;
	std::vector<std::pair<int, float3>> faceVerts(mesh.vertexLocations.size(), std::pair<int, float3>(0, float3(0.0f)));
	std::vector<std::pair<int, float3>> edgeVerts(mesh.vertexLocations.size(), std::pair<int, float3>(0, float3(0.0f)));
	uint2 edge;
	uint fid = 0;
	for (const uint3& face : mesh.triIndexes.data) {
		edge = (face.x < face.y) ? uint2(face.x, face.y) : uint2(face.y, face.x);
		edges.insert(edge);
		edgeTable[faceHashCode(edge)].faces.push_back(fid);

		edge = (face.y < face.z) ? uint2(face.y, face.z) : uint2(face.z, face.y);
		edges.insert(edge);
		edgeTable[faceHashCode(edge)].faces.push_back(fid);

		edge = (face.z < face.x) ? uint2(face.z, face.x) : uint2(face.x, face.z);
		edges.insert(edge);
		edgeTable[faceHashCode(edge)].faces.push_back(fid);
		fid++;
	}
	for (const uint4& face : mesh.quadIndexes.data) {
		edge = (face.x < face.y) ? uint2(face.x, face.y) : uint2(face.y, face.x);
		edges.insert(edge);
		edgeTable[faceHashCode(edge)].faces.push_back(fid);

		edge = (face.y < face.z) ? uint2(face.y, face.z) : uint2(face.z, face.y);
		edges.insert(edge);
		edgeTable[faceHashCode(edge)].faces.push_back(fid);

		edge = (face.z < face.w) ? uint2(face.z, face.w) : uint2(face.w, face.z);
		edges.insert(edge);
		edgeTable[faceHashCode(edge)].faces.push_back(fid);

		edge = (face.w < face.x) ? uint2(face.w, face.x) : uint2(face.x, face.w);
		edges.insert(edge);
		edgeTable[faceHashCode(edge)].faces.push_back(fid);
		fid++;
	}
	for (std::pair<const uint64_t, FaceEdge>& edgeList : edgeTable) {
		if (edgeList.second.faces.size() == 2) {
			uint fid1 = edgeList.second.faces.front();
			uint fid2 = edgeList.second.faces.back();
			if (fid1 != fid2) {
				faceFaceNbrs[fid1].push_back(fid2);
				faceFaceNbrs[fid2].push_back(fid1);
			}
		}
	}
	size_t backIndex = mesh.vertexLocations.size();
	size_t endIndex = backIndex;
	mesh.vertexLocations.resize(endIndex + mesh.quadIndexes.size() + mesh.triIndexes.size() + edges.size());
	if (hasColor) {
		mesh.vertexColors.resize(endIndex + mesh.quadIndexes.size() + mesh.triIndexes.size() + edges.size());
	}
	for (const uint3& face : mesh.triIndexes.data) {
		float3 pt1 = mesh.vertexLocations[face.x];
		float3 pt2 = mesh.vertexLocations[face.y];
		float3 pt3 = mesh.vertexLocations[face.z];
		float3 avg = 0.33333333f * (pt1 + pt2 + pt3);
		for (int k = 0; k < 3; k++) {
			std::pair<int, float3>& pr = faceVerts[face[k]];
			pr.first++;
			pr.second += avg;
		}
		if (hasColor) {
			mesh.vertexColors[endIndex] = 0.3333333f * (mesh.vertexColors[face.x] + mesh.vertexColors[face.y] + mesh.vertexColors[face.z]);
		}
		mesh.vertexLocations[endIndex++] = avg;
	}
	for (const uint4& face : mesh.quadIndexes.data) {
		float3 pt1 = mesh.vertexLocations[face.x];
		float3 pt2 = mesh.vertexLocations[face.y];
		float3 pt3 = mesh.vertexLocations[face.z];
		float3 pt4 = mesh.vertexLocations[face.w];
		float3 avg = 0.25f * (pt1 + pt2 + pt3 + pt4);
		for (int k = 0; k < 4; k++) {
			std::pair<int, float3>& pr = faceVerts[face[k]];
			pr.first++;
			pr.second += avg;
		}
		if (hasColor) {
			mesh.vertexColors[endIndex] = 0.25f
					* (mesh.vertexColors[face.x] + mesh.vertexColors[face.y] + mesh.vertexColors[face.z] + mesh.vertexColors[face.w]);
		}
		mesh.vertexLocations[endIndex++] = avg;
	}
	for (uint2 edge : edges) {
		float3 pt1 = mesh.vertexLocations[edge.x];
		float3 pt2 = mesh.vertexLocations[edge.y];
		FaceEdge& fe = edgeTable[faceHashCode(edge)];
		fe.edgePointIndex = endIndex;
		float3 avg;
		if (fe.faces.size() < 2) {
			avg = 0.5f * (pt1 + pt2);
		} else {
			avg = 0.25f * (pt1 + pt2 + mesh.vertexLocations[fe.faces.front() + backIndex] + mesh.vertexLocations[fe.faces.back() + backIndex]);
		}
		for (int k = 0; k < 2; k++) {
			std::pair<int, float3>& pr = edgeVerts[edge[k]];
			pr.first++;
			pr.second += avg;
		}
		if (hasColor) {
			mesh.vertexColors[endIndex] = 0.5f * (mesh.vertexColors[edge.x] + mesh.vertexColors[edge.y]);
		}
		mesh.vertexLocations[endIndex++] = avg;

	}
	std::vector<uint4> newQuads(4 * mesh.quadIndexes.size() + 3 * mesh.triIndexes.size());
	std::vector<float2> uvs(newQuads.size() * 4);
	size_t faceIndex = 0;
	size_t uvIndex = 0;
	fid = 0;
	endIndex = backIndex;
	for (const uint3& face : mesh.triIndexes.data) {
		edge = (face.x < face.y) ? uint2(face.x, face.y) : uint2(face.y, face.x);
		size_t ept1 = edgeTable[faceHashCode(edge)].edgePointIndex;
		edge = (face.y < face.z) ? uint2(face.y, face.z) : uint2(face.z, face.y);
		size_t ept2 = edgeTable[faceHashCode(edge)].edgePointIndex;
		edge = (face.z < face.x) ? uint2(face.z, face.x) : uint2(face.x, face.z);
		size_t ept3 = edgeTable[faceHashCode(edge)].edgePointIndex;

		if (hasUVs) {
			float2 uv1 = mesh.textureMap[fid];
			float2 uv2 = mesh.textureMap[fid + 1];
			float2 uv3 = mesh.textureMap[fid + 2];
			float2 uva = 0.33333333f * (uv1 + uv2 + uv3);
			float2 upt1 = 0.5f * (uv1 + uv2);
			float2 upt2 = 0.5f * (uv2 + uv3);
			float2 upt3 = 0.5f * (uv3 + uv1);

			uvs[uvIndex++] = uv1;
			uvs[uvIndex++] = upt1;
			uvs[uvIndex++] = uva;
			uvs[uvIndex++] = upt3;

			uvs[uvIndex++] = uv2;
			uvs[uvIndex++] = upt2;
			uvs[uvIndex++] = uva;
			uvs[uvIndex++] = upt1;

			uvs[uvIndex++] = uv3;
			uvs[uvIndex++] = upt3;
			uvs[uvIndex++] = uva;
			uvs[uvIndex++] = upt2;
			fid += 3;
		}
		newQuads[faceIndex++] = uint4(face.x, (uint32_t) ept1, (uint32_t) endIndex, (uint32_t) ept3);
		newQuads[faceIndex++] = uint4(face.y, (uint32_t) ept2, (uint32_t) endIndex, (uint32_t) ept1);
		newQuads[faceIndex++] = uint4(face.z, (uint32_t) ept3, (uint32_t) endIndex, (uint32_t) ept2);
		endIndex++;
	}
	for (const uint4& face : mesh.quadIndexes.data) {
		edge = (face.x < face.y) ? uint2(face.x, face.y) : uint2(face.y, face.x);
		size_t ept1 = edgeTable[faceHashCode(edge)].edgePointIndex;
		edge = (face.y < face.z) ? uint2(face.y, face.z) : uint2(face.z, face.y);
		size_t ept2 = edgeTable[faceHashCode(edge)].edgePointIndex;
		edge = (face.z < face.w) ? uint2(face.z, face.w) : uint2(face.w, face.z);
		size_t ept3 = edgeTable[faceHashCode(edge)].edgePointIndex;
		edge = (face.w < face.x) ? uint2(face.w, face.x) : uint2(face.x, face.w);
		size_t ept4 = edgeTable[faceHashCode(edge)].edgePointIndex;
		if (hasUVs) {
			float2 uv1 = mesh.textureMap[fid];
			float2 uv2 = mesh.textureMap[fid + 1];
			float2 uv3 = mesh.textureMap[fid + 2];
			float2 uv4 = mesh.textureMap[fid + 3];
			float2 uva = 0.25f * (uv1 + uv2 + uv3 + uv4);
			float2 upt1 = 0.5f * (uv1 + uv2);
			float2 upt2 = 0.5f * (uv2 + uv3);
			float2 upt3 = 0.5f * (uv3 + uv4);
			float2 upt4 = 0.5f * (uv4 + uv1);

			uvs[uvIndex++] = uv1;
			uvs[uvIndex++] = upt1;
			uvs[uvIndex++] = uva;
			uvs[uvIndex++] = upt4;

			uvs[uvIndex++] = uv2;
			uvs[uvIndex++] = upt2;
			uvs[uvIndex++] = uva;
			uvs[uvIndex++] = upt1;

			uvs[uvIndex++] = uv3;
			uvs[uvIndex++] = upt3;
			uvs[uvIndex++] = uva;
			uvs[uvIndex++] = upt2;

			uvs[uvIndex++] = uv4;
			uvs[uvIndex++] = upt4;
			uvs[uvIndex++] = uva;
			uvs[uvIndex++] = upt3;
			fid += 4;
		}
		newQuads[faceIndex++] = uint4(face.x, (uint32_t) ept1, (uint32_t) endIndex, (uint32_t) ept4);
		newQuads[faceIndex++] = uint4(face.y, (uint32_t) ept2, (uint32_t) endIndex, (uint32_t) ept1);
		newQuads[faceIndex++] = uint4(face.z, (uint32_t) ept3, (uint32_t) endIndex, (uint32_t) ept2);
		newQuads[faceIndex++] = uint4(face.w, (uint32_t) ept4, (uint32_t) endIndex, (uint32_t) ept3);

		endIndex++;
	}
	for (int n = 0; n < (int) faceVerts.size(); n++) {
		float3 P = mesh.vertexLocations[n];
		int fcount = faceVerts[n].first;
		int ecount = edgeVerts[n].first;

		float3 F = faceVerts[n].second / (float) fcount;
		float3 R = edgeVerts[n].second / (float) ecount;

		mesh.vertexLocations[n] = (F + 2.0f * R + (ecount - 3.0f) * P) / (float) ecount;
	}
	if (hasUVs)
		mesh.textureMap = uvs;
	mesh.quadIndexes = newQuads;
	mesh.triIndexes.clear();
	if (mesh.vertexNormals.size() > 0)
		mesh.updateVertexNormals();
	mesh.setDirty(true);
}
void SubdivideLoop(Mesh& mesh) {
	mesh.convertQuadsToTriangles();
	std::map<uint64_t, FaceEdge> edgeTable;
	std::set<uint2> edges;
	uint2 edge;
	uint fid = 0;
	bool hasUVs = mesh.textureMap.size() > 0;
	bool hasColor = mesh.vertexColors.size() > 0;
	for (const uint3& face : mesh.triIndexes.data) {
		edge = (face.x < face.y) ? uint2(face.x, face.y) : uint2(face.y, face.x);
		edges.insert(edge);
		edgeTable[faceHashCode(edge)].faces.push_back(fid);

		edge = (face.y < face.z) ? uint2(face.y, face.z) : uint2(face.z, face.y);
		edges.insert(edge);
		edgeTable[faceHashCode(edge)].faces.push_back(fid);

		edge = (face.z < face.x) ? uint2(face.z, face.x) : uint2(face.x, face.z);
		edges.insert(edge);
		edgeTable[faceHashCode(edge)].faces.push_back(fid);
		fid++;
	}

	size_t backIndex = mesh.vertexLocations.size();
	size_t endIndex = backIndex;
	mesh.vertexLocations.resize(endIndex + edges.size());
	if (hasColor)
		mesh.vertexColors.resize(mesh.vertexLocations.size());
	for (uint2 edge : edges) {
		float3 pt1 = mesh.vertexLocations[edge.x];
		float3 pt2 = mesh.vertexLocations[edge.y];
		FaceEdge& fe = edgeTable[faceHashCode(edge)];
		fe.edgePointIndex = endIndex;
		float3 avg;
		if (fe.faces.size() < 2) {
			avg = 0.5f * (pt1 + pt2);
		} else {
			uint3 face = mesh.triIndexes[fe.faces.front()];
			int other1 = -1, other2 = -1;
			if ((face.x == edge.x && face.y == edge.y) || (face.y == edge.x && face.x == edge.y)) {
				other1 = face.z;
			} else if ((face.y == edge.x && face.z == edge.y) || (face.z == edge.x && face.y == edge.y)) {
				other1 = face.x;
			} else if ((face.x == edge.x && face.z == edge.y) || (face.z == edge.x && face.x == edge.y)) {
				other1 = face.y;
			}
			face = mesh.triIndexes[fe.faces.back()];
			if ((face.x == edge.x && face.y == edge.y) || (face.y == edge.x && face.x == edge.y)) {
				other2 = face.z;
			} else if ((face.y == edge.x && face.z == edge.y) || (face.z == edge.x && face.y == edge.y)) {
				other2 = face.x;
			} else if ((face.x == edge.x && face.z == edge.y) || (face.z == edge.x && face.x == edge.y)) {
				other2 = face.y;
			}
			if (other1 >= 0 && other2 >= 0) {
				avg = 0.125f * (3.0f * pt1 + 3.0f * pt2 + mesh.vertexLocations[other1] + mesh.vertexLocations[other2]);
			} else {
				avg = 0.5f * (pt1 + pt2);
			}
		}
		if (hasColor) {
			mesh.vertexColors[endIndex] = 0.5f * (mesh.vertexColors[edge.x] + mesh.vertexColors[edge.y]);
		}
		mesh.vertexLocations[endIndex++] = avg;
	}
	std::vector<uint3> newTris(4 * mesh.triIndexes.size());
	size_t faceIndex = 0;
	endIndex = backIndex;
	fid = 0;
	std::vector<float2> uvs(newTris.size() * 3);
	size_t uvIndex = 0;
	for (const uint3& face : mesh.triIndexes.data) {
		edge = (face.x < face.y) ? uint2(face.x, face.y) : uint2(face.y, face.x);
		size_t ept1 = edgeTable[faceHashCode(edge)].edgePointIndex;
		edge = (face.y < face.z) ? uint2(face.y, face.z) : uint2(face.z, face.y);
		size_t ept2 = edgeTable[faceHashCode(edge)].edgePointIndex;
		edge = (face.z < face.x) ? uint2(face.z, face.x) : uint2(face.x, face.z);
		size_t ept3 = edgeTable[faceHashCode(edge)].edgePointIndex;
		if (hasUVs) {
			float2 uv1 = mesh.textureMap[fid];
			float2 uv2 = mesh.textureMap[fid + 1];
			float2 uv3 = mesh.textureMap[fid + 2];
			float2 upt1 = 0.5f * (uv1 + uv2);
			float2 upt2 = 0.5f * (uv2 + uv3);
			float2 upt3 = 0.5f * (uv3 + uv1);
			uvs[uvIndex++] = uv1;
			uvs[uvIndex++] = upt1;
			uvs[uvIndex++] = upt3;

			uvs[uvIndex++] = uv2;
			uvs[uvIndex++] = upt2;
			uvs[uvIndex++] = upt1;

			uvs[uvIndex++] = uv3;
			uvs[uvIndex++] = upt3;
			uvs[uvIndex++] = upt2;

			uvs[uvIndex++] = upt1;
			uvs[uvIndex++] = upt2;
			uvs[uvIndex++] = upt3;

			fid += 3;
		}
		newTris[faceIndex++] = uint3(face.x, (uint32_t) ept1, (uint32_t) ept3);
		newTris[faceIndex++] = uint3(face.y, (uint32_t) ept2, (uint32_t) ept1);
		newTris[faceIndex++] = uint3(face.z, (uint32_t) ept3, (uint32_t) ept2);
		newTris[faceIndex++] = uint3((uint32_t) ept1, (uint32_t) ept2, (uint32_t) ept3);
	}
	std::vector<std::set<uint32_t>> vertNbrs(mesh.vertexLocations.size());
	for (const uint3& face : mesh.triIndexes.data) {
		vertNbrs[face.x].insert(face.y);
		vertNbrs[face.y].insert(face.z);
		vertNbrs[face.z].insert(face.x);
		vertNbrs[face.z].insert(face.y);
		vertNbrs[face.y].insert(face.x);
		vertNbrs[face.x].insert(face.z);
	}
	mesh.triIndexes = newTris;
	const int MAX_VALENCE = 32;
	static std::vector<float> valenceWeights;
	if (valenceWeights.size() == 0) {
		valenceWeights.resize(MAX_VALENCE);
		for (int i = 1; i < MAX_VALENCE; i++) {
			float x = 3 / 8.0f + 0.25f * std::cos(2.0f * ALY_PI / i);
			float beta = (5 / 8.0f - x * x) / i;
			valenceWeights[i] = beta;
		}
	}

	for (int n = 0; n < (int) backIndex; n++) {
		int N = (int) vertNbrs[n].size();
		if (N > 0 && N < MAX_VALENCE) {
			float beta = valenceWeights[N];
			float alpha = (1 - N * beta);
			float3 pt = alpha * mesh.vertexLocations[n];
			for (uint32_t nbr : vertNbrs[n]) {
				pt += beta * mesh.vertexLocations[nbr];
			}
			mesh.vertexLocations[n] = pt;
		}
	}
	if (hasUVs)
		mesh.textureMap = uvs;
	if (mesh.vertexNormals.size() > 0)
		mesh.updateVertexNormals();
	mesh.setDirty(true);
}
void Subdivide(Mesh& mesh, SubDivisionScheme type) {
	if (type == SubDivisionScheme::CatmullClark) {
		SubdivideCatmullClark(mesh);
	} else if (type == SubDivisionScheme::Loop) {
		SubdivideLoop(mesh);
	}
}
} /* namespace imagesci */
