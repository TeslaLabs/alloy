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
#include "AlloyMeshTextureMap.h"
#include "AlloyUnits.h"
#include "AlloySparseMatrix.h"
#include "AlloySparseSolve.h"
#include "AlloyOptimization.h"
#include <algorithm>
#include <random>
namespace aly {
	int GetConnectedTextureComponents(const std::vector<int>& indexes, int N, std::vector<int>& cclist, std::vector<int>& labels) {
		std::vector<std::list<int>> vertNbrs(N);
		int indexCount = (int)indexes.size();
		for (int i = 0; i < indexCount; i += 3) {
			int v1 = indexes[i];
			int v2 = indexes[i + 1];
			int v3 = indexes[i + 2];
			vertNbrs[v1].push_back(v2);
			vertNbrs[v1].push_back(v3);
			vertNbrs[v2].push_back(v3);
			vertNbrs[v2].push_back(v1);
			vertNbrs[v3].push_back(v1);
			vertNbrs[v3].push_back(v2);
		}
		labels.resize(N, -1);
		int pivot = 0;
		int cc = 0;
		std::list<int> queue;

		cclist.clear();
		int masked = 0;
		int ccCount = 0;
		std::vector<int> nbrs;
		cclist.clear();
		do {
			ccCount = 0;
			queue.clear();
			labels[pivot] = cc;
			ccCount++;
			queue.push_back(pivot);
			masked++;
			int iter = 0;
			while (queue.size() > 0) {
				int v = queue.front();
				queue.pop_front();
				nbrs.clear();
				iter++;
				for (int nbr : vertNbrs[v]) {
					if (labels[nbr] < 0) {
						labels[nbr] = cc;
						ccCount++;
						masked++;
						queue.push_back(nbr);
					}
				}
			}
			cclist.push_back(ccCount);
			int lastPivot = pivot;
			pivot = -1;
			for (int i = lastPivot + 1; i < N; i++) {
				if (labels[i] < 0) {
					pivot = i;
					break;
				}
			}
			cc++;
		} while (pivot >= 0);
		return (int)cclist.size();
	}
	int GetConnectedVertexComponents(const aly::Mesh& mesh, std::vector<int>& cclist, std::vector<int>& labels) {
		int vertexCount = (int)mesh.vertexLocations.size();
		int faceCount = (int)mesh.triIndexes.size();
		std::vector<std::list<int>> vertNbrs(vertexCount);

		for (int i = 0; i < faceCount; i++) {
			uint3 face = mesh.triIndexes[i];
			int v1 = face.x;
			int v2 = face.y;
			int v3 = face.z;
			vertNbrs[v1].push_back(v2);
			vertNbrs[v1].push_back(v3);
			vertNbrs[v2].push_back(v3);
			vertNbrs[v2].push_back(v1);
			vertNbrs[v3].push_back(v1);
			vertNbrs[v3].push_back(v2);
		}
		labels.resize(vertexCount, -1);
		int pivot = 0;
		int cc = 0;
		std::list<int> queue;

		cclist.clear();
		int masked = 0;
		int ccCount = 0;
		std::vector<int> nbrs;
		cclist.clear();
		do {
			ccCount = 0;
			queue.clear();
			labels[pivot] = cc;
			ccCount++;
			queue.push_back(pivot);
			masked++;
			int iter = 0;
			while (queue.size() > 0) {
				int v = queue.front();
				queue.pop_front();
				nbrs.clear();
				iter++;
				for (int nbr : vertNbrs[v]) {
					if (labels[nbr] < 0) {
						labels[nbr] = cc;
						ccCount++;
						masked++;
						queue.push_back(nbr);
					}
				}
			}
			cclist.push_back(ccCount);
			int lastPivot = pivot;
			pivot = -1;
			for (int i = lastPivot + 1; i < vertexCount; i++) {
				if (labels[i] < 0) {
					pivot = i;
					break;
				}
			}
			cc++;
		} while (pivot >= 0);
		return (int)cclist.size();
	}
	int LabelTextureRegions(const aly::Mesh& mesh, std::vector<int>& indexes, std::vector<int>& cclist, std::vector<int>& labels, float distanceTolerance) {
		const Vector2f& uvs = mesh.textureMap;
		Locator2f locator(uvs);
		std::vector<float2i> result;
		indexes.resize(uvs.size(), -1);
		labels.resize(uvs.size(), -1);
		int index = 0;
		for (int i = 0; i < (int)uvs.size(); i++) {
			if (indexes[i] < 0) {
				float2 uv = uvs[i];
				locator.closest(uv, distanceTolerance, result);
				indexes[i] = index;
				for (float2i r : result) {
					if (indexes[r.index]<0) {
						indexes[r.index] = index;
					}
				}
				index++;
			}
		}
		GetConnectedTextureComponents(indexes, index, cclist, labels);
		return index;
	}
	int MeshTextureMap::makeLabelsUnique(std::vector<int>& vertexLabels, std::vector<int>& relabel, int minLabelSize) {
		int vertexCount = static_cast<int>(vertexLabels.size());
		int cc = 0;
		int count = 0;
		std::map<int, int> uniqueLabels;

		for (count = 0; count < vertexCount; count++) {
			int l = vertexLabels[count];
			if (uniqueLabels.find(l) == uniqueLabels.end()) {
				uniqueLabels[l] = cc;
				cc++;
			}
		}
		for (count = 0; count < vertexCount; count++) {
			vertexLabels[count] = uniqueLabels[vertexLabels[count]];
		}
		std::vector<std::pair<int, int>> counts(cc);
		std::vector<int> remap(cc);
		count = 0;
		for (std::pair<int, int>& p : counts) {
			p.first = 0;
			p.second = count++;
		}
		for (count = 0; count < vertexCount; count++) {
			counts[vertexLabels[count]].first--;
		}
		sort(counts.begin(), counts.end());

		remap.clear();
		remap.resize(cc, -1);
		int scc = 0;
		for (count = 0; count < cc; count++) {
			remap[counts[count].second] = count;
			if (-counts[count].first < minLabelSize)continue;
			//std::cout << count << " sorted " << counts[count].second << " " << (-counts[count].first) << std::endl;
			scc = count + 1;
		}
		relabel.clear();

		for (count = 0; count < vertexCount; count++) {
			int l = remap[vertexLabels[count]];
			vertexLabels[count] = l;
			//relabel un labeled
			if (l >= scc) {
				relabel.push_back(count);
			}
		}
		//std::cout << "Unique "<<scc<<" / "<<cc<<" Relabel list " << relabel.size() <<" / "<<vertexCount<< std::endl;
		return scc;
	}
	void MeshTextureMap::evaluate(aly::Mesh& mesh, const std::function<bool(const std::string& status, float progress)>& statusHandler){
		mesh.convertQuadsToTriangles();
		Vector3f vertexCopy;
		if (smoothIterations > 0) {		
			vertexCopy = mesh.vertexLocations;
			smooth(mesh, smoothIterations, 1E-10f);
		}
		labelComponents(mesh, statusHandler);
		computeMap(mesh, statusHandler);
		if (smoothIterations > 0) {
			mesh.vertexLocations = vertexCopy;
		}
	}
	void MeshTextureMap::unfold(aly::Mesh& mesh, std::vector<int>& rectId,std::vector<bvec2f>& rects) {
		//Flatten individual surface patches onto 2D plane using Least Squares Conformal Mapping.
		int N = 0;
		for (Mosaic& mIndexes : mosaics) {
			N += static_cast<int>(mIndexes.indexes.size());
		}

		int index = 0;
		float3 deviations;
		float totalArea = 0.0f;
		size_t F = 0;
		size_t C = 0;
		size_t T = 0;
		int V = 0;
		std::list<int> lockedList;
		for (Mosaic& mosaic : mosaics) {
			float4x4 P = fitPlane(mesh, mosaic.indexes, &deviations);
			std::map<int,size_t> uniqueSet;
			for (int2 fid : mosaic.indexes) {
				int vid = fid.x;
				if (uniqueSet.find(vid) == uniqueSet.end()) {
					float3 pt = mesh.vertexLocations[vid];
					float3 qt = Transform(P, pt);
					MosaicVertex mv(pt, qt.xy());
					int commonCount = 0;
					int pivot = vertexLabels[vid];
					std::list<int>& nbrs = vertNbrs[vid];
					for (int nbr : nbrs) {
						if (vertexLabels[nbr] == pivot) {
							commonCount++;
						}
					}
					mv.boundary = (commonCount <= 4);//Guess at whether point is boundary / butterfly
					mosaic.vertexMap[fid.y] = mosaic.vertexes.size();
					uniqueSet[vid] = mosaic.vertexes.size();
					mosaic.vertexes.push_back(mv);
				} else {
					mosaic.vertexMap[fid.y] = uniqueSet[vid];
				}
			}
			T += mosaic.indexes.size() / 3;
			int minIdx = -1;
			int maxIdx = -1;
			float minu = 1E30f;
			float maxu = -1E30f;
			int idx = 0;

			F += mosaic.vertexes.size();

			for (MosaicVertex& mv : mosaic.vertexes)
			{
				mv.id = V++;
				if (mv.boundary) {
					if (mv.uv.x < minu) {
						minIdx = idx;
						minu = mv.uv.x;
					}
					if (mv.uv.x > maxu) {
						maxIdx = idx;
						maxu = mv.uv.x;
					}
				}
				idx++;
			}
			
			if (minIdx >= 0) {
				mosaic.vertexes[minIdx].locked = true;
				lockedList.push_back(mosaic.vertexes[minIdx].id);
			}
			if (maxIdx >= 0&&maxIdx!=minIdx) {
				mosaic.vertexes[maxIdx].locked = true;
				lockedList.push_back(mosaic.vertexes[maxIdx].id);
			}
			
			index++;
		}
		C = (int)lockedList.size();
		SparseMat<double> A(2*(C+T), 2 * F);
		Vec<double> B(2 * (C + T));
		Vec<double> X(2 * F);
		float2x2 Rot;
		Rot(0, 0) =  0.0f;
		Rot(0, 1) = -1.0f;
		Rot(1, 0) =  1.0f;
		Rot(1, 1) =  0.0f;
		float2x3 Mt,Mr;
		int R = 0;
		int2 fid;
		float2 p1, p2, p3;
		float2 q1, q2, q3;
		float3 pv1, qv1, pv2, qv2;
		int ui1, vi1, ui2, vi2, ui3, vi3;
		for (Mosaic& mosaic : mosaics) {
			for (MosaicVertex& mv : mosaic.vertexes) {
				X[2 * mv.id  ] = mv.uv.x;
				X[2 * mv.id+1] = mv.uv.y;
			}

			for (auto iter = mosaic.indexes.begin(); iter != mosaic.indexes.end();) {
				fid = *iter;
				MosaicVertex& v1 = mosaic.getVertex(fid.y);
				iter++; 
				fid = *iter;
				MosaicVertex& v2 = mosaic.getVertex(fid.y);
				iter++; 
				fid = *iter;
				MosaicVertex& v3 = mosaic.getVertex(fid.y);
				iter++;
				projectTriangle(v1.pt, v2.pt, v3.pt, p1, p2, p3);

				Mt(0, 0) = p2.y - p3.y;
				Mt(0, 1) = p3.y - p1.y;
				Mt(0, 2) = p1.y - p2.y;
				Mt(1, 0) = p3.x - p2.x;
				Mt(1, 1) = p1.x - p3.x;
				Mt(1, 2) = p2.x - p1.x;
				Mr=Rot*Mt;
				ui1 = 2 * v1.id;
				vi1 = 2 * v1.id + 1;
				ui2 = 2 * v2.id;
				vi2 = 2 * v2.id + 1;
				ui3 = 2 * v3.id;
				vi3 = 2 * v3.id + 1;

				// Real part
				A.set(R, vi1,  Mt(0, 0));
				A.set(R, vi2,  Mt(0, 1));
				A.set(R, vi3,  Mt(0, 2));
				A.set(R, ui1, -Mr(0, 0));
				A.set(R, ui2, -Mr(0, 1));
				A.set(R, ui3, -Mr(0, 2));
				B[R] = 0.0f;
				R++;
				// Imaginary part
				A.set(R, vi1,  Mt(1, 0));
				A.set(R, vi2,  Mt(1, 1));
				A.set(R, vi3,  Mt(1, 2));
				A.set(R, ui1, -Mr(1, 0));
				A.set(R, ui2, -Mr(1, 1));
				A.set(R, ui3, -Mr(1, 2));
				B[R] = 0.0f;
				R++;
			}
		}
		for (int id : lockedList) {
			B[R] = X[2 * id];
			B[R+1] = X[2 * id+1];
			A.set(R,   2 * id  , 1.0f);
			A.set(R+1, 2 * id+1, 1.0f);
			R+=2;
		}
		SparseMat<double> At = A.transpose();
		SparseMat<double> AtA = At * A;
		Vec<double> Atb = At*B;
		SolveBICGStab(Atb, AtA, X, conformalIterations, 1E-30/*, [=](int iter, double err) {
			std::cout << "Iteration " << iter << " " << err << std::endl;
			return true;
		}*/);
		index = 0;
		for (Mosaic& mosaic:mosaics) {
			float2 minuv(1E30f);
			float2 maxuv(-1E30f);
			int idx = 0;
			for(MosaicVertex& mv: mosaic.vertexes)
			{
				mv.uv = float2(float(X[2 * mv.id]), float(X[2 * mv.id + 1]));
				minuv = aly::min(minuv, mv.uv);
				maxuv = aly::max(maxuv, mv.uv);
				idx++;
			}
			//Lock off only u-dimension extremes
			//float maxDim = std::max(1E-4f, std::max(maxuv.y - minuv.y, maxuv.x - minuv.x)); //Max dim should already be close to 1.0f
			bvec2f rect = bvec2f((maxuv.x - minuv.x), (maxuv.y - minuv.y), index);
			rects.push_back(rect);
			rectId.push_back(index);
			totalArea += rect.x*rect.y;
			//Is this necessary?
			for (MosaicVertex& mv : mosaic.vertexes) {
				mv.uv = mv.uv - minuv;
			}
			index++;
		}
	}
	void MeshTextureMap::projectTriangle(const float3& p0, const float3& p1, const float3& p2,float2& z0, float2& z1, float2& z2) {
			float3 X =normalize(p1 - p0);
			float3 Z = cross(X ,p2 - p0);
			float3 Y = normalize(cross(X , Z));
			z0 = float2(0.0f,0.0f);
			z1 = float2(length(p1 - p0), 0.0f);
			z2 = float2(dot(p2 - p0, X), dot(p2 - p0, Y));

	}
	void MeshTextureMap::computeMap(aly::Mesh& mesh, const std::function<bool(const std::string& status, float progress)>& statusHandler){
		std::multimap<bvec2f, float2, TextureBoxCompare> boxes;
		std::vector<int> rectId;
		std::vector<bvec2f> rects;
		unfold(mesh, rectId, rects);
		float totalArea = pack(rects, boxes);
		float  sc = 1.0f / totalArea;
		int index=0;
		mesh.textureMap.resize(mesh.triIndexes.size() * 3, float2(0, 0));
		for (int id = 0; id < rectId.size(); id++){
			Mosaic& mosaic = mosaics[id];

			bvec2f& rect = rects[id];
			std::multimap<bvec2f, float2, TextureBoxCompare>::iterator boxPair = boxes.find(rect);
			index = 0;
			if (boxPair == boxes.end()){
				for (int2 fid:mosaic.indexes){
					mesh.textureMap[fid.y]= float2(0.0f);
				}
			} else {
				float2 pt = boxPair->second;
				for (int2 fid : mosaic.indexes) {
					MosaicVertex& mv = mosaic.vertexes[mosaic.vertexMap[fid.y]];
					mesh.textureMap[fid.y] = (mv.uv + pt)*sc;
				}
			}
		}	
		if (mesh.textureImage.size() > 0) {
			const int horizTiles = 32;
			const int vertTiles = 32;
			const int width = mesh.textureImage.width;
			const int height = mesh.textureImage.height;
			const int cellWidth = width / horizTiles;
			const int cellHeight = height / vertTiles;
			mesh.textureImage.set(float4(0.0f));
			int textureHeight = mesh.textureImage.height;
			int textureWidth = mesh.textureImage.width;
			for (int id = 0; id < rectId.size(); id++) {

				bvec2f& rect = rects[id];
				std::multimap<bvec2f, float2, TextureBoxCompare>::iterator boxPair = boxes.find(rect);
				if (boxPair == boxes.end())continue;
				float2 pt = boxPair->second;
				int ymin = std::max((int)0, (int)(textureHeight*pt.y / totalArea));
				int ymax = std::min((int)(textureHeight*(rect.y + pt.y) / totalArea), (int)textureHeight);
				int xmin = std::max((int)0, (int)(textureWidth*pt.x / totalArea));
				int xmax = std::min((int)(textureWidth*(rect.x + pt.x) / totalArea), (int)textureWidth);
				for (int y = ymin; y < ymax; y++) {
					for (int x = xmin; x < xmax; x++) {
						int i = x;
						int j = textureHeight - 1 - y;
						bool vt = (i / cellWidth) % 2 == 0;
						bool ht = (j / cellHeight) % 2 == 0;
						mesh.textureImage(i, j) = ((vt && !ht) || (!vt && ht)) ? RGBAf(0.1f, 0.1f, 0.1f, 1.0f) : RGBAf(0.35f, 0.35f, 0.35f, 1.0f);
					}
				}
			}
		}
		mesh.setDirty(true);

	}
	float2 MeshTextureMap::packNaive(const std::vector<bvec2f> &rects, std::multimap<bvec2f, float2, TextureBoxCompare>& boxes, float area){
		float edgeLength = std::sqrt(area);
		float xOff=0, yOff=0;
		float2 maxPt(0,0);
		for (bvec2f rect : rects){
			std::pair<bvec2f, float2> boxPair = std::pair<bvec2f, float2>(rect, float2(xOff, yOff));
			xOff += rect.x;
			maxPt = aly::max(maxPt,float2(boxPair.first.x,boxPair.first.y) + boxPair.second);
			if (xOff > edgeLength){
				xOff = 0;
				yOff = maxPt.y;
			}
			boxes.insert(boxPair);
		}
		return maxPt;
	}
	float MeshTextureMap::packAllNaive(std::vector<bvec2f>& rects, std::multimap<bvec2f, float2, TextureBoxCompare>& boxMap){
		double totalArea = 0;
		int N = static_cast<int>(rects.size());
		for (int i = 0; i < N; i++){
			bvec2f rect = rects[i];
			double a = rect.x*rect.y;
			totalArea += a;
		}
		float edgeLength = (float)std::sqrt(totalArea);
		float2 packedRect = packNaive(rects, boxMap, (float)totalArea);
		edgeLength = std::max(packedRect.x, packedRect.y);
		return edgeLength;
	}
	float MeshTextureMap::pack(std::vector<bvec2f>& rects, std::multimap<bvec2f, float2, TextureBoxCompare>& boxMap){
		double totalArea = 0;
		int N = static_cast<int>(rects.size());
		std::vector<std::pair<double, int>> rectSortList(N);
		for (int i = 0; i < N; i++){
			bvec2f rect = rects[i];
			double a = rect.x*rect.y;
			rectSortList[i] = std::pair<double, int>(-a, i);
			totalArea += a;
		}
		sort(rectSortList.begin(), rectSortList.end());
		std::list<std::vector<bvec2f>> rectBatches;
		std::list<std::vector<int>> rectIdBatches;
		std::list<double> areas;
		std::vector<bvec2f> rectBatch;
		std::vector<int> rectIdBatch;
		double area = 0.0;
		double areaThreshold = totalArea*packingRatio;
		double areaSum=0.0;
		for (int i = 0; i < N; i++){
			int id = rectSortList[i].second;
			float a = -(float)rectSortList[i].first;
			rectBatch.push_back(rects[id]);
			area+=a;
			if (area >= areaThreshold||i==N-1){
				rectBatches.push_back(rectBatch);
				rectIdBatches.push_back(rectIdBatch);
				areas.push_back(area);
				rectBatch.clear();
				rectIdBatch.clear();
				areaSum += area;
				areaThreshold = totalArea - areaSum;
				if (areaThreshold <= 0.01f*totalArea){
					areaThreshold = 0.01f*totalArea;
				}
				else {
					areaThreshold = std::max(areaThreshold*0.5f, 0.01f*totalArea);
				}
				area = 0.0f;
			}
		}
		float edgeLength = (float)std::sqrt(totalArea);
		
		rectBatches.reverse();
		rectIdBatches.reverse();
		areas.reverse();
		std::list<std::vector<bvec2f>>::iterator rectIter = rectBatches.begin();
		std::list<std::vector<int>>::iterator rectIdIter = rectIdBatches.begin();
		std::list<double>::iterator areaIter = areas.begin();
		bvec2f lastRect(0.0f,0.0f,0);
		float2 packedRect(0.0f, 0.0f);
		std::multimap<bvec2f, float2, TextureBoxCompare> boxes;
		int count = 0;
		for (; rectIter != rectBatches.end();rectIter++,areaIter++,rectIdIter++){
			double area = *areaIter;
			//std::cout << "Pack [" << rectIter->size() << " , " << 100.0*area/totalArea <<" %]"<< std::endl;
			boxes.clear();
			std::vector<bvec2f>& rectList = *rectIter;
			if (boxMap.size() > 0){
				rectList.insert(rectList.begin(),lastRect);
			}
			if (area < 0.01f*totalArea) {
				packedRect = packNaive(rectList, boxes, (float)area);
			} else {
				packedRect= pack(rectList, boxes, (float)area);
			}
			if (boxMap.size() > 0){
				float2 offset = boxes.find(lastRect)->second;
				for (std::multimap<bvec2f, float2, TextureBoxCompare>::iterator boxPair = boxMap.begin(); boxPair != boxMap.end(); boxPair++){
					boxPair->second=boxPair->second+offset;				
				}
			}
			for (std::pair<bvec2f, float2> boxPair : boxes){
				bvec2f rect = boxPair.first;
				if (rect.x!=lastRect.x||rect.y!=lastRect.y)boxMap.insert(boxPair);
			}
			lastRect = bvec2f(packedRect.x,packedRect.y,-count);
			count++;
		}
		edgeLength = std::max(packedRect.x, packedRect.y);
		return edgeLength;
	}
	float2 MeshTextureMap::pack(const std::vector<bvec2f> &rects, std::multimap<bvec2f, float2, TextureBoxCompare>& boxes, float currentArea){
		
		bool fine = false;
		int count = 0;
		float totalArea = 0.0f;
		float edgeLength=0.0f;
		for (bvec2f rect:rects){
			totalArea += rect.x*rect.y;
			edgeLength = std::max(edgeLength,std::max(rect.x, rect.y));
		}
		edgeLength = std::max(std::sqrt(totalArea),edgeLength);
		do{
			boxes.clear();
			edgeLength *= 1.1f;
			fine = pack(rects, boxes, float2(edgeLength, edgeLength));	
			count++;
		} while (!fine);
		float2 maxPt(-1E30f, -1E30f);
		for (std::pair<bvec2f, float2> boxPair : boxes){
			maxPt = aly::max(maxPt,(boxPair.second + float2(boxPair.first.x, boxPair.first.y)));
		}
		return maxPt;
	}

