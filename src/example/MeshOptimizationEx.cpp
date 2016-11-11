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

#include "Alloy.h"
#include "AlloySparseSolve.h"
#include "AlloyOptimization.h"
#include "../../include/example/MeshOptimizationEx.h"

using namespace aly;
MeshOptimizationEx::MeshOptimizationEx() :
		Application(1200, 600, "Mesh Optimization Example"), matcapShader(getFullPath("images/JG_Silver.png")) {
}
bool MeshOptimizationEx::init(Composite& rootNode) {
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f), float3(1.0f, 1.0f, 1.0f));
	sourceMesh.load(getFullPath("models/female_head.ply"));
	sourceMesh.updateBoundingBox();
	sourceMesh.transform(MakeTranslation(-sourceMesh.getBoundingBox().center()));
	sourceMesh.updateVertexNormals();

	targetMesh.load(getFullPath("models/female_head.ply"));
	targetMesh.updateBoundingBox();
	targetMesh.transform(MakeTranslation(-targetMesh.getBoundingBox().center()));
	targetMesh.updateVertexNormals();

	int w = getContext()->getScreenHeight();
	int h = getContext()->getScreenHeight();

	//Initialize depth buffer to store the render
	sourceDepthBuffer.initialize(w, h);
	targetDepthBuffer.initialize(w, h);
	//Set up camera
	camera.setNearFarPlanes(0.1f, 2.0f);
	camera.setZoom(0.75f);
	camera.setCameraType(CameraType::Perspective);
	camera.setDirty(true);
	//Map object geometry into unit bounding box for draw.
	camera.setPose(MakeTransform(sourceMesh.getBoundingBox(), renderBBox));
	//Add listener to respond to mouse manipulations
	addListener(&camera);
	textLabel = TextLabelPtr(new TextLabel("", CoordPX(5, 5), CoordPercent(0.5f, 0.5f)));
	textLabel->fontSize = UnitPX(20.0f);
	textLabel->fontType = FontType::Bold;
	textLabel->fontStyle = FontStyle::Outline;
	buttonPanel = CompositePtr(new Composite("Buttons", CoordPerPX(0.5f, 1.0f, -121.0f, -35.0f), CoordPX(222.0f, 30.0f)));
	buttonPanel->setOrientation(Orientation::Horizontal, pixel2(2, 0), pixel2(0, 0));
	TextButtonPtr smoothButton = TextButtonPtr(new TextButton("Smooth", CoordPX(0, 0), CoordPX(120, 30)));
	TextButtonPtr detailButton = TextButtonPtr(new TextButton("Detail Transfer", CoordPX(0, 0), CoordPX(120, 30)));
	buttonPanel->add(smoothButton);
	buttonPanel->add(detailButton);

	rootNode.add(textLabel);
	rootNode.add(buttonPanel);
	smoothTask = WorkerTaskPtr(new WorkerTask([=] {
		textLabel->setLabel("Smoothing Mesh ...");
		buttonPanel->setVisible(false);
		smooth(sourceMesh);
		sourceMesh.setDirty(true);
		camera.setDirty(true);
		buttonPanel->setVisible(true);
		textLabel->setLabel("");
	}));

	detailTask = WorkerTaskPtr(new WorkerTask([=] {
		textLabel->setLabel("Detail Transfer ...");
		buttonPanel->setVisible(false);
		detailTransfer(sourceMesh,targetMesh,(sourceMesh.vertexLocations.size()==targetMesh.vertexLocations.size()));
		sourceMesh.setDirty(true);
		targetMesh.setDirty(true);
		camera.setDirty(true);
		buttonPanel->setVisible(true);
		textLabel->setLabel("");
	}));
	smoothButton->onMouseDown = [=](AlloyContext* context, const InputEvent& e) {
		smoothTask->cancel();
		smoothTask->execute();
		return true;
	};
	detailButton->onMouseDown = [=](AlloyContext* context, const InputEvent& e) {
		detailTask->cancel();
		detailTask->execute();
		return true;
	};
	return true;
}

