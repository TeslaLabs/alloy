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
#include <cereal/archives/binary.hpp>
#include <cereal/archives/xml.hpp>
#include "AlloyCamera.h"
#include "AlloyIntersector.h"
#include "AlloyLocator.h"
#include "AlloyDistanceField.h"
#include "AlloySparseSolve.h"
#include "AlloyMath.h"
#include "AlloyImage.h"
#include "AlloyVector.h"
#include "AlloyFileUtil.h"
#include "AlloyUI.h"
#include "AlloyMesh.h"
#include "AlloyDenseSolve.h"
#include "AlloyImageProcessing.h"
#include "AlloySparseMatrix.h"
#include "AlloyDenseMatrix.h"
#include "AlloyArray.h"
#include "AlloySpline.h"
#include "cereal/archives/json.hpp"
#include <iostream>
#include <fstream>
#include <random>
#ifndef ALY_WINDOWS
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
namespace aly {
	bool SANITY_CHECK_BSPLINE() {
		Vector2f controlPt;
		controlPt.push_back(float2(-1.75, -1.0f));
		controlPt.push_back(float2(-1.5, -0.5f));
		controlPt.push_back(float2(-1.5, 0.0f));
		controlPt.push_back(float2(-1.25, 0.5f));
		controlPt.push_back(float2(-0.75, 0.75f));
		controlPt.push_back(float2(0.0f, 0.5f));
		controlPt.push_back(float2(0.5f, 0.0f));
		BSpline2f bspline(controlPt, SplineType::Clamp, 3);

		int N = 100;
		Vector2f pts(N + 1);
		Vector1f knots;
		Vector2f controlPts;
		bspline.getKnots(knots);
		bspline.getControlPoints(controlPts);
		std::cout << "Knots\n" << knots << std::endl;
		std::cout << "Control Points\n" << controlPts << std::endl;
		for (int n = 0;n <= N;n++) {
			pts[n] = bspline.evaluate(n / (float)N);
		}
		std::cout << "Curve Points\n" << pts << std::endl;
		return true;
	}
	bool SANITY_CHECK_LOCATOR() {

		{
			int N = 1000;
			std::vector<float2> samples(N);
			for (int n = 0; n < N; n++) {
				samples[n] = float2(RandomUniform(0.0f, 1.0f),
					RandomUniform(0.0f, 1.0f));
			}
			float2 pivot(0.5f, 0.2f);
			std::sort(samples.begin(), samples.end(),
				[=](const float2& a, const float2& b) {
				return (distanceSqr(a, pivot) < distanceSqr(b, pivot));
			});
			Locator2f locator;
			locator.insert(pivot);
			locator.insert(samples);
			float2 q = float2(pivot.x + 1E-3f, pivot.y - 1E-3f);
			float2i qi = float2i(q, 0);
			std::cout << "[Locator2f] Query: " << qi << std::endl;
			float2i hit0 = locator.closest(qi);
			std::cout << "[Locator2f] Closest: " << hit0 << std::endl;
			float2i hit1 = locator.closest(q);
			std::cout << "[Locator2f] Closest: " << hit1 << std::endl;
			float2i hit2 = locator.closest(q, 0.1f);
			std::cout << "[Locator2f] Closest in radius: " << hit2 << std::endl;
			float2i hit3a = locator.closestExact(float2i(pivot, 0));
			float2i hit3b = locator.closestExact(qi);
			std::cout << "[Locator2f] Closest exact: " << hit3a << " " << hit3b
				<< std::endl;
			std::vector<float2i> hits;
			locator.closest(q, 0.05f, hits);
			std::cout << "[Locator2f] Nearest in radius: " << std::endl;
			for (int k = 0; k < (int)hits.size(); k++) {
				std::cout << k << ") " << hits[k] << " " << distance(hits[k], q)
					<< std::endl;
			}

			std::cout << "Locator size " << locator.size() << std::endl;
			locator.erase(float2i(samples[10], 9));
			locator.erase(samples[8]);
			locator.eraseExact(hits[0]);
			std::cout << "Locator size after erase " << locator.size() << std::endl;
		}
		{
			int N = 10000;
			std::vector<float3> samples(N);
			for (int n = 0; n < N; n++) {
				samples[n] = float3(RandomUniform(0.0f, 1.0f),
					RandomUniform(0.0f, 1.0f), RandomUniform(0.0f, 1.0f));
			}
			float3 pivot(0.5f, 0.2f, 0.4f);
			std::sort(samples.begin(), samples.end(),
				[=](const float3& a, const float3& b) {
				return (distanceSqr(a, pivot) < distanceSqr(b, pivot));
			});
			Locator3f locator;
			locator.insert(pivot);
			locator.insert(samples);
			float3 q = float3(pivot.x + 1E-3f, pivot.y - 1E-3f, pivot.z + 0.005f);
			float3i qi = float3i(q, 0);
			std::cout << "[Locator3f] Query: " << qi << std::endl;
			float3i hit0 = locator.closest(qi);
			std::cout << "[Locator3f] Closest: " << hit0 << std::endl;
			float3i hit1 = locator.closest(q);
			std::cout << "[Locator3f] Closest: " << hit1 << std::endl;
			float3i hit2 = locator.closest(q, 0.1f);
			std::cout << "[Locator3f] Closest in radius: " << hit2 << std::endl;
			float3i hit3a = locator.closestExact(float3i(pivot, 0));
			float3i hit3b = locator.closestExact(qi);
			std::cout << "[Locator3f] Closest exact: " << hit3a << " " << hit3b
				<< std::endl;
			std::vector<float3i> hits;
			std::vector<std::pair<float3i, float>> hitPair;
			locator.closest(q, 0.05f, hits);
			locator.closest(q, 0.05f, hitPair);
			std::cout << "[Locator3f] Nearest in radius: " << std::endl;
			for (int k = 0; k < (int)hitPair.size(); k++) {
				std::cout << k << ") " << hitPair[k].first << " "
					<< hitPair[k].second << " " << distance(hitPair[k].first, q)
					<< std::endl;
			}
		}
		{
			int N = 10000;
			const int C = 64;
			float scale = 1.0f / std::sqrt((float)C);
			std::vector<Array<float, C>> samples(N);
			for (int n = 0; n < N; n++) {
				for (int c = 0; c < C; c++) {
					samples[n][c] = RandomUniform(0.0f, 1.0f) * scale;
				}
			}
			Array<float, C> pivot(0.0f);
			for (int c = 0; c < C; c++) {
				pivot[c] = RandomUniform(0.49f, 0.51f) * scale;
			}
			float avg = pivot.mean();
			float med = pivot.median();
			float mn = pivot.min();
			float mx = pivot.max();
			float stddev = pivot.stdDev();
			std::cout << "Pivot mean: " << avg << " std: " << stddev << " median: "
				<< med << " min: " << mn << " max: " << mx << std::endl;
			std::sort(samples.begin(), samples.end(),
				[=](const Array<float, C>& a, const Array<float, C>& b) {
				return (distanceSqr(a, pivot) < distanceSqr(b, pivot));
			});

			Matcher<float, C> matcher(samples);
			std::vector<size_t> hits;
			std::vector<std::pair<size_t, float>> hitPair;
			matcher.closest(pivot, 0.238f, hits);
			matcher.closest(pivot, 0.238f, hitPair);
			matcher.closest(pivot, 0.238f);
			std::cout << "[Matcher<>] Nearest in radius:" << std::endl;
			for (int k = 0; k < (int)hits.size(); k++) {
				std::cout << k << ") " << hitPair[k].first << " "
					<< hitPair[k].second << " "
					<< distance(pivot, samples[hitPair[k].first]) << std::endl;
			}

			matcher.closest(pivot, 10, hits);
			matcher.closest(pivot, 10, hitPair);
			matcher.closest(pivot);
			std::cout << "[Matcher<>] Nearest:" << std::endl;
			for (int k = 0; k < (int)hitPair.size(); k++) {
				std::cout << k << ") " << hitPair[k].first << " "
					<< hitPair[k].second << " "
					<< distance(pivot, samples[hitPair[k].first]) << std::endl;
			}
		}
		{
			int N = 10000;
			Vector3f samples(N);
			for (int n = 0; n < N; n++) {
				samples[n] = float3(RandomUniform(0.0f, 1.0f),
					RandomUniform(0.0f, 1.0f), RandomUniform(0.0f, 1.0f));
			}
			float3 pivot(0.5f, 0.2f, 0.4f);
			std::sort(samples.begin(), samples.end(),
				[=](const float3& a, const float3& b) {
				return (distanceSqr(a, pivot) < distanceSqr(b, pivot));
			});
			Matcher3f matcher(samples);
			std::vector<size_t> hits;
			std::vector<std::pair<size_t, float>> hitPair;
			matcher.closest(pivot, 0.05f, hits);
			matcher.closest(pivot, 0.05f, hitPair);
			matcher.closest(pivot, 0.05f);
			std::cout << "[Matcher3f] Nearest in radius:" << std::endl;
			for (int k = 0; k < (int)hits.size(); k++) {
				std::cout << k << ") " << hitPair[k].first << " "
					<< hitPair[k].second << " "
					<< distance(pivot, samples[hitPair[k].first]) << std::endl;
			}

			matcher.closest(pivot, 10, hits);
			matcher.closest(pivot, 10, hitPair);
			matcher.closest(pivot);
			std::cout << "[Matcher3f] Nearest:" << std::endl;
			for (int k = 0; k < (int)hitPair.size(); k++) {
				std::cout << k << ") " << hitPair[k].first << " "
					<< hitPair[k].second << " "
					<< distance(pivot, samples[hitPair[k].first]) << std::endl;
			}
		}
		return true;
	}
	bool SANITY_CHECK_SUBDIVIDE() {
		Mesh mesh;
		mesh.load(AlloyDefaultContext()->getFullPath("models/monkey.obj"));
		Subdivide(mesh, SubDivisionScheme::CatmullClark);
		Subdivide(mesh, SubDivisionScheme::CatmullClark);
		Subdivide(mesh, SubDivisionScheme::CatmullClark);
		WriteMeshToFile("monkey_catmullclark.ply", mesh);

		mesh.load(AlloyDefaultContext()->getFullPath("models/monkey.obj"));
		Subdivide(mesh, SubDivisionScheme::Loop);
		Subdivide(mesh, SubDivisionScheme::Loop);
		Subdivide(mesh, SubDivisionScheme::Loop);
		WriteMeshToFile("monkey_loop.ply", mesh);

		mesh.load(AlloyDefaultContext()->getFullPath("models/monkey.obj"));
		box3f box = mesh.getBoundingBox();
		mesh.vertexColors.resize(mesh.vertexLocations.size());
		for (int n = 0; n < (int)mesh.vertexLocations.size(); n++) {
			float3 pt = mesh.vertexLocations[n];
			pt = (pt - box.position) / box.dimensions;
			mesh.vertexColors[n] = float4(pt, 1.0f);
		}
		Subdivide(mesh, SubDivisionScheme::CatmullClark);
		Subdivide(mesh, SubDivisionScheme::CatmullClark);
		Subdivide(mesh, SubDivisionScheme::CatmullClark);
		WriteMeshToFile("monkey_catmullclark_color.ply", mesh);

		mesh.load(AlloyDefaultContext()->getFullPath("models/monkey.obj"));
		mesh.vertexColors.resize(mesh.vertexLocations.size());
		for (int n = 0; n < (int)mesh.vertexLocations.size(); n++) {
			float3 pt = mesh.vertexLocations[n];
			pt = (pt - box.position) / box.dimensions;
			mesh.vertexColors[n] = float4(pt, 1.0f);
		}
		Subdivide(mesh, SubDivisionScheme::Loop);
		Subdivide(mesh, SubDivisionScheme::Loop);
		Subdivide(mesh, SubDivisionScheme::Loop);
		WriteMeshToFile("monkey_loop_color.ply", mesh);

		mesh.load(AlloyDefaultContext()->getFullPath("models/tanya.ply"));
		Subdivide(mesh, SubDivisionScheme::CatmullClark);
		mesh.convertQuadsToTriangles();
		WriteMeshToFile("tanya_catmullclark.ply", mesh);

		mesh.load(AlloyDefaultContext()->getFullPath("models/tanya.ply"));
		Subdivide(mesh, SubDivisionScheme::Loop);
		WriteMeshToFile("tanya_loop.ply", mesh);

		return true;
	}
	bool SANITY_CHECK_DENSE_MATRIX() {
		{
			DenseMatrix1f A(17, 9);
			Vector1f b2(A.cols);
			Vector1f b1(A.rows);
			srand(1123437);
			for (int i = 0; i < A.rows; i++) {
				for (int j = 0; j < A.cols; j++) {
					A[i][j] = float1((rand() % 1000) / 1000.0f);
					b2[j] = float1((rand() % 1000) / 1000.0f);
					b1[i] = float1((rand() % 1000) / 1000.0f);
				}
			}

			std::cout << "A=" << A << std::endl;
			DenseMatrix1f L, U, Q, R, S, D, Vt;
			Vector1f x1, x2, x3, x4;

			std::vector<int> piv;
			LU(A, L, U, piv, 0);
			std::cout << "L=" << L << std::endl;
			std::cout << "U=" << U << std::endl;
			QR(A, Q, R);
			std::cout << "Q=" << Q << std::endl;
			std::cout << "R=" << R << std::endl;

			x1 = Solve(A, b1);
			std::cout << "X1=\n" << x1 << std::endl;
			std::cout << "r1=\n" << A * x1 - b1 << std::endl;

			x2 = SolveLU(A, b1);
			std::cout << "X2=\n" << x2 << std::endl;
			std::cout << "r2=\n" << A * x2 - b1 << std::endl;

			x3 = SolveQR(A, b1);
			std::cout << "X3=\n" << x3 << std::endl;
			std::cout << "r3=\n" << A * x3 - b1 << std::endl;

			SVD(A, S, D, Vt);
			std::cout << "U=" << S << std::endl;
			std::cout << "D=" << D << std::endl;
			std::cout << "Vt=" << Vt << std::endl;

			x1 = Solve(A, b2);
			std::cout << "X1=\n" << x1 << std::endl;
			std::cout << "r1=\n" << A * x1 - b2 << std::endl;

			x2 = SolveLU(A, b2);
			std::cout << "X2=\n" << x2 << std::endl;
			std::cout << "r2=\n" << A * x2 - b2 << std::endl;

			x3 = SolveQR(A, b2);
			std::cout << "X3=\n" << x3 << std::endl;
			std::cout << "r3=\n" << A * x3 - b2 << std::endl;

		}

		{
			DenseMatrix3f A(8, 6);
			Vector3f b2(A.cols);
			Vector3f b1(A.rows);
			srand(1123437);
			for (int i = 0; i < A.rows; i++) {
				for (int j = 0; j < A.cols; j++) {
					for (int cc = 0; cc < 3; cc++) {
						A[i][j][cc] = float1((rand() % 1000) / 1000.0f);
						b2[j] = float3((rand() % 1000) / 1000.0f,
							(rand() % 1000) / 1000.0f,
							(rand() % 1000) / 1000.0f);
						b1[i] = float3((rand() % 1000) / 1000.0f,
							(rand() % 1000) / 1000.0f,
							(rand() % 1000) / 1000.0f);
					}
				}
			}
			std::cout << "A=" << A << std::endl;
			DenseMatrix3f Q, R, S, D, Vt;
			DenseMatrix1f L, U;
			Vector3f x1, x2, x3;
			std::vector<int> piv;
			LU(A, L, U, piv, 0);
			std::cout << "L=" << L << std::endl;
			std::cout << "U=" << U << std::endl;
			QR(A, Q, R);
			std::cout << "Q=" << Q << std::endl;
			std::cout << "R=" << R << std::endl;

			A = A.transpose() * A;
			SVD(A, S, D, Vt);
			std::cout << "U=" << S << std::endl;
			std::cout << "D=" << D << std::endl;
			std::cout << "Vt=" << Vt << std::endl;

			A = A.transpose() * A;
			SVD(A, S, D, Vt);
			std::cout << "U=" << S << std::endl;
			std::cout << "D=" << D << std::endl;
			std::cout << "Vt=" << Vt << std::endl;

			x1 = Solve(A, b2);
			std::cout << "X1=\n" << x1 << std::endl;
			std::cout << "r1=\n" << A * x1 - b2 << std::endl;

			x2 = SolveLU(A, b2);
			std::cout << "X2=\n" << x2 << std::endl;
			std::cout << "r2=\n" << A * x2 - b2 << std::endl;

			x3 = SolveQR(A, b2);
			std::cout << "X3=\n" << x3 << std::endl;
			std::cout << "r3=\n" << A * x3 - b2 << std::endl;
		}

		return true;
	}
	bool SANITY_CHECK_ALGO() {
		SparseMatrix4f A(128, 128);
		SparseMatrix1f A1(128, 128);
		Vector4f b(A.rows);
		Vector1f b1(A.rows);
		Vector4f x(A.cols);
		Vector1f x1(A.cols);

		for (int i = 0; i < (int)A.rows; i++) {
			for (int jj = -2; jj <= 2; jj++) {
				A.set(i, i + jj, float4(0.2f));
				A1.set(i, i + jj, float1(0.2f));
			}
			b[i] = float4((rand() % 1000) / 1000.0f);
			b1[i] = float1((rand() % 1000) / 1000.0f);
		}
		SolveVecCG(b, A, x);
		SolveCG(b1, A1, x1);
		SolveVecBICGStab(b, A, x);
		SolveBICGStab(b1, A1, x1);
		std::ofstream os("matrix.json");
		cereal::JSONOutputArchive archiver(os);
		archiver(A);

		return true;
	}
	bool SANITY_CHECK_DISTANCE_FIELD() {
		Mesh mesh;
		mesh.load(AlloyDefaultContext()->getFullPath("models/monkey.ply"));
		Intersector kdTree(mesh, 6);
		box3f bbox = mesh.getBoundingBox();
		float3 center = bbox.position + bbox.dimensions*0.5f;
		float maxDim = 1.1f*aly::max(bbox.dimensions);
		bbox.position = center - float3(0.5f*maxDim);
		bbox.dimensions = float3(maxDim);
		Volume1f vol(64, 64, 64);
		Image1f img(64, 64);
		Volume1f distVol;
		Image1f distImg;
		float voxelSize = maxDim / vol.rows;
		float maxDistance = 3.0f *voxelSize;
#pragma omp parallel for
		for (int k = 0; k < vol.slices; k++) {
			for (int j = 0; j < vol.cols; j++) {
				for (int i = 0; i < vol.rows; i++) {
					float3 pt1 = bbox.position
						+ bbox.dimensions
						* float3((i + 0.5f) / vol.rows, (j + 0.5f) / vol.cols, (k + 0.5f) / vol.slices);
					float3 lastPoint(0.0f);
					double d = kdTree.closestPointSignedDistance(pt1,maxDistance);
					if (d != NO_HIT_DISTANCE) {
						vol(i, j, k).x = (float)d/ voxelSize;
					}
					else {
						vol(i, j, k).x = DistanceField3f::DISTANCE_UNDEFINED;
					}
					if (k == vol.slices / 2) {
						if (d != NO_HIT_DISTANCE) {
							img(i, j).x = (float)d / voxelSize;
						}
						else {
							img(i, j).x = DistanceField3f::DISTANCE_UNDEFINED;
						}
					}
				}
			}
		}
		vol.writeToXML("vol_closest.xml");
		DistanceField3f df3;
		df3.solve(vol, distVol,10.0f);
		distVol.writeToXML("vol_df.xml");
		img.writeToXML("img_closest.xml");
		DistanceField2f df2;
		df2.solve(img, distImg, 10.0f);
		distImg.writeToXML("img_df.xml");
		return true;
	}
	bool SANITY_CHECK_KDTREE() {
		Mesh mesh;
		mesh.load(AlloyDefaultContext()->getFullPath("models/monkey.ply"));
		Intersector kdTree(mesh, 6);
		Camera camera;
		camera.setNearFarPlanes(0.1f, 2.0f);
		camera.setZoom(0.75f);
		mesh.updateBoundingBox();
		box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f),
			float3(1.0f, 1.0f, 1.0f));
		camera.setPose(MakeTransform(mesh.getBoundingBox(), renderBBox));
		camera.aim(box2px(float2(0, 0), float2(320, 240)));
		Image4f rgba(320, 240);
