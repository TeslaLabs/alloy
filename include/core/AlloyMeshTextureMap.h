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
	template<class T> struct bvec : public vec<T,2> {
		typedef T value_type;
		int index;
		bvec<T>& operator=(const bvec<T>& r) {
			x = r.x;
			y = r.y;
			index = r.index;
			return *this;
		}
		bvec() :vec<T, 2>(), index(-1) {
		}
		bvec(const vec<T, 2>& pos, int index) :
			vec<T, 2>(pos), index(index) {
		}
		bvec(T x, T y, int index) :
			vec<T, 2>(x,y), index(index) {
		}
		template<class U> explicit bvec(const bvec<U> & r) :
			x(T(r.x)), y(T(r.y)),index(r.index) {
		}
		double distance(const bvec<T> &node) const {
			vec<T, 2> diff = (*this) - node;
			return aly::max(diff);
		}
		bool operator <(const bvec<T> & r) const {
			return (std::make_tuple(x, y, float(index)) < std::make_tuple(r.x, r.y, float(index)));
		}
		bool operator >(const bvec<T> & r) const {
			return (std::make_tuple(x, y, float(index)) > std::make_tuple(r.x, r.y, float(index)));
		}
		bool operator ==(const bvec<T> & r) const {
			return (x==r.x&&y==r.y&&index == r.index);
		}
		bool operator !=(const bvec<T> & r) const {
			return (x != r.x||y != r.y||index != r.index);
		}
	};

	typedef bvec<float> bvec2f;
	typedef bvec<double> bvec2d;
	struct TextureBoxCompare
	{
		bool operator() (const bvec2f& lhs, const bvec2f& rhs) const
		{
			if (lhs.x < rhs.x) return true;
			else if (lhs.x > rhs.x) return false;
			if (lhs.y < rhs.y) return true;
			else if (lhs.y > rhs.y) return false;
			if (lhs.index < rhs.index) return true;
			else return false;
		}
	};
	struct MosaicVertex
	{
		float3 pt;
		float2 uv;
		bool locked;
		bool boundary;
		int id;
		MosaicVertex(float3 pt,float2 uv) : pt(pt), uv(uv), locked(false),boundary(false), id(-1) {
		}
	};
	struct Mosaic {
		std::list<int2> indexes;
		std::vector<MosaicVertex> vertexes;
		std::map<int, size_t> vertexMap;
		MosaicVertex& getVertex(int uid) {
			return vertexes[vertexMap[uid]];
		}
	};
	class MeshTextureMap
	{
	protected:
		float angleTolerance;
		float packingRatio;
		float smoothness;
		int minVertexPatchSize;
		int smoothIterations;
		int conformalIterations;
		float4x4 fitPlane(const aly::Mesh& mesh, std::list<int2>& indexes,aly::float3* deviations);
		std::vector<Mosaic> mosaics;
		std::vector<int> vertexLabels;
		std::vector<int> faceLabels;
		std::vector<std::list<int>> vertNbrs;
		void projectTriangle(const float3& p0, const float3& p1, const float3& p2, float2& z0, float2& z1, float2& z2);
		int makeLabelsUnique(std::vector<int>& labels, std::vector<int>& relabel,int minLabelSize);
		int splitLabelComponents(std::vector<int>& labels);
		float pack(std::vector<bvec2f>& boxes, std::multimap<bvec2f, float2, TextureBoxCompare>& boxMap);
		float packAllNaive(std::vector<bvec2f>& boxes, std::multimap<bvec2f, float2, TextureBoxCompare>& boxMap);
		bool pack(const std::vector<bvec2f> &temp, std::multimap<bvec2f, float2, TextureBoxCompare>& boxes, const float2 &size);
		float2 pack(const std::vector<bvec2f> &temp, std::multimap<bvec2f, float2, TextureBoxCompare>& boxes, float area);
		float2 packNaive(const std::vector<bvec2f> &temp, std::multimap<bvec2f, float2, TextureBoxCompare>& boxes, float area);
		void computeMap(aly::Mesh& mesh, const std::function<bool(const std::string& status, float progress)>& statusHandler = nullptr);
		void labelComponents(aly::Mesh& mesh, const std::function<bool(const std::string& status, float progress)>& statusHandler = nullptr);
		void smooth(aly::Mesh& mesh, int iterations, float errorTolerance);
		void unfold(aly::Mesh& mesh, std::vector<int>& rectId, std::vector<bvec2f>& rects);
	public:
		MeshTextureMap():angleTolerance(30.0f*(float)ALY_PI / 180.0f),packingRatio(0.8f),minVertexPatchSize(64), smoothIterations(20), conformalIterations(128), smoothness(20.0f) {
		}
		void setMinVertexPatchSize(int sz) {
			minVertexPatchSize = sz;
		}
		void setAngleTolerance(float tol) {
			angleTolerance = tol;
		}
		void setPackingRatio(float ratio) {
			packingRatio = ratio;
		}
		void setSmoothingIteartions(int iters) {
			smoothIterations = iters;
		}
		void setConformalIterations(int iters) {
			conformalIterations = iters;
		}
		void setSmoothness(float w) {
			smoothness = w;
		}
		~MeshTextureMap() {}
		void evaluate(aly::Mesh& mesh,const std::function<bool(const std::string& status, float progress)>& statusHandler =nullptr);
	};
	int LabelTextureRegions(const aly::Mesh& mesh, std::vector<int>& indexes, std::vector<int>& cc, std::vector<int>& labels, float distanceTolerance=1E-6f);
	int GetConnectedVertexComponents(const aly::Mesh& mesh, std::vector<int>& cc, std::vector<int>& labels);
	int GetConnectedTextureComponents(const std::vector<int>& indexes, int N, std::vector<int>& cc, std::vector<int>& labels);
}
#endif