	bool MeshTextureMap::pack(const std::vector<bvec2f> &temp, std::multimap<bvec2f, float2, TextureBoxCompare>& boxes, const float2 &size)
	{
		std::list<float4> freeBoxes;
		freeBoxes.push_back(float4(0, 0, size.x, size.y));
		std::list<bvec2f> rects(temp.begin(), temp.end());
		while (!rects.empty()){
			float min = std::max(size.x, size.y);
			std::list<bvec2f>::iterator minIterRects = rects.end();
			std::list<float4>::iterator minIterFreeBoxes = freeBoxes.end();
			for (auto iterRects = rects.begin(); iterRects != rects.end(); iterRects++){
				for (auto iterFreeBoxes = freeBoxes.begin(); iterFreeBoxes != freeBoxes.end(); iterFreeBoxes++){
					float distance = std::min(iterFreeBoxes->w - iterRects->y, iterFreeBoxes->z - iterRects->x);
					if (distance < 0){
						continue;
					}
					if (distance < min){
						min = distance;
						minIterRects = iterRects;
						minIterFreeBoxes = iterFreeBoxes;
					}
				}
			}
			if (minIterRects == rects.end() || minIterFreeBoxes == freeBoxes.end()){
				boxes.clear();
				return false;
			}
			float4 insertedBox = float4(minIterFreeBoxes->x, minIterFreeBoxes->y, (*minIterRects).x, (*minIterRects).y);
			boxes.insert(std::make_pair(*minIterRects, float2(minIterFreeBoxes->x, minIterFreeBoxes->y)));

			freeBoxes.push_front(float4(
				minIterFreeBoxes->x + minIterRects->x,
				minIterFreeBoxes->y + 0,
				minIterFreeBoxes->z - minIterRects->x,
				minIterFreeBoxes->w));

			freeBoxes.push_front(float4(
				minIterFreeBoxes->x + 0,
				minIterFreeBoxes->y + minIterRects->y,
				minIterFreeBoxes->z,
				minIterFreeBoxes->w - minIterRects->y));

			freeBoxes.erase(minIterFreeBoxes);

			float2 a1 = float2(insertedBox.x, insertedBox.y);
			float2 a2 = float2(insertedBox.x + insertedBox.z, insertedBox.y);
			float2 a3 = float2(insertedBox.x + insertedBox.z, insertedBox.y + insertedBox.w);
			//float2 a4 = float2(insertedBox.x, insertedBox.y + insertedBox.w);

			for (auto iterFreeBoxes = freeBoxes.begin(); iterFreeBoxes != freeBoxes.end();){
				assert(iterFreeBoxes != freeBoxes.end());
				float2 b1 = float2(iterFreeBoxes->x, iterFreeBoxes->y);
				//float2 b2 = float2(iterFreeBoxes->x + iterFreeBoxes->z, iterFreeBoxes->y);
				float2 b3 = float2(iterFreeBoxes->x + iterFreeBoxes->z, iterFreeBoxes->y + iterFreeBoxes->w);
				//float2 b4 = float2(iterFreeBoxes->x, iterFreeBoxes->y + iterFreeBoxes->w);

				if ((a1.x >= b3.x) || (a1.y >= b3.y) ||
					(a3.x <= b1.x) || (a3.y <= b1.y))
				{
					iterFreeBoxes++;
					continue;   // not intersecting
				}


				if ((a1.x > b1.x)  /*&& (a1.x < b3.x)*/ ) // If there is a chance that line a1,a2 is in the shape b
				{
					freeBoxes.push_front(float4(
						iterFreeBoxes->x,
						iterFreeBoxes->y,
						a1.x - b1.x,
						iterFreeBoxes->w));
				}

				if ((a3.x < b3.x) /* && (a1.x < b3.x) */) // If there is a chance that line a1,a2 is in the shape b
				{
					freeBoxes.push_front(float4(
						a3.x,
						iterFreeBoxes->y,
						b3.x - a3.x,
						iterFreeBoxes->w));
				}

				if ((a1.y > b1.y) /*  && (a1.x < b3.x) */) // If there is a chance that line a1,a2 is in the shape b
				{
					freeBoxes.push_front(float4(
						iterFreeBoxes->x,
						iterFreeBoxes->y,
						iterFreeBoxes->z,
						a1.y - b1.y));
				}

				if ((a3.y < b3.y) /*  && (a1.x < b3.x) */) // If there is a chance that line a1,a2 is in the shape b
				{
					freeBoxes.push_front(float4(
						iterFreeBoxes->x,
						a3.y,
						iterFreeBoxes->z,
						b3.y - a3.y));
				}

				auto blah = iterFreeBoxes++;
				freeBoxes.erase(blah);
			}

			for (auto iterFreeBoxes = freeBoxes.begin(); iterFreeBoxes != freeBoxes.end(); iterFreeBoxes++)
			{
				assert(iterFreeBoxes != freeBoxes.end());
				auto iterFreeBoxes2 = iterFreeBoxes;
				iterFreeBoxes2++;

				while (iterFreeBoxes2 != freeBoxes.end())
				{
					assert(iterFreeBoxes != freeBoxes.end());

					assert(iterFreeBoxes2 != freeBoxes.end());
					assert(iterFreeBoxes != iterFreeBoxes2);
					//assert(!((*iterFreeBoxes).pos == (*iterFreeBoxes2).pos));

					float2 b1 = float2(iterFreeBoxes->x, iterFreeBoxes->y);
					//float2 b2 = float2(iterFreeBoxes->x + iterFreeBoxes->z, iterFreeBoxes->y);
					float2 b3 = float2(iterFreeBoxes->x + iterFreeBoxes->z, iterFreeBoxes->y + iterFreeBoxes->w);
					//float2 b4 = float2(iterFreeBoxes->x, iterFreeBoxes->y + iterFreeBoxes->w);

					float2 c1 = float2(iterFreeBoxes2->x, iterFreeBoxes2->y);
					float2 c2 = float2(iterFreeBoxes2->x + iterFreeBoxes2->z, iterFreeBoxes2->y);
					float2 c3 = float2(iterFreeBoxes2->x + iterFreeBoxes2->z, iterFreeBoxes2->y + iterFreeBoxes2->w);
					//float2 c4 = float2(iterFreeBoxes2->x, iterFreeBoxes2->y + iterFreeBoxes2->w);

					if (c1.x >= b1.x && c1.y >= b1.y &&
						c3.x <= b3.x && c3.y <= b3.y)
					{
						//std::cout<<"Erasing a box "<<iterFreeBoxes->pos<<", "<<iterFreeBoxes->size<<" and "<<iterFreeBoxes2->pos<<", "<<iterFreeBoxes2->size<<std::endl;
						auto blah = iterFreeBoxes2++;
						freeBoxes.erase(blah);
					}
					else
						iterFreeBoxes2++;

				}
			}
			rects.erase(minIterRects);
			//std::cout<<"Finished with said loop"<<std::endl;
		}
		return true;
	}

