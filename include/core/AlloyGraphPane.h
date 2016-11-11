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
#ifndef ALLOYGRAPHPANE_H_
#define ALLOYGRAPHPANE_H_
#include "AlloyUI.h"
#include "AlloyWidget.h"
namespace aly {
	struct GraphData {
		std::string name;
		Color color;
		std::vector<float2> points;
		static const float NO_INTERSECT;
		GraphData(const std::string& name = "", Color color = Color(200, 64, 64)) :
			name(name), color(color) {

		}
		float interpolate(float x) const;
	};
	typedef std::shared_ptr<GraphData> GraphDataPtr;
	class GraphPane : public Region {
	protected:
		std::vector<GraphDataPtr> curves;
		box2f graphBounds;
		float2 cursorPosition;
		const float GRAPH_PADDING = 24.0f;
	public:
		std::string xAxisLabel;
		std::string yAxisLabel;
		void add(const GraphDataPtr& curve);
		std::shared_ptr<GraphData> add(const GraphData& curve);
		void clear() {
			curves.clear();
		}
		box2f updateGraphBounds();
		void setGraphBounds(const box2f& r) {
			graphBounds = r;
		}
		GraphPane(const std::string& name, const AUnit2D& pos, const AUnit2D& dims);
		virtual void draw(AlloyContext* context) override;
	};
	typedef std::shared_ptr<GraphPane> GraphPanePtr;
}

#endif