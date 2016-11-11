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

#ifndef ALLOYISOCONTOUR_H_
#define ALLOYISOCONTOUR_H_
#include "AlloyImage.h"
#include "AlloyVector.h"
#include <memory>
#include <list>
#include <map>
namespace aly {
enum class TopologyRule2D {
	Unconstrained, Connect4, Connect8
};
enum class Winding {
	Clockwise, CounterClockwise
};
struct Edge: public uint2 {
	Edge(uint32_t x, uint32_t y) :
			uint2(x, y) {

	}
	Edge(uint32_t x) :
			uint2(x) {

	}
};
typedef std::shared_ptr<Edge> EdgePtr;
struct EdgeSplit {
	EdgePtr e1, e2;
	uint2 pt1;
	uint2 pt2;
	float2 pt2d;
	uint vid;
public:
	EdgeSplit(uint2 pt1 = uint2(std::numeric_limits<uint32_t>::max()),
			uint2 pt2 = uint2(std::numeric_limits<uint32_t>::max())) :
			pt1(pt1), pt2(pt2), vid(std::numeric_limits<uint32_t>::max()) {
	}
	inline uint64_t hashValue(uint2 pt, int w) const {
		return pt.x + w * pt.y;
	}
	inline uint64_t hashValue(int w, int h) const {
		uint64_t h1 = hashValue(pt1, w);
		uint64_t h2 = hashValue(pt2, w);
		uint64_t offset = w * h;
		if (h1 < h2) {
			return h1 + h2 * offset;
		} else {
			return h2 + h1 * offset;
		}

	}

};
typedef std::shared_ptr<EdgeSplit> EdgeSplitPtr;
inline bool operator==(const Edge& e1, const Edge e2) {
	return ((e1.x == e2.x && e1.y == e2.y) || (e1.x == e2.y && e1.y == e2.x));
}
inline bool operator==(const EdgeSplit& split1, const EdgeSplit& split2) {
	return ((split1.pt1 == split2.pt1 && split1.pt2 == split2.pt2)
			|| (split1.pt1 == split2.pt2 && split1.pt2 == split2.pt1));
}

class IsoContour {
protected:
	uint32_t vertCount = 0;
	const int a2fVertex1Offset[4][2] =
			{ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	const int a2fVertex2Offset[4][2] =
			{ { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 } };
	const int afSquareValue4[16][4] = { { 4, 4, 4, 4 }, // 0000 0
			{ 3, 0, 4, 4 }, // 0001 1
			{ 0, 1, 4, 4 }, // 0010 2
			{ 3, 1, 4, 4 }, // 0011 3
			{ 1, 2, 4, 4 }, // 0100 4
			{ 0, 1, 2, 3 }, // 0101 5
			{ 0, 2, 4, 4 }, // 0110 6
			{ 3, 2, 4, 4 }, // 0111 7
			{ 2, 3, 4, 4 }, // 1000 8
			{ 2, 0, 4, 4 }, // 1001 9
			{ 1, 2, 3, 0 }, // 1010 10
			{ 2, 1, 4, 4 }, // 1011 11
			{ 1, 3, 4, 4 }, // 1100 12
			{ 1, 0, 4, 4 }, // 1101 13
			{ 0, 3, 4, 4 }, // 1110 14
			{ 4, 4, 4, 4 } // 1111 15
	};
	const int afSquareValue8[16][4] = { { 4, 4, 4, 4 }, // 0000 0
			{ 3, 0, 4, 4 }, // 0001 1
			{ 0, 1, 4, 4 }, // 0010 2
			{ 3, 1, 4, 4 }, // 0011 3
			{ 1, 2, 4, 4 }, // 0100 4
			{ 2, 1, 0, 3 }, // 0101 5
			{ 0, 2, 4, 4 }, // 0110 6
			{ 3, 2, 4, 4 }, // 0111 7
			{ 2, 3, 4, 4 }, // 1000 8
			{ 2, 0, 4, 4 }, // 1001 9
			{ 1, 0, 3, 2 }, // 1010 10s
			{ 2, 1, 4, 4 }, // 1011 11
			{ 1, 3, 4, 4 }, // 1100 12
			{ 1, 0, 4, 4 }, // 1101 13
			{ 0, 3, 4, 4 }, // 1110 14
			{ 4, 4, 4, 4 } // 1111 15
	};
	float isoLevel = 0.0f;
	bool nudgeLevelSet;
	const float LEVEL_SET_TOLERANCE;
	TopologyRule2D rule = TopologyRule2D::Unconstrained;
	int rows=0, cols=0;
	const Image1f* img;
	float getValue(int x, int y);
	float fGetOffset(uint2 v1, uint2 v2);
	EdgeSplitPtr createSplit(std::map<uint64_t, EdgeSplitPtr>& splits, int p1x,
			int p1y, int p2x, int p2y);
	void addEdge(std::map<uint64_t, EdgeSplitPtr>& splits,
			std::list<EdgePtr>& edges, int p1x, int p1y, int p2x, int p2y,
			int p3x, int p3y, int p4x, int p4y);
	void processSquare2(int x, int y, std::map<uint64_t, EdgeSplitPtr>& splits,
			std::list<EdgePtr>& edges);
	void processSquare1(int x, int y, std::map<uint64_t, EdgeSplitPtr>& splits,
			std::list<EdgePtr>& edges);
	void processSquare(int x, int y, std::map<uint64_t, EdgeSplitPtr>& splits,
			std::list<EdgePtr>& edges);
	bool orient(const Image1f& img, const EdgeSplit& split1,
			const EdgeSplit& split2, Edge& edge);
public:
	IsoContour(bool nudgeLevelSet = true, float levelSetTolerance = 1E-3f) :
			nudgeLevelSet(nudgeLevelSet), LEVEL_SET_TOLERANCE(
					levelSetTolerance), img(nullptr) {

	}
	virtual ~IsoContour() {
	}
	void solve(const Image1f& levelset, Vector2f& points, Vector2ui& indexes,
			float isoLevel = 0.0f, const TopologyRule2D& rule =
					TopologyRule2D::Unconstrained, const Winding& winding =
					Winding::CounterClockwise);
	void solve(const Image1f& levelset, Vector2f& points,
			std::vector<std::list<uint32_t>>& indexes, float isoLevel = 0.0f,
			const TopologyRule2D& rule = TopologyRule2D::Unconstrained,
			const Winding& winding = Winding::CounterClockwise);
};
}
#endif 