	void MeshTextureMap::labelComponents(aly::Mesh& mesh, const std::function<bool(const std::string& status, float progress)>& statusHandler){
		std::vector<int> cclist;
		Vector3ui& faceArray = mesh.triIndexes;
		Vector3f& vertexArray = mesh.vertexLocations;
		Vector3f& normalArray = mesh.vertexNormals;

		int vertexCount = static_cast<int>(vertexArray.size());
		int faceCount = static_cast<int>(faceArray.size());
		int indexCount = faceCount * 3;
		vertexLabels.resize(vertexCount,-1);
		faceLabels.resize(faceCount,-1);

		vertNbrs.resize(vertexCount);

		int count;
		int itrStep = indexCount / 100;
		for (int i = 0; i < faceCount; i ++){
			if (i%itrStep == 0 && statusHandler) {
				statusHandler("Connected Components ...", 0.5f*(i / (float)indexCount));
			}
			uint3 tri = faceArray[i];
			vertNbrs[tri.x].push_back(tri.y);
			vertNbrs[tri.x].push_back(tri.z);
			vertNbrs[tri.y].push_back(tri.z);
			vertNbrs[tri.y].push_back(tri.x);
			vertNbrs[tri.z].push_back(tri.x);
			vertNbrs[tri.z].push_back(tri.y);
		}
		vertexLabels.resize(vertexCount);
		std::vector<int> order(vertexCount);
		for (count = 0; count < vertexCount;count++){
			vertexLabels[count] = count;
			order[count] = count;
		}
		Shuffle(order);
		int FIRST_PASS_MAX_ITERATIONS = vertexCount/16;
		float dotTolerance = std::cos(angleTolerance);
		for (int iter = 0; iter < FIRST_PASS_MAX_ITERATIONS; iter++){
			int changeCount = 0;
			for (count = 0; count < vertexCount; count++){
				int v = order[count];
				int l = vertexLabels[v];
				float3 norm = normalArray[l];// label corresponds to vertex of seed ... sneaky 
				for (int nbr : vertNbrs[v]){
					if (l<vertexLabels[nbr]){
						if (dot(normalArray[nbr],norm) > dotTolerance){
							vertexLabels[nbr] = l;
							changeCount++;
						}
					}
				}
			}
			if (changeCount == 0)break;
			if(iter%4==0) Shuffle(order);
		}

		std::vector<int> relabel;
		int scc = makeLabelsUnique(vertexLabels, relabel, minVertexPatchSize);
		
		const int SECOND_PASS_MAX_ITERATIONS =vertexCount/16;
		for (int iter = 0; iter < SECOND_PASS_MAX_ITERATIONS; iter++){

			int changeCount = 0;
			for (int v:relabel){
				int l = vertexLabels[v];
				if (l <scc)continue;
				float bestScore=0;
				int bestLabel = -1;
				float score;
				float3 norm = normalArray[v];
				for (int nbr : vertNbrs[v]){
					int nl = vertexLabels[nbr];
					if (nl<scc){
						score = dot(norm,normalArray[nbr]);
						if (score>bestScore){
							bestScore = score;
							bestLabel = nl;
						}
					}
				}
				if (bestLabel >= 0){
					vertexLabels[v] = bestLabel;
					changeCount++;
				}
			}
			if (changeCount == 0)break;
		}
		
		//
		scc = splitLabelComponents(vertexLabels);
		scc = makeLabelsUnique(vertexLabels, relabel, 0);

		mosaics.resize(scc);
		int idx = 0;
		for (count = 0; count < faceCount; count++){
			uint3 tri = faceArray[count];
			int l1 = vertexLabels[tri.x];
			int l2 = vertexLabels[tri.y];
			int l3 = vertexLabels[tri.z];
			if (l1 == l2&&l2 == l3&&l3 == l1){
				faceLabels[idx] = l1;
			}
			else {
				if (l1 == l2){
					faceLabels[idx] = l1;
				} 
				else
				if (l2 == l3){
					faceLabels[idx] = l2;
				}
				else
				if (l3 == l1){
					faceLabels[idx] = l3;
				}
				else {
					faceLabels[idx] = min(min(l1, l2), l3);
				}
			}
			idx++;
		}
		

		for (count = 0; count < faceCount; count ++){
			uint3 tri=faceArray[count];
			vertexLabels[tri.x] = faceLabels[count];
			vertexLabels[tri.y] = faceLabels[count];
			vertexLabels[tri.z] = faceLabels[count];
		}
	
		mosaics.clear();
		mosaics.resize(scc);

		for (count = 0; count < faceCount; count++){
			int l = faceLabels[count];
			if (l < scc){
				uint3 tri = faceArray[count];
				mosaics[l].indexes.push_back(int2(tri.x, 3 * count));
				mosaics[l].indexes.push_back(int2(tri.y, 3 * count + 1));
				mosaics[l].indexes.push_back(int2(tri.z, 3 * count + 2));
			}
		}
		for (auto iter = mosaics.begin(); iter != mosaics.end();iter++) {
			if (iter->indexes.size() == 0) {
				mosaics.erase(iter);
				if (mosaics.size() > 0) {
					iter--;
				}
			}
		}
	}
	int MeshTextureMap::splitLabelComponents(std::vector<int>& vertxLabels){
		std::vector<int> cclist;
		
		int vertexCount = static_cast<int>(vertxLabels.size());
		int pivot = 0;
		int cc = 0;
		std::list<int> queue;
		cclist.clear();
		int masked = 0;
		int ccCount = 0;

		std::vector<int> labels(vertexCount,-1);
		std::vector<int> nbrs;
		cclist.clear();
		do {
			ccCount = 0;
			queue.clear();
			labels[pivot] = cc;
			ccCount++;
			queue.push_back(pivot);
			masked++;
			int iter = 0;
			int l = vertexLabels[pivot];
			while (queue.size() > 0){
				int v = queue.front();
				queue.pop_front();
				nbrs.clear();
				iter++;
				int nl;
				for (int nbr : vertNbrs[v]){
					nl = vertexLabels[nbr]; 
					if (labels[nbr] < 0&&nl==l){
						labels[nbr] = cc;
						ccCount++;
						masked++;
						queue.push_back(nbr);
					}
				}
			}
			cclist.push_back(ccCount);
			int lastPivot = pivot;
			pivot = -1;
			for (int i = lastPivot + 1; i < vertexCount; i++){
				if (labels[i] < 0){
					pivot = i;
					break;
				}
			}
			cc++;
		} while (pivot >= 0);
		
		for (int i = 0; i < vertexCount;i++){
			vertxLabels[i] = labels[i];
		}
		return static_cast<int>(cclist.size());
	}
	

