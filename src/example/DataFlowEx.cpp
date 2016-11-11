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
#include "../../include/example/DataFlowEx.h"
using namespace aly;
using namespace aly::dataflow;
DataFlowEx::DataFlowEx() :
	Application(1080, 1080, "Data Flow GraphPane Example") {
}
bool DataFlowEx::init(Composite& rootNode) {
	graph = MakeDataFlow("Data Flow", CoordPX(10, 10),
		CoordPerPX(1.0f, 1.0f, -20.0f, -20.0f));
	ComputePtr computeNode1 = MakeComputeNode("Compute 1", pixel2(200, 600));
	ComputePtr computeNode2 = MakeComputeNode("Compute 2", pixel2(600, 600));
	ComputePtr computeNode3 = MakeComputeNode("Compute 3", pixel2(100, 700));
	ComputePtr computeNode4 = MakeComputeNode("Compute 4", pixel2(300, 700));
	ComputePtr computeNode5 = MakeComputeNode("Compute 5", pixel2(500, 700));
	ComputePtr computeNode6 = MakeComputeNode("Compute 6", pixel2(700, 700));
	ComputePtr computeNode7 = MakeComputeNode("Compute 7", pixel2(400, 800));

	ViewPtr viewNode1 = MakeViewNode("View 1", pixel2(200, 800));
	ViewPtr viewNode2 = MakeViewNode("View 2", pixel2(600, 800));

	DataPtr dataNode1 = MakeDataNode("Data 1", pixel2(100, 300));
	DataPtr dataNode2 = MakeDataNode("Data 2", pixel2(300, 300));
	DataPtr dataNode3 = MakeDataNode("Data 3", pixel2(300, 500));
	DataPtr dataNode4 = MakeDataNode("Data 4", pixel2(100, 500));

	SourcePtr sourceNode1 = MakeSourceNode("Source 1", pixel2(100, 100));
	SourcePtr sourceNode2 = MakeSourceNode("Source 2", pixel2(200, 200));
	SourcePtr sourceNode3 = MakeSourceNode("Source 3", pixel2(300, 200));


	GroupPtr groupNode1 = MakeGroupNode("Group 1", pixel2(600, 400));
	GroupPtr groupNode2 = MakeGroupNode("Group 2", pixel2(800, 400));

	
	DestinationPtr destNode1 = MakeDestinationNode("Destination 1",
		pixel2(350, 900));
	DestinationPtr destNode2 = MakeDestinationNode("Destination 2",
		pixel2(450, 900));
	for (int i = 0; i <2; i++) {
		groupNode1->add(MakeInputPort(MakeString() << "Input " << i));
		groupNode2->add(MakeInputPort(MakeString() << "Input " << i));
	}
	for (int i = 0; i <2; i++) {
		groupNode1->add(MakeOutputPort(MakeString() << "Output " << i));
		groupNode2->add(MakeOutputPort(MakeString() << "Output " << i));
	}
	for (int i = 0; i < 4; i++) {
		dataNode1->add(MakeInputPort(MakeString() << "Input " << i));
		dataNode2->add(MakeInputPort(MakeString() << "Input " << i));
		dataNode3->add(MakeInputPort(MakeString() << "Input " << i));
		dataNode4->add(MakeInputPort(MakeString() << "Input " << i));

	}
	for (int i = 0; i < 3; i++) {
		dataNode1->add(MakeOutputPort(MakeString() << "Output " << i));
		dataNode2->add(MakeOutputPort(MakeString() << "Output " << i));
		dataNode3->add(MakeOutputPort(MakeString() << "Output " << i));
		dataNode4->add(MakeOutputPort(MakeString() << "Output " << i));

	}
	for (int i = 0; i < 7; i++) {
		computeNode1->add(MakeInputPort(MakeString() << "Input " << i));
		computeNode2->add(MakeInputPort(MakeString() << "Input " << i));
		computeNode3->add(MakeInputPort(MakeString() << "Input " << i));
		computeNode4->add(MakeInputPort(MakeString() << "Input " << i));
		computeNode5->add(MakeInputPort(MakeString() << "Input " << i));
		computeNode6->add(MakeInputPort(MakeString() << "Input " << i));
		computeNode7->add(MakeInputPort(MakeString() << "Input " << i));
	}
	for (int i = 0; i < 2; i++) {
		computeNode1->add(MakeOutputPort(MakeString() << "Output " << i));
		computeNode2->add(MakeOutputPort(MakeString() << "Output " << i));
		computeNode3->add(MakeOutputPort(MakeString() << "Output " << i));
		computeNode4->add(MakeOutputPort(MakeString() << "Output " << i));
		computeNode5->add(MakeOutputPort(MakeString() << "Output " << i));
		computeNode6->add(MakeOutputPort(MakeString() << "Output " << i));
		computeNode7->add(MakeOutputPort(MakeString() << "Output " << i));
	}

	for (int i = 0; i < 3; i++) {
		viewNode1->add(MakeInputPort(MakeString() << "Input " << i));
		viewNode2->add(MakeInputPort(MakeString() << "Input " << i));
	}
	graph->add(computeNode1);
	graph->add(computeNode2);
	graph->add(computeNode3);
	graph->add(computeNode4);
	graph->add(computeNode5);
	graph->add(computeNode6);
	graph->add(computeNode7);
	
	graph->add(viewNode1);
	graph->add(viewNode2);

	graph->add(groupNode1);
	graph->add(groupNode2);

	graph->add(dataNode1);
	graph->add(dataNode2);
	graph->add(dataNode3);
	graph->add(dataNode4);

	graph->add(MakeConnection(groupNode1->getOutputPort(0), computeNode2->getInputPort(3)));
	graph->add(MakeConnection(groupNode2->getOutputPort(1), computeNode2->getInputPort(4)));

	graph->add(MakeConnection(computeNode1->getOutputPort(0), computeNode3->getInputPort(1)));
	graph->add(MakeConnection(computeNode1->getOutputPort(1), computeNode4->getInputPort(2)));
	graph->add(MakeConnection(computeNode2->getOutputPort(0), computeNode5->getInputPort(1)));
	graph->add(MakeConnection(computeNode2->getOutputPort(0), computeNode6->getInputPort(0)));
	graph->add(MakeConnection(computeNode4->getOutputPort(0), computeNode7->getInputPort(0)));
	graph->add(MakeConnection(computeNode5->getOutputPort(1), computeNode7->getInputPort(1)));

	graph->add(MakeConnection(computeNode3->getOutputPort(1), viewNode1->getInputPort(1)));
	graph->add(MakeConnection(computeNode6->getOutputPort(1), viewNode2->getInputPort(1)));

	graph->add(MakeConnection(computeNode7->getOutputPort(0), destNode1->getInputPort()));
	graph->add(MakeConnection(computeNode7->getOutputPort(1), destNode2->getInputPort()));

	graph->add(MakeConnection(sourceNode1->getOutputPort(), dataNode1->getInputPort(0)));
	graph->add(MakeConnection(sourceNode2->getOutputPort(), computeNode1->getInputPort(2)));
	graph->add(MakeConnection(sourceNode3->getOutputPort(), computeNode2->getInputPort(2)));
	graph->add(MakeConnection(dataNode2->getOutputPort(0), computeNode2->getInputPort(0)));
	graph->add(MakeConnection(dataNode3->getOutputPort(1), computeNode2->getInputPort(1)));
	graph->add(MakeConnection(dataNode3->getOutputPort(1), computeNode1->getInputPort(3)));

	graph->add(MakeConnection(dataNode4->getOutputPort(1), computeNode3->getInputPort(0)));
	graph->add(MakeConnection(dataNode1->getOutputPort(0), computeNode5->getInputPort(1)));

	graph->add(MakeRelationship(dataNode1, "uses", dataNode2));
	graph->add(MakeRelationship(dataNode1, "uses", dataNode3));
	graph->add(MakeRelationship(dataNode2, "extends", dataNode3));
	graph->add(MakeRelationship(dataNode4, "extends", dataNode2));
	graph->add(MakeRelationship(dataNode3, "has", dataNode4));

	graph->add(sourceNode1);
	graph->add(sourceNode2);
	graph->add(sourceNode3);
	graph->add(MakeConnection(sourceNode1->getChildPort(), sourceNode2->getParentPort()));
	graph->add(MakeConnection(sourceNode1->getChildPort(), sourceNode3->getParentPort()));

	graph->add(destNode1);
	graph->add(destNode2);
	rootNode.add(graph);
	graph->start();
	return true;
}

