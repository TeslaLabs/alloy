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

#ifndef ALLOYMESHKDTREE_H_
#define ALLOYMESHKDTREE_H_
#include "AlloyMath.h"

//Mesh intersection implemented with a KD-tree of triangles.
//The term "Intersector" is used to disambiguate this KD-tree from the one used for points.

namespace aly {
	bool SANITY_CHECK_KDTREE();
	class Mesh;
	static const float3 NO_HIT_POINT = float3(
		std::numeric_limits<float>::infinity());
	static const float NO_HIT_DISTANCE=std::numeric_limits<float>::infinity();
	class KDBox {
	protected:
		int depth;
		float3 minPoint, maxPoint;
		std::vector<KDBox*> children;
		static const int LEFT = 0, RIGHT = 1, MIDDLE = 2;
	public:
		const bool isLeaf;
		KDBox(int depth = 0) :
			depth(depth), isLeaf(false) {
			minPoint = float3(0, 0, 0);
			maxPoint = float3(0, 0, 0);
			children = std::vector<KDBox*>();
		}
		KDBox(int depth, bool isLeaf) :
			depth(depth), isLeaf(isLeaf) {
			minPoint = float3(0, 0, 0);
			maxPoint = float3(0, 0, 0);
			children = std::vector<KDBox*>();
		}
		int getDepth() const {
			return depth;
		}
		void setDepth(int d) {
			depth = d;
		}
		float3 getMin() const {
			return minPoint;
		}
		float3 getMax() const {
			return maxPoint;
		}
		std::vector<KDBox*>& getChildren() {
			return children;
		}
		const std::vector<KDBox*>& getChildren() const {
			return children;
		}
		double volume() const {
			return (maxPoint.z - minPoint.z) * (maxPoint.y - minPoint.y)
				* (maxPoint.x - minPoint.x);
		}
		template<class C, class R> friend std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const KDBox& a);
		bool intersects(const KDBox& test) const;
		bool inside(const float3& test) const;
		virtual void update();
		double distanceToBox(const float3& p) const;
		bool intersectRayBox(const float3& org, const float3& dr) const;
		bool intersectSegmentBox(const float3& org, const float3& end) const;
		virtual ~KDBox() {
		}
	};
	template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const KDBox& a) {
		return ss << "[" << a.minPoint << "," << a.maxPoint << ","<<a.children.size()<<"]";
	}
	class KDBoxEdge {
	protected:
		double value;
		bool start;
		KDBox* box;
	public:
		template<class C, class R> friend std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const KDBoxEdge& a);
		KDBox* getBox() {
			return box;
		}
		double getValue() const {
			return value;
		}
		bool isMin() const {
			return start;
		}
		void set(KDBox* b, int dim, bool start);
		KDBoxEdge() :
			value(1E30), start(true), box(nullptr) {
		}
		KDBoxEdge(KDBox* b, int dim, bool start) {
			set(b, dim, start);
		}
		~KDBoxEdge() {}
	};
	template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const KDBoxEdge& a) {
		return ss << "<" << *a.box << ": " << ((a.start) ? "start" : "") << ": " << a.value << ">";
	}
	class KDSegment {
	public:
		float extent;
		float3 direction;
		float3 origin;
		KDSegment() :
			extent(0), direction(0.0f), origin(0.0f) {
		}
		static KDSegment createFromSegment(const float3& p1, const float3& p2);
		static KDSegment createFromRay(const float3& p1, const float3& v);
		double distance(const KDSegment& seg, float3& lastIntersect) const;
		bool intersectPoint(const float3& test) const;
		float3 intersectionPointRay(const float3& center,
			const float3& kNormal) const;
	};
	class KDTriangle : public KDBox {
	protected:
		float3 pts[3];
	public:
		const uint64_t id;
		KDTriangle(const float3& pt1, const float3& pt2, const float3& pt3,
			uint64_t id = 0, int depth = 0) :
			KDBox(depth, true), id(id) {
			pts[0] = pt1;
			pts[1] = pt2;
			pts[2] = pt3;
			update();
		}
		virtual void update() override {
			minPoint = aly::min(aly::min(pts[0], pts[1]), pts[2]);
			maxPoint = aly::max(aly::max(pts[0], pts[1]), pts[2]);
		}
		float3 getNormal() const {
			return normalize(cross(pts[1] - pts[0], pts[2] - pts[0]));
		}
		float getArea() const {
			return 0.5f * length(cross(pts[1] - pts[0], pts[2] - pts[0]));
		}
		float3 getCentroid() const {
			return 0.3333333f * (pts[0] + pts[1] + pts[2]);
		}
		float3 fromBary(const double3& b) const {
			return float3(
				(float)(pts[0].x * b.x + pts[1].x * b.y + pts[2].x * b.z),
				(float)(pts[0].y * b.x + pts[1].y * b.y + pts[2].y * b.z),
				(float)(pts[0].z * b.x + pts[1].z * b.y + pts[2].z * b.z));
		}
		double3 toBary(const float3& p) const;
		float3 intersectionPointSegment(const float3& p1, const float3& p2) const;
		float3 intersectionPointRay(const float3& org, const float3& v) const;
		double distance(const float3& p, float3& lastIntersect) const;
	};

	struct KDBoxDistance {
		KDBox* box;
		double dist;
		KDBoxDistance(const float3& ref, KDBox* tri) :
			box(tri) {
			dist = tri->distanceToBox(ref);
		}

	};
	template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const KDBoxDistance& a) {
		return ss << a.box << ": " << a.dist;
	}
	inline bool operator<(const KDBoxDistance& a, const KDBoxDistance& b) {
		//Priority queue puts largest first, so we need to use a >= operator to get ascending order.
		return (a.dist > b.dist);
	}
        struct CompareKDBox{
                bool operator()(const KDBoxDistance& a, const KDBoxDistance& b){
                    return (a.dist > b.dist);
                }
        };
	class Intersector {
	protected:
		std::shared_ptr<KDBox> root;
		std::vector<std::shared_ptr<KDBox>> storage;
		const double intersectCost = 80;
		const double traversalCost = 1;
		const double emptyBonus = 0.2;
		int splitPosition(std::vector<KDBox*>& children,
			std::vector<KDBoxEdge>& edges, KDBox* box);
		void buildTree(int maxDepth);
	public:
		void reset() {
			root.reset();
			storage.clear();
			storage.shrink_to_fit();
		}
		KDBox* getRoot() const {
			return root.get();
		}

		void build(const Mesh& mesh, int maxDepth = 16);
		Intersector(const Mesh& mesh, int maxDepth = 16) {
			build(mesh, maxDepth);
		}
		Intersector() {
		}
		double intersectRayDistance(const float3& p1, const float3& v,
			float3& lastPoint, KDTriangle*& lastTriangle) const;
		double intersectSegmentDistance(const float3& p1, const float3& p2,
			float3& lastPoint, KDTriangle*& lastTriangle) const;
		double closestPointSignedDistance(const float3& r, float3& lastPoint,
			KDTriangle*& lastTriangle) const;
		double closestPoint(const float3& pt, float3& lastPoint,
			KDTriangle*& lastTriangle) const;
		double closestPoint(const float3& pt,const float& maxDistance, float3& lastPoint,
			KDTriangle*& lastTriangle) const;
		double closestPointSignedDistance(const float3& r, const float& maxDistance, float3& lastPoint, KDTriangle*& lastTriangle) const;
		double closestPointOutside(const float3& r, const float3& v,
			float3& lastPoint, KDTriangle*& lastTriangle) const;

		double intersectRayDistance(const float3& p1, const float3& v,
			float3& lastPoint) const {
			KDTriangle* lastTriangle;
			return intersectRayDistance(p1, v, lastPoint, lastTriangle);
		}
		double intersectSegmentDistance(const float3& p1, const float3& p2,
			float3& lastPoint) const {
			KDTriangle* lastTriangle;
			return intersectSegmentDistance(p1, p2, lastPoint, lastTriangle);
		}
		double closestPointSignedDistance(const float3& r,
			float3& lastPoint) const {
			KDTriangle* lastTriangle;
			return closestPointSignedDistance(r, lastPoint, lastTriangle);
		}
		double closestPoint(const float3& pt,const float& maxDistance, float3& lastPoint) const{
			KDTriangle* lastTriangle;
			return closestPoint(pt,maxDistance,lastPoint,lastTriangle);
		}
		double closestPoint(const float3& pt, float3& lastPoint) const {
			KDTriangle* lastTriangle;
			return closestPoint(pt, lastPoint, lastTriangle);
		}
		double closestPointOutside(const float3& r, const float3& v,
			float3& lastPoint) const {
			KDTriangle* lastTriangle;
			return closestPointOutside(r, v, lastPoint, lastTriangle);
		}
		double intersectRayDistance(const float3& p1, const float3& v) const {
			float3 lastPoint;
			KDTriangle* lastTriangle;
			return intersectRayDistance(p1, v, lastPoint, lastTriangle);
		}
		double intersectSegmentDistance(const float3& p1, const float3& p2) const {
			float3 lastPoint;
			KDTriangle* lastTriangle;
			return intersectSegmentDistance(p1, p2, lastPoint, lastTriangle);
		}
		double closestPointSignedDistance(const float3& r) const {
			float3 lastPoint;
			KDTriangle* lastTriangle;
			return closestPointSignedDistance(r, lastPoint, lastTriangle);
		}
		double closestPointSignedDistance(const float3& r, const float& maxDistance) const {
			float3 lastPoint;
			KDTriangle* lastTriangle;
			return closestPointSignedDistance(r, maxDistance, lastPoint, lastTriangle);
		}
		double closestPoint(const float3& pt,const float& maxDistance) const{
			float3 lastPoint;
			KDTriangle* lastTriangle;
			return closestPoint(pt,maxDistance,lastPoint,lastTriangle);
		}
		double closestPoint(const float3& pt) const {
			float3 lastPoint;
			KDTriangle* lastTriangle;
			return closestPoint(pt, lastPoint, lastTriangle);
		}
		double closestPointOutside(const float3& r, const float3& v) const {
			float3 lastPoint;
			KDTriangle* lastTriangle;
			return closestPointOutside(r, v, lastPoint, lastTriangle);
		}

		double intersectRayDistance(const float3& p1, const float3& v,
			KDTriangle*& lastTriangle) const {
			float3 lastPoint;
			return intersectRayDistance(p1, v, lastPoint, lastTriangle);
		}
		double intersectSegmentDistance(const float3& p1, const float3& p2,
			KDTriangle*& lastTriangle) const {
			float3 lastPoint;
			return intersectSegmentDistance(p1, p2, lastPoint, lastTriangle);
		}
		double closestPointSignedDistance(const float3& r, KDTriangle*& lastTriangle) const {
			float3 lastPoint;
			return closestPointSignedDistance(r, lastPoint, lastTriangle);
		}
		double closestPoint(const float3& pt, KDTriangle*& lastTriangle) const {
			float3 lastPoint;
			return closestPoint(pt, lastPoint, lastTriangle);
		}
		double closestPointOutside(const float3& r, const float3& v,
			KDTriangle*& lastTriangle) const {
			float3 lastPoint;
			return closestPointOutside(r, v, lastPoint, lastTriangle);
		}
	};
}
#endif
