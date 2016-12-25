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
#include "poisson/CmdLineParser.h"
#include "poisson/PPolynomial.h"
#include "poisson/Ply.h"
#include "poisson/MemoryUsage.h"
#include <map>
#ifdef _OPENMP
#include "omp.h"
#endif // _OPENMP
void DumpOutput(const char* format, ...);
#include <poisson/MultiGridOctreeData.h>

#define DEFAULT_FULL_DEPTH 5

#define XSTR(x) STR(x)
#define STR(x) #x
#if DEFAULT_FULL_DEPTH
//#pragma message ( "[WARNING] Setting default full depth to " XSTR(DEFAULT_FULL_DEPTH) )
#endif // DEFAULT_FULL_DEPTH

#include <stdarg.h>
char* outputFile = NULL;
int echoStdout = 0;
void DumpOutput(const char* format, ...)
{
	/*
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	*/
}
using namespace aly;
long long EdgeKey(int key1, int key2)
{
	if (key1 < key2)
		return (((long long)key1) << 32) | ((long long)key2);
	else
		return (((long long)key2) << 32) | ((long long)key1);
}

template<class Real, class Vertex>
Vertex InterpolateVertices(const Vertex& v1, const Vertex& v2, Real value)
{
	typename Vertex::Wrapper _v1(v1), _v2(v2);
	if (_v1.value == _v2.value)
		return Vertex((_v1 + _v2) / Real(2.));

	Real dx = (_v1.value - value) / (_v1.value - _v2.value);
	return Vertex(_v1 * (1.f - dx) + _v2 * dx);
}
template<class Real, class Vertex>
void SmoothValues(std::vector<Vertex>& vertices, const std::vector<std::vector<int> >& polygons)
{
	std::vector<int> count(vertices.size());
	std::vector<Real> sums(vertices.size(), 0);
	for (size_t i = 0; i < polygons.size(); i++)
	{
		int sz = int(polygons[i].size());
		for (int j = 0; j < sz; j++)
		{
			int j1 = j, j2 = (j + 1) % sz;
			int v1 = polygons[i][j1], v2 = polygons[i][j2];
			count[v1]++, count[v2]++;
			sums[v1] += vertices[v2].value, sums[v2] += vertices[v1].value;
		}
	}
	for (size_t i = 0; i < vertices.size(); i++)
		vertices[i].value = (sums[i] + vertices[i].value) / (count[i] + 1);
}
template<class Real, class Vertex>
void SplitPolygon(const std::vector<int>& polygon,
	std::vector<Vertex>& vertices,
	std::vector<std::vector<int> >* ltPolygons,
	std::vector<std::vector<int> >* gtPolygons,
	std::vector<bool>* ltFlags,
	std::vector<bool>* gtFlags,
	std::map<uint64_t, int>& vertexTable,
	Real trimValue)
{
	int sz = int(polygon.size());
	std::vector<bool> gt(sz);
	int gtCount = 0;
	for (int j = 0; j < sz; j++)
	{
		gt[j] = (vertices[polygon[j]].value > trimValue);
		if (gt[j])
			gtCount++;
	}
	if (gtCount == sz)
	{
		if (gtPolygons)
			gtPolygons->push_back(polygon);
		if (gtFlags)
			gtFlags->push_back(false);
	}
	else if (gtCount == 0)
	{
		if (ltPolygons)
			ltPolygons->push_back(polygon);
		if (ltFlags)
			ltFlags->push_back(false);
	}
	else
	{
		int start;
		for (start = 0; start < sz; start++)
			if (gt[start] && !gt[(start + sz - 1) % sz])
				break;

		bool gtFlag = true;
		std::vector<int> poly;

		// Add the initial vertex
		{
			int j1 = (start + int(sz) - 1) % sz, j2 = start;
			int v1 = polygon[j1], v2 = polygon[j2];
			int vIdx;
			std::map<uint64_t, int>::iterator iter = vertexTable.find(EdgeKey(v1, v2));
			if (iter == vertexTable.end())
			{
				vertexTable[EdgeKey(v1, v2)] = vIdx = int(vertices.size());
				vertices.push_back(InterpolateVertices(vertices[v1], vertices[v2], trimValue));
			}
			else
				vIdx = iter->second;
			poly.push_back(vIdx);
		}

		for (int _j = 0; _j <= sz; _j++)
		{
			int j1 = (_j + start + sz - 1) % sz, j2 = (_j + start) % sz;
			int v1 = polygon[j1], v2 = polygon[j2];
			if (gt[j2] == gtFlag)
				poly.push_back(v2);
			else
			{
				int vIdx;
				std::map<uint64_t, int>::iterator iter = vertexTable.find(EdgeKey(v1, v2));
				if (iter == vertexTable.end())
				{
					vertexTable[EdgeKey(v1, v2)] = vIdx = int(vertices.size());
					vertices.push_back(InterpolateVertices(vertices[v1], vertices[v2], trimValue));
				}
				else
					vIdx = iter->second;
				poly.push_back(vIdx);
				if (gtFlag)
				{
					if (gtPolygons)
						gtPolygons->push_back(poly);
					if (ltFlags)
						ltFlags->push_back(true);
				}
				else
				{
					if (ltPolygons)
						ltPolygons->push_back(poly);
					if (gtFlags)
						gtFlags->push_back(true);
				}
				poly.clear(), poly.push_back(vIdx), poly.push_back(v2);
				gtFlag = !gtFlag;
			}
		}
	}
}
template<class Real, class Vertex>
void Triangulate(const std::vector<Vertex>& vertices, const std::vector<std::vector<int> >& polygons, std::vector<std::vector<int> >& triangles)
{
	triangles.clear();
	for (size_t i = 0; i < polygons.size(); i++)
		if (polygons.size() > 3)
		{
			MinimalAreaTriangulation<Real> mat;
			std::vector<Point3D<Real> > _vertices(polygons[i].size());
			std::vector<TriangleIndex> _triangles;
			for (int j = 0; j < int(polygons[i].size()); j++)
				_vertices[j] = vertices[polygons[i][j]].point;
			mat.GetTriangulation(_vertices, _triangles);

			// Add the triangles to the mesh
			size_t idx = triangles.size();
			triangles.resize(idx + _triangles.size());
			for (int j = 0; j < int(_triangles.size()); j++)
			{
				triangles[idx + j].resize(3);
				for (int k = 0; k < 3; k++)
					triangles[idx + j][k] = polygons[i][_triangles[j].idx[k]];
			}
		}
		else if (polygons[i].size() == 3)
			triangles.push_back(polygons[i]);
}
template<class Real, class Vertex>
double PolygonArea(const std::vector<Vertex>& vertices, const std::vector<int>& polygon)
{
	if (polygon.size() < 3)
		return 0.;
	else if (polygon.size() == 3)
		return TriangleArea(vertices[polygon[0]].point, vertices[polygon[1]].point, vertices[polygon[2]].point);
	else
	{
		Point3D<Real> center;
		for (size_t i = 0; i < polygon.size(); i++)
			center += vertices[polygon[i]].point;
		center /= Real(polygon.size());
		double area = 0;
		for (size_t i = 0; i < polygon.size(); i++)
			area += TriangleArea(center, vertices[polygon[i]].point, vertices[polygon[(i + 1) % polygon.size()]].point);
		return area;
	}
}