void MeshOptimizationEx::detailTransfer(aly::Mesh& src, const aly::Mesh& tar, bool aligned) {
	float alpha = 0.75f;
	float smoothness = 10.0f;
	textLabel->setLabel("Computing Laplacian ...");
	int N = (int) src.vertexLocations.size();
	float3 dims = src.getBoundingBox().dimensions;
	float searchDistance = 0.1f * std::max(std::max(dims.x, dims.y), dims.z);
	Vector3f matchedLaplacian;
	if (aligned) {
		matchedLaplacian = computeLaplacian(tar);
	} else {
		matchedLaplacian.resize(N);
		Vector3f laplacian = computeLaplacian(tar);
		Intersector locator;
		textLabel->setLabel("Building Locator ...");
		locator.build(tar, 8);
		Vector3f matchedLaplacian(N);
		textLabel->setLabel("Matching Laplacian ...");
		const int stride = 1;
#pragma omp parallel for
		for (int i = 0; i < N; i += stride) {
			int n = (stride == 1) ? i : RandomUniform(i, std::min(i + stride, N) - 1);
			float3 pt;
			KDTriangle* tri;
			double d = locator.closestPoint(src.vertexLocations[n], searchDistance, pt, tri);
			if (d != NO_HIT_DISTANCE) {
				float3 bary = float3(tri->toBary(pt));
				uint3 face = tar.triIndexes[tri->id];
				matchedLaplacian[n] = bary.x * laplacian[face.x] + bary.y * laplacian[face.y] + bary.z * laplacian[face.z];
			}
		}
	}
	textLabel->setLabel("Constructing Solver ...");
	MeshListNeighborTable nbrTable;
	CreateOrderedVertexNeighborTable(src, nbrTable, false);
	int index = 0;
	std::vector<float> angles;
	std::vector<float> weights;

	N = 3 * (int) src.vertexLocations.size();
	SparseMat<float> A(N, N);
	Vec<float> b(N);
	Vec<float> X(N);
	index = 0;
	for (std::list<uint32_t>& nbrs : nbrTable) {
		int K = (int)nbrs.size();
		float3 pt = src.vertexLocations[index];
		X[3 * index] = pt.x;
		X[3 * index + 1] = pt.y;
		X[3 * index + 2] = pt.z;
		if (K < 2) {
			b[3 * index] = float1(0.0f);
			b[3 * index + 1] = float1(0.0f);
			b[3 * index + 2] = float1(0.0f);
			index++;
			continue;
		}
		float3 Ls = pt;
		float w = 1.0f / K;
		for (uint32_t offset : nbrs) {
			Ls -= w * src.vertexLocations[offset];
			A.set(3 * index, 3 * offset, -smoothness * w);
			A.set(3 * index + 1, 3 * offset + 1, -smoothness * w);
			A.set(3 * index + 2, 3 * offset + 2, -smoothness * w);
		}
		float3 Lt = matchedLaplacian[index];
		float3 L = mix(Ls, Lt, alpha);
		A.set(3 * index, 3 * index, smoothness + 1);
		A.set(3 * index + 1, 3 * index + 1, smoothness + 1);
		A.set(3 * index + 2, 3 * index + 2, smoothness + 1);
		b[3 * index] = smoothness * L.x + +pt.x;
		b[3 * index + 1] = smoothness * L.y + pt.y;
		b[3 * index + 2] = smoothness * L.z + pt.z;
		index++;
	}
	SolveBICGStab(b, A, X, 100, 1E-9f, [this](int innerIter, double error) {
		textLabel->setLabel(MakeString()<< "Detail Transfer [" << innerIter << "] Error: " <<error);
		return true;
	});
	src.vertexLocations.set(X.data());
	src.updateVertexNormals();
	sourceMesh.setDirty(true);
	camera.setDirty(true);

}
aly::Vector3f MeshOptimizationEx::computeLaplacian(const aly::Mesh& mesh) {
	aly::Vector3f out(mesh.vertexLocations.size());
	MeshListNeighborTable nbrTable;
	CreateOrderedVertexNeighborTable(mesh, nbrTable, false);
	int index = 0;
	std::vector<float> angles;
	std::vector<float> weights;
	for (std::list<uint32_t>& nbrs : nbrTable) {
		int K = (int)nbrs.size();
		float3 Laplacian = mesh.vertexLocations[index];
		float w = 1.0f / K;
		for (uint32_t offset : nbrs) {
			Laplacian -= w * mesh.vertexLocations[offset];
		}
		out[index++] = Laplacian;
	}
	return out;
}
void MeshOptimizationEx::smooth(aly::Mesh& mesh) {
	MeshListNeighborTable nbrTable;
	CreateOrderedVertexNeighborTable(mesh, nbrTable, true);
	int index = 0;
	std::vector<float> angles;
	std::vector<float> weights;
	float smoothness = 10.0f;
	int N = 3 * (int) mesh.vertexLocations.size();
	SparseMat<float> A(N, N);
	Vec<float> b(N);
	Vec<float> X(N);
	for (std::list<uint32_t>& nbrs : nbrTable) {
		int K = (int) nbrs.size() - 1;
		float3 pt = mesh.vertexLocations[index];
		X[3 * index] = pt.x;
		X[3 * index + 1] = pt.y;
		X[3 * index + 2] = pt.z;
		angles.resize(K);
		weights.resize(K);
		{
			auto nbrIter = nbrs.begin();
			for (int k = 0; k < K; k++) {
				float3 current = mesh.vertexLocations[*nbrIter];
				nbrIter++;
				float3 next = mesh.vertexLocations[*nbrIter];
				angles[k] = std::tan(Angle(next, pt, current) * 0.5f);
			}
		}
		float wsum = 0.0f;
		{
			auto nbrIter = nbrs.begin();
			nbrIter++;
			for (int k = 0; k < K; k++) {
				float3 ptNext = mesh.vertexLocations[*nbrIter];
				float d = distance(pt, ptNext);
				if (d > 1E-6f) {
					float w = (angles[k] + angles[(k + 1) % K]) / d;
					wsum += w;
					weights[k] = w;
				} else {
					weights[k] = 0;
				}
				nbrIter++;
			}
		}
		{
			auto nbrIter = nbrs.begin();
			nbrIter++;
			if (wsum > 0) {
				for (int k = 0; k < K; k++) {
					float w = -smoothness * weights[k] / wsum;
					int offset = *nbrIter;
					A.set(3 * index, 3 * offset, w);
					A.set(3 * index + 1, 3 * offset + 1, w);
					A.set(3 * index + 2, 3 * offset + 2, w);
					nbrIter++;
				}
			} else {
				for (int k = 0; k < K; k++) {
					float w = -smoothness / K;
					int offset = *nbrIter;
					A.set(3 * index, 3 * offset, w);
					A.set(3 * index + 1, 3 * offset + 1, w);
					A.set(3 * index + 2, 3 * offset + 2, w);
					nbrIter++;
				}
			}
		}
		A.set(3 * index, 3 * index, smoothness + 1);
		A.set(3 * index + 1, 3 * index + 1, smoothness + 1);
		A.set(3 * index + 2, 3 * index + 2, smoothness + 1);

		b[3 * index] = pt.x;
		b[3 * index + 1] = pt.y;
		b[3 * index + 2] = pt.z;
		index++;
	}
	SolveBICGStab(b, A, X, 100, 1E-9f, [=](int iter, double error) {
		textLabel->setLabel(MakeString() << "Smooth [" << iter << "] Error: " << error);
		return true;
	});
	mesh.vertexLocations.set(X.data());
	mesh.updateVertexNormals();
}
void MeshOptimizationEx::draw(AlloyContext* context) {
	if (camera.isDirty()) {
		//Compute depth and normals only when camera view changes.
		depthAndNormalShader.draw(sourceMesh, camera, sourceDepthBuffer);
		depthAndNormalShader.draw(targetMesh, camera, targetDepthBuffer);
	}
	//Recompute lighting at every draw pass.
	box2f srcBox = context->pixelRatio * box2px(float2(0.0f, 0.0f), float2((float)sourceDepthBuffer.width(), (float)sourceDepthBuffer.height()));
	box2f tarBox = context->pixelRatio * box2px(float2((float)sourceDepthBuffer.width(), 0.0f), float2((float)targetDepthBuffer.width(), (float)targetDepthBuffer.height()));

	matcapShader.draw(sourceDepthBuffer.getTexture(), camera, srcBox, context->getViewport(), RGBAf(1.0f, 0.5f, 0.5f, 1.0f));
	matcapShader.draw(targetDepthBuffer.getTexture(), camera, tarBox, context->getViewport(), RGBAf(0.5f, 1.0f, 0.5f, 1.0f));

	camera.setDirty(false);
}

