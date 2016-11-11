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
#include "../../include/example/MenuEx.h"
using namespace aly;
MenuEx::MenuEx() :
		Application(800, 600, "Menu Example") {
}
bool MenuEx::init(Composite& rootNode) {
	MenuBarPtr menuBar = MenuBarPtr(new MenuBar("Menu Bar",CoordPX(0,0),CoordPercent(1.0f,1.0f)));
	MenuPtr fileMenu = MenuPtr(new Menu("File"));
	MenuPtr editMenu = MenuPtr(new Menu("Edit"));
	MenuPtr actionMenu = MenuPtr(new Menu("Action"));
	MenuPtr helpMenu = MenuPtr(new Menu("Help"));
	MenuPtr newMenu = MenuPtr(new Menu("New"));
	newMenu->addItem("House")->onSelect = [=]() {std::cout << "New:House" << std::endl;};
	newMenu->addItem("Car")->onSelect = [=]() {std::cout << "New:Car" << std::endl;};
	newMenu->addItem("Bike")->onSelect = [=]() {std::cout << "New:Bike" << std::endl;};

	MenuPtr importMenu = MenuPtr(new Menu("Import"));
	importMenu->addItem("Coffee")->onSelect = [=] {std::cout << "Import:Coffee" << std::endl;};
	importMenu->addItem("Booze")->onSelect = [=] {std::cout << "Import:Booze" << std::endl;};
	importMenu->addItem("Umbrellas")->onSelect = [=] {std::cout << "Import:Umbrellas" << std::endl;};

	fileMenu->addItem(newMenu);
	fileMenu->addItem("Open")->onSelect = [=]() {std::cout << "File:Open" << std::endl;};
	fileMenu->addItem("Save")->onSelect = [=]() {std::cout << "File:Save" << std::endl;};
	fileMenu->addItem(importMenu);
	fileMenu->addItem("Close")->onSelect = [=]() {std::cout << "File:Close" << std::endl;};
	fileMenu->addItem("Exit")->onSelect = [=]() {std::cout << "File:Exit" << std::endl;};

	editMenu->addItem("Cut")->onSelect = [=]() {std::cout << "Edit:Cut" << std::endl;};
	editMenu->addItem("Copy")->onSelect = [=]() {std::cout << "Edit:Copy" << std::endl;};
	editMenu->addItem("Paste")->onSelect = [=]() {std::cout << "Edit:Paste" << std::endl;};
	editMenu->addItem("Undo")->onSelect = [=]() {std::cout << "Edit:Undo" << std::endl;};
	editMenu->addItem("Redo")->onSelect = [=]() {std::cout << "Edit:Redo" << std::endl;};

	actionMenu->addItem("Eat")->onSelect = [=]() {std::cout << "Action:Eat" << std::endl;};
	actionMenu->addItem("Sleep")->onSelect = [=]() {std::cout << "Action:Sleep" << std::endl;};
	actionMenu->addItem("Run")->onSelect = [=]() {std::cout << "Action:Run" << std::endl;};
	actionMenu->addItem("Jump")->onSelect = [=]() {std::cout << "Action:Jump" << std::endl;};
	actionMenu->addItem("Squat")->onSelect = [=]() {std::cout << "Action:Squat" << std::endl;};


	MenuPtr bigMenu = MenuPtr(new Menu("Big Menu"));
	MenuPtr subMenu1 = MenuPtr(new Menu("Submenu 1"));
	subMenu1->addItem("Item 1")->onSelect = [=]() {std::cout << "subMenu1:Item 1" << std::endl;};
	subMenu1->addItem("Item 2")->onSelect = [=]() {std::cout << "subMenu1:Item 2" << std::endl;};
	subMenu1->addItem("Item 3")->onSelect = [=]() {std::cout << "subMenu1:Item 3" << std::endl;};

	MenuPtr subsubMenu1 = MenuPtr(new Menu("Subsubmenu 1"));
	subsubMenu1->addItem("Item 1")->onSelect = [=]() {std::cout << "subsubMenu1:Item 1" << std::endl;};
	subsubMenu1->addItem("Item 2")->onSelect = [=]() {std::cout << "subsubMenu1:Item 2" << std::endl;};
	subsubMenu1->addItem("Item 3")->onSelect = [=]() {std::cout << "subsubMenu1:Item 3" << std::endl;};

	MenuPtr subsubMenu2 = MenuPtr(new Menu("Subsubmenu 2"));
	subsubMenu2->addItem("Item 1")->onSelect = [=]() {std::cout << "subsubMenu2:Item 1" << std::endl;};
	subsubMenu2->addItem("Item 2")->onSelect = [=]() {std::cout << "subsubMenu2:Item 2" << std::endl;};
	subsubMenu2->addItem("Item 3")->onSelect = [=]() {std::cout << "subsubMenu2:Item 3" << std::endl;};

	MenuPtr subsubMenu3 = MenuPtr(new Menu("Subsubmenu 3"));
	subsubMenu3->addItem("Item 1")->onSelect = [=]() {std::cout << "subsubMenu3:Item 1" << std::endl;};
	subsubMenu3->addItem("Item 2")->onSelect = [=]() {std::cout << "subsubMenu3:Item 2" << std::endl;};
	subsubMenu3->addItem("Item 3")->onSelect = [=]() {std::cout << "subsubMenu3:Item 3" << std::endl;};

	MenuPtr subMenu2 = MenuPtr(new Menu("Submenu 2"));
	subMenu2->addItem("Item 1")->onSelect = [=]() {std::cout << "subMenu2:Item 1" << std::endl;};
	subMenu2->addItem("Item 2")->onSelect = [=]() {std::cout << "subMenu2:Item 2" << std::endl;};
	subMenu2->addItem(subsubMenu1);
	subMenu2->addItem(subsubMenu2);
	subMenu2->addItem(subsubMenu3);
	subMenu2->addItem("Item 3")->onSelect = [=]() {std::cout << "subMenu2:Item 3" << std::endl;};

	MenuPtr subMenu3 = MenuPtr(new Menu("Submenu 3"));
	subMenu3->addItem("Item 1")->onSelect = [=]() {std::cout << "subMenu3:Item 1" << std::endl;};
	subMenu3->addItem("Item 2")->onSelect = [=]() {std::cout << "subMenu3:Item 2" << std::endl;};
	subMenu3->addItem("Item 3")->onSelect = [=]() {std::cout << "subMenu3:Item 3" << std::endl;};
	
	bigMenu->addItem("Item 1")->onSelect = [=]() {std::cout << "bigMenu:Item 1" << std::endl;};
	bigMenu->addItem("Item 2")->onSelect = [=]() {std::cout << "bigMenu:Item 2" << std::endl;};
	bigMenu->addItem(subMenu1);
	bigMenu->addItem(subMenu2);
	bigMenu->addItem(subMenu3);
	bigMenu->addItem("Item 3")->onSelect = [=]() {std::cout << "bigMenu:Item 3" << std::endl;};
	bigMenu->addItem("Item 4")->onSelect = [=]() {std::cout << "bigMenu:Item 4" << std::endl;};

	helpMenu->addItem("How did I get here?")->onSelect = [=](){std::cout << "Help:How did I get here?" << std::endl;};
	helpMenu->addItem("Where am I going?")->onSelect = [=]() {std::cout << "Help:Where am I going?" << std::endl;};
	helpMenu->addItem("What day is it?")->onSelect = [=]() {std::cout << "Help:What day is it?" << std::endl;};

	menuBar->addMenu(fileMenu);
	menuBar->addMenu(editMenu);
	menuBar->addMenu(actionMenu);
	menuBar->addMenu(bigMenu);
	menuBar->addMenu(helpMenu);

	CompositePtr north = MakeComposite("North Composite", CoordPX(0, 0), CoordPercent(1.0, 1.0), Color(128, 16, 32), COLOR_NONE, UnitPX(0));
	north->add(menuBar);
	TextLabelPtr label;
	CompositePtr center = MakeComposite("Center Composite", CoordPX(0, 0),CoordPercent(1.0, 1.0), Color(16, 128, 128));
	center->add(label = MakeTextLabel("Center", CoordPercent(0.5f, 0.5f), CoordPX(100, 30), FontType::Bold, UnitPT(16.0f), COLOR_WHITE, HorizontalAlignment::Center, VerticalAlignment::Middle));
	label->setOrigin(Origin::MiddleCenter);
	BorderCompositePtr bcomp = std::shared_ptr<BorderComposite>(new BorderComposite("Border Layout", CoordPX(0, 0),CoordPercent(1.0f, 1.0f)));
	bcomp->setNorth(north,UnitPX(30.0f));
	bcomp->setCenter(center);
	rootNode.add(bcomp);
	return true;
}

