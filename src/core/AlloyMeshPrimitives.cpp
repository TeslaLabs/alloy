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
#include "AlloyMeshPrimitives.h"
namespace aly {
	Plane::Plane(float w, float h, std::shared_ptr<AlloyContext>& context) :Mesh(context) {
		vertexLocations.push_back(float3(-0.5f*w, -0.5f*h, 0.0f));
		vertexLocations.push_back(float3(-0.5f*w, +0.5f*h, 0.0f));
		vertexLocations.push_back(float3(+0.5f*w, +0.5f*h, 0.0f));
		vertexLocations.push_back(float3(+0.5f*w, -0.5f*h, 0.0f));
		vertexNormals.push_back(float3(0, 0, 1));
		vertexNormals.push_back(float3(0, 0, 1));
		vertexNormals.push_back(float3(0, 0, 1));
		vertexNormals.push_back(float3(0, 0, 1));
		quadIndexes.push_back(uint4(0, 1, 2, 3));
		boundingBox = box3f(float3(-0.5f*w, -0.5f*h, 0.0f), float3(w, h, 0));
		setDirty(true);
	}
	Grid::Grid(float w, float h, int rows, int cols, std::shared_ptr<AlloyContext>& context) :Mesh(context) {
		float cellX = w / rows;
		float cellY = h / cols;
		float offX = -w*0.5f;
		float offY = -h*0.5f;
		for (int j = 0; j <= cols; j++) {
			for (int i = 0; i <= rows; i++) {
				vertexLocations.push_back(float3(offX + cellX*i, offY + cellY*j, 0.0f));
				vertexNormals.push_back(float3(0, 0, 1));
			}
		}
		for (int j = 0; j < cols; j++) {
			for (int i = 0; i < rows; i++) {
				uint32_t v0 = j*(rows+1) + i;
				uint32_t v1 = j*(rows + 1) + i + 1;
				uint32_t v2 = (j + 1)*(rows + 1) + i + 1;
				uint32_t v3 = (j + 1)*(rows + 1) + i;
				quadIndexes.push_back(uint4(v3, v2, v1, v0));
			}
		}
		boundingBox = box3f(float3(-0.5f*w, -0.5f*h, 0.0f), float3(w, h, 0));
		setDirty(true);
	}
	Asteroid::Asteroid(int subdivisions, std::shared_ptr<AlloyContext>& context) :Icosahedron(1.0f, context) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> noise(0.0f, 1.0f);
		for (int c = 0; c <= subdivisions; c++) {
			Subdivide(*this, SubDivisionScheme::Loop);
			updateVertexNormals();
			int N = (int)vertexLocations.size();
			for (int n = 0; n < N; n++) {
				float3 norm = -vertexNormals[n];
				vertexLocations[n] += 0.6f * norm* (noise(gen) - 0.3f) / (float)(1 << c);
			}
		}
		updateVertexNormals();
		updateBoundingBox();
		setDirty(true);
	}
	Pyramid::Pyramid(float w, float h, float d, std::shared_ptr<AlloyContext>& context) :Mesh(context) {
		vertexLocations.push_back(float3(-0.5f*w, -0.5f*h, 0.0f));
		vertexLocations.push_back(float3(-0.5f*w, +0.5f*h, 0.0f));
		vertexLocations.push_back(float3(+0.5f*w, +0.5f*h, 0.0f));
		vertexLocations.push_back(float3(+0.5f*w, -0.5f*h, 0.0f));
		vertexLocations.push_back(float3(0.0f, 0.0f, d));
		vertexNormals.push_back(float3(0, 0, -1));
		vertexNormals.push_back(float3(0, 0, -1));
		vertexNormals.push_back(float3(0, 0, -1));
		vertexNormals.push_back(float3(0, 0, -1));
		vertexNormals.push_back(float3(0, 0, 1));
		quadIndexes.push_back(uint4(0, 1, 2, 3));
		triIndexes.push_back(uint3(3, 2, 4));
		triIndexes.push_back(uint3(2, 1, 4));
		triIndexes.push_back(uint3(1, 0, 4));
		triIndexes.push_back(uint3(0, 3, 4));
		boundingBox = box3f(float3(-0.5f*w, -0.5f*h, 0.0f), float3(w, h, d));
		setDirty(true);
	}
	Frustum::Frustum(const CameraParameters& cam, std::shared_ptr<AlloyContext>& context) :Mesh(context) {

		float3 pt1 = cam.transformNormalizedImageDepthToWorld(float3(0, 0, 0));
		float3 pt2 = cam.transformNormalizedImageDepthToWorld(float3(1, 0, 0));
		float3 pt3 = cam.transformNormalizedImageDepthToWorld(float3(1, 1, 0));
		float3 pt4 = cam.transformNormalizedImageDepthToWorld(float3(0, 1, 0));

		float3 qt1 = cam.transformNormalizedImageDepthToWorld(float3(0, 0, 1));
		float3 qt2 = cam.transformNormalizedImageDepthToWorld(float3(1, 0, 1));
		float3 qt3 = cam.transformNormalizedImageDepthToWorld(float3(1, 1, 1));
		float3 qt4 = cam.transformNormalizedImageDepthToWorld(float3(0, 1, 1));

		vertexLocations.push_back(pt1);
		vertexLocations.push_back(pt2);
		vertexLocations.push_back(pt3);
		vertexLocations.push_back(pt4);

		vertexLocations.push_back(qt1);
		vertexLocations.push_back(qt2);
		vertexLocations.push_back(qt3);
		vertexLocations.push_back(qt4);

		quadIndexes.push_back(uint4(3, 2, 1, 0));
		quadIndexes.push_back(uint4(6, 7, 4, 5));
		quadIndexes.push_back(uint4(5, 1, 2, 6));
		quadIndexes.push_back(uint4(0, 4, 7, 3));
		quadIndexes.push_back(uint4(6, 2, 3, 7));
		quadIndexes.push_back(uint4(4, 0, 1, 5));

		updateVertexNormals();
		updateBoundingBox();
		setDirty(true);
	}
	Box::Box(const box3f& box, std::shared_ptr<AlloyContext>& context) :Mesh(context) {
		float3 minPt = box.min();
		float3 maxPt = box.max();
		vertexLocations.push_back(float3(minPt.x, minPt.y, minPt.z));
		vertexLocations.push_back(float3(maxPt.x, minPt.y, minPt.z));
		vertexLocations.push_back(float3(maxPt.x, maxPt.y, minPt.z));
		vertexLocations.push_back(float3(minPt.x, maxPt.y, minPt.z));
		vertexLocations.push_back(float3(minPt.x, minPt.y, maxPt.z));
		vertexLocations.push_back(float3(maxPt.x, minPt.y, maxPt.z));
		vertexLocations.push_back(float3(maxPt.x, maxPt.y, maxPt.z));
		vertexLocations.push_back(float3(minPt.x, maxPt.y, maxPt.z));


		quadIndexes.push_back(uint4(3, 2, 1, 0));
		quadIndexes.push_back(uint4(6, 7, 4, 5));
		quadIndexes.push_back(uint4(5, 1, 2, 6));
		quadIndexes.push_back(uint4(0, 4, 7, 3));
		quadIndexes.push_back(uint4(6, 2, 3, 7));
		quadIndexes.push_back(uint4(4, 0, 1, 5));

		updateVertexNormals();
		boundingBox = box;
		setDirty(true);
	}
	Sphere::Sphere(float radius, int slices, int stacks, std::shared_ptr<AlloyContext>& context) :Mesh(context) {
		std::vector<std::vector<uint32_t>> rings(stacks - 1, std::vector<uint32_t>(slices));
		vertexLocations.push_back(float3(0.0f, 0.0f, radius));
		vertexNormals.push_back(float3(0.0f, 0.0f, 1.0f));
		uint32_t idx = 1;
		for (int i = 1; i < stacks; i++)
		{
			float cosi = std::cos(ALY_PI*i / (float)(stacks));
			float sini = std::sin(ALY_PI*i / (float)(stacks));
			for (int j = 0; j < slices; j++)
			{
				float cosj = std::cos(2 * ALY_PI*j / (float)slices);
				float sinj = std::sin(2 * ALY_PI*j / (float)slices);
				float x = cosj * sini;
				float y = sinj * sini;
				float z = cosi;
				rings[i - 1][j] = idx;
				vertexLocations.push_back(radius*float3(x, y, z));
				vertexNormals.push_back(float3(x, y, z));
				idx++;
			}
		}
		vertexLocations.push_back(float3(0.0f, 0.0f, -radius));
		vertexNormals.push_back(float3(0.0f, 0.0f, -1.0f));
		uint32_t v0, v1, v2, v3;
		for (int j = 0; j < slices; j++)
		{
			v0 = 0;
			v2 = rings[0][(j + 1) % slices];
			v1 = rings[0][j];
			triIndexes.push_back(uint3(v0, v1, v2));
			v0 = idx;
			v2 = rings[stacks - 2][(j + 1) % slices];
			v1 = rings[stacks - 2][j];
			triIndexes.push_back(uint3(v1, v0, v2));

			for (int i = 0;i < stacks - 2;i++) {
				v0 = rings[i][j];
				v1 = rings[i][(j + 1) % slices];
				v2 = rings[i + 1][j];
				v3 = rings[i + 1][(j + 1) % slices];
				quadIndexes.push_back(uint4(v0, v2, v3, v1));
			}
		}
		boundingBox = box3f(float3(-radius, -radius, -radius), float3(2 * radius, 2 * radius, 2 * radius));
		setDirty(true);
	}
	Cylinder::Cylinder(float radius, float height, int slices, std::shared_ptr<AlloyContext>& context) :Mesh(context) {
		vertexLocations.push_back(float3(0.0f, 0.0f, 0.5f*height));
		vertexNormals.push_back(float3(0.0f, 0.0f, 1.0f));
		std::vector<uint32_t> topRing(slices);
		std::vector<uint32_t> bottomRing(slices);
		uint32_t idx = 1;
		for (int j = 0; j < slices; j++)
		{
			float cosj = std::cos(2 * ALY_PI*j / (float)slices);
			float sinj = std::sin(2 * ALY_PI*j / (float)slices);
			float x = cosj;
			float y = sinj;
			float z = 0;
			vertexLocations.push_back(float3(radius*x, radius*y, 0.5f*height));
			vertexNormals.push_back(float3(x, y, z));
			topRing[j] = idx;
			idx++;
			vertexLocations.push_back(float3(radius*x, radius*y, -0.5f*height));
			vertexNormals.push_back(float3(x, y, z));
			bottomRing[j] = idx;
			idx++;
		}
		vertexLocations.push_back(float3(0.0f, 0.0f, -0.5f* height));
		vertexNormals.push_back(float3(0.0f, 0.0f, -1.0f));
		uint32_t v0, v1, v2, v3;
		for (int j = 0; j < slices; j++)
		{
			v0 = 0;
			v1 = topRing[j];
			v2 = topRing[(j + 1) % slices];
			triIndexes.push_back(uint3(v0, v1, v2));
			v0 = idx;
			v1 = bottomRing[j];
			v2 = bottomRing[(j + 1) % slices];
			triIndexes.push_back(uint3(v1, v0, v2));

			v0 = topRing[j];
			v1 = topRing[(j + 1) % slices];
			v2 = bottomRing[j];
			v3 = bottomRing[(j + 1) % slices];
			quadIndexes.push_back(uint4(v0, v2, v3, v1));

		}
		boundingBox = box3f(float3(-radius, -radius, -0.5f*height), float3(2 * radius, 2 * radius, height));

		setDirty(true);
	}
	Torus::Torus(float innerRadius, float outerRadius, int stacks, int slices, std::shared_ptr<AlloyContext>& context) :Mesh(context) {
		float r = 0.5f*(outerRadius - innerRadius);
		float R = 0.5f*(outerRadius + innerRadius);
		std::vector<std::vector<uint32_t>> rings(stacks, std::vector<uint32_t>(slices));
		uint32_t idx = 0;
		for (int i = 0; i < stacks; i++)
		{
			float cosi = std::cos(2 * ALY_PI*i / (float)(stacks));
			float sini = std::sin(2 * ALY_PI*i / (float)(stacks));
			for (int j = 0; j < slices; j++)
			{
				float cosj = std::cos(2 * ALY_PI*j / (float)slices);
				float sinj = std::sin(2 * ALY_PI*j / (float)slices);
				float x = cosi*(R + r*sinj);
				float y = sini*(R + r*sinj);
				float z = r*cosj;
				rings[i][j] = idx;
				vertexLocations.push_back(float3(x, y, z));
				vertexNormals.push_back(float3(cosi*sinj, sini*sinj, cosj));
				idx++;
			}
		}
		uint32_t v0, v1, v2, v3;
		for (int i = 0;i < stacks;i++) {
			for (int j = 0; j < slices; j++)
			{
				v0 = rings[i][j];
				v1 = rings[i][(j + 1) % slices];
				v2 = rings[(i + 1) % stacks][j];
				v3 = rings[(i + 1) % stacks][(j + 1) % slices];
				quadIndexes.push_back(uint4(v0, v1, v3, v2));
			}
		}
		boundingBox = box3f(float3(-outerRadius, -outerRadius, -r), float3(2 * outerRadius, 2 * outerRadius, 2 * r));
		setDirty(true);
	}
	Cone::Cone(float radius, float height, int slices, std::shared_ptr<AlloyContext>& context) :Mesh(context) {
		vertexLocations.push_back(float3(0.0f, 0.0f, 0.5f*height));
		vertexNormals.push_back(float3(0.0f, 0.0f, 1.0f));
		std::vector<uint32_t> bottomRing(slices);
		uint32_t idx = 1;
		for (int j = 0; j < slices; j++)
		{
			float cosj = std::cos(2 * ALY_PI*j / (float)slices);
			float sinj = std::sin(2 * ALY_PI*j / (float)slices);
			float x = cosj;
			float y = sinj;
			float z = 0;
			vertexLocations.push_back(float3(radius*x, radius*y, -0.5f*height));
			vertexNormals.push_back(float3(x, y, z));
			bottomRing[j] = idx;
			idx++;
		}
		vertexLocations.push_back(float3(0.0f, 0.0f, -0.5f* height));
		vertexNormals.push_back(float3(0.0f, 0.0f, -1.0f));
		uint32_t v0, v1, v2, v3;
		for (int j = 0; j < slices; j++)
		{
			v0 = 0;
			v1 = bottomRing[j];
			v2 = bottomRing[(j + 1) % slices];
			triIndexes.push_back(uint3(v0, v1, v2));
			v0 = idx;
			triIndexes.push_back(uint3(v1, v0, v2));

			v2 = bottomRing[j];
			v3 = bottomRing[(j + 1) % slices];
			quadIndexes.push_back(uint4(v0, v2, v3, v1));

		}
		boundingBox = box3f(float3(-radius, -radius, -0.5f*height), float3(2 * radius, 2 * radius, height));
		setDirty(true);
	}
	Icosahedron::Icosahedron(float radius, std::shared_ptr<AlloyContext>& context) :Mesh(context) {
		vertexLocations.push_back(float3(0.0f, 0.0f, -1.0f));
		vertexLocations.push_back(float3(0.7236f, -0.52572f, -0.447215f));
		vertexLocations.push_back(float3(-0.276385f, -0.85064f, -0.447215f));
		vertexLocations.push_back(float3(0.7236f, 0.52572f, -0.447215f));
		vertexLocations.push_back(float3(-0.894425f, 0.0f, -0.447215f));
		vertexLocations.push_back(float3(-0.276385f, 0.85064f, -0.447215f));
		vertexLocations.push_back(float3(-0.7236f, -0.52572f, 0.447215f));
		vertexLocations.push_back(float3(0.894425f, 0.0f, 0.447215f));
		vertexLocations.push_back(float3(0.276385f, -0.85064f, 0.447215f));
		vertexLocations.push_back(float3(-0.7236f, 0.52572f, 0.447215f));
		vertexLocations.push_back(float3(0.276385f, 0.85064f, 0.447215f));
		vertexLocations.push_back(float3(0.0f, 0.0f, 1.0f));
		vertexLocations *= float3(radius);
		triIndexes.push_back(uint3(0, 1, 2));
		triIndexes.push_back(uint3(1, 0, 3));
		triIndexes.push_back(uint3(0, 2, 4));
		triIndexes.push_back(uint3(0, 4, 5));
		triIndexes.push_back(uint3(0, 5, 3));
		triIndexes.push_back(uint3(1, 3, 7));
		triIndexes.push_back(uint3(2, 1, 8));
		triIndexes.push_back(uint3(4, 2, 6));
		triIndexes.push_back(uint3(5, 4, 9));
		triIndexes.push_back(uint3(3, 5, 10));
		triIndexes.push_back(uint3(1, 7, 8));
		triIndexes.push_back(uint3(2, 8, 6));
		triIndexes.push_back(uint3(4, 6, 9));
		triIndexes.push_back(uint3(5, 9, 10));
		triIndexes.push_back(uint3(3, 10, 7));
		triIndexes.push_back(uint3(8, 7, 11));
		triIndexes.push_back(uint3(6, 8, 11));
		triIndexes.push_back(uint3(9, 6, 11));
		triIndexes.push_back(uint3(10, 9, 11));
		triIndexes.push_back(uint3(7, 10, 11));
		updateVertexNormals();

		boundingBox = box3f(float3(-radius, -radius, -radius), float3(2 * radius, 2 * radius, 2 * radius));
		setDirty(true);

	}
	Capsule::Capsule(float radius, float height, int slices, int stacks, std::shared_ptr<AlloyContext>& context) :Mesh(context) {

		if (stacks % 2 == 0)stacks++;//Make odd
		std::vector<std::vector<uint32_t>> rings(stacks - 1, std::vector<uint32_t>(slices));
		height -= 2 * radius;
		vertexLocations.push_back(float3(0.0f, 0.0f, radius + 0.5f*height));
		vertexNormals.push_back(float3(0.0f, 0.0f, 1.0f));
		uint32_t idx = 1;
		for (int i = 1; i < stacks; i++)
		{
			float cosi, sini;
			cosi = std::cos(ALY_PI*i / (float)(stacks));
			sini = std::sin(ALY_PI*i / (float)(stacks));
			for (int j = 0; j < slices; j++)
			{
				float cosj = std::cos(2 * ALY_PI*j / (float)slices);
				float sinj = std::sin(2 * ALY_PI*j / (float)slices);
				float x = cosj * sini;
				float y = sinj * sini;
				float z = cosi;
				rings[i - 1][j] = idx;
				vertexLocations.push_back(float3(radius*x, radius*y, radius*z + ((i <= stacks / 2) ? 0.5f*height : -0.5f*height)));
				vertexNormals.push_back(float3(x, y, z));
				idx++;
			}
		}
		vertexLocations.push_back(float3(0.0f, 0.0f, -radius - 0.5f*height));
		vertexNormals.push_back(float3(0.0f, 0.0f, -1.0f));
		uint32_t v0, v1, v2, v3;
		for (int j = 0; j < slices; j++)
		{
			v0 = 0;
			v2 = rings[0][(j + 1) % slices];
			v1 = rings[0][j];
			triIndexes.push_back(uint3(v0, v1, v2));
			v0 = idx;
			v2 = rings[stacks - 2][(j + 1) % slices];
			v1 = rings[stacks - 2][j];
			triIndexes.push_back(uint3(v1, v0, v2));

			for (int i = 0;i < stacks - 2;i++) {
				v0 = rings[i][j];
				v1 = rings[i][(j + 1) % slices];
				v2 = rings[i + 1][j];
				v3 = rings[i + 1][(j + 1) % slices];
				quadIndexes.push_back(uint4(v0, v2, v3, v1));
			}
		}
		boundingBox = box3f(float3(-radius, -radius, -0.5f*height), float3(2 * radius, 2 * radius, height));
		setDirty(true);
	}

	TessellatedSphere::TessellatedSphere(float radius, int subdivisions, const SubDivisionScheme& scheme, std::shared_ptr<AlloyContext>& context) :Mesh(context) {
		if (scheme == SubDivisionScheme::Loop) {
			vertexLocations.push_back(float3(0.0f, 0.0f, -1.0f));
			vertexLocations.push_back(float3(0.7236f, -0.52572f, -0.447215f));
			vertexLocations.push_back(float3(-0.276385f, -0.85064f, -0.447215f));
			vertexLocations.push_back(float3(0.7236f, 0.52572f, -0.447215f));
			vertexLocations.push_back(float3(-0.894425f, 0.0f, -0.447215f));
			vertexLocations.push_back(float3(-0.276385f, 0.85064f, -0.447215f));
			vertexLocations.push_back(float3(-0.7236f, -0.52572f, 0.447215f));
			vertexLocations.push_back(float3(0.894425f, 0.0f, 0.447215f));
			vertexLocations.push_back(float3(0.276385f, -0.85064f, 0.447215f));
			vertexLocations.push_back(float3(-0.7236f, 0.52572f, 0.447215f));
			vertexLocations.push_back(float3(0.276385f, 0.85064f, 0.447215f));
			vertexLocations.push_back(float3(0.0f, 0.0f, 1.0f));

			triIndexes.push_back(uint3(0, 1, 2));
			triIndexes.push_back(uint3(1, 0, 3));
			triIndexes.push_back(uint3(0, 2, 4));
			triIndexes.push_back(uint3(0, 4, 5));
			triIndexes.push_back(uint3(0, 5, 3));
			triIndexes.push_back(uint3(1, 3, 7));
			triIndexes.push_back(uint3(2, 1, 8));
			triIndexes.push_back(uint3(4, 2, 6));
			triIndexes.push_back(uint3(5, 4, 9));
			triIndexes.push_back(uint3(3, 5, 10));
			triIndexes.push_back(uint3(1, 7, 8));
			triIndexes.push_back(uint3(2, 8, 6));
			triIndexes.push_back(uint3(4, 6, 9));
			triIndexes.push_back(uint3(5, 9, 10));
			triIndexes.push_back(uint3(3, 10, 7));
			triIndexes.push_back(uint3(8, 7, 11));
			triIndexes.push_back(uint3(6, 8, 11));
			triIndexes.push_back(uint3(9, 6, 11));
			triIndexes.push_back(uint3(10, 9, 11));
			triIndexes.push_back(uint3(7, 10, 11));
		}
		else if (scheme == SubDivisionScheme::CatmullClark) {
			vertexLocations.push_back(float3(-1, -1, -1));
			vertexLocations.push_back(float3(+1, -1, -1));
			vertexLocations.push_back(float3(+1, +1, -1));
			vertexLocations.push_back(float3(-1, +1, -1));
			vertexLocations.push_back(float3(-1, -1, +1));
			vertexLocations.push_back(float3(+1, -1, +1));
			vertexLocations.push_back(float3(+1, +1, +1));
			vertexLocations.push_back(float3(-1, +1, +1));

			quadIndexes.push_back(uint4(3, 2, 1, 0));
			quadIndexes.push_back(uint4(6, 7, 4, 5));
			quadIndexes.push_back(uint4(5, 1, 2, 6));
			quadIndexes.push_back(uint4(0, 4, 7, 3));
			quadIndexes.push_back(uint4(6, 2, 3, 7));
			quadIndexes.push_back(uint4(4, 0, 1, 5));
			subdivisions++;
		}
		for (int n = 0;n < subdivisions;n++) {
			Subdivide(*this, scheme);
		}
		vertexNormals.resize(vertexLocations.size());
		for (int n = 0;n < (int)vertexLocations.size();n++) {
			float3 pt = vertexLocations[n];
			pt = normalize(pt);
			vertexLocations[n] = radius*pt;
			vertexNormals[n] = pt;
		}
		boundingBox = box3f(float3(-radius, -radius, -radius), float3(2 * radius, 2 * radius, 2 * radius));
		setDirty(true);

	}
}