template<class Vertex>
void RemoveHangingVertices(std::vector<Vertex>& vertices, std::vector<std::vector<int> >& polygons)
{
	std::map<int, int> vMap;
	std::vector<bool> vertexFlags(vertices.size(), false);
	for (size_t i = 0; i < polygons.size(); i++)
		for (size_t j = 0; j < polygons[i].size(); j++)
			vertexFlags[polygons[i][j]] = true;
	int vCount = 0;
	for (int i = 0; i < int(vertices.size()); i++)
		if (vertexFlags[i])
			vMap[i] = vCount++;
	for (size_t i = 0; i < polygons.size(); i++)
		for (size_t j = 0; j < polygons[i].size(); j++)
			polygons[i][j] = vMap[polygons[i][j]];

	std::vector<Vertex> _vertices(vCount);
	for (int i = 0; i < int(vertices.size()); i++)
		if (vertexFlags[i])
			_vertices[vMap[i]] = vertices[i];
	vertices = _vertices;
}
void SetConnectedComponents(const std::vector<std::vector<int> >& polygons, std::vector<std::vector<int> >& components)
{
	std::vector<int> polygonRoots(polygons.size());
	for (size_t i = 0; i < polygons.size(); i++)
		polygonRoots[i] = int(i);
	std::map<uint64_t, int> edgeTable;
	for (size_t i = 0; i < polygons.size(); i++)
	{
		int sz = int(polygons[i].size());
		for (int j = 0; j < sz; j++)
		{
			int j1 = j, j2 = (j + 1) % sz;
			int v1 = polygons[i][j1], v2 = polygons[i][j2];
			long long eKey = EdgeKey(v1, v2);
			std::map<uint64_t, int>::iterator iter = edgeTable.find(eKey);
			if (iter == edgeTable.end())
				edgeTable[eKey] = int(i);
			else
			{
				int p = iter->second;
				while (polygonRoots[p] != p)
				{
					int temp = polygonRoots[p];
					polygonRoots[p] = int(i);
					p = temp;
				}
				polygonRoots[p] = int(i);
			}
		}
	}
	for (size_t i = 0; i < polygonRoots.size(); i++)
	{
		int p = int(i);
		while (polygonRoots[p] != p)
			p = polygonRoots[p];
		int root = p;
		p = int(i);
		while (polygonRoots[p] != p)
		{
			int temp = polygonRoots[p];
			polygonRoots[p] = root;
			p = temp;
		}
	}
	int cCount = 0;
	std::map<int, int> vMap;
	for (int i = 0; i < int(polygonRoots.size()); i++)
		if (polygonRoots[i] == i)
			vMap[i] = cCount++;
	components.resize(cCount);
	for (int i = 0; i < int(polygonRoots.size()); i++)
		components[vMap[polygonRoots[i]]].push_back(i);
}
template<class Real>
inline Point3D<Real> CrossProduct(Point3D<Real> p1, Point3D<Real> p2)
{
	return Point3D<Real>(p1[1] * p2[2] - p1[2] * p2[1], p1[2] * p2[0] - p1[0] * p2[2], p1[0] * p1[1] - p1[1] * p2[0]);
}
template<class Real>
double TriangleArea(Point3D<Real> v1, Point3D<Real> v2, Point3D<Real> v3)
{
	Point3D<Real> n = CrossProduct(v2 - v1, v3 - v1);
	return sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]) / 2.;
}

