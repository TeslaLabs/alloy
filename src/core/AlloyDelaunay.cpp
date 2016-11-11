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

//Re-implementation of Paul Bourke's incremental Delaunay construction algorithm.
//http://paulbourke.net/papers/triangulate/cpp.zip
#include "AlloyDelaunay.h"
#include <algorithm>
namespace aly {
	bool CircumCircle(float xp, float yp, float x1, float y1, float x2,
		float y2, float x3, float y3, float &xc, float &yc, float &r) {

		const float EPSILON = 1E-12f;
		float m1, m2, mx1, mx2, my1, my2;
		float dx, dy, rsqr, drsqr;

		/* Check for coincident points */
		if (std::abs(y1 - y2) < EPSILON && std::abs(y2 - y3) < EPSILON)
			return(false);
		if (std::abs(y2 - y1) < EPSILON) {
			m2 = -(x3 - x2) / (y3 - y2);
			mx2 = (x2 + x3) *0.5f;
			my2 = (y2 + y3) *0.5f;
			xc = (x2 + x1) *0.5f;
			yc = m2 * (xc - mx2) + my2;
		}
		else if (std::abs(y3 - y2) < EPSILON) {
			m1 = -(x2 - x1) / (y2 - y1);
			mx1 = (x1 + x2) *0.5f;
			my1 = (y1 + y2) *0.5f;
			xc = (x3 + x2) *0.5f;
			yc = m1 * (xc - mx1) + my1;
		}
		else {
			m1 = -(x2 - x1) / (y2 - y1);
			m2 = -(x3 - x2) / (y3 - y2);
			mx1 = (x1 + x2) *0.5f;
			mx2 = (x2 + x3) *0.5f;
			my1 = (y1 + y2) *0.5f;
			my2 = (y2 + y3) *0.5f;
			xc = (m1 * mx1 - m2 * mx2 + my2 - my1) / (m1 - m2);
			yc = m1 * (xc - mx1) + my1;
		}
		dx = x2 - xc;
		dy = y2 - yc;
		rsqr = dx * dx + dy * dy;
		r = std::sqrt(rsqr);
		dx = xp - xc;
		dy = yp - yc;
		drsqr = dx * dx + dy * dy;
		return((drsqr <= rsqr) ? true : false);
	}
	void Triangulate(std::vector<float2>& pxyz, std::vector<int3>& triangles) {
		int nv = (int)pxyz.size();
		std::vector<int2> edges;
		bool inside;
		int i, j, k;
		float xp, yp, x1, y1, x2, y2, x3, y3, xc, yc, r;
		float xmin, xmax, ymin, ymax, xmid, ymid;
		float dx, dy, dmax;
		std::vector<bool> complete;
		/*
		Find the maximum and minimum vertex bounds.
		This is to allow calculation of the bounding triangle
		*/
		xmin = pxyz[0].x;
		ymin = pxyz[0].y;
		xmax = xmin;
		ymax = ymin;
		for (i = 1; i < nv; i++) {
			if (pxyz[i].x < xmin) xmin = pxyz[i].x;
			if (pxyz[i].x > xmax) xmax = pxyz[i].x;
			if (pxyz[i].y < ymin) ymin = pxyz[i].y;
			if (pxyz[i].y > ymax) ymax = pxyz[i].y;
		}
		dx = xmax - xmin;
		dy = ymax - ymin;
		dmax = (dx > dy) ? dx : dy;
		xmid = (xmax + xmin) *0.5f;
		ymid = (ymax + ymin) *0.5f;
		float scale = 20.0f;
		triangles.clear();
		pxyz.push_back(float2(xmid - scale * dmax, ymid - dmax));
		pxyz.push_back(float2(xmid, ymid + scale * dmax));
		pxyz.push_back(float2(xmid + scale * dmax, ymid - dmax));
		triangles.push_back(int3( nv, nv + 1, nv + 2 ));
		complete.push_back(false);
		for (i = 0; i < (int)pxyz.size(); i++) {
			xp = pxyz[i].x;
			yp = pxyz[i].y;
			edges.clear();
			for (j = 0; j < (int)triangles.size(); j++) {
				if (complete[j])
					continue;
				x1 = pxyz[triangles[j].x].x;
				y1 = pxyz[triangles[j].x].y;
				x2 = pxyz[triangles[j].y].x;
				y2 = pxyz[triangles[j].y].y;
				x3 = pxyz[triangles[j].z].x;
				y3 = pxyz[triangles[j].z].y;
				inside = CircumCircle(xp, yp, x1, y1, x2, y2, x3, y3, xc, yc, r);
				if (xc + r < xp)complete[j] = true;
				if (inside) {
					edges.push_back(int2( triangles[j].x, triangles[j].y ));
					edges.push_back(int2(triangles[j].y, triangles[j].z ));
					edges.push_back(int2(triangles[j].z, triangles[j].x ));
					triangles.erase(triangles.begin() + j);
					complete.erase(complete.begin() + j);
					j--;
				}
			}
			/*
			Tag multiple edges
			Note: if all triangles are specified anticlockwise then all
			interior edges are opposite pointing in direction.
			*/
			for (j = 0; j < (int)edges.size() - 1; j++) {
				for (k = j + 1; k < (int)edges.size(); k++) {
					if ((edges[j].x == edges[k].y) && (edges[j].y == edges[k].x)) {
						edges[j] = int2(-1, -1 );
						edges[k] = int2(-1, -1 );
					}
					/* Shouldn't need the following, see note above */
					if ((edges[j].x == edges[k].x) && (edges[j].y == edges[k].y)) {
						edges[j] = int2(-1, -1 );
						edges[k] = int2(-1, -1 );
					}
				}
			}
			/*
			Form new triangles for the current point
			Skipping over any tagged edges.
			All edges are arranged in clockwise order.
			*/
			for (j = 0; j < (int)edges.size(); j++) {
				if (edges[j].x < 0 || edges[j].y < 0)
					continue;
				triangles.push_back(int3(edges[j].x, edges[j].y, i ));
				complete.push_back(false);
			}
		}
	}
	void MakeDelaunay(const std::vector<float2>& vertexes, std::vector<uint3>& output)
	{
		output.clear();
		if (vertexes.size() < 3) {
			return;
		}
		int N = (int)vertexes.size();
		std::vector<std::pair<float2, int>> vertPairs(N);
		int index = 0;
		for (const float2& vert : vertexes) {
			vertPairs[index] = std::pair<float2, int>(vert, index);
			index++;
		}
		std::sort(vertPairs.begin(), vertPairs.end());
		std::vector<float2> tmp;
		tmp.reserve(vertPairs.size());
		for (std::pair<float2, int>& pr : vertPairs) {
			tmp.push_back(pr.first);
		}
		std::vector<int3>	cTriangles;
		Triangulate(tmp, cTriangles);
		for (int3& triangle : cTriangles) {
			if (triangle.x < N &&
				triangle.y < N &&
				triangle.z < N) {
				output.push_back(uint3((uint32_t)vertPairs[triangle.x].second, (uint32_t)vertPairs[triangle.y].second, vertPairs[triangle.z].second));
			}
		}
	}
}
