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
#include "OpenNL.h"
#include "AlloySparseMatrix.h"
#include "AlloySparseSolve.h"
#include <algorithm>
#include <random>
namespace aly {
	int MeshTexureMap::makeLabelsUnique(std::vector<int>& vertexLabels, std::vector<int>& relabel, int minLabelSize) {
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
	void MeshTexureMap::evaluate(aly::Mesh& mesh, const std::function<bool(const std::string& status, float progress)>& statusHandler){
		Vector3f vertexCopy = mesh.vertexLocations;
		smooth(mesh,3, 1E-6f);
		std::cout << "Label ..." << std::endl;
		labelComponents(mesh, statusHandler);
		std::cout << "Compute UVs ..." << std::endl;
		computeMap(mesh, statusHandler);
		mesh.vertexLocations = vertexCopy;
		std::cout << "Saving ..." << std::endl;
	}
	void MeshTexureMap::computeMap(aly::Mesh& mesh, const std::function<bool(const std::string& status, float progress)>& statusHandler){
		int index = 0;
		float3 deviations;
		float totalArea = 0.0f;
		IndexedMesh mosaic;
		int M = static_cast<int>(mosaicIndexes.size());
		std::vector<std::map<int, int>> uniqueVertexLists(M);
		std::vector<float> scales(M, 1.0f);
		int N = 0;
		for (std::list<int2>& mIndexes : mosaicIndexes){
			N += static_cast<int>(mIndexes.size());
		}
		std::vector<int> faceList(N);
		int v = 0;
		for (std::list<int2>& mIndexes : mosaicIndexes){
			if (mIndexes.size() == 0){
				index++;
				continue;
			}
			float4x4 P = fitPlane(mesh, mIndexes, &deviations, &scales[index]);
			//std::cout << "Pose " << mIndexes.size() << "[" << deviations << "] =\n" << mIndexes.size()<<std::endl;

			std::map<int, int>& uniqueVertexes = uniqueVertexLists[index];
			int lockCount = 0;
			int fid = 0;
			for (int2 fid : mIndexes){
				int vid = fid.x;
				if (uniqueVertexes.find(vid) == uniqueVertexes.end()) {
					float3 pt = mesh.vertexLocations[vid];
					float3 qt = Transform(P, pt);
					Vertex* V = mosaic.add_vertex(Vector3(pt.x, pt.y, pt.z), Vector2(qt.x, qt.y));
					uniqueVertexes[vid] = V->id;
					int commonCount = 0;
					int pivot = vertexLabels[vid];
					std::list<int>& nbrs = vertNbrs[vid];
					for (int nbr : nbrs){
						if (vertexLabels[nbr] == pivot){
							commonCount++;
						}
					}
					V->locked = (commonCount < 4);
					if (V->locked)lockCount++;
				}
				faceList[v] = fid.y;
				int id = uniqueVertexes[vid];
				if (v % 3 == 0){
					mosaic.begin_facet();
				}
				mosaic.add_vertex_to_facet(id);
				v++;
				if (v % 3 == 0){
					mosaic.end_facet();
				}

			}
			index++;
		}
		std::vector<int> rectId;
		std::vector<float2i> rects;
		OpenNL_Parameterization(std::string("CG"), mosaic, 1E-6, 1000);
		for (index = 0; index < M; index++){
			std::map<int, int>& uniqueVertexes = uniqueVertexLists[index];
			if (uniqueVertexes.size() == 0)continue;
			float2 minuv(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			float2 maxuv(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
			for (std::pair<int, int> pr : uniqueVertexes)
			{
				int k = pr.second;
				minuv.x = std::min(minuv.x, float(mosaic.vertex[k].tex_coord.x));
				minuv.y = std::min(minuv.y, float(mosaic.vertex[k].tex_coord.y));
				maxuv.x = std::max(maxuv.x, float(mosaic.vertex[k].tex_coord.x));
				maxuv.y = std::max(maxuv.y, float(mosaic.vertex[k].tex_coord.y));
			}

			float maxDim = std::max(1E-4f, std::max(maxuv.y - minuv.y, maxuv.x - minuv.x)); //Max dim should already be close to 1.0f
			float scale = scales[index];
			//float sc = 1.0f/ maxDim;
			float2i rect = float2i(float2(scale*(maxuv.x - minuv.x), scale*(maxuv.y - minuv.y)),index);
			rects.push_back(rect);
			rectId.push_back(index);
			totalArea += rect.x*rect.y;
			for (std::pair<int, int> pr : uniqueVertexes)
			{
				int k = pr.second;
				mosaic.vertex[k].tex_coord.x = (mosaic.vertex[k].tex_coord.x - minuv.x);
				mosaic.vertex[k].tex_coord.y = (mosaic.vertex[k].tex_coord.y - minuv.y);
			}
		}
		std::multimap<float2i, float2, TextureBoxCompare> boxes;
		totalArea = pack(rects, boxes);
		float  sc = 1.0f / totalArea;
		
		for (int id = 0; id < rectId.size(); id++){
			int idx = rectId[id];
			float2i& rect = rects[id];
			std::multimap<float2i, float2, TextureBoxCompare>::iterator boxPair = boxes.find(rect);
			std::map<int, int>& uniqueVertexes = uniqueVertexLists[idx];
			if (boxPair == boxes.end()){
				for (std::pair<int, int> pr : uniqueVertexes)
				{
					int k = pr.second;
					mosaic.vertex[k].tex_coord.x = 0.0f;
					mosaic.vertex[k].tex_coord.y = 0.0f;
				}
			}
			else {
				float2 pt = boxPair->second;
				float scale = scales[idx];
				for (std::pair<int, int> pr : uniqueVertexes)
				{
					int k = pr.second;
					mosaic.vertex[k].tex_coord.x = (scale*mosaic.vertex[k].tex_coord.x + pt.x)*sc;
					mosaic.vertex[k].tex_coord.y = (scale*mosaic.vertex[k].tex_coord.y + pt.y)*sc;
				}
			}
		}	
		mesh.textureMap.resize(mesh.triIndexes.size()*3,float2(0,0));
		index = 0;
		for (Facet F:mosaic.facet){
			for (int vid : F){
				Vertex& V = mosaic.vertex[vid];
				mesh.textureMap[faceList[index++]] = float2((float)V.tex_coord.x, (float)V.tex_coord.y);
			}
		}
		int textureWidth = 2048;// RoundToWorkgroupPower(totalArea, 8);
		int textureHeight = 2048; //textureWidth;
		mesh.textureImage.resize(textureHeight,textureWidth);
		std::cout << "Create texture image" << std::endl;
#ifdef USE_OPENCV
		using namespace cv;
		Mat img(textureHeight, textureWidth, CV_8UC4,&mesh->textureImage[0]);

		for (int id = 0; id < rectId.size(); id++){
			int idx = rectId[id];
			float3& rect = rects[id];

			std::multimap<float3, float2, TextureBoxCompare>::iterator boxPair = boxes.find(rect);
			if (boxPair == boxes.end())continue;
			float2 pt = boxPair->second;
			float4 c = (idx<colors.size())?colors[idx]:float4(1.0f);
			uchar4 rgb((uchar)(c.x * 255), (uchar)(c.y * 255), (uchar)(c.z * 255), (uchar)255);
			int ymin = std::max((int)0, (int)(textureHeight*pt.y / totalArea));
			int ymax = std::min((int)(textureHeight*(rect.y + pt.y) / totalArea), (int)textureHeight);
			int xmin = std::max((int)0, (int)(textureWidth*pt.x / totalArea));
			int xmax = std::min((int)(textureWidth*(rect.x + pt.x) / totalArea), (int)textureWidth);
			for (int y = ymin; y < ymax; y++){
				for (int x = xmin; x < xmax; x++){
					img.at<uchar4>(y, x) = rgb;
				}
			}
		}
#endif
		//mosaic.save("C:\\Users\\bclucas\\Desktop\\capture\\test\\mosaics_mapped2.obj");

	}
	float2 MeshTexureMap::packNaive(const std::vector<float2i> &rects, std::multimap<float2i, float2, TextureBoxCompare>& boxes, float area){
		float edgeLength = std::sqrt(area);
		int N = static_cast<int>(rects.size());
		int counter = 0;
		float xOff=0, yOff=0;
		float2 maxPt(0,0);
		for (float2i rect : rects){
			std::pair<float2i, float2> boxPair = std::pair<float2i, float2>(rect, float2(xOff, yOff));
			xOff += rect.x;
			maxPt = std::max(maxPt,float2(boxPair.first.x,boxPair.first.y) + boxPair.second);
			if (xOff > edgeLength){
				xOff = 0;
				yOff = maxPt.y;
			}
			boxes.insert(boxPair);
		}
		return maxPt;
	}
	float MeshTexureMap::packAllNaive(std::vector<float2i>& rects, std::multimap<float2i, float2, TextureBoxCompare>& boxMap){
		double totalArea = 0;
		int N = static_cast<int>(rects.size());
		for (int i = 0; i < N; i++){
			float2i rect = rects[i];
			double a = rect.x*rect.y;
			totalArea += a;
		}
		float edgeLength = (float)std::sqrt(totalArea);
		float2 packedRect = packNaive(rects, boxMap, (float)totalArea);
		edgeLength = std::max(packedRect.x, packedRect.y);
		return edgeLength;
	}
	float MeshTexureMap::pack(std::vector<float2i>& rects, std::multimap<float2i, float2, TextureBoxCompare>& boxMap){
		double totalArea = 0;
		int N = static_cast<int>(rects.size());
		std::vector<std::pair<double, int>> rectSortList(N);
		for (int i = 0; i < N; i++){
			float2i rect = rects[i];
			double a = rect.x*rect.y;
			rectSortList[i] = std::pair<double, int>(-a, i);
			totalArea += a;
		}
		sort(rectSortList.begin(), rectSortList.end());
	
		std::list<std::vector<float2i>> rectBatches;
		std::list<std::vector<int>> rectIdBatches;
		std::list<double> areas;
		
		std::vector<float2i> rectBatch;
		std::vector<int> rectIdBatch;
		
		double area = 0.0;
		double areaThreshold = totalArea*0.5f;
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
		std::list<std::vector<float2i>>::iterator rectIter = rectBatches.begin();
		std::list<std::vector<int>>::iterator rectIdIter = rectIdBatches.begin();
		std::list<double>::iterator areaIter = areas.begin();
		float2i lastRect(float2(0.0f,0.0f),0);
		float2 packedRect(0.0f, 0.0f);
		std::multimap<float2i, float2, TextureBoxCompare> boxes;
		int count = 0;
		for (; rectIter != rectBatches.end();rectIter++,areaIter++,rectIdIter++){
			double area = *areaIter;
			std::cout << "Pack [" << rectIter->size() << " , " << 100.0*area/totalArea <<" %]"<< std::endl;
			boxes.clear();
			std::vector<float2i>& rectList = *rectIter;
			if (boxMap.size() > 0){
				rectList.insert(rectList.begin(),lastRect);
			}
			packedRect = (area<0.01f*totalArea) ? packNaive(rectList, boxes, (float)area) : pack(rectList, boxes, (float)area);
			if (boxMap.size() > 0){
				float2 offset = boxes.find(lastRect)->second;
				for (std::multimap<float2i, float2, TextureBoxCompare>::iterator boxPair = boxMap.begin(); boxPair != boxMap.end(); boxPair++){
					boxPair->second=boxPair->second+offset;				
				}
			}
			for (std::pair<float2i, float2> boxPair : boxes){
				float2i rect = boxPair.first;
				if (rect.x!=lastRect.x||rect.y!=lastRect.y)boxMap.insert(boxPair);
			}
			lastRect = float2i(float2(packedRect.x,packedRect.y),-count);
			count++;
		}
		edgeLength = std::max(packedRect.x, packedRect.y);
		return edgeLength;
	}
	float2 MeshTexureMap::pack(const std::vector<float2i> &rects, std::multimap<float2i, float2, TextureBoxCompare>& boxes, float totalArea){
		
		bool fine = false;
		int count = 0;
		totalArea = 0.0f;
		float edgeLength=0.0f;
		for (float2i rect:rects){
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
		for (std::pair<float2i, float2> boxPair : boxes){
			maxPt = aly::max(maxPt,(boxPair.second + float2(boxPair.first.x, boxPair.first.y)));
		}
		return maxPt;
	}

	bool MeshTexureMap::pack(const std::vector<float2i> &temp, std::multimap<float2i, float2, TextureBoxCompare>& boxes, const float2 &size)
	{
		std::list<float4> freeBoxes;
		freeBoxes.push_back(float4(0, 0, size.x, size.y));
		std::list<float2i> rects(temp.begin(), temp.end());
		
		while (!rects.empty())
		{
			int min = (int)std::max(size.x, size.y);
			std::list<float2i>::iterator minIterRects = rects.end();
			std::list<float4>::iterator minIterFreeBoxes = freeBoxes.end();

			for (auto iterRects = rects.begin(); iterRects != rects.end(); iterRects++)
			{
				for (auto iterFreeBoxes = freeBoxes.begin(); iterFreeBoxes != freeBoxes.end(); iterFreeBoxes++)
				{
					int distance = (int)std::min(iterFreeBoxes->w - iterRects->y, iterFreeBoxes->z - iterRects->x);
					if (distance < 0)
					{
						//std::cout<<"The box "<<iterFreeBoxes->pos<<" with a size "<<iterFreeBoxes->size<<" is too small for "<<*iterRects<<std::endl;
						continue;
					}
					if (distance < min)
					{
						//std::cout<<"The box "<<iterFreeBoxes->pos<<" with a size "<<iterFreeBoxes->size<<" is ok for "<<*iterRects<<std::endl;
						min = distance;
						minIterRects = iterRects;
						minIterFreeBoxes = iterFreeBoxes;
					}
				}
			}

			if (minIterRects == rects.end() || minIterFreeBoxes == freeBoxes.end())
			{
				boxes.clear();
				return false;
			}

			//std::cout<<"I am placing the box "<<*minIterRects<<" into the position "<<minIterFreeBoxes->pos<<std::endl;
			//std::cout<<minIterFreeBoxes->pos<<std::endl;

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
			float2 a4 = float2(insertedBox.x, insertedBox.y + insertedBox.w);

			for (auto iterFreeBoxes = freeBoxes.begin(); iterFreeBoxes != freeBoxes.end();)
			{
				assert(iterFreeBoxes != freeBoxes.end());
				//assert(!(iterFreeBoxes->pos == insertedBox.pos));
				float2 b1 = float2(iterFreeBoxes->x, iterFreeBoxes->y);
				float2 b2 = float2(iterFreeBoxes->x + iterFreeBoxes->z, iterFreeBoxes->y);
				float2 b3 = float2(iterFreeBoxes->x + iterFreeBoxes->z, iterFreeBoxes->y + iterFreeBoxes->w);
				float2 b4 = float2(iterFreeBoxes->x, iterFreeBoxes->y + iterFreeBoxes->w);

				if ((a1.x >= b3.x) || (a1.y >= b3.y) ||
					(a3.x <= b1.x) || (a3.y <= b1.y))
				{
					iterFreeBoxes++;
					continue;   // not intersecting
				}


				if ((a1.x > b1.x) /*  && (a1.x < b3.x) */) // If there is a chance that line a1,a2 is in the shape b
				{
					freeBoxes.push_front(float4(
						iterFreeBoxes->x,
						iterFreeBoxes->y,
						a1.x - b1.x,
						iterFreeBoxes->w));
				}

				if ((a3.x < b3.x) /*  && (a1.x < b3.x) */) // If there is a chance that line a1,a2 is in the shape b
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
					float2 b2 = float2(iterFreeBoxes->x + iterFreeBoxes->z, iterFreeBoxes->y);
					float2 b3 = float2(iterFreeBoxes->x + iterFreeBoxes->z, iterFreeBoxes->y + iterFreeBoxes->w);
					float2 b4 = float2(iterFreeBoxes->x, iterFreeBoxes->y + iterFreeBoxes->w);

					float2 c1 = float2(iterFreeBoxes2->x, iterFreeBoxes2->y);
					float2 c2 = float2(iterFreeBoxes2->x + iterFreeBoxes2->z, iterFreeBoxes2->y);
					float2 c3 = float2(iterFreeBoxes2->x + iterFreeBoxes2->z, iterFreeBoxes2->y + iterFreeBoxes2->w);
					float2 c4 = float2(iterFreeBoxes2->x, iterFreeBoxes2->y + iterFreeBoxes2->w);

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

	void MeshTexureMap::labelComponents(aly::Mesh& mesh, const std::function<bool(const std::string& status, float progress)>& statusHandler){
		const int MIN_PATCH_SIZE = 128;
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
		const float ANGLE_TOL = (float)std::cos(60.0f*(float)ALY_PI / 180.0f);
		for (int iter = 0; iter < FIRST_PASS_MAX_ITERATIONS; iter++){
			int changeCount = 0;
			for (count = 0; count < vertexCount; count++){
				int v = order[count];
				int l = vertexLabels[v];
				float3 norm = normalArray[l];// label corresponds to vertex of seed ... sneaky 
				for (int nbr : vertNbrs[v]){
					if (l<vertexLabels[nbr]){
						if (dot(normalArray[nbr],norm) > ANGLE_TOL){
							vertexLabels[nbr] = l;
							changeCount++;
						}
					}
				}
			}
			std::cout << "Iteration " << iter<<" "<<changeCount << std::endl;
			if (changeCount == 0)break;
			if(iter%4==0) Shuffle(order);
		}

		int cc = 0;
		std::vector<int> relabel;
		int scc = makeLabelsUnique(vertexLabels, relabel, MIN_PATCH_SIZE);
		
		const int SECOND_PASS_MAX_ITERATIONS =vertexCount/16;
		for (int iter = 0; iter < SECOND_PASS_MAX_ITERATIONS; iter++){
			int sz = static_cast<int>(relabel.size());
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
			std::cout << "Iteration " << iter << " " << changeCount << std::endl;
			if (changeCount == 0)break;
		}
		
		//
		scc = splitLabelComponents(vertexLabels);
		scc = makeLabelsUnique(vertexLabels, relabel, 0);

		mosaicIndexes.resize(scc);
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
	
		mosaicIndexes.clear();
		mosaicIndexes.resize(scc);

		for (count = 0; count < faceCount; count++){
			int l = faceLabels[count];
			if (l < scc){
				uint3 tri = faceArray[count];
				mosaicIndexes[l].push_back(int2(tri.x, 3*count));
				mosaicIndexes[l].push_back(int2(tri.y, 3 * count + 1));
				mosaicIndexes[l].push_back(int2(tri.z, 3 * count + 2));
			}
		}

		colors.resize(scc);
		count = 0;
		for (float4& color : colors){
			color = HSVAtoRGBAf(float4(count / (float)scc, 0.7f, 0.7f, 1.0f));
			count++;
		}
		Shuffle(colors);
	}
	int MeshTexureMap::splitLabelComponents(std::vector<int>& vertxLabels){
		std::vector<int> cclist;
		
		int vertexCount = static_cast<int>(vertxLabels.size());
		int pivot = 0;
		int cc = 0;
		std::list<int> queue;
		bool found = false;
		cclist.clear();
		int masked = 0;
		int ccCount = 0;
		int itrStep = vertexCount / 100;

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
	

	float4x4 MeshTexureMap::fitPlane(const aly::Mesh& mesh, std::list<int2>& indexes,float3* deviations,float* scale){
		float3x3 AtA;
		float3 v;
		float3 centerPoint(0.0f);

		int count=0;
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
		float3 zaxis=Q.row(2);
		float3 xaxis=Q.row(0);
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
		float3 minPt = float3(1E30f, 1E30f, 1E30f);
		float3 maxPt= float3(-1E30f, -1E30f, -1E30f);
		for (int2 idx : indexes){
			float3 pt = mesh.vertexLocations[idx.x];
			float3 qt= Transform(PInv,pt);
			minPt = aly::min(minPt,qt);
			maxPt = aly::max(maxPt,qt);
		}
		float3 delta = maxPt-minPt;
		float maxDim = std::max(std::max(delta.x, delta.y), std::max(1E-12f,delta.z));
		*scale = maxDim;
		PInv = MakeScale(float3(1.0f/maxDim))*MakeTranslation(-1.0f*minPt)*PInv;
		return PInv;

	}
	void MeshTexureMap::smooth(aly::Mesh& mesh,  int iters, float errorTolerance){
		MeshListNeighborTable nbrTable;
		CreateOrderedVertexNeighborTable(mesh, nbrTable, true);
		int index = 0;
		std::vector<float> angles;
		std::vector<float> weights;
		float smoothness = 10.0f;
		int N = (int)mesh.vertexLocations.size();
		SparseMatrix1f A(N, N);
		Vector3f b(N);
		for (std::list<uint32_t>& nbrs : nbrTable) {
			int K = (int)nbrs.size() - 1;
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
		SolveBICGStab(b, A, mesh.vertexLocations, iters,errorTolerance);
		mesh.updateVertexNormals();
	}
}