template<class Real, int Degree, class Vertex, BoundaryType BType> bool ExecuteInternal(const ReconstructionParameters& params, const aly::Mesh& input, aly::Mesh& output,
	const std::function<bool(const std::string& status, float progress)>& monitor)
{
	int solveDepth = params.MaxSolveDepth.value;
	Reset<Real>();
	XForm4x4<Real> xForm, iXForm;
	xForm = XForm4x4<Real>::Identity();
	iXForm = xForm.inverse();
	double t;
	double tt = MyTime();
	Real isoValue = 0;
	Octree<Real> tree;
	tree.threads = params.Threads.value;
	if (monitor)monitor("Initializing", 0.01f);
	OctNode<TreeNodeData>::SetAllocator(MEMORY_ALLOCATOR_BLOCK_SIZE);
	t = MyTime();
	int kernelDepth = params.KernelDepth.set ? params.KernelDepth.value : params.Depth.value - 2;
	if (kernelDepth > params.Depth.value)
	{
		kernelDepth = params.Depth.value;
	}
	t = MyTime();
	typedef ProjectiveData<Point3D<Real>,Real > ProjectiveColor;
	typedef typename Octree< Real >::template DensityEstimator< WEIGHT_DEGREE > DensityEstimator;
	typedef typename Octree< Real >::template InterpolationInfo< false > InterpolationInfo;
	Real targetValue = (Real)0.5;
	CoredVectorMeshData<Vertex> mesh;
	{
		std::vector< typename Octree< Real >::PointSample > samples;
		std::vector< ProjectiveData< Point3D< Real >, Real > > sampleData;
		std::shared_ptr<DensityEstimator> density;
		std::shared_ptr<InterpolationInfo> iInfo;
		SparseNodeData<Point3D< Real >, NORMAL_DEGREE > normalInfo;
		DenseNodeData< Real, Degree > constraints;
		int pointCount = 0;
		//DumpOutput("Poisson Reconstruction:: Threads: %d Tree Depth: %d\n", params.Threads.value, params.Depth.value);
		{
			AlloyPointStream pointStream(input);
			pointCount = tree.template init< Point3D< Real > >(pointStream, params.Depth.value, params.Confidence.set, samples, &sampleData);

		}
		Real pointWeightSum;
		density.reset(tree.template setDensityEstimator< WEIGHT_DEGREE >(samples, kernelDepth, params.SamplesPerNode.value));
		normalInfo=tree.template setNormalField< NORMAL_DEGREE >(samples, *density, pointWeightSum, BType == BOUNDARY_NEUMANN);
		{
			std::vector< int > indexMap;
			constexpr int MAX_DEGREE = NORMAL_DEGREE > Degree ? NORMAL_DEGREE : Degree;
			tree.template inalizeForBroodedMultigrid< MAX_DEGREE, Degree, BType >(params.FullDepth.value, typename Octree< Real >::template HasNormalDataFunctor< NORMAL_DEGREE >(normalInfo), &indexMap);
			normalInfo.remapIndices(indexMap);
			if (density.get()) density->remapIndices(indexMap);
		}
		{
			constraints = tree.template initDenseNodeData< Degree >();
			tree.template addFEMConstraints< Degree, BType, NORMAL_DEGREE, BType >(FEMVFConstraintFunctor< NORMAL_DEGREE, BType, Degree, BType >(1., 0.), normalInfo, constraints, solveDepth);
		}
		if (params.PointWeight.value>0)
		{
			iInfo.reset(new InterpolationInfo(tree, samples, targetValue, params.AdaptiveExponent.value, (Real)params.PointWeight.value * pointWeightSum, (Real)0));
			tree.template addInterpolationConstraints< Degree, BType >(*iInfo, constraints, solveDepth);
		}
		
		typename Octree< Real >::SolverInfo solverInfo;
		solverInfo.cgDepth = params.CGDepth.value;
		solverInfo.iters = params.Iters.value;
		solverInfo.cgAccuracy = params.CGSolverAccuracy.value;
		solverInfo.verbose = false;
		solverInfo.showResidual = params.ShowResidual.set;
		solverInfo.lowResIterMultiplier = std::max< double >(1., params.LowResIterMultiplier.value);
		DenseNodeData< Real, Degree > solution = tree.template solveSystem< Degree, BType >(FEMSystemFunctor< Degree, BType >(0, 1., 0), iInfo.get(), constraints, solveDepth, solverInfo);

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

		tree.template getMCIsoSurface< Degree, BType, WEIGHT_DEGREE, DATA_DEGREE >(density.get(), &colorData, solution, isoValue, mesh, !params.LinearFit.set, !params.NonManifold.set, params.PolygonMesh.set);

	}
	if (monitor)monitor("Trimming Mesh", 0.9f);
	std::cout << "Trimming Mesh..." << std::endl;
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
	t = MyTime();
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
			output.vertexLocations[i] = float3(pt[0],pt[1],pt[2]);
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
	return true;
}

