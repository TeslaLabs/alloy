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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <memory>
#include "AlloyReconstruction.h"
#ifdef _WIN32
#include <Windows.h>
#include <Psapi.h>
#endif // _WIN32
#include "poisson/MyTime.h"
#include "poisson/MarchingCubes.h"
#include "poisson/Octree.h"
#include "poisson/MultiGridOctreeData.h"
#include "poisson/SparseMatrix.h"
#include "poisson/ArgumentParser.h"
#include "poisson/PPolynomial.h"
#include "poisson/Ply.h"
#include "poisson/MemoryUsage.h"
#include "poisson/Trimmer.h"
#include <map>
#ifdef _OPENMP
#include "omp.h"
#endif
#include <poisson/MultiGridOctreeData.h>
#include <stdarg.h>
#define DEFAULT_FULL_DEPTH 5
int echoStdout = 0;
using namespace aly;
template<class Real, int Degree, class Vertex, BoundaryType BType> bool ExecuteInternal(const ReconstructionParameters& params, const aly::Mesh& input, aly::Mesh& output,
	const std::function<bool(const std::string& status, float progress)>& monitor)
{
	Reset<Real>();
	Octree<Real> tree;
	const Real targetValue = (Real)0.5;
	Real isoValue = 0;
	const box3f bbox(float3(0.01f, 0.01f, 0.01f), float3(0.99f, 0.99f, 0.99f));
	float4x4 M = MakeTransform(input.getBoundingBox(), bbox);
	float4x4 Minv = inverse(M);	
	int solveDepth = params.MaxSolveDepth.value;
	tree.threads = params.Threads.value;
	if (monitor)monitor("Initializing", 0.01f);
	OctNode<TreeNodeData>::SetAllocator(MEMORY_ALLOCATOR_BLOCK_SIZE);
	int kernelDepth = params.KernelDepth.set ? params.KernelDepth.value : params.Depth.value - 2;
	if (kernelDepth > params.Depth.value)
	{
		kernelDepth = params.Depth.value;
	}
	typedef ProjectiveData<Point3D<Real>,Real > ProjectiveColor;
	typedef typename Octree< Real >::template DensityEstimator< WEIGHT_DEGREE > DensityEstimator;
	typedef typename Octree< Real >::template InterpolationInfo< false > InterpolationInfo;

	CoredVectorMeshData<Vertex> mesh;
	{
		std::vector< typename Octree< Real >::PointSample > samples;
		std::vector< ProjectiveData< Point3D< Real >, Real > > sampleData;
		std::shared_ptr<DensityEstimator> density;
		std::shared_ptr<InterpolationInfo> iInfo;
		SparseNodeData<Point3D< Real >, NORMAL_DEGREE > normalInfo;
		DenseNodeData< Real, Degree > constraints;
		int pointCount = 0;
		{
			if (monitor)monitor("Building Oct-Tree", 0.1f);
			AlloyPointStream pointStream(M,input);
			pointCount = tree.template init< Point3D< Real > >(pointStream, params.Depth.value, params.Confidence.set, samples, &sampleData);

		}
		Real pointWeightSum;
		density.reset(tree.template setDensityEstimator< WEIGHT_DEGREE >(samples, kernelDepth, params.SamplesPerNode.value));
		normalInfo=tree.template setNormalField< NORMAL_DEGREE >(samples, *density, pointWeightSum, BType == BOUNDARY_NEUMANN);
		{
			if (monitor)monitor("Initializing Multi-grid", 0.2f);
			std::vector< int > indexMap;
			constexpr int MAX_DEGREE = NORMAL_DEGREE > Degree ? NORMAL_DEGREE : Degree;
			tree.template inalizeForBroodedMultigrid< MAX_DEGREE, Degree, BType >(params.FullDepth.value, typename Octree< Real >::template HasNormalDataFunctor< NORMAL_DEGREE >(normalInfo), &indexMap);
			normalInfo.remapIndices(indexMap);
			if (density.get()) density->remapIndices(indexMap);
		}

		{
			if (monitor)monitor("Adding FEM constraints", 0.3f);
			constraints = tree.template initDenseNodeData< Degree >();
			tree.template addFEMConstraints< Degree, BType, NORMAL_DEGREE, BType >(FEMVFConstraintFunctor< NORMAL_DEGREE, BType, Degree, BType >(1., 0.), normalInfo, constraints, solveDepth);
		}
		if (params.PointWeight.value>0)
		{
			if (monitor)monitor("Adding Boundary Conditions", 0.4f);
			iInfo.reset(new InterpolationInfo(tree, samples, targetValue, params.AdaptiveExponent.value, (Real)params.PointWeight.value * pointWeightSum, (Real)0));
			tree.template addInterpolationConstraints< Degree, BType >(*iInfo, constraints, solveDepth);
		}
		if (monitor)monitor("Solving Linear System", 0.5f);
		typename Octree< Real >::SolverInfo solverInfo;
		solverInfo.cgDepth = params.CGDepth.value;
		solverInfo.iters = params.Iters.value;
		solverInfo.cgAccuracy = params.CGSolverAccuracy.value;
		solverInfo.verbose = false;
		solverInfo.showResidual = params.ShowResidual.set;
		solverInfo.lowResIterMultiplier = std::max< double >(1., params.LowResIterMultiplier.value);
		DenseNodeData< Real, Degree > solution = tree.template solveSystem< Degree, BType >(FEMSystemFunctor< Degree, BType >(0, 1., 0), iInfo.get(), constraints, solveDepth, solverInfo);
		if (monitor)monitor("Applying Color", 0.6f);
		double valueSum = 0, weightSum = 0;
		typename Octree< Real >::template MultiThreadedEvaluator< Degree, BType > evaluator(&tree, solution, params.Threads.value);
#pragma omp parallel for num_threads( params.Threads.value ) reduction( + : valueSum , weightSum )
		for (int j = 0; j<samples.size(); j++)
		{
			ProjectiveData< OrientedPoint3D< Real >, Real >& sample = samples[j].sample;
			Real w = sample.weight;
			if (w>0) weightSum += w, valueSum += evaluator.value(sample.data.p / sample.weight, omp_get_thread_num(), samples[j].node) * w;
		}
		isoValue = (Real)(valueSum / weightSum);
		SparseNodeData<ProjectiveData< Point3D< Real >, Real >, DATA_DEGREE > colorData= tree.template setDataField< DATA_DEGREE, false >(samples, sampleData, (DensityEstimator*)nullptr);
		sampleData.clear();
		for (const OctNode< TreeNodeData >* n = tree.tree().nextNode(); n; n = tree.tree().nextNode(n))
		{
			ProjectiveData< Point3D< Real >, Real >* clr = colorData(n);
			if (clr) (*clr) *= (Real)std::pow(params.Color.value, tree.depth(n));
		}
		if (monitor)monitor("Generating Mesh", 0.7f);
		tree.template getMCIsoSurface< Degree, BType, WEIGHT_DEGREE, DATA_DEGREE >(density.get(), &colorData, solution, isoValue, mesh, !params.LinearFit.set, !params.NonManifold.set, params.PolygonMesh.set);

	}
	if (monitor)monitor("Trimming Mesh", 0.9f);
	float min, max;
	std::vector<Vertex> vertices;
	std::vector<std::vector<int> > polygons;
	{
		int sz = (int)mesh.inCorePoints.size();
		int vertCount = int(mesh.outOfCorePointCount()) + sz;
		int faceCount = mesh.polygonCount();
		polygons.reserve(faceCount);
		vertices.reserve(vertCount);
		mesh.resetIterator();
		for (int i = 0; i < sz; i++)
		{
			vertices.push_back(mesh.inCorePoints[i]);
		}
		for (int i = 0; i < mesh.outOfCorePointCount(); i++)
		{
			Vertex vertex;
			mesh.nextOutOfCorePoint(vertex);
			vertices.push_back(vertex);
		}
		std::vector<CoredVertexIndex> polygon;
		for (int i = 0; i < faceCount; i++)
		{
			mesh.nextPolygon(polygon);
			std::vector<int> poly;
			for (CoredVertexIndex p : polygon)
			{
				if (p.inCore)
					poly.push_back(p.idx);
				else
					poly.push_back(p.idx + sz);
			}
			polygons.push_back(poly);
		}
	}

	for (int i = 0; i < params.Smooth.value; i++)
	{
		SmoothValues<float, Vertex>(vertices, polygons);
	}
	min = max = vertices[0].value;
	for (size_t i = 0; i < vertices.size(); i++)
	{
		min = std::min<float>(min, vertices[i].value);
		max = std::max<float>(max, vertices[i].value);
	}
	const int BINS = 100;
	std::vector<int> histogram(BINS, 0);
	float binSize = (max - min) / BINS;
	size_t N = vertices.size();
	for (size_t i = 0; i < N; i++)
	{
		int b = std::min(std::max(0, (int)std::floor((vertices[i].value - min) / binSize)), BINS - 1);
		histogram[b]++;
	}
	float thresh = params.Trim.value;
	float trim = min;
	int bsum = 0;
	for (int b = 1; b < histogram.size(); b++)
	{
		histogram[b] += histogram[b - 1];
	}
	for (int b = 0; b < histogram.size(); b++)
	{
		float bCurrent = (b == 0) ? 0.0f : (histogram[b - 1] / (float)N);
		float bNext = histogram[b] / (float)N;
		if (thresh >= bCurrent && thresh < bNext)
		{
			trim = binSize * (thresh - bCurrent) / (bNext - bCurrent) + b * binSize + min;
		}
	}
	printf("Density Value Range: [%f,%f] Trim Value: %f \n", min, max, trim);
	
	std::map<uint64_t, int> vertexTable;
	std::vector<std::vector<int> > ltPolygons, gtPolygons;
	std::vector<bool> ltFlags, gtFlags;
	for (size_t i = 0; i < polygons.size(); i++)
	{
		SplitPolygon(polygons[i], vertices, &ltPolygons, &gtPolygons, &ltFlags, &gtFlags, vertexTable, trim);
	}
	if (params.IslandAreaRatio.value > 0)
	{
		std::vector<std::vector<int> > _ltPolygons, _gtPolygons;
		std::vector<std::vector<int> > ltComponents, gtComponents;
		SetConnectedComponents(ltPolygons, ltComponents);
		SetConnectedComponents(gtPolygons, gtComponents);
		std::vector<double> ltAreas(ltComponents.size(), 0.), gtAreas(gtComponents.size(), 0.);
		std::vector<bool> ltComponentFlags(ltComponents.size(), false), gtComponentFlags(gtComponents.size(), false);
		double area = 0.;
		for (size_t i = 0; i < ltComponents.size(); i++)
		{
			for (size_t j = 0; j < ltComponents[i].size(); j++)
			{
				ltAreas[i] += PolygonArea<float, Vertex>(vertices, ltPolygons[ltComponents[i][j]]);
				ltComponentFlags[i] = (ltComponentFlags[i] || ltFlags[ltComponents[i][j]]);
			}
			area += ltAreas[i];
		}
		for (size_t i = 0; i < gtComponents.size(); i++)
		{
			for (size_t j = 0; j < gtComponents[i].size(); j++)
			{
				gtAreas[i] += PolygonArea<float, Vertex>(vertices, gtPolygons[gtComponents[i][j]]);
				gtComponentFlags[i] = (gtComponentFlags[i] || gtFlags[gtComponents[i][j]]);
			}
			area += gtAreas[i];
		}
		for (size_t i = 0; i < ltComponents.size(); i++)
		{
			if (ltAreas[i] < area * params.IslandAreaRatio.value && ltComponentFlags[i])
				for (size_t j = 0; j < ltComponents[i].size(); j++)
					_gtPolygons.push_back(ltPolygons[ltComponents[i][j]]);
			else
				for (size_t j = 0; j < ltComponents[i].size(); j++)
					_ltPolygons.push_back(ltPolygons[ltComponents[i][j]]);
		}
		for (size_t i = 0; i < gtComponents.size(); i++)
		{
			if (gtAreas[i] < area * params.IslandAreaRatio.value && gtComponentFlags[i])
				for (size_t j = 0; j < gtComponents[i].size(); j++)
					_ltPolygons.push_back(gtPolygons[gtComponents[i][j]]);
			else
				for (size_t j = 0; j < gtComponents[i].size(); j++)
					_gtPolygons.push_back(gtPolygons[gtComponents[i][j]]);
		}
		ltPolygons = _ltPolygons, gtPolygons = _gtPolygons;
	}
	if (!params.PolygonMesh.set)
	{
		{
			std::vector<std::vector<int> > polys = ltPolygons;
			Triangulate<float, Vertex>(vertices, ltPolygons, polys), ltPolygons = polys;
		}
		{
			std::vector<std::vector<int> > polys = gtPolygons;
			Triangulate<float, Vertex>(vertices, gtPolygons, polys), gtPolygons = polys;
		}
	}
	RemoveHangingVertices(vertices, gtPolygons);
	ltPolygons.clear();

	polygons = gtPolygons;
	{
		size_t vertCount = vertices.size();
		size_t faceCount = polygons.size();
		output.vertexLocations.resize(vertCount);
		output.vertexColors.resize(vertCount);
		output.triIndexes.clear();
		output.quadIndexes.clear();
		for (int i = 0; i < vertCount; i++)
		{
			Vertex vertex = vertices[i];
			Point3D<float> pt = vertex.point;
			const unsigned char* c = vertex.color;
			RGBAf rgba=RGBAf(c[0]/255.0f, c[1] / 255.0f, c[2] / 255.0f, (vertex.value - min) / std::max(1E-6f, max - min));
			output.vertexColors[i] = rgba;
			output.vertexLocations[i] = Transform(Minv,float3(pt[0],pt[1],pt[2]));
		}
		for (int i = 0; i < faceCount; i++)
		{
			std::vector<int> polygon = polygons[i];
			int N = int(polygon.size());
			if (N == 3)
			{
				uint3 triFace;
				for (int n = 0; n < N; n++)
				{
					triFace[N - 1 - n] = polygon[n];
				}
				output.triIndexes.push_back(triFace);
			}
			else if (N == 4)
			{
				uint4 quadFace;
				for (int n = 0; n < N; n++)
				{
					quadFace[N - 1 - n] = polygon[n];
				}
				output.quadIndexes.push_back(quadFace);
			}
		}
		output.updateBoundingBox();
		output.updateVertexNormals();
	}
	if (monitor)monitor("Done", 1.0f);
	OctNode<TreeNodeData>::ResetAllocator();
	return true;
}

