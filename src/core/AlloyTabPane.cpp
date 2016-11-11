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

#include "AlloyTabPane.h"
#include "AlloyApplication.h"
#include "AlloyDrawUtil.h"
namespace aly {
	const float TabBar::TAB_HEIGHT = 30.0f;
	const float TabBar::TAB_SPACING = 10.0f;
	pixel2 TabHeader::getPreferredDimensions(AlloyContext* context) const {
		return textLabel->getTextDimensions(context) + pixel2((closeable)?TabBar::TAB_HEIGHT + 4:8.0f, TabBar::TAB_HEIGHT);
	}
	TabHeader::TabHeader(const std::string& name, bool closeable,TabPane* parent) :
		Composite(name, CoordPX(0, 0), CoordPX(120, 30)), parentPane(parent), focused(
			false),closeable(closeable){
		backgroundColor = MakeColor(COLOR_NONE);
		textLabel = TextLabelPtr(
			new TextLabel(name, CoordPX(2, 2),
				CoordPerPX(1.0f, 1.0f, (closeable)?-TabBar::TAB_HEIGHT:-4.0f, -4.0f)));
		textLabel->fontSize = UnitPX(20.0f);
		textLabel->fontType = FontType::Bold;
		textLabel->setRoundCorners(false);
		textLabel->borderColor=MakeColor(COLOR_NONE);
		textLabel->backgroundColor=MakeColor(COLOR_NONE);
		textLabel->textColor = MakeColor(
			AlloyApplicationContext()->theme.LIGHTER);
		textLabel->setAlignment(HorizontalAlignment::Left,
			VerticalAlignment::Middle);
		add(textLabel);
		textLabel->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
			if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
				setSelected();
				return false;
			}
			return false;
		};

		if(closeable){
			IconButtonPtr closeButton = std::shared_ptr<IconButton>(
				new IconButton(0xf00d, CoordPerPX(1.0, 0.0, -TabBar::TAB_HEIGHT + 4.0f, 4.0f),
					CoordPX(22, 22), IconType::CIRCLE));
			closeButton->borderWidth = UnitPX(0.0f);
			closeButton->backgroundColor = MakeColor(0, 0, 0, 0);
			closeButton->foregroundColor = MakeColor(0, 0, 0, 0);
			setClampDragToParentBounds(true);
			add(closeButton);
			closeButton->onMouseDown =
				[this](AlloyContext* context, const InputEvent& event) {
				if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
					parentPane->parent->close(parentPane);
					context->requestPack();
					return true;
				}
				return false;
			};
		}

	}
	void TabHeader::setSelected() const {
		getBar()->setSelected(parentPane);
	}

	bool TabHeader::isSelected() const {
		return getBar()->isSelected(parentPane);
	}
	void TabHeader::draw(AlloyContext* context) {
		focused = context->isMouseOver(textLabel.get());
		Color bgColor;
		if (isSelected()) {
			bgColor = AlloyApplicationContext()->theme.DARK.toLighter(0.5f);
		}
		else {
			bgColor = AlloyApplicationContext()->theme.DARK;

		}
		if (focused) {
			textLabel->textColor = MakeColor(
				AlloyApplicationContext()->theme.LIGHTEST);
		}
		else {
			textLabel->textColor = MakeColor(
				AlloyApplicationContext()->theme.LIGHTER);
		}
		NVGcontext* nvg = context->nvgContext;
		if (isScrollEnabled()) {
			pushScissor(nvg, getCursorBounds());
		}
		nvgFillColor(nvg, bgColor);
		nvgStrokeColor(nvg, bgColor.toDarker(0.5f));
		nvgBeginPath(nvg);
		box2px bounds = getBounds();
		nvgStrokeWidth(nvg, 2.0f);
		nvgMoveTo(nvg, bounds.position.x, bounds.position.y + 1);
		nvgLineTo(nvg, bounds.position.x + bounds.dimensions.x, bounds.position.y + 1);
		nvgLineTo(nvg, bounds.position.x + bounds.dimensions.x + TabBar::TAB_SPACING, bounds.position.y + bounds.dimensions.y);
		nvgLineTo(nvg, bounds.position.x - TabBar::TAB_SPACING, bounds.position.y + bounds.dimensions.y);
		nvgClosePath(nvg);
		nvgFill(nvg);
		nvgStroke(nvg);
		for (std::shared_ptr<Region>& region : children) {
			if (region->isVisible()) {
				region->draw(context);
			}
		}
		if (isScrollEnabled()) {
			popScissor(nvg);
		}
	}
	void TabBar::draw(AlloyContext* context) {
		pushScissor(context->nvgContext, getCursorBounds());
		Composite::draw(context);
		popScissor(context->nvgContext);

	}
	void TabBar::pack(const pixel2& pos, const pixel2& dims, const double2& dpmm, double pixelRatio, bool clamp) {
		float maxExtent = dims.x;
		for (TabPanePtr tabPane : panes) {
			tabPane->header->position = CoordPX(tabPane->bounds.position);
			if (tabPane->bounds.position.x + tabPane->bounds.dimensions.x < maxExtent || tabPane.get() == dragPane) {
				tabPane->header->setVisible(true);
			}
			else {
				tabPane->header->setVisible(false);
			}
		}
		if (selectedPane == nullptr) {
			for (int i = (int)panes.size() - 1;i >= 0;i--) {
				if (panes[i]->header->isVisible()) {
					setSelected(panes[i].get());
					break;
				}
			}
		}
		if (panes.size()>0 && !panes.back()->header->isVisible()) {
			tabDropButton->setVisible(true);
		}
		else {
			tabDropButton->setVisible(false);
		}
		Composite::pack(pos, dims, dpmm, pixelRatio, clamp);
	}
	void TabBar::close(TabPane* pane) {
		bool wasSelected=false;
		if (pane == selectedPane) {
			wasSelected=true;
			selectedPane = nullptr;
		}
		if (onClose) {
			if (!onClose(pane)){
				if(wasSelected)selectedPane=pane;
				return;
			}
		}
		barRegion->erase(pane->header);
		contentRegion->erase(pane->region);
		for (auto iter = panes.begin();iter != panes.end();iter++) {
			if (iter->get() == pane) {
				panes.erase(iter);
				break;
			}
		}
		selectionBox->clearSelections();
		for (TabPanePtr pane : panes) {
			selectionBox->addSelection(pane->header->getName());
		}
		sortPanes();
	}
	void TabBar::addPane(const std::shared_ptr<TabPane>& tabPane) {
		if (panes.size() > 0) {
			TabPanePtr bck = panes.back();
			tabPane->bounds.position = pixel2(bck->bounds.position.x + bck->bounds.dimensions.x + TabBar::TAB_SPACING, 0.0f);
		}
		else {
			tabPane->bounds.position = pixel2(TabBar::TAB_SPACING, 0.0f);
		}
		pixel2 dims = tabPane->header->getPreferredDimensions(AlloyApplicationContext().get());
		tabPane->bounds.dimensions = pixel2(dims.x, TAB_HEIGHT);
		tabPane->header->dimensions = CoordPerPX(0.0f, 1.0f, tabPane->bounds.dimensions.x, 0.0f);
		barRegion->insertAtFront(tabPane->header);
		tabPane->region->setVisible(false);
		contentRegion->add(tabPane->region);
		tabPane->parent = this;
		panes.push_back(tabPane);
		setSelected(tabPane.get());
	}
	void TabPane::setLabel(const std::string& name){
		header->textLabel->setLabel(name);
		pixel2 dims = header->getPreferredDimensions(AlloyApplicationContext().get());
		bounds.dimensions = pixel2(dims.x, TabBar::TAB_HEIGHT);
		header->dimensions = CoordPerPX(0.0f, 1.0f, bounds.dimensions.x, 0.0f);
	}
	std::shared_ptr<TabPane> TabBar::getPane(const std::shared_ptr<Region>& region) const{
		for(TabPanePtr pane:panes){
			if(pane->region.get()==region.get()){
				return pane;
			}
		}
		return std::shared_ptr<TabPane>();
	}
	TabPane::TabPane(const std::shared_ptr<Composite>& region,bool closeable) :
		header(
			std::shared_ptr<TabHeader>(
				new TabHeader(region->getName(),closeable, this))), region(
					region), parent(nullptr) {

	}
	TabBar* TabHeader::getBar() const {
		return parentPane->parent;
	}
	void TabBar::setSelected(TabPane* s) {
		if (selectedPane != nullptr) {
			selectedPane->region->setVisible(false);
		}
		selectedPane = s;
		barRegion->putLast(s->header);
		s->region->setVisible(true);
		if(onSelect){
			onSelect(selectedPane);
		}
	}
	bool TabBar::onEventHandler(AlloyContext* context, const InputEvent& e) {
		if (e.type == InputType::MouseButton) {
			if (e.isDown() && e.button == GLFW_MOUSE_BUTTON_LEFT) {
				for (std::shared_ptr<TabPane> pane : panes) {
					if (pane->isFocused()) {
						dragPane = pane.get();
						cursorDownPosition = e.cursor - dragPane->header->getBoundsPosition();
						break;
					}
				}
			}
			else if (e.isUp()) {
				if (dragPane != nullptr) {
					dragPane = nullptr;
					sortPanes();
				}
				else {
					dragPane = nullptr;
				}
			}
		}

		if (dragPane != nullptr && (e.type == InputType::Cursor || e.type == InputType::MouseButton)) {
			box2px bounds = barRegion->getBounds();
			dragPane->bounds.position.x = aly::clamp(e.cursor.x - cursorDownPosition.x, bounds.position.x + TabBar::TAB_SPACING, bounds.position.x + bounds.dimensions.x - dragPane->bounds.dimensions.x - TabBar::TAB_SPACING) - bounds.position.x;
			sortPanes();
			context->requestPack();
		}
		return false;
	}
	void TabBar::sortPanes() {
		if (dragPane != nullptr) {
			std::sort(panes.begin(), panes.end(), [this](const TabPanePtr& a, const TabPanePtr& b) {
				if (a->header->isVisible() && !b->header->isVisible()) {
					return true;
				}
				else if (!a->header->isVisible() && b->header->isVisible()) {
					return false;
				}
				else return (a->bounds.position.x < b->bounds.center().x);
			});
		}
		float xOffset = TabBar::TAB_SPACING;
		for (int index = 0;index <(int)panes.size();index++) {
			TabPanePtr tabPane = panes[index];
			if (tabPane.get() != dragPane) {
				tabPane->bounds.position = pixel2(xOffset, 0.0f);
			}
			xOffset += tabPane->bounds.dimensions.x + TabBar::TAB_SPACING;
		}
	}
	TabBar::TabBar(const std::string& name, const AUnit2D& position,
		const AUnit2D& dimensions) :
		Composite(name, position, dimensions), selectedPane(nullptr), dragPane(
			nullptr) {
		backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARKEST);
		barRegion = std::shared_ptr<Composite>(
			new Composite("Content", CoordPX(0, 0),
				CoordPerPX(1.0f, 0.0f, 0.0f, TAB_HEIGHT)));
		DrawPtr fadeRegion = DrawPtr(new Draw("Fade Region", CoordPerPX(1.0, 0.0, -2 * TAB_HEIGHT, 0.0f),
			CoordPX(2 * TAB_HEIGHT, TAB_HEIGHT)));
		fadeRegion->onDraw = [this](AlloyContext* context, const box2px& bounds) {
			NVGcontext* nvg = context->nvgContext;
			NVGpaint hightlightPaint = nvgLinearGradient(nvg, bounds.position.x, bounds.position.y,
				bounds.position.x + bounds.dimensions.x, bounds.position.y,
				backgroundColor->toSemiTransparent(0.0f), *backgroundColor);
			nvgBeginPath(nvg);
			nvgRect(nvg, bounds.position.x, bounds.position.y, bounds.dimensions.x, bounds.dimensions.y);
			nvgFillPaint(nvg, hightlightPaint);
			nvgFill(nvg);
		};
		fadeRegion->setIgnoreCursorEvents(true);
		tabDropButton = std::shared_ptr<IconButton>(
			new IconButton(0xf103, CoordPerPX(1.0, 0.0, -TAB_HEIGHT, 0.0f),
				CoordPX(TAB_HEIGHT, TAB_HEIGHT), IconType::SQUARE));
		tabDropButton->setVisible(false);
		tabDropButton->onMouseDown = [this](AlloyContext* context, const InputEvent& e) {
			if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
				if (selectionBox->isVisible()) {
					selectionBox->setVisible(false);
					context->removeOnTopRegion(selectionBox.get());
				}
				else {
					if (panes.size() > 0) {
						selectionBox->clearSelections();
						for (TabPanePtr pane : panes) {
							selectionBox->addSelection(pane->header->getName());
						}
						selectionBox->setMaxDisplayEntries(std::min(8, (int)panes.size()));
						selectionBox->setVisible(true);
						context->setOnTopRegion(selectionBox.get());
					}
				}
				return true;
			}
			return false;
		};
		tabDropButton->backgroundColor = MakeColor(0, 0, 0, 0);//AlloyApplicationContext()->theme.DARKEST.toSemiTransparent(0.5f));// MakeColor(AlloyApplicationContext()->theme.DARK.toLighter(0.5f));
		tabDropButton->setRoundCorners(false);
		tabDropButton->foregroundColor = MakeColor(0, 0, 0, 0);
		tabDropButton->borderColor = MakeColor(0, 0, 0, 0);
		tabDropButton->iconColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
		contentRegion = std::shared_ptr<Composite>(
			new Composite("Content", CoordPX(0.0f, TAB_HEIGHT),
				CoordPerPX(1.0f, 1.0f, 0.0f, -TAB_HEIGHT)));
		contentRegion->backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARK.toLighter(0.5f));
		selectionBox = SelectionBoxPtr(new SelectionBox(MakeString() << name << "_tab", CoordPerPX(1.0f, 0.0f, -120.0f, 0.0f), CoordPX(120.0f, TAB_HEIGHT - 6.0f)));
		selectionBox->backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARK);
		selectionBox->borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
		selectionBox->borderWidth = UnitPX(1.0f);
		selectionBox->setDetached(true);
		selectionBox->setVisible(false);
		selectionBox->textColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
		selectionBox->textAltColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
		selectionBox->onSelect = [this](SelectionBox* box) {
			int sIndex = selectionBox->getSelectedIndex();
			if (sIndex >= 0 && sIndex < (int)panes.size()) {
				selectionBox->setVisible(false);
				AlloyApplicationContext()->removeOnTopRegion(selectionBox.get());
				TabPanePtr current = panes[sIndex];
				TabPanePtr front = panes.front();
				if (current.get() != selectedPane) {
					if (!current->header->isVisible()) {
						panes[0] = current;
						panes[sIndex] = front;
					}
					setSelected(current.get());
					selectionBox->clearSelections();
					sortPanes();
					AlloyApplicationContext()->requestPack();
					return true;
				}
			}
			return false;
		};
		selectionBox->onMouseUp =
			[this](AlloyContext* context, const InputEvent& event) {
			if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
				int sIndex = selectionBox->getSelectedIndex();
				if (sIndex >= 0 && sIndex < (int)panes.size()) {
					selectionBox->setVisible(false);
					AlloyApplicationContext()->removeOnTopRegion(selectionBox.get());
					TabPanePtr current = panes[sIndex];
					TabPanePtr front = panes.front();
					if (current.get() != selectedPane) {
						if (!current->header->isVisible()) {
							panes[0] = current;
							panes[sIndex] = front;
						}
						setSelected(current.get());
						selectionBox->clearSelections();
						sortPanes();
						AlloyApplicationContext()->requestPack();
					}
				}
			}
			return false;
		};
		Composite::add(barRegion);
		Composite::add(fadeRegion);
		Composite::add(tabDropButton);
		barRegion->add(selectionBox);
		Composite::add(contentRegion);
		Application::addListener(this);
	}
}