#pragma omp parallel for
		for (int i = 0; i < rgba.width; i++) {
			for (int j = 0; j < rgba.height; j++) {
				float3 pt1 = camera.transformImageToWorld(
					float3((float)i, (float)j, 0.0f), rgba.width,
					rgba.height);
				float3 pt2 = camera.transformImageToWorld(
					float3((float)i, (float)j, 1.0f), rgba.width,
					rgba.height);
				float3 v = normalize(pt2 - pt1);
				float3 lastPoint(0.0f);
				double d = kdTree.intersectRayDistance(pt1, v, lastPoint);
				if (d != NO_HIT_DISTANCE) {
					rgba(i, j) = float4(lastPoint, (float)d);
				}
				else {
					rgba(i, j) = float4(0, 0, 0, 0);
				}
			}
		}
		rgba.writeToXML("depth.xml");
		box3f bbox = mesh.getBoundingBox();
#pragma omp parallel for
		for (int i = 0; i < rgba.width; i++) {
			for (int j = 0; j < rgba.height; j++) {
				float3 pt1 = bbox.position
					+ bbox.dimensions
					* float3(i / (float)rgba.width, 0.5f,
						j / (float)rgba.height);
				float3 lastPoint(0.0f);
				double d = kdTree.closestPoint(pt1, lastPoint);
				if (d != NO_HIT_DISTANCE) {
					rgba(i, j) = float4(lastPoint, (float)d);
				}
				else {
					rgba(i, j) = float4(0, 0, 0, 0);
				}
			}
		}
		rgba.writeToXML("closest.xml");