bool AlloyPointStream::nextPoint(OrientedPoint3D<float>& p, Point3D<float>& d)
{
	if (counter >= mesh->vertexLocations.size())
		return false;
	float3 v = mesh->vertexLocations[counter];
	float3 n = mesh->vertexNormals[counter];
	float4 c = mesh->vertexColors[counter];
	v = Transform(M, v);
	p.p = Point3D<float>(v.x, v.y, v.z);
	p.n = Point3D<float>(n.x, n.y, n.z);
	d = Point3D<float>(255.0f*c.x, 255.0f*c.y, 255.0f*c.z);
	counter++;
	return true;
}
template<class Real, int Degree, class Vertex> bool ExecuteInternal(const ReconstructionParameters& params, const aly::Mesh& input, aly::Mesh& output, const BoundaryType& BType, const std::function<bool(const std::string& status, float progress)>& monitor)
{
	switch (params.BType.value)
	{
	case BoundaryType::BOUNDARY_FREE:
		return ExecuteInternal<float, 1, PlyColorAndValueVertex<float>, BoundaryType::BOUNDARY_FREE>(params, input, output, monitor);
		break;
	case BoundaryType::BOUNDARY_DIRICHLET:
		return ExecuteInternal<float, 2, PlyColorAndValueVertex<float>, BoundaryType::BOUNDARY_DIRICHLET>(params, input, output, monitor);
		break;
	case BoundaryType::BOUNDARY_NEUMANN:
		return ExecuteInternal<float, 3, PlyColorAndValueVertex<float>, BoundaryType::BOUNDARY_NEUMANN>(params, input, output, monitor);
		break;
	case BoundaryType::BOUNDARY_COUNT:
		return ExecuteInternal<float, 4, PlyColorAndValueVertex<float>, BoundaryType::BOUNDARY_COUNT>(params, input, output, monitor);
		break;
	default:
		throw std::runtime_error("Boundary type not supported.");
	}
	return false;
}
void SurfaceReconstruct(const ReconstructionParameters& params, const aly::Mesh& input, aly::Mesh& output, const std::function<bool(const std::string& status, float progress)>& monitor)
{
	BoundaryType BType = static_cast<BoundaryType>(params.BType.value);
	switch (params.Degree.value)
	{
	case 1:
		ExecuteInternal<float, 1,PlyColorAndValueVertex<float> >(params, input, output, BType, monitor);
		break;
	case 2:
		ExecuteInternal<float, 2, PlyColorAndValueVertex<float> >(params, input, output, BType, monitor);
		break;
	case 3:
		ExecuteInternal<float, 3, PlyColorAndValueVertex<float> >(params, input, output, BType, monitor);
		break;
	case 4:
		ExecuteInternal<float, 4, PlyColorAndValueVertex<float> >(params, input, output, BType, monitor);
		break;
	default:
		throw std::runtime_error("Degree not supported.");
	}
}