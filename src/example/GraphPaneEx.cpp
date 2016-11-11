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
#include "AlloyGraphPane.h"
#include "../../include/example/GraphPaneEx.h"
using namespace aly;
GraphPaneEx::GraphPaneEx() :
		Application(800, 600, "GraphPane Example") {


}
bool GraphPaneEx::init(Composite& rootNode) {
	int N = 5;
	float delta = 0.001f;

	GraphPanePtr graphRegion = GraphPanePtr(new GraphPane("GraphPane Example", CoordPerPX(0.5f, 0.0f, -300.0f, 10.0f), CoordPX(600, 400)));
	for (int n = 0;n < N;n++) {
		float startX = RandomUniform(-1.0f, 1.0f);
		float endX = RandomUniform(3.0f, 4.0f);
		float freq = RandomUniform(0.25f, 1.0f);
		float amp = RandomUniform(0.25f, 1.0f);
		float yOffset = RandomUniform(-0.5f, 0.5f);
		float phase = RandomUniform(0.0f, ALY_PI);
		GraphDataPtr graph = GraphDataPtr(new GraphData(MakeString() << "Curve " << n + 1, HSVtoColor(HSV(n / (float)(N - 1), 0.7f, 0.8f))));
		for (float x = startX;x <= endX;x += delta) {
			float y = amp*std::sin(2.0f*ALY_PI*freq*x + phase)+ yOffset;
			graph->points.push_back(float2(x, y));
		}
		graphRegion->add(graph);
	}
	rootNode.add(graphRegion);
	return true;
}

