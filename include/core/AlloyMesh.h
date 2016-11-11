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
#ifndef ALLOYMESH_H_
#define ALLOYMESH_H_

#include <mutex>
#include "GLComponent.h"
#include "AlloyMath.h"
#include "AlloyVector.h"
#include "AlloyImage.h"
#include "AlloyContext.h"
#include <vector>
#include <set>
#include <list>
namespace aly {
	bool SANITY_CHECK_SUBDIVIDE();
class Mesh;
enum class SubDivisionScheme {
	CatmullClark,Loop
};
struct GLMesh: public GLComponent {
public:
	enum class PrimitiveType {
		ALL = 0, QUADS = 4, TRIANGLES = 3, POINTS = 1
	};
	Mesh& mesh;
	GLuint vao;
	GLuint vertexBuffer;
	GLuint normalBuffer;
	GLuint colorBuffer;
	GLuint triIndexBuffer;
	GLuint quadIndexBuffer;
	GLuint triColorBuffer[3];
	GLuint triVertexBuffer[3];
	GLuint quadVertexBuffer[4];
	GLuint triNormalBuffer[3];
	GLuint quadColorBuffer[4];
	GLuint quadNormalBuffer[4];
	GLuint triTextureBuffer[3];
	GLuint quadTextureBuffer[4];
	GLuint triCount;
	GLuint quadCount;
	GLuint vertexCount;
	GLuint triIndexCount;
	GLuint quadIndexCount;

	virtual void draw() const override;
	virtual void draw(const PrimitiveType& type,bool forceVertexColor) const;
	virtual void update() override;
	GLMesh(Mesh& mesh,bool onScreen,const std::shared_ptr<AlloyContext>& context =
			AlloyDefaultContext());
	GLMesh(Mesh& mesh,bool onScreen);
	virtual ~GLMesh();
};
class Mesh {
private:
	bool dirtyOnScreen = false;
	bool dirtyOffScreen = false;
protected:
	box3f boundingBox;
public:
	friend struct GLMesh;

	Vector3f vertexLocations;
	Vector3f vertexNormals;
	Vector4f vertexColors;

	Vector4ui quadIndexes;
	Vector3ui triIndexes;

	Vector2f textureMap;
	Image4f textureImage;

	std::shared_ptr<GLMesh> glOnScreen;
	std::shared_ptr<GLMesh> glOffScreen;
	float4x4 pose;

	inline void clone(Mesh& mesh) const {
		mesh.boundingBox = boundingBox;
		mesh.vertexLocations = vertexLocations;
		mesh.vertexNormals = vertexNormals;
		mesh.vertexColors = vertexColors;

		mesh.quadIndexes = quadIndexes;
		mesh.triIndexes = triIndexes;

		mesh.textureMap = textureMap;
		mesh.textureImage = textureImage;
		mesh.pose = pose;
		mesh.dirtyOnScreen = true;
		mesh.dirtyOffScreen = true;

	}
	bool isEmpty()const {
		return (vertexLocations.size()==0&&vertexNormals.size()==0&&vertexColors.size()==0&&quadIndexes.size()==0&&triIndexes.size()==0&&textureMap.size()==0);
	}
	template<class Archive> void serialize(Archive & archive) {
		archive(CEREAL_NVP(pose), CEREAL_NVP(vertexLocations),
				CEREAL_NVP(vertexNormals), CEREAL_NVP(vertexColors),
				CEREAL_NVP(quadIndexes), CEREAL_NVP(triIndexes),
				CEREAL_NVP(textureMap), CEREAL_NVP(textureImage));
	}
	void setContext(const std::shared_ptr<AlloyContext>& context);
	Mesh(const std::shared_ptr<AlloyContext>& context);
	Mesh();
	inline box3f getBoundingBox() const {
		return boundingBox;
	}
	virtual void draw(const GLMesh::PrimitiveType& type, bool onScreen, bool froceVertexColor);
	box3f updateBoundingBox();
	void scale(float sc);
	void transform(const float4x4& M);
	float estimateVoxelSize(int stride = 1);
	void update(bool onScreen=true);
	void clear();
	void setDirty(bool d) {
		this->dirtyOnScreen = d;
		this->dirtyOffScreen = d;
	}
	void setDirty(bool onScreen,bool d) {
		if (onScreen) {
			this->dirtyOnScreen = d;
		}
		else {
			this->dirtyOffScreen = d;
		}
	}
	inline bool isDirty(bool onScreen) const {
		return (onScreen)?dirtyOnScreen:dirtyOffScreen;
	}
	bool load(const std::string& file);
	void updateVertexNormals(int SMOOTH_ITERATIONS = 0, float DOT_TOLERANCE =
			0.75f);
	bool convertQuadsToTriangles();
	void mapIntoBoundingBox(float voxelSize);
	void mapOutOfBoundingBox(float voxelSize);
	bool save(const std::string& file);
	virtual ~Mesh();
};
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const Mesh & m) {
	ss << "Mesh: " << std::endl;
	ss << "\tBounds: " << m.getBoundingBox() << std::endl;
	ss << "\tVertex Locations: " << m.vertexLocations.size() << std::endl;
	ss << "\tVertex Normals: " << m.vertexNormals.size() << std::endl;
	ss << "\tVertex Colors: " << m.vertexColors.size() << std::endl;
	ss << "\tQuad Faces: " << m.quadIndexes.size() << std::endl;
	ss << "\tTriangle Faces: " << m.triIndexes.size() << std::endl;
	ss << "\tUVs: " << m.textureMap.size() << std::endl;
	ss << "\tTexture: " << m.textureImage << std::endl;
	ss << "\tPose: " << m.pose << std::endl;
	return ss;
}
void ReadMeshFromFile(const std::string& file, Mesh& mesh);
void ReadPlyMeshFromFile(const std::string& file, Mesh& mesh);
void ReadObjMeshFromFile(const std::string& file, std::vector<Mesh>& mesh);
void ReadObjMeshFromFile(const std::string& file, Mesh& mesh);
void WritePlyMeshToFile(const std::string& file, const Mesh& mesh, bool binary =
		true);
void WriteMeshToFile(const std::string& file, const Mesh& mesh);
void WriteObjMeshToFile(const std::string& file,const Mesh& mesh);
typedef std::vector<std::set<uint32_t>> MeshSetNeighborTable;
typedef std::vector<std::list<uint32_t>> MeshListNeighborTable;
void CreateVertexNeighborTable(const Mesh& mesh, MeshSetNeighborTable& vertNbrs);
void CreateOrderedVertexNeighborTable(const Mesh& mesh,
	MeshListNeighborTable& vertNbrs, bool leaveTail = false);
void CreateFaceNeighborTable(const Mesh& mesh, MeshListNeighborTable& faceNbrs);
void Subdivide(Mesh& mesh, SubDivisionScheme type= SubDivisionScheme::CatmullClark);
}
#endif /* MESH_H_ */
