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
#include "../../include/example/MeshSmoothEx.h"
using namespace aly;
MeshSmoothEx::MeshSmoothEx() :
		Application(800, 600, "Mesh Smoothing Example"), matcapShader(
				getFullPath("images/JG_Gold.png")) {
}
bool MeshSmoothEx::init(Composite& rootNode) {
	box3f renderBBox = box3f(float3(-0.5f, -0.5f, -0.5f),
			float3(1.0f, 1.0f, 1.0f));
	mesh.load(getFullPath("models/icosahedron.ply"));
	std::srand(812731);
	for (int c = 0; c < 5; c++) {
		Subdivide(mesh, SubDivisionScheme::Loop);
		mesh.updateVertexNormals();
		int N = (int) mesh.vertexLocations.size();
		for (int n = 0; n < N; n++) {
			float3 norm = -mesh.vertexNormals[n];
			mesh.vertexLocations[n] += 0.6f * norm
					* ((rand() % 4096) / 4096.0f - 0.3f) / (float) (1 << c);
		}
	}
	Subdivide(mesh, SubDivisionScheme::Loop);
	mesh.updateVertexNormals();
	mesh.updateBoundingBox();
	int w = getContext()->getScreenWidth();
	int h = getContext()->getScreenHeight();
	//Make region on screen to render 3d view
	renderRegion = MakeRegion("Render View",
			CoordPerPX(0.5, 0.5, -w / 2, -h / 2), CoordPX(w, h), COLOR_NONE,
			COLOR_WHITE, UnitPX(1.0f));
	//Initialize depth buffer to store the render
	depthFrameBuffer.initialize(w, h);
	//Set up camera
	camera.setNearFarPlanes(0.1f, 2.0f);
	camera.setZoom(0.75f);
	camera.setCameraType(CameraType::Perspective);
	camera.setDirty(true);
	//Map object geometry into unit bounding box for draw.
	camera.setPose(MakeTransform(mesh.getBoundingBox(), renderBBox));
	//Add listener to respond to mouse manipulations
	addListener(&camera);
	textLabel = TextLabelPtr(
			new TextLabel("", CoordPX(5, 5), CoordPercent(0.5f, 0.5f)));
	textLabel->fontSize = UnitPX(20.0f);
	textLabel->fontType = FontType::Bold;
	textLabel->fontStyle = FontStyle::Outline;
	TextButtonPtr smoothButton = TextButtonPtr(
			new TextButton("Smooth", CoordPerPX(0.5f, 1.0f, -50.0f, -35.0f),
					CoordPX(100, 30)));
	rootNode.add(renderRegion);
	rootNode.add(textLabel);
	rootNode.add(smoothButton);
	workerTask = WorkerTaskPtr(new WorkerTask([=] {
		textLabel->setLabel("Smoothing Mesh ...");
		smoothButton->setVisible(false);
		smooth();
		mesh.setDirty(true);
		camera.setDirty(true);
		smoothButton->setVisible(true);
		textLabel->setLabel("");
	}));
	smoothButton->onMouseDown =
			[=](AlloyContext* context, const InputEvent& e) {
				workerTask->cancel();
				workerTask->execute();
				return true;
			};
	return true;
}
void MeshSmoothEx::smooth() {
	static MeshListNeighborTable nbrTable;
	if(nbrTable.size()==0){
		//Only need to compute this once since topology doesn't change.
		CreateOrderedVertexNeighborTable(mesh, nbrTable, true);
	}
	int index = 0;
	std::vector<float> angles;
	std::vector<float> weights;
	float smoothness = 10.0f;
	int N = (int) mesh.vertexLocations.size();
	SparseMatrix1f A(N, N);
	Vector3f b(N);
	for (std::list<uint32_t>& nbrs : nbrTable) {
		int K = (int) nbrs.size() - 1;
		float3 pt = mesh.vertexLocations[index];
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
				float w = (angles[k] + angles[(k + 1) % K])
						/ distance(pt, ptNext);
				wsum += w;
				weights[k] = w;
				nbrIter++;
			}
		}
		{
			auto nbrIter = nbrs.begin();
			nbrIter++;
			for (int k = 0; k < K; k++) {
				float w = -smoothness * weights[k] / wsum;
				A.set(index, *nbrIter, w);
				nbrIter++;
			}
		}
		A.set(index, index, smoothness + 1);
		b[index] = pt;
		index++;
	}
	SolveBICGStab(b, A, mesh.vertexLocations, 100, 1E-6f,
			[this](int iter,double error) {
				textLabel->setLabel( MakeString() << "Smooth [" << iter << "] Error: " << error);
				return true;
			});
	mesh.updateVertexNormals();
}
void MeshSmoothEx::draw(AlloyContext* context) {
	if (camera.isDirty()) {
		//Compute depth and normals only when camera view changes.
		depthAndNormalShader.draw(mesh, camera, depthFrameBuffer);
	}
	//Recompute lighting at every draw pass.
	matcapShader.draw(depthFrameBuffer.getTexture(), camera,
			context->pixelRatio*renderRegion->getBounds(), context->getViewport(), RGBAf(1.0f));
	camera.setDirty(false);
}