#pragma omp parallel for
		for (int i = 0; i < rgba.width; i++) {
			for (int j = 0; j < rgba.height; j++) {
				float3 pt1 = bbox.position
					+ bbox.dimensions
					* float3(i / (float)rgba.width, 0.5f,
						j / (float)rgba.height);
				float3 lastPoint(0.0f);
				double d = kdTree.closestPoint(pt1, 0.1f, lastPoint);
				if (d != NO_HIT_DISTANCE) {
					rgba(i, j) = float4(lastPoint, (float)d);
				}
				else {
					rgba(i, j) = float4(0, 0, 0, 0);
				}
			}
		}
		rgba.writeToXML("closest_clamped.xml");
		return true;
	}
	bool SANITY_CHECK_IMAGE_PROCESSING() {
		ImageRGBAf img;
		ImageRGBAf laplacian;
		ImageRGBAf smoothed;
		ImageRGBAf gX, gY;

		ReadImageFromFile(AlloyDefaultContext()->getFullPath("images/sfmarket.png"),
			img);

		ImageRGB img2;
		ReadImageFromFile(AlloyDefaultContext()->getFullPath("images/sfmarket.png"),
			img2);

		ImageRGBA img3;
		ReadImageFromFile(AlloyDefaultContext()->getFullPath("images/sfmarket.png"),
			img3);

		ImageRGBf img4;
		ReadImageFromFile(AlloyDefaultContext()->getFullPath("images/sfmarket.png"),
			img4);
		Image1f gray;
		ConvertImage(img, gray);
		Image1ub gray2;
		ConvertImage(img2, gray2);
		Image2f gray3;
		ConvertImage(img2, gray);
		ConvertImage(img3, gray);
		ConvertImage(img4, gray2);
		ConvertImage(img3, gray2);
		const int N = 5;
		Smooth<N, N>(img, smoothed);
		Laplacian<N, N>(img, laplacian);
		Gradient<N, N>(img, gX, gY);
		GaussianOperators<double, N, N> kernel;
		std::cout << "Smoothed=\n";
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				std::cout << kernel.filter[i][j] << " ";
			}
			std::cout << std::endl;
		}

		std::cout << "Gradient=\n";
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				std::cout << "(" << kernel.filterGradX[i][j] << ", "
					<< kernel.filterGradY[i][j] << ") ";
			}
			std::cout << std::endl;
		}

		std::cout << "Laplacian=\n";
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				std::cout << kernel.filterLaplacian[i][j] << " ";
			}
			std::cout << std::endl;
		}
		WriteImageToFile("smoothed.png", smoothed);

		laplacian.writeToXML("laplacian.xml");

		gX.writeToXML("gradient_x.xml");
		gY.writeToXML("gradient_y.xml");

		return true;
	}
	bool SANITY_CHECK_ROBUST_SOLVE() {
		int N = 1000;
		DenseMatrix1f A(N, 5);
		Vector1f b(N);
		Vector1f Y(5);
		for (int i = 0; i < 5; i++) {
			Y[i] = float1(10 * (rand() % 1000) / 1000.0f);
		}
		for (int i = 0; i < N; i++) {
			std::vector<float1> row(5);
			float1 sum(0.0f);
			for (int j = 0; j < 5; j++) {
				row[j] = float1(10 * ((rand() % 1000) / 1000.0f - 0.5f));
				sum += row[j] * Y[j];
				row[j] += float1(0.1f * ((rand() % 1000) / 1000.0f - 0.5f));
			}
			A[i] = row;
			b[i] = sum;
		}
		std::vector<int> order(N);
		std::random_device rd;
		std::mt19937 g(rd());
		for (int n = 0; n < N; n++) {
			order[n] = n;
		}
		std::shuffle(order.begin(), order.end(), g);

		Vector1f X0 = Solve(A, b);
		Vector1f X1 = SolveRobust(A, b);

		for (int i = 0; i < 10; i++) {
			A[order[i]][rand() % 5] += (i % 2) ? -1000.0f : 1000.0f;
		}

		Vector1f X2 = SolveRansac(A, b, 100, 0.01f);
		std::cout << "Truth\n" << Y << std::endl;
		std::cout << "Linear Solve\n" << X0 << std::endl;
		std::cout << "Robust Solve\n" << X1 << std::endl;
		std::cout << "RANSAC Solve\n" << X2 << std::endl;
		return true;
	}
	bool SANITY_CHECK_DENSE_SOLVE() {
		ImageRGBAf src, tar;
		ReadImageFromFile(AlloyDefaultContext()->getFullPath("images/sfmarket.png"),
			src);
		ReadImageFromFile(AlloyDefaultContext()->getFullPath("images/sfsunset.png"),
			tar);

		std::cout << "Min " << src.min() << std::endl;
		std::cout << "Max " << src.max() << std::endl;
		std::cout << "Range " << src.range() << std::endl;
		std::cout << "Mean " << src.mean() << std::endl;
		std::cout << "Median " << src.median() << std::endl;
		std::cout << "Std. Dev. " << src.stdDev() << std::endl;
		std::cout << "Mad " << src.mad() << std::endl;
		std::cout << "Mad Std. Dev. " << src.madStdDev() << std::endl;
		std::cout << "Min " << aly::min(float3(4, 3, 2), float3(2, 4, 3))
			<< std::endl;
		int w = src.width;
		int h = src.height;
		float r = h * 0.4f;
		ImageRGBAf mask = tar;
		for (int i = 0; i < w; i++) {
			for (int j = 0; j < h; j++) {
				float diff = r
					- length(float2((float)(i - w / 2), (float)(j - h / 2)));
				float alpha = 1.0f - clamp(diff / 128, 0.0f, 1.0f);
				mask(i, j).w = alpha;
			}
		}
		WriteImageToFile("init.png", mask);
		ImageRGBAf out;
		out = tar;
		std::cout << "Normal Laplace Fill" << std::endl;
		LaplaceFill(mask, out, 128);
		WriteImageToFile("laplace_fill.png", out);

		std::cout << "Pyramid Laplace Fill" << std::endl;
		out = tar;
		LaplaceFill(mask, out, 64, 6);
		WriteImageToFile("laplace_fill_pyr.png", out);

		out = tar;
		std::cout << "Poisson Inpaint" << std::endl;
		PoissonInpaint(mask, src, out, 128);
		WriteImageToFile("poisson_inpaint.png", out);

		std::cout << "Poisson Inpaint Pyramid" << std::endl;
		out = tar;
		PoissonInpaint(mask, src, out, 64, 6);
		WriteImageToFile("poisson_inpaint_pyr.png", out);

		out = tar;
		std::cout << "Normal Poisson Blend" << std::endl;
		PoissonBlend(src, out, 128);
		WriteImageToFile("poisson_blend.png", out);
		out = tar;

		std::cout << "Pyramid Poisson Blend" << std::endl;
		PoissonBlend(src, out, 32, 6);
		WriteImageToFile("poisson_blend_pyr.png", out);
		std::cout << "Done!" << std::endl;

		return true;
	}
	bool SANITY_CHECK_PYRAMID() {
		ImageRGBAf img;
		ReadImageFromFile(AlloyDefaultContext()->getFullPath("images/sfmarket.png"),
			img);
		std::cout << "Down sample" << std::endl;
		ImageRGBAf imgDown = img.downSample();
		WriteImageToFile("image_downsample.png", imgDown);
		std::cout << "Up sample" << std::endl;
		ImageRGBAf imgUp = imgDown.upSample();
		WriteImageToFile("image_upsample.png", imgUp);

		ImageRGBAf compose, crop;

		Crop(img, crop, int2(120, 170), int2(640, 480));
		WriteImageToFile("crop.png", crop);
		ImageRGBAf imgUpDown = imgUp.downSample();
		ImageRGBAf diff(imgUpDown.width, imgUpDown.height);

		float1 val;
		val = 3.0f;
		for (int i = 0; i < imgUpDown.width; i++) {
			for (int j = 0; j < imgUpDown.height; j++) {
				RGBAf c = float4(imgDown(i, j)) - float4(imgUpDown(i, j));
				diff(i, j) = float4(c.xyz(), 1.0f);
			}
		}

		Tile(std::vector<ImageRGBAf> { imgDown, imgUp, img, imgDown.downSample(),
			imgDown.downSample(), imgDown.downSample().downSample(), crop },
			compose, 3, 3);
		WriteImageToFile("compose1.png", compose);
		std::vector<ImageRGBAf> ilist = { imgDown, imgUp, img, imgDown.downSample(),
				imgDown.downSample(), imgDown.downSample().downSample(), crop };
		Tile(ilist, compose, 4, 2);
		WriteImageToFile("compose2.png", compose);
		diff.writeToXML("image_diff.xml");
		return true;
	}
	bool SANITY_CHECK_MESH_IO() {
		Mesh tmpMesh;
		tmpMesh.load(AlloyDefaultContext()->getFullPath("models/torus.ply"));
		tmpMesh.updateVertexNormals();
		tmpMesh.textureMap.resize(
			tmpMesh.quadIndexes.size() * 4 + tmpMesh.triIndexes.size() * 3);
		for (int i = 0; i < (int)tmpMesh.textureMap.size(); i++) {
			tmpMesh.textureMap[i] = float2((rand() % 1024) / 1024.0f,
				(rand() % 1024) / 1024.0f);
		}
		WritePlyMeshToFile("torus2.ply", tmpMesh, true);
		ReadMeshFromFile("torus2.ply", tmpMesh);
		WritePlyMeshToFile("torus3.ply", tmpMesh, false);
		ReadMeshFromFile("torus3.ply", tmpMesh);

		tmpMesh.load(AlloyDefaultContext()->getFullPath("models/icosahedron.ply"));
		tmpMesh.updateVertexNormals();
		tmpMesh.textureMap.resize(
			tmpMesh.quadIndexes.size() * 4 + tmpMesh.triIndexes.size() * 3);
		for (int i = 0; i < (int)tmpMesh.textureMap.size(); i++) {
			tmpMesh.textureMap[i] = float2((rand() % 1024) / 1024.0f,
				(rand() % 1024) / 1024.0f);
		}

		WritePlyMeshToFile("icosahedron2.ply", tmpMesh, true);
		ReadMeshFromFile("icosahedron2.ply", tmpMesh);
		tmpMesh.textureImage.resize(640, 480);
		tmpMesh.textureImage.set(RGBAf(1.0f, 0.0, 0.0, 1.0f));
		WritePlyMeshToFile("icosahedron3.ply", tmpMesh, false);
		ReadMeshFromFile("icosahedron3.ply", tmpMesh);
		return true;
	}
	bool SANITY_CHECK_SPARSE_SOLVE() {
		SparseMatrix1f A(4, 3);
		SparseMatrix1f B(3, 4);
		float4x3 Ad;
		float3x4 Bd;
		A(0, 0) = float1(1.2f);
		A(3, 2) = float1(3.5f);
		A(2, 2) = float1(1);
		A(1, 1) = float1(3);
		A(0, 2) = float1(4);

		B(0, 0) = float1(1.0f);
		B(2, 1) = float1(2.5f);
		B(2, 2) = float1(7);
		B(1, 1) = float1(0.1f);
		B(2, 0) = float1(2);
		for (int i = 0; i < (int)A.rows; i++) {
			for (int j = 0; j < (int)A.cols; j++) {
				Ad(i, j) = A(i, j);
			}
		}
		for (int i = 0; i < (int)B.rows; i++) {
			for (int j = 0; j < (int)B.cols; j++) {
				Bd(i, j) = B(i, j);
			}
		}
		std::cout << A << std::endl;
		std::cout << B << std::endl;
		SparseMatrix1f C;
		SparseMatrix4f I = SparseMatrix4f::identity(30, 25);
		{
			auto Cd = Ad * Bd;
			C = A * B;
			std::cout << "Cd=\n" << Cd << std::endl;
			for (int i = 0; i < (int)C.rows; i++) {
				for (int j = 0; j < (int)C.cols; j++) {
					Cd(i, j) = C(i, j);
				}
			}
			std::cout << "C=\n" << Cd << std::endl;
		}

		{
			C = B * A;
			auto Cd = Bd * Ad;
			std::cout << "Cd=\n" << Cd << std::endl;
			for (int i = 0; i < (int)C.rows; i++) {
				for (int j = 0; j < (int)C.cols; j++) {
					Cd(i, j) = C(i, j);
				}
			}
			std::cout << "C=\n" << Cd << std::endl;
		}
		A = A.transpose();
		std::cout << "At=\n" << A << std::endl;
		return true;
		MeshListNeighborTable vertTable;
		Mesh mesh;
		mesh.load(AlloyDefaultContext()->getFullPath("models/monkey.ply"));
		CreateOrderedVertexNeighborTable(mesh, vertTable);
		srand(1023172413L);
		mesh.updateVertexNormals();
		SparseMatrix<float, 1> L(mesh.vertexLocations.size(),
			mesh.vertexLocations.size());
		Vector3f b(L.rows);
		srand(213823);
		for (size_t i = 0; i < L.rows; i++) {

			const float w = 0.9f;
			for (uint32_t v : vertTable[i]) {
				L.set(i, v, float1(-w));
			}
			L.set(i, i, float1(1.0f + (float)w * vertTable[i].size()));
			float3 norm = mesh.vertexNormals[i];
			mesh.vertexLocations[i] += 5.0f * norm
				* (((rand() % 1024) / 1024.0f) - 0.5f);
			b[i] = mesh.vertexLocations[i];
		}
		//WriteMeshToFile("smoothed_before.ply", mesh);
		/*
		 SolveVecCG(b, L, mesh.vertexLocations, 100, 1E-6f,
		 [this](int iter,double err) {
		 std::cout<<"Iteration "<<iter<<":: "<<err<<std::endl;
		 });
		 */
		SolveBICGStab(b, L, mesh.vertexLocations, 500, 1E-6f,
			[=](int iter, double err) {
			std::cout << "Iteration " << iter << ":: " << err << std::endl;
			return true;
		});
		return true;
	}
	bool SANITY_CHECK_MATH() {
		try {
			std::cout << "Sanity Check .." << std::endl;
			float optimized1;
			std::function<double(const float& value)> scoreFunc1 = [=](const float& val) {
				return (double)(val - 3.0f)*(val - 3.0f);
			};
			double err = LineSearch(optimized1, -1.0f, 5.0f, scoreFunc1);
			std::cout << "Min " << optimized1 << " Error " << err << std::endl;
			float2 optimized2;
			std::function<double(const float2& value)> scoreFunc2 = [=](const float2& val) {
				return (double)((val.x - 3.0f)*(val.x - 3.0f) + (val.y - 5.0f)*(val.y - 5.0f));
			};
			float2 dir = normalize(float2(7.0f, 4.0f));
			err = LineSearch(optimized2, float2(3.0f, 5.0f) - 2.0f*dir, float2(3.0f, 5.0f) + 3.0f*dir, scoreFunc2);
			std::cout << "Min " << optimized2 << " Error " << err << std::endl;
			float4 pt(0.1f, 5.3f, 2.4f, 6.7f);
			float4x4 M1(1.0f);
			float4x4 M2({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 });
			float4x4 M3 = { { 0, 1, 2, 3 }, { 4, 5, 6, 7 }, { 8, 9, 10, 11 }, { 12,
					13, 14, 15 } };
			float4x4 Id = float4x4::identity();
			M1 = ALY_PI * M1;
			pt = M2 * pt;
			M2 *= 1.0f / 16.0f;

			M2 += transpose(M2);

			M2 += float4x4::identity() - float4x4::zero();

			return true;
			float det = determinant(M2);

			M2 = inverse(M2);
			pt = pt.xyz().xy().xyz().xyzw();
			float3 pt2(0.1f, 0.4f, 0.3f);
			float3x4 M3r = SubRowMatrix(M3);
			float4x3 M3c = SubColMatrix(M3);
			float3x3 M3rc = SubMatrix(M3);
			float2x2 M2rc = SubMatrix(M3rc);

			float4x4 Zr = inverse(M2) * M2 - Identity<float, 4, 4>();

			float3 v = pt * M3c * pt2;
			float len = length(v);
			float d = distance(v, float3(0, 2, 1));
			v = floor(v + 0.1f);
			v = ceil(v - 0.2f);
			v = abs(-v);
			float4x4 T = MakeTranslation(float3(1, 2, 3));
			float4x4 S = MakeScale(float3(0.1f, 0.2f, 0.3f));
			float4x4 S2 = MakeScale(0.1f);

			float3 neg = -float3(0.1f, 0.2f, 0.3f);
			float single = float1(0.7f);
			float4x4 RX = MakeRotationX(0.3f);
			float4x4 RY = MakeRotationY(0.7f);
			float4x4 RZ = MakeRotationZ(0.5f);
			float4x4 R = MakeRotation(normalize(float3(1, 1, 0)),
				0.3333f * ALY_PI_2);
			float3 out1 = Transform(R, float3(1, 2, 3));
			float4 out2 = Transform(SubMatrix(R), float4(1, 2, 3, 0));

			out2 = Transform(R, float4(1, 2, 3, 0));
			out1 = Transform(SubMatrix(R), float3(1, 2, 3));
			float angle = Angle(float3(0.1f, 0.6f, 0.2f), float3(0, 0, 0),
				SubMatrix(R) * float3(0.1f, 0.6f, 0.2f));
			v = aly::max(v, pt.xyz());
			float3 v2 = mix(v, float3(0, 3, 5), float3(0.1f, 0.2f, 0.5));
			float3 v3 = v2 * 2.0f - 1.0f;
			v = mix(v2, v3, 0.7f);
			return true;
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl;
			return false;
		}

	}
	bool SANITY_CHECK_CEREAL() {

		float4 v1(1, 2, 3, 4);
		float3 v2(1, 2, 3);
		float2 v3(1, 2);
		float1 v4(1);

		float4x4 MR1 = MakeRotation(normalize(float3(0.1f, 0.5f, 0.3f)),
			ALY_PI * 0.333f) * MakeTranslation(float4(89, 43, 21, 1));
		float3x3 MR2 = SubMatrix(MakeRotation(normalize(float3(0.1f, 0.5f, 0.3f)),
			ALY_PI * 0.1f) * MakeTranslation(float4(89, 43, 21, 1)));
		float4x3 MR3 = SubColMatrix(
			MakeRotation(normalize(float3(0.1f, 0.5f, 0.3f)),
				ALY_PI * 0.1f) * MakeTranslation(float4(89, 43, 21, 1)));

		Image4f im1(32, 24);
		Image4f im2;
		im2 = im1;
		std::srand(10832145);
		for (int i = 0; i < im1.width; i++) {
			for (int j = 0; j < im1.height; j++) {
				im1(i, j) = float4((float)i, (float)j,
					(std::rand() % 256) / (256.0f), 1.0f);
			}
		}
		Vector4f data(16);
		for (int i = 0; i < (int)data.size(); i++) {
			data[i] = float4((float)i, (float)((i + 32) % 4),
				(std::rand() % 256) / (256.0f), 1.0f);
		}

		Vector4f deserial(16);
		std::string encodedString = EncodeBase64(data.data);
		DecodeBase64(encodedString, deserial.data);
		for (int i = 0; i < (int)data.size(); i++) {
			std::cout << i << ") " << data[i] << " " << deserial[i] << std::endl;
		}

		std::cout << im1.updateHashCode(0, HashMethod::SHA1) << std::endl;
		std::cout << im1.updateHashCode(0, HashMethod::SHA224) << std::endl;
		std::cout << im1.updateHashCode(0, HashMethod::SHA256) << std::endl;
		std::cout << im1.updateHashCode(0, HashMethod::SHA384) << std::endl;
		std::cout << im1.updateHashCode(0, HashMethod::SHA512) << std::endl;

		Integer value1(4);
		Double value2(3.14159);
		Float value3(3.14f);
		Color c(0.5f, 0.4f, 0.1f, 1.0f);
		Number num = Float(1.222f);
		AUnit1D unit1 = UnitPerPT(0.5f, 3.0f);
		AUnit2D unit2 = CoordPerPX(0.5f, 0.3f, 3.0f, 10.0f);
		AUnit1D unit3 = UnitDP(0.2f);
		AUnit2D unit4 = CoordMM(100, 200);

		{
			std::ofstream os("nums.xml");
			cereal::XMLOutputArchive archiver(os);
			archiver(value1, value2, value3, num, c, unit1, unit2, unit3, unit4);
		}
		{
			std::ofstream os("data.xml");
			cereal::XMLOutputArchive archiver(os);
			archiver(v1, v2, v3, v4, MR1, MR2, MR3, im1, data);
		}
		{
			std::ofstream os("data.json");
			cereal::JSONOutputArchive archiver(os);
			archiver(v1, v2, v3, v4, MR1, MR2, MR3, im1, data);
		}
		{
			std::ofstream os("data.bin");
			cereal::BinaryOutputArchive archiver(os);
			archiver(v1, v2, v3, v4, MR1, MR2, MR3, im1, data);
		}
		{

			Mesh mesh;
			ReadMeshFromFile("assets/models/monkey.ply", mesh);
			std::ofstream os("monkey.json");
			cereal::JSONOutputArchive archiver(os);
			archiver(cereal::make_nvp("monkey", mesh));
		}
		//std::cout << "Hit any key ..." << std::endl;
		//std::cout << "Data Size " << im1.data.size() << std::endl;
		//getchar();
		return true;
	}
	bool SANITY_CHECK_IMAGE_IO() {
		ImageRGBAf srcRGBAf;
		ImageRGBf srcRGBf;
		ImageAf srcAf;
		ImageRGB srcRGB;
		ReadImageFromFile(AlloyDefaultContext()->getFullPath("images/sfmarket.png"),
			srcRGBAf);
		ReadImageFromFile(AlloyDefaultContext()->getFullPath("images/sfmarket.png"),
			srcRGB);
		ReadImageFromFile(AlloyDefaultContext()->getFullPath("images/sfmarket.png"),
			srcRGBf);
		ReadImageFromFile(AlloyDefaultContext()->getFullPath("images/sfmarket.png"),
			srcAf);
		aly::WriteImageToFile("sfmarket_rgba.exr", srcRGBAf);
		aly::WriteImageToFile("sfmarket_rgb.exr", srcRGBf);
		aly::WriteImageToFile("sfmarket_r.exr", srcAf);

		aly::WriteImageToFile("sfmarket_rgba.hdr", srcRGBAf);
		aly::WriteImageToFile("sfmarket_rgb.hdr", srcRGBf);
		aly::WriteImageToFile("sfmarket_r.hdr", srcAf);

		ReadImageFromFile("sfmarket_rgba.exr", srcRGBAf);
		ReadImageFromFile("sfmarket_rgb.exr", srcRGBf);
		ReadImageFromFile("sfmarket_r.exr", srcAf);

		aly::WriteImageToFile("sfmarket_rgba_exr.png", srcRGBAf);
		aly::WriteImageToFile("sfmarket_rgb_exr.png", srcRGBf);
		aly::WriteImageToFile("sfmarket_r_exr.png", srcAf);

		aly::ReadImageFromFile("sfmarket_rgba.hdr", srcRGBAf);
		aly::ReadImageFromFile("sfmarket_rgb.hdr", srcRGBf);
		aly::ReadImageFromFile("sfmarket_r.hdr", srcAf);

		aly::WriteImageToFile("sfmarket_rgba_hdr.png", srcRGBAf);
		aly::WriteImageToFile("sfmarket_rgb2_hdr.png", srcRGBf);
		aly::WriteImageToFile("sfmarket_r_hdr.png", srcAf);
		return true;
	}
	bool SANITY_CHECK_IMAGE() {
		try {
			std::cout << "Sanity check image ..." << std::endl;
			Image4f im1(320, 240);
			Image4f im2;
			im2.resize(320, 240);
			im2.setZero();
			std::cout << im1 << std::endl;
			std::cout << im2 << std::endl;
			Image4f out = float4(1, 2, 3, 1) + im2;
			Image4f im3 = -out;
			std::cout << out << std::endl;
			out += im1;
			std::cout << out << std::endl;
			out /= float4(0.3f);
			out += float4(1.0f);
			out = im2 - im1;
			float4 val2 = im1(37.1f, 32.2f) + im2(float2(21.4f, 56.2f));
			float4 val3 = im1(37, 32) + im2(float2(21, 56));
			return true;
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl;
			return false;
		}
	}
	bool SANITY_CHECK_SVD() {

		int N = 100;
		std::uniform_real_distribution<float> r(-1.0f, 1.0f);
		std::random_device rd;
		std::mt19937 gen(rd());
		float3x3 M = float3x3::identity();
		float3x3 R = SubMatrix(
			MakeRotation(normalize(float3(r(gen), r(gen), r(gen))),
				(float)(r(gen) * ALY_PI * 2)));
		float3x3 S = SubMatrix(MakeScale(float3(r(gen), r(gen), r(gen))));
		std::vector<float3> in(N);
		float3 avgIn;
		float3 avgOut;
		for (int n = 0; n < N; n++) {
			float x = 10 * r(gen);
			float y = 10 * r(gen);
			float z = 10 * r(gen);
			float3 pt(x, y, z);
			float3 qt = R * pt - pt;
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					M(i, j) += qt[i] * qt[j];
				}
			}
		}
		std::cout << "M=\n" << M << std::endl;
		float3x3 Q, D;
		Eigen(M, Q, D);
		std::cout << "Q=\n" << Q << std::endl;
		std::cout << "R=\n" << R << std::endl;
		std::cout << "D=\n" << D << std::endl;
		float3x3 QDQt = Q * D * transpose(Q);
		std::cout << "QDQt=\n" << QDQt * inverse(M) << std::endl;
		float3x3 U, Vt;
		SVD(M, U, D, Vt);
		std::cout << "U=\n" << U << std::endl;
		std::cout << "D=\n" << D << std::endl;
		std::cout << "Vt=\n" << Vt << std::endl;
		std::cout << "UDVt=\n" << U * D * Vt * inverse(M) << std::endl;
		float3x3 Rest = FactorRotation(M);
		float3x3 A1 = U * D * Vt;
		float3x3 A2 = U * MakeDiagonal(float3(1, 1, -1)) * D * Vt;
		std::cout << "Determinant " << determinant(A1) << " " << determinant(A2)
			<< std::endl;
		std::cout << "Rotation " << Rest << std::endl;
		/*

		 */
		return true;
	}
	bool SANITY_CHECK_LINALG() {

		try {
			std::cout << "Sanity check vector ..." << std::endl;
			Vector4f im1(32);
			Vector4f im2;
			im2.resize(32);
			im2.apply(
				[](size_t offset, float4& val) {val = float4((float)(offset % 8), 0.0f, -(float)offset, 1);});
			im1.apply(
				[](size_t offset, float4& val) {val = float4((float)(offset % 8), (float)((offset + 7) % 5), (float)(offset % 3), 1.0f);});
			std::cout << im1 << std::endl;
			std::cout << im2 << std::endl;
			Vector4f im3 = -im1;
			float d1 = (float)(dot(im1, im2) + length(im1) + lengthL1(im2)
				+ max(im1) - min(im1));
			float4 d2 = float4(dotVec(im1, im2)) + float4(lengthVec(im2))
				+ lengthVecL1(im1) + maxVec(im1) - minVec(im1);
			Vector4f out = float4(1, 2, 3, 1) + im2;
			std::cout << "Dots " << d1 << " " << d2 << std::endl;

			out += im1;
			out /= float4(0.3f);
			out += float4(1.0f);
			out = im2 - im1;

			return true;
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl;
			return false;
		}
	}
	bool SANITY_CHECK_UI() {
		CoordPercent rel(0.5f, 0.75f);
		CoordDP abs(40, 30);
		CoordIN in(1, 1);
		CoordPerDP pt(0.5f, 0.75f, 40, 20);
		std::cout << rel << std::endl;
		std::cout << abs << std::endl;
		std::cout << in << std::endl;
		std::cout << pt << std::endl;
		TextLabel* r1 = new TextLabel();
		TextLabel* r2 = new TextLabel("region 2");
		Composite comp;
		comp.add(TextLabelPtr(r1));
		comp.add(TextLabelPtr(r2));
		return true;
	}

#ifndef WIN32
	bool SANITY_CHECK_FILE_IO() {
		try {
			std::cout << RemoveTrailingSlash("/usr/local/bin/") << "::"
				<< RemoveTrailingSlash("/usr/local/bin/") << std::endl;
			std::vector<std::string> files = GetDirectoryFileListing("./images/");

			for (std::string f : files) {
				ImageRGB im1;
				ImageRGBA im2;
				std::cout << f << std::endl;
				GetFileExtension(f);
				GetFileNameWithoutExtension(f);
				GetFileWithoutExtension(f);
				GetFileName(f);
				ConcatPath(GetFileDirectoryPath(f), GetFileName(f));
				ReadImageFromFile(f, im1);
				ReadImageFromFile(f, im2);
				WriteImageToFile("/home/blake/" + GetFileName(f) + "_copy_rgb.png",
					im1);
				WriteImageToFile("/home/blake/" + GetFileName(f) + "_copy_rgba.png",
					im2);
			}
			return true;
		}
		catch (std::exception& e) {
			return false;
		}
	}
#endif

	}

