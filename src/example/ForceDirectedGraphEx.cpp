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
#include "AlloyExpandBar.h"
#include "../../include/example/ForceDirectedGraphEx.h"
using namespace aly;
using namespace aly::dataflow;
ForceDirectedGraphEx::ForceDirectedGraphEx() :
		Application(1920,1080, "Force Directed GraphPane Example") {

}
void ForceDirectedGraphEx::createDescendantGraph(
		const aly::dataflow::ForceSimulatorPtr& graph,
		const aly::dataflow::ForceItemPtr& node) {
	std::vector<ForceItemPtr> childNodes;
	int D=3;
	int N=2;
	childNodes.push_back(node);
	for (int d = 0; d < D; d++) {
		std::vector<ForceItemPtr> tmpList;
		for (ForceItemPtr parent : childNodes) {
			for (int n = 0; n < N; n++) {
				ForceItemPtr child=ForceItemPtr(new ForceItem());
				child->location=parent->location+float2((n*(float)pow(N, D-1-d))*ForceSimulator::RADIUS*2.0f - (float)pow(N, D - 1 - d)*ForceSimulator::RADIUS,4*ForceSimulator::RADIUS);
				child->color=Compute::COLOR.toRGBAf();
				child->shape=NodeShape::Hexagon;
				child->buoyancy=1.0f;
				SpringItemPtr spring=graph->addSpringItem(parent, child);
				//Add oriented springs to preserve shape of tree
				spring->length = distance(parent->location, child->location);
				spring->gamma=0.1f;
				graph->addForceItem(child);
				tmpList.push_back(child);
			}
			for (int n = 1; n < (int)tmpList.size(); n++) {
				//Add oriented springs between children to preserve shape of tree
				ForceItemPtr item1 = tmpList[n - 1];
				ForceItemPtr item2 = tmpList[n];
				SpringItemPtr spring = graph->addSpringItem(item1,item2);
				spring->length = distance(item1->location,item2->location);
				spring->gamma = 0.1f;
				spring->visible = false;
			}
		}
		childNodes = tmpList;
	}
}
void ForceDirectedGraphEx::createAncestorGraph(
		const aly::dataflow::ForceSimulatorPtr& graph,
		const aly::dataflow::ForceItemPtr& node) {
	std::vector<ForceItemPtr> childNodes;
	int D=3;
	int N=2;
	childNodes.push_back(node);
	for (int d = 0; d < D; d++) {
		std::vector<ForceItemPtr> tmpList;
		for (ForceItemPtr parent : childNodes) {
			for (int n = 0; n < N; n++) {
				ForceItemPtr child=ForceItemPtr(new ForceItem());
				child->location = parent->location + float2((n*(float)pow(N, D - 1 - d))*ForceSimulator::RADIUS*2.0f - (float)pow(N, D - 1 - d)*ForceSimulator::RADIUS, -4 * ForceSimulator::RADIUS);
				child->color=Source::COLOR.toRGBAf();
				child->shape=NodeShape::Square;
				child->buoyancy=-1.0f;
				SpringItemPtr item=graph->addSpringItem(parent, child);
				item->direction = 0.1f*normalize(child->location - parent->location);
				graph->addForceItem(child);
				tmpList.push_back(child);
			}
		}
		childNodes = tmpList;
	}
}