	float4x4 MeshTextureMap::fitPlane(const aly::Mesh& mesh, std::list<int2>& indexes,float3* deviations){
		float3x3 AtA= float3x3::zero();
		float3 v;
		float3 centerPoint(0.0f);

		for (int2 idx : indexes){
			centerPoint += mesh.vertexLocations[idx.x];
		}
		int N = static_cast<int>(indexes.size());
		centerPoint= centerPoint*(1.0f/N);
		for (int2 idx : indexes){
			v = mesh.vertexLocations[idx.x];
			v = v - centerPoint;

			AtA(0, 0) += v.x*v.x;
			AtA(1, 0) += v.x*v.y;
			AtA(2, 0) += v.x*v.z;
			AtA(0, 1) += v.y*v.x;
			AtA(1, 1) += v.y*v.y;
			AtA(2, 1) += v.y*v.z;
			AtA(0, 2) += v.z*v.x;
			AtA(1, 2) += v.z*v.y;
			AtA(2, 2) += v.z*v.z;

		}
		AtA = (1.0f/N)*AtA;
		float3x3 Q, D;
		Eigen(AtA, Q, D);
		
		*deviations = float3((float)std::sqrt(D(2,2)), (float)std::sqrt(D(1,1)), (float)std::sqrt(D(0,0)));
		float3 zaxis=Q.z;
		float3 xaxis=Q.x;
		float3 yaxis = cross(zaxis,xaxis);
		float4x4 Pose = float4x4::identity();
		Pose(0, 0) = xaxis.x;
		Pose(1, 0) = xaxis.y;
		Pose(2, 0) = xaxis.z;

		Pose(0, 1) = yaxis.x;
		Pose(1, 1) = yaxis.y;
		Pose(2, 1) = yaxis.z;

		Pose(0, 2) = zaxis.x;
		Pose(1, 2) = zaxis.y;
		Pose(2, 2) = zaxis.z;

		Pose(0, 3) = centerPoint.x;
		Pose(1, 3) = centerPoint.y;
		Pose(2, 3) = centerPoint.z;
		Pose(3, 3) = 1.0f;

		float4x4 PInv=inverse(Pose);

		float dotSum = 0.0f;
		for (int2 idx : indexes){
			float3 norm = mesh.vertexNormals[idx.x];
			float4 n4=PInv*float4(norm.x, norm.y, norm.z,0.0f);
			dotSum += (n4.z>0.0f)?1.0f:-1.0f;
		}
		//Re-orient to face up
		if (dotSum < 0.0f){
			PInv = MakeRotationX((float)ALY_PI)*PInv;
		}

		return PInv;

	}
	void MeshTextureMap::smooth(aly::Mesh& mesh,  int iters, float errorTolerance){

		MeshSetNeighborTable nbrTable;
		CreateVertexNeighborTable(mesh, nbrTable);
		int index = 0;
		int N =  (int)mesh.vertexLocations.size();
		SparseMatrix1f A(N, N);
		Vector3f b(N);
		for (std::set<uint32_t>& nbrs : nbrTable) {
			int K = (int)nbrs.size() - 1;
			float3 pt = mesh.vertexLocations[index];
			{
				auto nbrIter = nbrs.begin();
				nbrIter++;
				for (int k = 0; k < K; k++) {
					float w = -smoothness / K;
					int offset = *nbrIter;
					A.set(index, offset, w);
					nbrIter++;
				}
			}
			A.set(index, index, smoothness + 1);
			b[index] = pt;
			index++;
		}
		SolveBICGStab(b, A, mesh.vertexLocations, smoothIterations,errorTolerance);
		mesh.updateVertexNormals();
	}
}
