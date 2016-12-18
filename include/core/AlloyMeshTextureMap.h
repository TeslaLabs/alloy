/*
* Copyright(C) 2016, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
#ifndef INCLUDE_MESHPARAETERIZATION_H
#define INCLUDE_MESHPARAETERIZATION_H
#include "AlloyMath.h"
#include "AlloyMesh.h"
#include "AlloyLocator.h"
#include <map>
#include <functional>
namespace aly{
	struct TextureBoxCompare
	{
		bool operator() (const float3& lhs, const float3& rhs) const
		{
			if (lhs.x < rhs.x) return true;
			else if (lhs.x > rhs.x) return false;
			if (lhs.y < rhs.y) return true;
			else if (lhs.y > rhs.y) return false;
			if (lhs.z < rhs.z) return true;
			else if (lhs.z > rhs.z) return false;
			return false;
		}
	};
	class MeshTexureMap
	{
	protected:
		float angleTolerance;
		float4x4 fitPlane(const aly::Mesh& mesh, std::list<int2>& indexes,float3* deviations,float* scale);
		std::vector<std::list<int2>> mosaicIndexes;
		std::vector<int> vertexLabels;
		std::vector<int> faceLabels;
		std::vector<std::list<int>> vertNbrs;
		std::vector<float4> colors;

		int makeLabelsUnique(std::vector<int>& labels, std::vector<int>& relabel,int minLabelSize);
		int splitLabelComponents(std::vector<int>& labels);
		float pack(std::vector<float3>& boxes, std::multimap<float3, float2, TextureBoxCompare>& boxMap);
		float packAllNaive(std::vector<float3>& boxes, std::multimap<float3, float2, TextureBoxCompare>& boxMap);
		bool pack(const std::vector<float3> &temp, std::multimap<float3, float2, TextureBoxCompare>& boxes, const float2 &size);
		float2 pack(const std::vector<float3> &temp, std::multimap<float3, float2, TextureBoxCompare>& boxes, float area);
		float2 packNaive(const std::vector<float3> &temp, std::multimap<float3, float2, TextureBoxCompare>& boxes, float area);
		void computeMap(aly::Mesh& mesh, const std::function<bool(const std::string& status, float progress)>& statusHandler = nullptr);
		void labelComponents(aly::Mesh& mesh, const std::function<bool(const std::string& status, float progress)>& statusHandler = nullptr);
		void smooth(aly::Mesh& mesh, int iterations, float errorTolerance);
	public:
		MeshTexureMap():angleTolerance(30.0f*(float)ALY_PI / 180.0f) {
		}
		~MeshTexureMap() {}
		void evaluate(aly::Mesh& mesh, int smoothIterations=3, const std::function<bool(const std::string& status, float progress)>& statusHandler =nullptr);
	};
	int LabelTextureRegions(const aly::Mesh& mesh, std::vector<int>& indexes, std::vector<int>& cc, std::vector<int>& labels, float distanceTolerance=1E-6f);
	int GetConnectedVertexComponents(const aly::Mesh& mesh, std::vector<int>& cc, std::vector<int>& labels);
	int GetConnectedTextureComponents(const std::vector<int>& indexes, int N, std::vector<int>& cc, std::vector<int>& labels);
}
#endif
