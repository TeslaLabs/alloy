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

//Based on avoidance routing algorithm in Java Image Science Toolkit.
//Originally written by Blake Lucas at Johns Hopkins University.
//https://www.nitrc.org/plugins/scmcvs/cvsweb.php/JIST/src/edu/jhu/ece/iacl/jist/pipeline/graph/AvoidanceRouting.java?rev=1.5;content-type=text%2Fx-cvsweb-markup;cvsroot=jist


#ifndef INCLUDE_AVOIDANCEROUTING_H_
#define INCLUDE_AVOIDANCEROUTING_H_
#include "AlloyMath.h"
#include "AlloyVector.h"
#include "AlloyUI.h"
#include <vector>
#include <memory>
#include <queue>
namespace aly {
	namespace dataflow {
		class Connection;
		class Node;
		enum class Direction { Unkown, North, South, East, West };
		template<class C, class R> std::basic_ostream<C, R> & operator <<(
			std::basic_ostream<C, R> & ss, const Direction& type) {
			switch (type) {
			case Direction::Unkown:
				return ss << "Unkown";
			case Direction::North:
				return ss << "North";
			case Direction::South:
				return ss << "South";
			case Direction::East:
				return ss << "East";
			case Direction::West:
				return ss << "West";
			}
			return ss;
		}
		struct AvoidancePath {
			float borderSpace=10.0f;
			Direction direction = Direction::Unkown;
			float distToDest;
			float pathLength;
			int depth;
			std::vector<box2px>& obstacles;
			std::vector<std::shared_ptr<AvoidancePath>> children;
			AvoidancePath* parent;
			lineseg2f path;
			box2f obstacle;
			AvoidancePath(std::vector<box2px>& obstacles, const float2& from, const float2& to, Direction direction, AvoidancePath* parent = nullptr);
			void addChild(const std::shared_ptr<AvoidancePath>& child);
			float2 backTrack(std::vector<float2>& pointList);
			std::vector<float2> backTrack();
			float updatePathLength();
			bool createChildren(const float2& to);
			void createDescendants(std::vector<std::shared_ptr<AvoidancePath>>& ret, const float2& to, int depth);
			float2 findNextBoundary(const float2 &point);
			void getDescendants(std::vector<std::shared_ptr<AvoidancePath>>& ret, int depth);
			float getDistanceToDestination() const;
			float getPathLength() const;
			float lineLength();
			float updateDistToDestination(float2 target);
			lineseg2f updatePath( const float2& to);
		};
		template<class C, class R> std::basic_ostream<C, R> & operator <<(
			std::basic_ostream<C, R> & ss, const AvoidancePath& apath) {
			if ((apath.parent != nullptr) && (apath.parent->obstacle.dimensions.x*apath.parent->obstacle.dimensions.y > 0)) {
				float2 min = apath.parent->obstacle.min();
				float2 max = apath.parent->obstacle.max();
				return ss << "Path ["<<apath.depth<<"] "<< apath.direction << " " << apath.path << " " << min << ":" << max<<" "<<apath.getDistanceToDestination()<<" "<<apath.getPathLength();
			}
			else {
				return ss << "Path [" << apath.depth << "] " << apath.direction << " " << apath.path << " " << apath.getDistanceToDestination() << " " << apath.getPathLength();
			}
		}
		struct ComparePaths{
			bool operator()(const std::shared_ptr<AvoidancePath>& a, const std::shared_ptr<AvoidancePath>& b);
		};
		class AvoidanceRouting {
		protected:
			float borderSpace=10.0f;
			static const int DEPTH_LIMIT = 4;
			static const int MAX_PATHS = 128;
			std::vector<box2px> obstacles;
			box2px getPathBounds(float2 from, float2 to) const;
			void simplifyPath(std::vector<float2>& path,int parity);
		public:
			std::vector<std::shared_ptr<Node>> nodes;
			void update();
			const std::vector<box2px>& getObstacles() const {
				return obstacles;
			}
			void setBorderSpacing(float b){
				borderSpace=b;
			}
			void evaluate( const std::shared_ptr<Connection>& edge);
			void evaluate(std::vector<float2>& path, float2 from, float2 to, Direction direction);
			void add(const std::shared_ptr<Node>& node) {
				nodes.push_back(node);
			}
			void erase(const std::shared_ptr<Node>& node);
			void erase(const std::list<std::shared_ptr<Node>>& node);
			void clear() {
				nodes.clear();
			}
		};
	}
}
#endif
