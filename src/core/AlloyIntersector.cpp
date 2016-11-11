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

// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
/*
 pbrt source code is Copyright(c) 1998-2015
 Matt Pharr, Greg Humphreys, and Wenzel Jakob.

 This file is part of pbrt.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 - Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#include <AlloyIntersector.h>
#include "AlloyMesh.h"

#include <list>
#include <queue>
#include <vector>
#include <algorithm>
namespace aly {
static const double ZERO_TOLERANCE = 1E-6;
void KDBox::update() {
	minPoint.x = minPoint.y = minPoint.z = 1E30f;
	maxPoint.x = maxPoint.y = maxPoint.z = -1E30f;
	for (KDBox* child : children) {
		child->update();
		float3 p = child->getMin();
		minPoint.x = aly::min(minPoint.x, p.x);
		minPoint.y = aly::min(minPoint.y, p.y);
		minPoint.z = aly::min(minPoint.z, p.z);
		p = child->getMax();
		maxPoint.x = aly::max(maxPoint.x, p.x);
		maxPoint.y = aly::max(maxPoint.y, p.y);
		maxPoint.z = aly::max(maxPoint.z, p.z);
	}
}
double KDBox::distanceToBox(const float3& p) const {
	if (inside(p)) {
		return -1;
	}
	float3 ref;
	ref.x = aly::clamp(p.x, minPoint.x, maxPoint.x);
	ref.y = aly::clamp(p.y, minPoint.y, maxPoint.y);
	ref.z = aly::clamp(p.z, minPoint.z, maxPoint.z);
	return distance(p, ref);
}
bool KDBox::intersects(const KDBox& test) const {
	double dx = aly::min(test.maxPoint.x, maxPoint.x)
			- aly::max(test.minPoint.x, minPoint.x);
	double dy = aly::min(test.maxPoint.y, maxPoint.y)
			- aly::max(test.minPoint.y, minPoint.y);
	double dz = aly::min(test.maxPoint.z, maxPoint.z)
			- aly::max(test.minPoint.z, minPoint.z);
	return (dx >= 0) && (dy >= 0) && (dz >= 0);
}
bool KDBox::inside(const float3& test) const {
	return (test.x >= minPoint.x) && (test.x <= maxPoint.x)
			&& (test.y >= minPoint.y) && (test.y <= maxPoint.y)
			&& (test.z >= minPoint.z) && (test.z <= maxPoint.z);
}
bool KDBox::intersectRayBox(const float3& org, const float3& dr) const {
	double3 minB(minPoint);
	double3 maxB(maxPoint);
	double3 origin(org);
	double3 dir(dr);
	double3 coord;
	int3 quadrant;
	bool inside = true;
	int i;
	int whichPlane;
	double maxT[] = { 0, 0, 0 };
	double3 candidatePlane;
	for (i = 0; i < 3; i++) {
		if (origin[i] < minB[i]) {
			quadrant[i] = LEFT;
			candidatePlane[i] = minB[i];
			inside = false;
		} else if (origin[i] > maxB[i]) {
			quadrant[i] = RIGHT;
			candidatePlane[i] = maxB[i];
			inside = false;
		} else {
			quadrant[i] = MIDDLE;
		}
	}
	if (inside) {
		return true;
	}
	/* Calculate T distances to candidate planes */
	for (i = 0; i < 3; i++) {
		if (quadrant[i] != MIDDLE && dir[i] != 0.) {
			maxT[i] = (candidatePlane[i] - origin[i]) / dir[i];
		} else {
			maxT[i] = -1.;
		}
	}
	/* Get largest of the maxT's for final choice of intersection */
	whichPlane = 0;
	for (i = 1; i < 3; i++) {
		if (maxT[whichPlane] < maxT[i])
			whichPlane = i;
	}
	/* Check final candidate actually inside box */;
	if (maxT[whichPlane] < 0) {
		return false;
	}
	for (i = 0; i < 3; i++) {
		if (whichPlane != i) {
			coord[i] = origin[i] + maxT[whichPlane] * dir[i];
			if (coord[i] < minB[i] || coord[i] > maxB[i]) {
				return false;
			}
		} else {
			coord[i] = candidatePlane[i];
		}
	}
	return true;
}
void KDBoxEdge::set(KDBox* b, int dim, bool start) {
	box = b;
	this->start = start;
	if (b == nullptr)
		return;
	switch (dim) {
	case 0:
		value = (start) ? b->getMin().x : b->getMax().x;
		break;
	case 1:
		value = (start) ? b->getMin().y : b->getMax().y;
		break;
	case 2:
		value = (start) ? b->getMin().z : b->getMax().z;
		break;
	default:
		value = 0;
	}

}
bool KDBox::intersectSegmentBox(const float3& org, const float3& end) const {
	if (inside(org) || inside(end))
		return true;
	float3 dr = end - org;
	float len = length(dr);
	dr /= len;
	double3 minB(minPoint);
	double3 maxB(maxPoint);
	double3 origin(org);
	double3 dir(dr);
	float3 coord;
	int3 quadrant;
	bool inside = true;
	int i;
	int whichPlane;
	double maxT[] = { 0, 0, 0 };
	double3 candidatePlane;
	for (i = 0; i < 3; i++) {
		if (origin[i] < minB[i]) {
			quadrant[i] = LEFT;
			candidatePlane[i] = minB[i];
			inside = false;
		} else if (origin[i] > maxB[i]) {
			quadrant[i] = RIGHT;
			candidatePlane[i] = maxB[i];
			inside = false;
		} else {
			quadrant[i] = MIDDLE;
		}
	}
	if (inside) {
		return true;
	}
	for (i = 0; i < 3; i++) {
		if (quadrant[i] != MIDDLE && dir[i] != 0.) {
			maxT[i] = (candidatePlane[i] - origin[i]) / dir[i];
		} else {
			maxT[i] = -1.;
		}
	}
	whichPlane = 0;
	for (i = 1; i < 3; i++) {
		if (maxT[whichPlane] < maxT[i])
			whichPlane = i;
	}
	if (maxT[whichPlane] < 0) {
		return false;
	}
	for (i = 0; i < 3; i++) {
		if (whichPlane != i) {
			coord[i] = (float) (origin[i] + maxT[whichPlane] * dir[i]);
			if (coord[i] < minB[i] || coord[i] > maxB[i]) {
				return false;
			}
		} else {
			coord[i] = (float) candidatePlane[i];
		}
	}
	if (distance(org, coord) <= len) {
		return true;
	} else {
		return false;
	}
}
KDSegment KDSegment::createFromSegment(const float3& p1, const float3& p2) {
	KDSegment seg = KDSegment();
	seg.origin = p1;
	seg.direction = p2 - p1;
	seg.extent = length(seg.direction);
	seg.direction /= seg.extent;
	return seg;
}
KDSegment KDSegment::createFromRay(const float3& p1, const float3& v) {
	KDSegment seg = KDSegment();
	seg.origin = p1;
	seg.direction = v;
	seg.extent = 1E30f;
	return seg;
}
double KDSegment::distance(const KDSegment& seg, float3& lastIntersect) const {
	float3 kDiff = origin - seg.origin;
	double fA01 = -dot(direction, seg.direction);
	double fB0 = dot(kDiff, direction);
	double fB1 = -dot(kDiff, seg.direction);
	double fC = lengthSqr(kDiff);
	double fDet = std::abs((double) 1.0 - fA01 * fA01);
	double fS0, fS1, fSqrDist, fExtDet0, fExtDet1, fTmpS0, fTmpS1;
	if (fDet >= ZERO_TOLERANCE) {
		// segments are not parallel
		fS0 = fA01 * fB1 - fB0;
		fS1 = fA01 * fB0 - fB1;
		fExtDet0 = extent * fDet;
		fExtDet1 = seg.extent * fDet;
		if (fS0 >= -fExtDet0) {
			if (fS0 <= fExtDet0) {
				if (fS1 >= -fExtDet1) {
					if (fS1 <= fExtDet1) // region 0 (interior)
							{
						// minimum at two interior points of 3D lines
						double fInvDet = ((double) 1.0) / fDet;
						fS0 *= fInvDet;
						fS1 *= fInvDet;
						fSqrDist = fS0
								* (fS0 + fA01 * fS1 + ((double) 2.0) * fB0)
								+ fS1
										* (fA01 * fS0 + fS1
												+ ((double) 2.0) * fB1) + fC;
					} else // region 3 (side)
					{
						fS1 = seg.extent;
						fTmpS0 = -(fA01 * fS1 + fB0);
						if (fTmpS0 < -extent) {
							fS0 = -extent;
							fSqrDist = fS0 * (fS0 - ((double) 2.0) * fTmpS0)
									+ fS1 * (fS1 + ((double) 2.0) * fB1) + fC;
						} else if (fTmpS0 <= extent) {
							fS0 = fTmpS0;
							fSqrDist = -fS0 * fS0
									+ fS1 * (fS1 + ((double) 2.0) * fB1) + fC;
						} else {
							fS0 = extent;
							fSqrDist = fS0 * (fS0 - ((double) 2.0) * fTmpS0)
									+ fS1 * (fS1 + ((double) 2.0) * fB1) + fC;
						}
					}
				} else // region 7 (side)
				{
					fS1 = -seg.extent;
					fTmpS0 = -(fA01 * fS1 + fB0);
					if (fTmpS0 < -extent) {
						fS0 = -extent;
						fSqrDist = fS0 * (fS0 - ((double) 2.0) * fTmpS0)
								+ fS1 * (fS1 + ((double) 2.0) * fB1) + fC;
					} else if (fTmpS0 <= extent) {
						fS0 = fTmpS0;
						fSqrDist = -fS0 * fS0
								+ fS1 * (fS1 + ((double) 2.0) * fB1) + fC;
					} else {
						fS0 = extent;
						fSqrDist = fS0 * (fS0 - ((double) 2.0) * fTmpS0)
								+ fS1 * (fS1 + ((double) 2.0) * fB1) + fC;
					}
				}
			} else {
				if (fS1 >= -fExtDet1) {
					if (fS1 <= fExtDet1) // region 1 (side)
							{
						fS0 = extent;
						fTmpS1 = -(fA01 * fS0 + fB1);
						if (fTmpS1 < -seg.extent) {
							fS1 = -seg.extent;
							fSqrDist = fS1 * (fS1 - ((double) 2.0) * fTmpS1)
									+ fS0 * (fS0 + ((double) 2.0) * fB0) + fC;
						} else if (fTmpS1 <= seg.extent) {
							fS1 = fTmpS1;
							fSqrDist = -fS1 * fS1
									+ fS0 * (fS0 + ((double) 2.0) * fB0) + fC;
						} else {
							fS1 = seg.extent;
							fSqrDist = fS1 * (fS1 - ((double) 2.0) * fTmpS1)
									+ fS0 * (fS0 + ((double) 2.0) * fB0) + fC;
						}
					} else // region 2 (corner)
					{
						fS1 = seg.extent;
						fTmpS0 = -(fA01 * fS1 + fB0);
						if (fTmpS0 < -extent) {
							fS0 = -extent;
							fSqrDist = fS0 * (fS0 - ((double) 2.0) * fTmpS0)
									+ fS1 * (fS1 + ((double) 2.0) * fB1) + fC;
						} else if (fTmpS0 <= extent) {
							fS0 = fTmpS0;
							fSqrDist = -fS0 * fS0
									+ fS1 * (fS1 + ((double) 2.0) * fB1) + fC;
						} else {
							fS0 = extent;
							fTmpS1 = -(fA01 * fS0 + fB1);
							if (fTmpS1 < -seg.extent) {
								fS1 = -seg.extent;
								fSqrDist = fS1 * (fS1 - ((double) 2.0) * fTmpS1)
										+ fS0 * (fS0 + ((double) 2.0) * fB0)
										+ fC;
							} else if (fTmpS1 <= seg.extent) {
								fS1 = fTmpS1;
								fSqrDist = -fS1 * fS1
										+ fS0 * (fS0 + ((double) 2.0) * fB0)
										+ fC;
							} else {
								fS1 = seg.extent;
								fSqrDist = fS1 * (fS1 - ((double) 2.0) * fTmpS1)
										+ fS0 * (fS0 + ((double) 2.0) * fB0)
										+ fC;
							}
						}
					}
				} else // region 8 (corner)
				{
					fS1 = -seg.extent;
					fTmpS0 = -(fA01 * fS1 + fB0);
					if (fTmpS0 < -extent) {
						fS0 = -extent;
						fSqrDist = fS0 * (fS0 - ((double) 2.0) * fTmpS0)
								+ fS1 * (fS1 + ((double) 2.0) * fB1) + fC;
					} else if (fTmpS0 <= extent) {
						fS0 = fTmpS0;
						fSqrDist = -fS0 * fS0
								+ fS1 * (fS1 + ((double) 2.0) * fB1) + fC;
					} else {
						fS0 = extent;
						fTmpS1 = -(fA01 * fS0 + fB1);
						if (fTmpS1 > seg.extent) {
							fS1 = seg.extent;
							fSqrDist = fS1 * (fS1 - ((double) 2.0) * fTmpS1)
									+ fS0 * (fS0 + ((double) 2.0) * fB0) + fC;
						} else if (fTmpS1 >= -seg.extent) {
							fS1 = fTmpS1;
							fSqrDist = -fS1 * fS1
									+ fS0 * (fS0 + ((double) 2.0) * fB0) + fC;
						} else {
							fS1 = -seg.extent;
							fSqrDist = fS1 * (fS1 - ((double) 2.0) * fTmpS1)
									+ fS0 * (fS0 + ((double) 2.0) * fB0) + fC;
						}
					}
				}
			}
		} else {
			if (fS1 >= -fExtDet1) {
				if (fS1 <= fExtDet1) // region 5 (side)
						{
					fS0 = -extent;
					fTmpS1 = -(fA01 * fS0 + fB1);
					if (fTmpS1 < -seg.extent) {
						fS1 = -seg.extent;
						fSqrDist = fS1 * (fS1 - ((double) 2.0) * fTmpS1)
								+ fS0 * (fS0 + ((double) 2.0) * fB0) + fC;
					} else if (fTmpS1 <= seg.extent) {
						fS1 = fTmpS1;
						fSqrDist = -fS1 * fS1
								+ fS0 * (fS0 + ((double) 2.0) * fB0) + fC;
					} else {
						fS1 = seg.extent;
						fSqrDist = fS1 * (fS1 - ((double) 2.0) * fTmpS1)
								+ fS0 * (fS0 + ((double) 2.0) * fB0) + fC;
					}
				} else // region 4 (corner)
				{
					fS1 = seg.extent;
					fTmpS0 = -(fA01 * fS1 + fB0);
					if (fTmpS0 > extent) {
						fS0 = extent;
						fSqrDist = fS0 * (fS0 - ((double) 2.0) * fTmpS0)
								+ fS1 * (fS1 + ((double) 2.0) * fB1) + fC;
					} else if (fTmpS0 >= -extent) {
						fS0 = fTmpS0;
						fSqrDist = -fS0 * fS0
								+ fS1 * (fS1 + ((double) 2.0) * fB1) + fC;
					} else {
						fS0 = -extent;
						fTmpS1 = -(fA01 * fS0 + fB1);
						if (fTmpS1 < -seg.extent) {
							fS1 = -seg.extent;
							fSqrDist = fS1 * (fS1 - ((double) 2.0) * fTmpS1)
									+ fS0 * (fS0 + ((double) 2.0) * fB0) + fC;
						} else if (fTmpS1 <= seg.extent) {
							fS1 = fTmpS1;
							fSqrDist = -fS1 * fS1
									+ fS0 * (fS0 + ((double) 2.0) * fB0) + fC;
						} else {
							fS1 = seg.extent;
							fSqrDist = fS1 * (fS1 - ((double) 2.0) * fTmpS1)
									+ fS0 * (fS0 + ((double) 2.0) * fB0) + fC;
						}
					}
				}
			} else // region 6 (corner)
			{
				fS1 = -seg.extent;
				fTmpS0 = -(fA01 * fS1 + fB0);
				if (fTmpS0 > extent) {
					fS0 = extent;
					fSqrDist = fS0 * (fS0 - ((double) 2.0) * fTmpS0)
							+ fS1 * (fS1 + ((double) 2.0) * fB1) + fC;
				} else if (fTmpS0 >= -extent) {
					fS0 = fTmpS0;
					fSqrDist = -fS0 * fS0 + fS1 * (fS1 + ((double) 2.0) * fB1)
							+ fC;
				} else {
					fS0 = -extent;
					fTmpS1 = -(fA01 * fS0 + fB1);
					if (fTmpS1 < -seg.extent) {
						fS1 = -seg.extent;
						fSqrDist = fS1 * (fS1 - ((double) 2.0) * fTmpS1)
								+ fS0 * (fS0 + ((double) 2.0) * fB0) + fC;
					} else if (fTmpS1 <= seg.extent) {
						fS1 = fTmpS1;
						fSqrDist = -fS1 * fS1
								+ fS0 * (fS0 + ((double) 2.0) * fB0) + fC;
					} else {
						fS1 = seg.extent;
						fSqrDist = fS1 * (fS1 - ((double) 2.0) * fTmpS1)
								+ fS0 * (fS0 + ((double) 2.0) * fB0) + fC;
					}
				}
			}
		}
	} else {
		// The segments are parallel. The average b0 term is designed to
		// ensure symmetry of the function. That is, dist(seg0,seg1) and
		// dist(seg1,seg0) should produce the same number.
		double fE0pE1 = extent + seg.extent;
		double fSign = (fA01 > (double) 0.0 ? (double) -1.0 : (double) 1.0);
		double fB0Avr = ((double) 0.5) * (fB0 - fSign * fB1);
		double fLambda = -fB0Avr;
		if (fLambda < -fE0pE1) {
			fLambda = -fE0pE1;
		} else if (fLambda > fE0pE1) {
			fLambda = fE0pE1;
		}

		fS1 = -fSign * fLambda * seg.extent / fE0pE1;
		fS0 = fLambda + fSign * fS1;
		fSqrDist = fLambda * (fLambda + ((double) 2.0) * fB0Avr) + fC;
	}

	float3 m_kClosestPoint0;
	float3 m_kClosestPoint1;

	float3 tmpDir = direction;
	tmpDir *= (float) fS0;
	m_kClosestPoint0 = origin + tmpDir;
	lastIntersect = m_kClosestPoint0;
	//tmpDir = seg.direction;
	//tmpDir *= (float)fS1;
	//m_kClosestPoint1 = seg.origin + tmpDir;
	return std::sqrt(std::abs(fSqrDist));
}
bool KDSegment::intersectPoint(const float3& test) const {
	static const float EPS = 5e-4f;
	float3 v1 = test - origin;
	float3 v2 = direction;
	double dt = dot(v1, direction);
	if (dt <= EPS)
		return false;
	v2 *= (float) dt;
	v1 -= v2;
	if (length(v1) > EPS)
		return false;
	if (dt < extent - EPS)
		return true;
	return false;
}
float3 KDSegment::intersectionPointRay(const float3& center,
		const float3& kNormal) const {
	static const float EPS = 1E-30f;
	float A = kNormal.x;
	float B = kNormal.y;
	float C = kNormal.z;
	float numer = A * (center.x - origin.x) + B * (center.y - origin.y)
			+ C * (center.z - origin.z);
	float denom = (A * direction.x + B * direction.y + C * direction.z);
	if (denom != 0) {
		float u = numer / denom;
		if (u > EPS && u < extent - EPS) {
			return origin + direction * u;
		} else {
			return NO_HIT_POINT;
		}
	} else {
		return NO_HIT_POINT;
	}
}
double3 KDTriangle::toBary(const float3& p) const {
	double a = pts[0].x - pts[2].x;
	double b = pts[1].x - pts[2].x;
	double c = pts[2].x - p.x;

	double d = pts[0].y - pts[2].y;
	double e = pts[1].y - pts[2].y;
	double f = pts[2].y - p.y;

	double g = pts[0].z - pts[2].z;
	double h = pts[1].z - pts[2].z;
	double i = pts[2].z - p.z;

	double l1 = (b * (f + i) - c * (e + h)) / (a * (e + h) - b * (d + g));
	double l2 = (a * (f + i) - c * (d + g)) / (b * (d + g) - a * (e + h));
	if (l1 > 1 || l2 > 1 || l1 + l2 > 1 || l1 < 0 || l2 < 0) {
		l1 = clamp(l1, 0.0, 1.0);
		l2 = clamp(l2, 0.0, 1.0);
		if (l1 + l2 > 1) {
			double diff = 0.5 * (1 - l1 - l2);
			l1 += diff;
			l2 += diff;
		}
	}
	return double3(l1, l2, 1 - l1 - l2);
}
float3 KDTriangle::intersectionPointSegment(const float3& p1,
		const float3& p2) const {
	static const float EPS = 1e-5f;
	// compute the offset origin, edges, and normal
	KDSegment seg = KDSegment::createFromSegment(p1, p2);
	float3 dr = seg.direction;
	float len = seg.extent;
	float3 kDiff = p1 - pts[0];
	float3 kEdge1 = pts[1] - pts[0];
	float3 kEdge2 = pts[2] - pts[0];
	float3 kNormal = cross(kEdge1, kEdge2);
	double fDdN = dot(dr, kNormal);
	double fSign;
	if (fDdN > 0) {
		fSign = (double) 1.0;
	} else if (fDdN < 0) {
		fSign = (double) -1.0;
		fDdN = -fDdN;
	} else {
		return NO_HIT_POINT;
	}
	float3 crs = cross(kDiff, kEdge2);
	double fDdQxE2 = fSign * dot(dr, crs);
	if (fDdQxE2 >= (double) 0.0) {
		crs = cross(kEdge1, kDiff);
		double fDdE1xQ = fSign * dot(dr, crs);
		if (fDdE1xQ >= (double) 0.0) {
			if (fDdQxE2 + fDdE1xQ <= fDdN) {
				// line intersects triangle, check if segment does
				double fQdN = -fSign * dot(kDiff, kNormal);
				double fInv = ((double) 1.0) / fDdN;
				double m_fSegmentT = fQdN * fInv;
				if (m_fSegmentT >= 0 && m_fSegmentT <= len) {
					return p1 + dr * (float) m_fSegmentT;
				}
			}
		}
	}
	float3 intersect = NO_HIT_POINT;
	double d;
	KDSegment e1 = KDSegment::createFromSegment(pts[0], pts[1]);
	float3 lastIntersect;
	d = seg.distance(e1, lastIntersect);
	if (d < EPS) {
		intersect = lastIntersect;
	} else {
		KDSegment e2 = KDSegment::createFromSegment(pts[1], pts[2]);
		d = seg.distance(e2, lastIntersect);
		if (d < EPS) {
			intersect = lastIntersect;
		} else {
			KDSegment e3 = KDSegment::createFromSegment(pts[2], pts[0]);
			d = seg.distance(e3, lastIntersect);
			if (d < EPS) {
				intersect = lastIntersect;
			}
		}
	}
	if (intersect != NO_HIT_POINT) {
		kDiff = intersect - p1;
		double l = dot(seg.direction, kDiff);
		if (l >= 0 && l <= len) {
			return intersect;
		}
	}
	return NO_HIT_POINT;

}
float3 KDTriangle::intersectionPointRay(const float3& org,
		const float3& v) const {
	static const float EPS = 1e-5f;
	// compute the offset origin, edges, and normal
	KDSegment seg = KDSegment::createFromRay(org, v);
	float3 dr = seg.direction;
	float len = seg.extent;
	float3 kDiff = org - pts[0];
	float3 kEdge1 = pts[1] - pts[0];
	float3 kEdge2 = pts[2] - pts[0];
	float3 kNormal = cross(kEdge1, kEdge2);
	double fDdN = dot(dr, kNormal);
	double fSign;
	if (fDdN > 0) {
		fSign = (double) 1.0;
	} else if (fDdN < 0) {
		fSign = (double) -1.0;
		fDdN = -fDdN;
	} else {
		// System.out.println("SEGMENT PARALLEL !");
		// Segment and triangle are parallel, call it a "no intersection"
		// even if the segment does intersect.
		return NO_HIT_POINT;
	}
	float3 crs = cross(kDiff, kEdge2);
	double fDdQxE2 = fSign * dot(dr, crs);
	if (fDdQxE2 >= (double) 0.0) {
		crs = cross(kEdge1, kDiff);
		double fDdE1xQ = fSign * dot(dr, crs);
		if (fDdE1xQ >= (double) 0.0) {
			if (fDdQxE2 + fDdE1xQ <= fDdN) {
				// line intersects triangle, check if segment does
				double fQdN = -fSign * dot(kDiff, kNormal);
				double fExtDdN = len * fDdN;
				if (-fExtDdN <= fQdN && fQdN <= fExtDdN) {
					// segment intersects triangle
					double fInv = ((double) 1.0) / fDdN;
					double m_fSegmentT = fQdN * fInv;
					return org + dr * (float) m_fSegmentT;
				}
			}
		}
	}
	//Handle cracks between triangle faces
	double d;
	KDSegment e1 = KDSegment::createFromSegment(pts[0], pts[1]);
	float3 lastIntersect;
	d = seg.distance(e1, lastIntersect);
	if (d < EPS) {
		return lastIntersect;
	} else {
		KDSegment e2 = KDSegment::createFromSegment(pts[1], pts[2]);
		d = seg.distance(e2, lastIntersect);
		if (d < EPS) {
			return lastIntersect;
		} else {
			KDSegment e3 = KDSegment::createFromSegment(pts[2], pts[0]);
			d = seg.distance(e3, lastIntersect);
			if (d < EPS) {
				return lastIntersect;
			}
		}
	}
	return NO_HIT_POINT;
}
double KDTriangle::distance(const float3& p, float3& lastIntersect) const {
	float3 p1 = pts[0];
	float3 p2 = pts[1];
	float3 p3 = pts[2];
	float3 kDiff = (p1 - p);
	float3 kEdge0 = (p2 - p1);
	float3 kEdge1 = (p3 - p1);
	float fA00 = (float) lengthSqr(kEdge0);
	float fA01 = dot(kEdge0, kEdge1);
	float fA11 = lengthSqr(kEdge1);
	float fB0 = dot(kDiff, kEdge0);
	float fB1 = dot(kDiff, kEdge1);
	float fC = lengthSqr(kDiff);
	float fDet = std::abs(fA00 * fA11 - fA01 * fA01);
	float fS = fA01 * fB1 - fA11 * fB0;
	float fT = fA01 * fB0 - fA00 * fB1;
	float fSqrDistance;
	if (fS + fT <= fDet) {
		if (fS < (float) 0.0) {
			if (fT < (float) 0.0) // region 4
					{
				if (fB0 < (float) 0.0) {
					fT = (float) 0.0;
					if (-fB0 >= fA00) {
						fS = (float) 1.0;
						fSqrDistance = fA00 + ((float) 2.0) * fB0 + fC;
					} else {
						fS = -fB0 / fA00;
						fSqrDistance = fB0 * fS + fC;
					}

				} else {
					fS = (float) 0.0;
					if (fB1 >= (float) 0.0) {
						fT = (float) 0.0;
						fSqrDistance = fC;
					} else if (-fB1 >= fA11) {
						fT = (float) 1.0;
						fSqrDistance = fA11 + ((float) 2.0) * fB1 + fC;
					} else {
						fT = -fB1 / fA11;
						fSqrDistance = fB1 * fT + fC;
					}
				}

			} else // region 3
			{
				fS = (float) 0.0;
				if (fB1 >= (float) 0.0) {
					fT = (float) 0.0;
					fSqrDistance = fC;
				} else if (-fB1 >= fA11) {
					fT = (float) 1.0;
					fSqrDistance = fA11 + ((float) 2.0) * fB1 + fC;
				} else {
					fT = -fB1 / fA11;
					fSqrDistance = fB1 * fT + fC;
				}
			}
		} else if (fT < (float) 0.0) // region 5
				{
			fT = (float) 0.0;
			if (fB0 >= (float) 0.0) {
				fS = (float) 0.0;
				fSqrDistance = fC;
			} else if (-fB0 >= fA00) {
				fS = (float) 1.0;
				fSqrDistance = fA00 + ((float) 2.0) * fB0 + fC;
			} else {
				fS = -fB0 / fA00;
				fSqrDistance = fB0 * fS + fC;
			}
		} else // region 0
		{
			// minimum at interior point
			float fInvDet = ((float) 1.0) / fDet;
			fS *= fInvDet;
			fT *= fInvDet;
			fSqrDistance = fS * (fA00 * fS + fA01 * fT + ((float) 2.0) * fB0)
					+ fT * (fA01 * fS + fA11 * fT + ((float) 2.0) * fB1) + fC;
		}
	} else {
		float fTmp0, fTmp1, fNumer, fDenom;

		if (fS < (float) 0.0) // region 2
				{
			fTmp0 = fA01 + fB0;
			fTmp1 = fA11 + fB1;
			if (fTmp1 > fTmp0) {
				fNumer = fTmp1 - fTmp0;
				fDenom = fA00 - 2.0f * fA01 + fA11;
				if (fNumer >= fDenom) {
					fS = (float) 1.0;
					fT = (float) 0.0;
					fSqrDistance = fA00 + ((float) 2.0) * fB0 + fC;
				} else {
					fS = fNumer / fDenom;
					fT = (float) 1.0 - fS;
					fSqrDistance = fS * (fA00 * fS + fA01 * fT + 2.0f * fB0)
							+ fT * (fA01 * fS + fA11 * fT + ((float) 2.0) * fB1)
							+ fC;
				}
			} else {
				fS = (float) 0.0;
				if (fTmp1 <= (float) 0.0) {
					fT = (float) 1.0;
					fSqrDistance = fA11 + ((float) 2.0) * fB1 + fC;
				} else if (fB1 >= (float) 0.0) {
					fT = (float) 0.0;
					fSqrDistance = fC;
				} else {
					fT = -fB1 / fA11;
					fSqrDistance = fB1 * fT + fC;
				}
			}
		} else if (fT < (float) 0.0) // region 6
				{
			fTmp0 = fA01 + fB1;
			fTmp1 = fA00 + fB0;
			if (fTmp1 > fTmp0) {
				fNumer = fTmp1 - fTmp0;
				fDenom = fA00 - ((float) 2.0) * fA01 + fA11;
				if (fNumer >= fDenom) {
					fT = (float) 1.0;
					fS = (float) 0.0;
					fSqrDistance = fA11 + ((float) 2.0) * fB1 + fC;
				} else {
					fT = fNumer / fDenom;
					fS = (float) 1.0 - fT;
					fSqrDistance = fS
							* (fA00 * fS + fA01 * fT + ((float) 2.0) * fB0)
							+ fT * (fA01 * fS + fA11 * fT + ((float) 2.0) * fB1)
							+ fC;
				}
			} else {
				fT = (float) 0.0;
				if (fTmp1 <= (float) 0.0) {
					fS = (float) 1.0;
					fSqrDistance = fA00 + ((float) 2.0) * fB0 + fC;
				} else if (fB0 >= (float) 0.0) {
					fS = (float) 0.0;
					fSqrDistance = fC;
				} else {
					fS = -fB0 / fA00;
					fSqrDistance = fB0 * fS + fC;
				}
			}
		} else // region 1
		{
			fNumer = fA11 + fB1 - fA01 - fB0;
			if (fNumer <= (float) 0.0) {
				fS = (float) 0.0;
				fT = (float) 1.0;
				fSqrDistance = fA11 + ((float) 2.0) * fB1 + fC;
			} else {
				fDenom = fA00 - 2.0f * fA01 + fA11;
				if (fNumer >= fDenom) {
					fS = (float) 1.0;
					fT = (float) 0.0;
					fSqrDistance = fA00 + ((float) 2.0) * fB0 + fC;
				} else {
					fS = fNumer / fDenom;
					fT = (float) 1.0 - fS;
					fSqrDistance = fS
							* (fA00 * fS + fA01 * fT + ((float) 2.0) * fB0)
							+ fT * (fA01 * fS + fA11 * fT + ((float) 2.0) * fB1)
							+ fC;
				}
			}
		}
	}
	// account for numerical round-off error
	if (fSqrDistance < (float) 0.0) {
		fSqrDistance = (float) 0.0;
	}
	lastIntersect = pts[0] + kEdge0 * (float) fS + kEdge1 * (float) fT;
	return std::sqrt(fSqrDistance);
}
void Intersector::build(const Mesh& mesh, int maxDepth) {
	root = std::shared_ptr<KDBox>(new KDBox());
	uint64_t id = 0;
	KDTriangle* tri;
	for (const uint4& face : mesh.quadIndexes.data) {
		float3 pt1 = mesh.vertexLocations[face.x];
		float3 pt2 = mesh.vertexLocations[face.y];
		float3 pt3 = mesh.vertexLocations[face.z];
		float3 pt4 = mesh.vertexLocations[face.w];
		if (distanceSqr(pt1, pt3) < distanceSqr(pt2, pt4)) {
			storage.push_back(
					std::shared_ptr<KDTriangle>(
							tri = new KDTriangle(pt1, pt2, pt3, id, 1)));
			root->getChildren().push_back(tri);
			storage.push_back(
					std::shared_ptr<KDTriangle>(
							tri = new KDTriangle(pt3, pt4, pt1, id, 1)));
			root->getChildren().push_back(tri);
		} else {
			storage.push_back(
					std::shared_ptr<KDTriangle>(
							tri = new KDTriangle(pt1, pt2, pt4, id, 1)));
			root->getChildren().push_back(tri);
			storage.push_back(
					std::shared_ptr<KDTriangle>(
							tri = new KDTriangle(pt4, pt2, pt3, id, 1)));
			root->getChildren().push_back(tri);

		}
		id++;
	}
	for (const uint3& face : mesh.triIndexes.data) {
		storage.push_back(
				std::shared_ptr<KDTriangle>(
						tri = new KDTriangle(mesh.vertexLocations[face.x],
								mesh.vertexLocations[face.y],
								mesh.vertexLocations[face.z], id, 1)));
		root->getChildren().push_back(tri);
		id++;
	}
	root->update();
	buildTree(maxDepth);
}
void Intersector::buildTree(int maxDepth) {
	std::list<KDBox*> boxes;
	boxes.push_back(root.get());
	int sz;
	int splitPos;
	KDBox* leftChild;
	KDBox* rightChild;
	int depthCount = 0;
	while (boxes.size() > 0) {
		KDBox* box = boxes.front();
		boxes.pop_front();
		if (box->getDepth() > maxDepth)
			continue;
		if (box->getDepth() > depthCount) {
			depthCount = box->getDepth();
		}
		std::vector<KDBox*>& children = box->getChildren();
		if (children.size() < 2)
			continue;
		sz = (int) children.size() * 2;
		std::vector<KDBoxEdge> edges;
		splitPos = splitPosition(children, edges, box);
		if (splitPos <= 0 || splitPos == sz - 1) {
			continue;
		}
		storage.push_back(
				std::shared_ptr<KDBox>(
						leftChild = new KDBox(box->getDepth() + 1)));
		storage.push_back(
				std::shared_ptr<KDBox>(
						rightChild = new KDBox(box->getDepth() + 1)));
		std::vector<KDBox*>& lchildren = leftChild->getChildren();
		std::vector<KDBox*>& rchildren = rightChild->getChildren();
		for (int i = 0; i < splitPos; i++) {
			if (edges[i].isMin()) {
				edges[i].getBox()->setDepth(leftChild->getDepth() + 1);
				lchildren.push_back(edges[i].getBox());

			}
		}
		for (int i = splitPos + 1; i < sz; i++) {
			if (!edges[i].isMin()) {
				edges[i].getBox()->setDepth(rightChild->getDepth() + 1);
				rchildren.push_back(edges[i].getBox());
			}
		}
		if (lchildren.size() >= children.size()
				|| rchildren.size() >= children.size()) {
			continue;
		}
		leftChild->update();
		rightChild->update();
		children.clear();
		children.push_back(leftChild);
		children.push_back(rightChild);
		box->update();
		//std::cout << "Left: " << *leftChild<<std::endl;
		//std::cout << "Right: " << *rightChild << std::endl;
		boxes.push_back(leftChild);
		boxes.push_back(rightChild);

	}
}
int Intersector::splitPosition(std::vector<KDBox*>& children,
		std::vector<KDBoxEdge>& edges, KDBox* box) {
	int nAbove = 0;
	int nBelow = 0;
	double pAbove = 0;
	double pBelow = 0;
	double cost = 0;
	int dim = 0;
	int bestSplit = -1;
	double sum = 0;
	double prod = 0;
	double voxel;
	double val;
	double minCost;
	int sz;
	//KDBoxEdge e1, e2;
	float3 v = box->getMax() - box->getMin();
	if (v.x > v.y && v.x > v.z)
		dim = 0;
	else if (v.y > v.z)
		dim = 1;
	else
		dim = 2;
	int retry = 3;
	voxel = 1.0 / (v.x + v.y + v.z);
	do {
		bestSplit = -1;
		switch (dim) {
		case 0:
			sum = v.y + v.z;
			prod = v.y * v.z;
			break;
		case 1:
			sum = v.x + v.z;
			prod = v.x * v.z;
			break;
		case 2:
			sum = v.x + v.y;
			prod = v.x * v.y;
			break;
		}

		sz = (int) children.size() * 2;
		edges.resize(sz);
		for (int i = 0; i < sz; i++) {
			KDBox* child = children[i / 2];
			edges[i] = KDBoxEdge(child, dim, (i % 2 == 1) ? true : false);
		}

		std::sort(edges.begin(), edges.end(),
				[](const KDBoxEdge& e1, const KDBoxEdge& e2) {
					return (e1.getValue() < e2.getValue());
				});
		for (int i = 0; i < (int) edges.size() - 1; i++) {
			KDBoxEdge& e1 = edges[i];
			KDBoxEdge& e2 = edges[i + 1];
			if (e1.getValue() == e2.getValue()) {
				if (!e1.isMin() && e2.isMin()) {
					std::swap(e1, e2);
				}
			}
		}
		nBelow = 0;
		nAbove = (int) children.size();
		// Do not assume prior minimum cost because the spherical
		// orientation
		// is in some sense optimal when rays are cast from outside the
		// sphere.
		// If minimum cost is assumed, then no branches in kd-tree will be
		// generated
		// minCost=intersectCost*nAbove;
		minCost = 1E30;

		KDBoxEdge e1(box, dim, true);
		KDBoxEdge e2(box, dim, false);
		for (int i = 0; i < sz; i++) {
			if (!edges[i].isMin())
				nAbove--;
			val = edges[i].getValue();
			if (val > e1.getValue() && e2.getValue() > val) {
				pBelow = (sum * (val - e1.getValue()) + prod) * voxel;
				pAbove = (sum * (e2.getValue() - val) + prod) * voxel;
				cost = traversalCost
						+ intersectCost
								* (1
										- ((nAbove == 0 || nBelow == 0) ?
												emptyBonus : 0))
								* (pBelow * nBelow + pAbove * nAbove);
				if (cost < minCost) {
					minCost = cost;
					bestSplit = i;
				}
			}
			if (edges[i].isMin())
				nBelow++;
		}
		dim = (dim + 1) % 3;
		retry--;
	} while (bestSplit == -1 && retry > 0);
	return bestSplit;
}
double Intersector::intersectRayDistance(const float3& p1, const float3& v,
		float3& lastPoint, KDTriangle*& lastTriangle) const {
	if (root->getChildren().size() == 0)
		throw new std::runtime_error("KD-Tree has not been initialized.");
	std::list<KDBox*> boxes;
	boxes.push_back(getRoot());
	double d;
	double mind = 1E30;
	KDTriangle* resultTriangle = nullptr;
	float3 resultIntersect = NO_HIT_POINT;
	KDTriangle* tri;
	while (boxes.size() > 0) {
		KDBox* box = boxes.front();
		//std::cout << *box << std::endl;
		boxes.pop_front();
		if (box->intersectRayBox(p1, v)) {
			if (box->isLeaf) {
				tri = dynamic_cast<KDTriangle*>(box);
				float3 intersect = tri->intersectionPointRay(p1, v);
				if (intersect != NO_HIT_POINT) {
					d = distance(p1, intersect);
					if (d < mind) {
						mind = d;
						resultTriangle = tri;
						resultIntersect = intersect;
					}
				}
			}
			boxes.insert(boxes.end(), box->getChildren().begin(),
					box->getChildren().end());
		}
	}
	lastTriangle = resultTriangle;
	lastPoint = resultIntersect;
	if (resultTriangle == nullptr) {
		return NO_HIT_DISTANCE;
	} else {
		return mind;
	}
}
double Intersector::intersectSegmentDistance(const float3& p1, const float3& p2,
		float3& lastPoint, KDTriangle*& lastTriangle) const {
	if (root->getChildren().size() == 0)
		throw new std::runtime_error("KD-Tree has not been initialized.");
	std::list<KDBox*> boxes;
	boxes.push_back(getRoot());
	double d;
	double mind = 1E30;
	KDTriangle* resultTriangle = nullptr;
	float3 resultIntersect = NO_HIT_POINT;
	KDTriangle* tri;
	while (boxes.size() > 0) {
		KDBox* box = boxes.front();
		boxes.pop_front();
		if (box->intersectSegmentBox(p1, p2)) {
			if (box->isLeaf) {
				tri = dynamic_cast<KDTriangle*>(box);
				float3 intersect = tri->intersectionPointSegment(p1, p2);
				if (intersect != NO_HIT_POINT) {
					d = distance(p1, intersect);
					if (d < mind) {
						mind = d;
						resultTriangle = tri;
						resultIntersect = intersect;
					}
				}
			}
			boxes.insert(boxes.end(), box->getChildren().begin(),
					box->getChildren().end());
		}
	}
	lastTriangle = resultTriangle;
	lastPoint = resultIntersect;
	if (resultTriangle == nullptr) {
		return NO_HIT_DISTANCE;
	} else {
		return mind;
	}
}
double Intersector::closestPointSignedDistance(const float3& r, float3& lastPoint,
		KDTriangle*& lastTriangle) const {
	double d = closestPoint(r, lastPoint, lastTriangle);
	if (d >= 0&& d!=NO_HIT_DISTANCE) {
		float3 norm = lastTriangle->getNormal();
		float3 center = lastTriangle->getCentroid();
		float3 diff = r - center;
		return sign(dot(diff, norm)) * d;
	} else {
		return NO_HIT_DISTANCE;
	}
}
double Intersector::closestPointSignedDistance(const float3& r,const float& maxDistance, float3& lastPoint,
	KDTriangle*& lastTriangle) const {
	double d = closestPoint(r, maxDistance, lastPoint, lastTriangle);
	if (d >= 0&&d!= NO_HIT_DISTANCE) {
		float3 norm = lastTriangle->getNormal();
		float3 center = lastTriangle->getCentroid();
		float3 diff = r - center;
		return sign(dot(diff, norm)) * d;
	}
	else {
		return NO_HIT_DISTANCE;
	}
}
double Intersector::closestPoint(const float3& pt, const float& maxDistance,
		float3& lastPoint, KDTriangle*& lastTriangle) const {
	if (root->getChildren().size() == 0)
		throw new std::runtime_error("KD-Tree has not been initialized.");
	double d;
	double triangleDist = maxDistance;
	std::priority_queue<KDBoxDistance,std::vector<KDBoxDistance>,CompareKDBox> queue;
	queue.push(KDBoxDistance(pt, getRoot()));
	lastTriangle = nullptr;
	lastPoint = NO_HIT_POINT;
	float3 lastIntersect = NO_HIT_POINT;
	while (queue.size() > 0) {
		KDBoxDistance boxd = queue.top();
		queue.pop();
		if (boxd.box->isLeaf) {
			KDTriangle* tri = dynamic_cast<KDTriangle*>(boxd.box);
			d = tri->distance(pt, lastIntersect);
			if (d <= triangleDist) {
				triangleDist = d;
				lastTriangle = tri;
				lastPoint = lastIntersect;
			}
		} else {
			for (KDBox* child : boxd.box->getChildren()) {
				KDBoxDistance kdb(pt, child);
				if (kdb.dist <= maxDistance) {
					queue.push(kdb);
				}
			}
		}
	}
	if (lastTriangle == nullptr) {
		return NO_HIT_DISTANCE;
	} else {
		return triangleDist;
	}
}
double Intersector::closestPoint(const float3& pt, float3& lastPoint,
		KDTriangle*& lastTriangle) const {
	if (root->getChildren().size() == 0)
		throw new std::runtime_error("KD-Tree has not been initialized.");
	double d;
	double mind = 1E30;
	double triangleDist = 1E30;
	std::priority_queue<KDBoxDistance,std::vector<KDBoxDistance>,CompareKDBox> queue;
	queue.push(KDBoxDistance(pt, getRoot()));
	lastTriangle = nullptr;
	lastPoint = NO_HIT_POINT;
	float3 lastIntersect = NO_HIT_POINT;
	while (queue.size() > 0) {
		KDBoxDistance boxd = queue.top();
		queue.pop();
		if (boxd.dist <= mind || boxd.box->isLeaf) {
			if (boxd.box->isLeaf) {
				mind = boxd.dist;
				KDTriangle* tri = dynamic_cast<KDTriangle*>(boxd.box);
				d = tri->distance(pt, lastIntersect);
				if (d < triangleDist) {
					triangleDist = d;
					lastTriangle = tri;
					lastPoint = lastIntersect;
				}
			} else {
				for (KDBox* child : boxd.box->getChildren()) {
					KDBoxDistance kdb(pt, child);
					queue.push(kdb);
				}
			}
		}
	}
	if (lastTriangle == nullptr) {
		return NO_HIT_DISTANCE;
	} else {
		return triangleDist;
	}
}