void ForceDirectedGraphEx::createRadialGraph(const ForceSimulatorPtr& graph) {
	int D = 3;
	int N = 6;
	float armLength = 500.0f;
	float2 center(1000.0f, 1000.0f);
	std::vector<ForceItemPtr> childNodes;
	ForceItemPtr child = ForceItemPtr(new ForceItem(center));
	child->buoyancy = 0;
	child->color=Data::COLOR.toRGBAf();
	child->shape = NodeShape::Circle;
	childNodes.push_back(child);
	graph->addForceItem(childNodes.front());
	for (int d = 0; d < D; d++) {
		std::vector<ForceItemPtr> tmpList;
		for (ForceItemPtr parent : childNodes) {
			for (int n = 0; n < N; n++) {
				float2 pt = parent->location
						+ (armLength * std::pow(0.3f, (float) d))
								* float2(
										std::cos(n * ALY_PI * 2.0f / (float) N),
										std::sin(
												n * ALY_PI * 2.0f / (float) N));
				child = ForceItemPtr(new ForceItem(pt));
				child->buoyancy = 0;
				child->shape = NodeShape::Circle;
				child->color=Data::COLOR.toRGBAf();
				graph->addForceItem(child);
				SpringItemPtr item=graph->addSpringItem(parent, child);
				tmpList.push_back(child);
			}
		}
		childNodes = tmpList;
	}
	
	std::sort(childNodes.begin(),childNodes.end(),[=](const ForceItemPtr& a,const ForceItemPtr& b){
		return (a->location.y>b->location.y);
	});
	for(int k=0;k<2;k++){
		createDescendantGraph(graph,childNodes[k]);
	}
	for(int k=(int)childNodes.size()-1;k>=(int)childNodes.size()-4;k--){
		createAncestorGraph(graph,childNodes[k]);
	}
	
}
bool ForceDirectedGraphEx::init(Composite& rootNode) {

	ExpandBarPtr controlRegion = ExpandBarPtr(
			new ExpandBar("Controls", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f)));
	controlRegion->setScrollEnabled(true);
	controlRegion->setOrientation(Orientation::Vertical);
	controlRegion->backgroundColor = MakeColor(32, 32, 32);
	BorderCompositePtr borderRegion = BorderCompositePtr(
			new BorderComposite("Layout", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0f, 1.0f),false));
	borderRegion->setWest(controlRegion, UnitPX(300.0f));
	graph = ForceSimulatorPtr(
			new ForceSimulator("Force Simulator", CoordPX(0.0f, 0.0f),
					CoordPercent(1.0, 1.0)));
	borderRegion->setCenter(graph);
	createRadialGraph(graph);
	graph->backgroundColor = MakeColor(64, 64, 64);
	graph->addForce(NBodyForcePtr(new NBodyForce(-0.7f)));
	graph->addForce(SpringForcePtr(new SpringForce()));
	graph->addForce(DragForcePtr(new DragForce(0.001f)));
	//graph->addForce(GravitationalForcePtr(new GravitationalForce()));
	graph->addForce(BuoyancyForcePtr(new BuoyancyForce()));
	graph->addForce(
			BoxForcePtr(
					new BoxForce(
							box2f(float2(0.0f, 0.0f),
									float2(2000.0f, 2000.0f)))));
	graph->addForce(
			CircularWallForcePtr(
					new CircularWallForce(float2(1000.0f, 1000.0f), 1000.0f)));

	bool firstTime = true;
	for (ForcePtr f : graph->getForces()) {
		int N = (int) f->getParameterCount();
		CompositePtr paramRegion = MakeComposite(f->getName(),
				CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f));
		paramRegion->setOrientation(Orientation::Vertical, pixel2(5.0f),
				pixel2(5.0f));
		ToggleBoxPtr enabledToggle = ToggleBoxPtr(
				new ToggleBox("Force Enabled", CoordPX(5.0f, 5.0f),
						CoordPerPX(1.0f, 0.0f, -10.0f, 25.0f), true,true));
		enabledToggle->onChange = [=](bool b) {
			f->setEnabled(b);
		};
		paramRegion->add(enabledToggle);
		for (int n = 0; n < N; n++) {
			HorizontalSliderPtr hslider = HorizontalSliderPtr(
					new HorizontalSlider(f->getParameterName(n),
							CoordPX(5.0f, 5.0f),
							CoordPerPX(1.0f, 0.0f, -10.0f, 40.0f),
						true,
							Float(f->getMinValue(n)), Float(f->getMaxValue(n)),
							Float(f->getParameterValue(n))));
			paramRegion->add(hslider);
			hslider->setOnChangeEvent([=](const Number& val) {
				f->setParameter(n,val.toFloat());
			});
		}
		controlRegion->addRegion(paramRegion, 30.0f + N * (40.0f + 5.0f) + 5.0f,
				firstTime);
		firstTime = false;
	}
	rootNode.add(borderRegion);
	addListener(graph.get());
	graph->optimize(0.5f,256);
	graph->fit();
	graph->start();
	return true;
}