bool AlloyPointStream::nextPoint(OrientedPoint3D<float>& p, Point3D<float>& d)
{
	if (counter >= mesh.vertexLocations.size())
		return false;
	float3 v = mesh.vertexLocations[counter];
	float3 n = mesh.vertexNormals[counter];
	float4 c = mesh.vertexColors[counter];
	p.p = Point3D<float>(v.x, v.y, v.z);
	p.n = Point3D<float>(n.x, n.y, n.z);
	d = Point3D<float>(255.0f*c.x, 255.0f*c.y, 255.0f*c.z);
	counter++;
	return true;
}

void PoissonReconstruct(const ReconstructionParameters& params, const aly::Mesh& input, aly::Mesh& output, const std::function<bool(const std::string& status, float progress)>& monitor)
{
	static const BoundaryType BType = BoundaryType::BOUNDARY_NEUMANN;
	switch (params.Degree.value)
	{
	case 1:
		ExecuteInternal<float, 1,PlyColorAndValueVertex<float>, BType>(params, input, output, monitor);
		break;
	case 2:
		ExecuteInternal<float, 2, PlyColorAndValueVertex<float>, BType>(params, input, output, monitor);
		break;
	case 3:
		ExecuteInternal<float, 3, PlyColorAndValueVertex<float>, BType>(params, input, output, monitor);
		break;
	case 4:
		ExecuteInternal<float, 4, PlyColorAndValueVertex<float>, BType>(params, input, output, monitor);
		break;
	default:
		throw std::runtime_error("Degree not supported.");
	}
}
/*
int main(int argc, char* argv[])
{
ReconstructionParameters params;
params.Threads.value = omp_get_max_threads() - 2;
params.Depth.value=9;
if (argc > 2)
{
std::string inputFile(argv[1]);
std::string outputFile(argv[2]);
aly::Mesh inputMesh;
aly::Mesh outputMesh;
std::cout << "Reading " << inputFile << std::endl;
ReadMeshFromFile(inputFile, inputMesh);
PoissonReconstruct(params, inputMesh, outputMesh);
std::cout << "Writing " << outputFile << std::endl;
WriteMeshToFile(outputFile, outputMesh);
}
}
*/