double Intersector::closestPointOutside(const float3& r, const float3& v,
		float3& lastPoint, KDTriangle*& lastTriangle) const {
	if (root->getChildren().size() == 0)
		throw new std::runtime_error("KD-Tree has not been initialized.");

	double d;
	double mind = 1E30;
	double triangleDist = 1E30;
	std::priority_queue<KDBoxDistance,std::vector<KDBoxDistance>,CompareKDBox> queue;
	queue.push(KDBoxDistance(r, getRoot()));

	lastTriangle = nullptr;
	lastPoint = NO_HIT_POINT;
	float3 pt;
	float3 testv;
	float3 lastIntersect;
	while (queue.size() > 0) {
		KDBoxDistance boxd = queue.top();
		queue.pop();
		if (boxd.dist <= mind || boxd.box->isLeaf) {
			if (boxd.box->isLeaf) {
				mind = boxd.dist;
				KDTriangle* tri = dynamic_cast<KDTriangle*>(boxd.box);
				d = tri->distance(r, lastIntersect);
				boxd.dist = d;
				if (d < triangleDist) {
					pt = lastIntersect;
					testv = pt - r;
					// Test if point is on one side of plane
					if (dot(testv, v) >= 0) {
						triangleDist = d;
						lastTriangle = tri;
						lastPoint = pt;
					}
				}
			} else {
				for (KDBox* child : boxd.box->getChildren()) {
					queue.push(KDBoxDistance(pt, child));
				}
			}
		}
	}
	if (lastTriangle == nullptr) {
		return NO_HIT_DISTANCE;
	} else {
		return mind;
	}
}

}
