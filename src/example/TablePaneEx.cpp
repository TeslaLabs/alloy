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
#include "AlloyTablePane.h"
#include "../../include/example/TablePaneEx.h"
using namespace aly;
TablePaneEx::TablePaneEx() :
		Application(1024, 600, "Table Pane Example") {
}
bool TablePaneEx::init(Composite& rootNode) {
	TablePanePtr tablePane = TablePanePtr(new TablePane("Table Pane", CoordPX(10.0f, 10.0f), CoordPerPX(1.0f, 1.0f, -20.0f, -20.0f),8));
	int R = 100;
	tablePane->setColumnWidth(2, UnitPercent(1 / 16.0f));
	tablePane->setColumnWidth(3, UnitPercent(1 / 16.0f));
	tablePane->setColumnWidth(5, UnitPercent(3 / 16.0f));
	tablePane->setColumnWidth(6, UnitPercent(1 / 16.0f));
	tablePane->setColumnWidth(7, UnitPercent(1 / 4.0f));
	tablePane->setColumnLabel(0, "String");
	tablePane->setColumnLabel(1, "Float");
	tablePane->setColumnLabel(2, "Int");
	tablePane->setColumnLabel(3, "Check");
	tablePane->setColumnLabel(4, "Toggle");
	tablePane->setColumnLabel(5, "Selection");
	tablePane->setColumnLabel(6, "Color");
	tablePane->setColumnLabel(7, "Progress");
	for (int r = 0;r < R;r++) {
		TableRowPtr row = tablePane->addRow();
		row->setColumn(0,TableStringEntryPtr(new TableStringEntry("String", MakeString()<<"Row "<<std::setw(2)<<std::setfill('0')<<(r+1), false)));
		row->setColumn(1, TableNumberEntryPtr(new TableNumberEntry("Float", Float(RandomUniform(0.0f,10.0f)),true)));
		row->setColumn(2, TableNumberEntryPtr(new TableNumberEntry("Integer", Integer(RandomUniform(0,100)), true)));
		row->setColumn(3, TableCheckBoxEntryPtr(new TableCheckBoxEntry("Check",(RandomUniform(0,1)!=0))));
		row->setColumn(4, TableToggleBoxEntryPtr(new TableToggleBoxEntry("Toggle", (RandomUniform(0, 1) != 0))));
		row->setColumn(5, TableSelectionEntryPtr(new TableSelectionEntry("Selection", std::vector<std::string>{"car","plane","boat"},RandomUniform(0,2))));
		row->setColumn(6, TableColorEntryPtr(new TableColorEntry("Color",HSVtoColor(HSV(RandomUniform(0.0f,1.0f),1.0f,RandomUniform(0.5f,1.0f))))));
		row->setColumn(7, TableProgressBarEntryPtr(new TableProgressBarEntry("Progress", RandomUniform(0.0f, 1.0f))));
	}
	tablePane->setEnableMultiSelection(true);
	rootNode.add(tablePane);
	return true;
}

