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
#include "AlloyExpandTree.h"
#include "AlloyApplication.h"
#include "AlloyDrawUtil.h"
namespace aly {
	ExpandTree::ExpandTree(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims) :
		Composite(name, pos, dims), selectedItem(nullptr), dirty(true) {
		setScrollEnabled(true);
		setAlwaysShowVerticalScrollBar(true);
		drawRegion = DrawPtr(
			new Draw("Tree Region", CoordPX(0.0f, 0.0f),
				CoordPercent(1.0f, 1.0f)));
		drawRegion->onDraw = [this](AlloyContext* context, const box2px& bounds) {
			root.draw(this, context, bounds.position);
		};
		drawRegion->onMouseOver =
			[this](AlloyContext* context, const InputEvent& e) {
			box2px box = drawRegion->getBounds();
			overArrow = false;
			selectedItem = root.locate(context, e.cursor - box.position,overArrow);
			return false;
		};
		drawRegion->onMouseDown =
			[this](AlloyContext* context, const InputEvent& e) {
			if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
				if (selectedItem != nullptr) {
					if ((isOverArrow()|| !selectedItem->onSelect)&&(selectedItem->onExpand || selectedItem->hasChildren())) {
						selectedItem->setExpanded(!selectedItem->isExpanded());
					}
					else {
						if (selectedItem->onSelect) {
							selectedItem->onSelect(selectedItem, e);
						}
					}
					setDirty(true);
					update(context);
					return true;
				}
			}
			return false;
		};
		Composite::add(drawRegion);
	}
	void ExpandTree::pack(const pixel2& pos, const pixel2& dims,
		const double2& dpmm, double pixelRatio, bool clamp) {
		update(AlloyApplicationContext().get());
		drawRegion->dimensions = CoordPX(
			aly::max(getBounds().dimensions,
				root.getBounds().dimensions
				+ pixel2(Composite::scrollBarSize)));
		Composite::pack(pos, dims, dpmm, pixelRatio, clamp);
	}
	void ExpandTree::draw(AlloyContext* context) {
		if (!context->isMouseOver(this, true)) {
			selectedItem = nullptr;
		}
		Composite::draw(context);
	}
	void ExpandTree::drawDebug(AlloyContext* context) {
		if (selectedItem) {
			NVGcontext* nvg = context->nvgContext;
			nvgStrokeWidth(nvg, 1.0f);
			nvgStrokeColor(nvg, Color(220, 64, 64));
			nvgBeginPath(nvg);
			box2px bounds = selectedItem->getBounds();
			box2px pbounds = drawRegion->getBounds();
			bounds.position += pbounds.position;
			bounds.intersect(getBounds());
			nvgRect(nvg, bounds.position.x, bounds.position.y, bounds.dimensions.x,
				bounds.dimensions.y);
			nvgStroke(nvg);
		}
		Composite::drawDebug(context);
	}
	void ExpandTree::update(AlloyContext* context) {
		if (dirty) {
			root.update(context);
			dirty = false;
		}
	}
	void ExpandTree::addItem(const std::shared_ptr<TreeItem>& item) {
		root.addItem(item);
	}
	const int TreeItem::PADDING = 2;

	void TreeItem::setExpanded(bool ex) {
		expanded = ex;
		if (ex) {
			if (onExpand) {
				onExpand(this);
			}
		}
		else {
			if (onCollapse) {
				onCollapse(this);
			}
		}
	}
	void TreeItem::addItem(const std::shared_ptr<TreeItem>& item) {
		children.push_back(item);

	}
	TreeItem* TreeItem::locate(AlloyContext* context, const pixel2& pt,bool& overArrow) {

		if (isExpanded()) {
			for (TreeItemPtr& item : children) {
				TreeItem* selected = item->locate(context, pt,overArrow);
				if (selected != nullptr)
					return selected;
			}
		}
		if (pt.y >= selectionBounds.position.y
			&& pt.y
			< selectionBounds.position.y
			+ selectionBounds.dimensions.y) {
			if (pt.x >= selectionBounds.position.x&&pt.x <= selectionBounds.position.x + spaceWidth) {
				overArrow = true;
			}
			else {
				overArrow = false;
			}
			return this;
		}
		return nullptr;
	}
	TreeItem* LeafItem::locate(AlloyContext* context, const pixel2& pt, bool& overArrow) {
		if (bounds.contains(pt)) {
			return this;
		}
		return nullptr;
	}
	box2px TreeItem::getBounds() const {
		return bounds;
	}
	box2px TreeItem::update(AlloyContext* context, const pixel2& offset) {
		NVGcontext* nvg = context->nvgContext;
		nvgTextAlign(nvg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgFontSize(nvg, fontSize);
		nvgFontFaceId(nvg, context->getFontHandle(FontType::Bold));
		spaceWidth = fontSize + PADDING * 2;
		float textWidth = nvgTextBounds(nvg, 0, 0, name.c_str(), nullptr, nullptr);
		nvgFontFaceId(nvg, context->getFontHandle(FontType::Icon));
		float iconWidth =
			(iconCodeString.length() == 0) ?
			0 :
			nvgTextBounds(nvg, 0, 0, iconCodeString.c_str(), nullptr,
				nullptr) + PADDING * 2;
		float th = (name.length() > 0) ? fontSize + PADDING * 2 : 0;
		selectionBounds = box2px(offset,
			pixel2(textWidth + iconWidth + spaceWidth + PADDING, th));
		bounds = selectionBounds;
		if (isExpanded()) {
			pixel2 pt = offset + pixel2((name.length() > 0) ? spaceWidth : 0, th);
			for (TreeItemPtr& item : children) {
				box2px cdims = item->update(context, pt);
				bounds.dimensions = aly::max(bounds.max(), cdims.max())
					- aly::min(bounds.min(), cdims.min());
				pt += pixel2(0.0f, cdims.dimensions.y);
			}
		}
		return bounds;
	}
	TreeItem::TreeItem(const std::string& name, int iconCode, float fontSize) :
		name(name), fontSize(fontSize),spaceWidth(0.0f), expanded(name.length() == 0) {
		if (iconCode != 0) {
			iconCodeString = CodePointToUTF8(iconCode);
		}
	}
	LeafItem::LeafItem(
		const std::function<void(AlloyContext* context, const box2px& bounds)>& onDraw,
		const pixel2& dimensions) :
		onDraw(onDraw) {
		bounds = box2px(pixel2(0.0f), dimensions);
	}
	box2px LeafItem::getBounds() const {
		return bounds;
	}
	box2px LeafItem::update(AlloyContext* context, const pixel2& offset) {
		bounds.position = offset;
		return bounds;
	}
	void LeafItem::draw(ExpandTree* tree, AlloyContext* context,
		const pixel2& offset) {
		box2px box(bounds.position + offset, bounds.dimensions);
		onDraw(context, box);
	}
	void TreeItem::draw(ExpandTree* tree, AlloyContext* context,
		const pixel2& offset) {
		box2px bounds = getBounds();
		NVGcontext* nvg = context->nvgContext;
		nvgFontFaceId(nvg, context->getFontHandle(FontType::Icon));
		float spaceWidth = fontSize + PADDING * 2;
		float iconWidth = 0;
		static const std::string rightArrow = CodePointToUTF8(0xf0da);
		static const std::string downArrow = CodePointToUTF8(0xf0d7);
		pixel2 pt = bounds.position + offset;
		bool selected = (tree->getSelectedItem() == this)&&!tree->isOverArrow();
		nvgFontSize(nvg, fontSize);
		if(selected&&onSelect){
			context->setCursor(&Cursor::Hand);
		}
		if (iconCodeString.length() > 0) {
			iconWidth = nvgTextBounds(nvg, 0, 0, iconCodeString.c_str(), nullptr,
				nullptr) + PADDING * 2;

			if (children.size() > 0 || onExpand) {
				nvgTextAlign(nvg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
				if (tree->isOverArrow()) {
					nvgFillColor(nvg, context->theme.LIGHTEST);
				}
				else {
					nvgFillColor(nvg, context->theme.LIGHTER);
				}
				nvgText(nvg, pt.x + spaceWidth * 0.5f, pt.y + PADDING,
					(expanded) ? downArrow.c_str() : rightArrow.c_str(),
					nullptr);
			}
			if (selected) {
				nvgFillColor(nvg, context->theme.LIGHTEST);
			}
			else {
				nvgFillColor(nvg, context->theme.LIGHTER);
			}
			nvgTextAlign(nvg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
			nvgText(nvg, pt.x + spaceWidth, pt.y + PADDING, iconCodeString.c_str(),
				nullptr);
		}
		if (name.length() > 0) {
			if (selected) {
				nvgFillColor(nvg, context->theme.LIGHTEST);
			}
			else {
				nvgFillColor(nvg, context->theme.LIGHTER);
			}
			nvgTextAlign(nvg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
			nvgFontFaceId(nvg, context->getFontHandle(FontType::Bold));
			nvgText(nvg, pt.x + iconWidth + spaceWidth, pt.y + PADDING,
				name.c_str(), nullptr);
		}
		if (expanded) {
			for (TreeItemPtr& item : children) {
				item->draw(tree, context, offset);
			}
		}
	}
}
