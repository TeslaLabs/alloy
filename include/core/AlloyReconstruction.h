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
#ifndef POISSONRECONAPI_H_
#define POISSONRECONAPI_H_

#include <AlloyMesh.h>
#include <functional>
#include <omp.h>
#include <poisson/ArgumentParser.h>
#include <poisson/Geometry.h>
#include <poisson/PointStream.h>
#include <string>
class AlloyPointStream : public OrientedPointStreamWithData<float, Point3D< float> >
{
protected:
	const aly::Mesh* mesh;
	aly::float4x4 M;
	size_t counter;
public:
	AlloyPointStream(const aly::float4x4& M, const aly::Mesh& mesh) :M(M),mesh(&mesh), counter(0) {
	}
	void reset(void) {
		counter = 0;
	}
	virtual bool nextPoint(OrientedPoint3D< float >& p, Point3D< float >& d) override;
};

struct ReconstructionParameters {
	ArgumentReadable
		Complete,
		ShowResidual,
		PolygonMesh,
		Confidence,
		NormalWeights,
		NonManifold,
		LinearFit,
		PrimalVoxel;
	ArgumentInt
		Degree,
		Depth,
		CGDepth,
		KernelDepth,
		AdaptiveExponent,
		Iters,
		VoxelDepth,
		FullDepth,
		MinDepth,
		MaxSolveDepth,
		Threads,
		Smooth,
		BType;
	ArgumentFloat
		Color,
		SamplesPerNode,
		Scale,
		CGSolverAccuracy,
		LowResIterMultiplier,
		PointWeight,
		Trim,
		IslandAreaRatio;

	ReconstructionParameters() :
		Complete("complete"),
		ShowResidual("showResidual"),
		PolygonMesh("polygonMesh"),
		Confidence("confidence"),
		NormalWeights("nWeights"),
		NonManifold("nonManifold"),
		LinearFit("linearFit"),
		PrimalVoxel("primalVoxel"),
		Degree("degree", 2),
		Depth("depth", 8),
		CGDepth("cgDepth", 0),
		KernelDepth("kernelDepth"),
		AdaptiveExponent("adaptiveExp", 1),
		Iters("iters", 8),
		VoxelDepth("voxelDepth", -1),
		FullDepth("fullDepth", 5),
		MinDepth("minDepth", 0),
		MaxSolveDepth("maxSolveDepth", 8),
		Threads("threads", omp_get_num_procs()),
		Smooth("smooth", 5),
		Color("color", 16.f),
		SamplesPerNode("samplesPerNode", 1.5f),
		Scale("scale", 1.1f),
		CGSolverAccuracy("cgAccuracy", float(1e-3)),
		LowResIterMultiplier("iterMultiplier", 1.f),
		PointWeight("pointWeight", 4.f),
		Trim("trim", 0.05f),
		IslandAreaRatio("aRatio", 0.001f),
		BType("bType", BOUNDARY_NEUMANN){

	}
};
void SurfaceReconstruct(const ReconstructionParameters& params, const aly::Mesh& input, aly::Mesh& output,
	const std::function<bool(const std::string& status, float progress)>& monitor=nullptr);

#endif /* POISSONRECONAPI_H_ */
