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
#include "AlloyExpandBar.h"
#include "AlloyApplication.h"
#include "AlloyDrawUtil.h"
namespace aly {
	void ExpandRegion::setExpanded(bool expanded) {
		contentRegion->setVisible(expanded);
		if (this->expanded != expanded) {
			AlloyApplicationContext()->requestPack();
		}
		this->expanded = expanded;
		arrowIcon->setLabel(
			(expanded) ? CodePointToUTF8(0xf056) : CodePointToUTF8(0xf055));

	}
	void ExpandRegion::draw(AlloyContext* context) {
		if (context->isMouseOver(titleContainer.get(), true)) {
			selectionLabel->textColor = MakeColor(context->theme.LIGHTEST);
			arrowIcon->textColor = MakeColor(context->theme.LIGHTEST);
		}
		else {
			selectionLabel->textColor = MakeColor(context->theme.LIGHTER);
			arrowIcon->textColor = MakeColor(context->theme.LIGHTER);
		}
		Composite::draw(context);
	}
	ExpandRegion::ExpandRegion(const std::string& name,
		const std::shared_ptr<Composite>& region, const AUnit2D& pos,
		const AUnit2D& dims, pixel expandHeight, bool expanded) :
		Composite(name + "_eregion", pos, dims), expanded(expanded), expandHeight(
			expandHeight) {
		this->contentRegion = region;
		backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARK);
		setRoundCorners(true);
		titleContainer = MakeComposite(MakeString() << name << "_container",
			CoordPerPX(0.0f, 0.0f, 5.0f, 5.0f),
			CoordPerPX(1.0f, 1.0f, -10.0f, -10.0f));
		selectionLabel = MakeTextLabel(name, CoordPX(0.0f, 0.0f),
			CoordPercent(1.0f, 1.0f), FontType::Bold, UnitPercent(1.0f),
			AlloyApplicationContext()->theme.LIGHTER.toRGBA(),
			HorizontalAlignment::Left, VerticalAlignment::Middle);

		arrowIcon = MakeTextLabel(CodePointToUTF8(0xf056), CoordPercent(1.0f, 0.0f),
			CoordPercent(0.0f, 1.0f), FontType::Icon, UnitPercent(1.0f),
			AlloyApplicationContext()->theme.LIGHTER.toRGBA(),
			HorizontalAlignment::Center, VerticalAlignment::Middle);

		arrowIcon->setAspectRatio(1.0f);
		arrowIcon->setOrigin(Origin::TopRight);
		arrowIcon->setAspectRule(AspectRule::FixedHeight);
		titleContainer->add(selectionLabel);
		titleContainer->add(arrowIcon);
		add(titleContainer);
		selectionLabel->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
			if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
				setExpanded(!this->expanded);
				return true;
			}
			return false;
		};
		this->onMouseDown = [this](AlloyContext* context, const InputEvent& event) {
			if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
				setExpanded(!this->expanded);
				return true;
			}
			return false;
		};
		arrowIcon->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
			if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
				setExpanded(!this->expanded);
				return true;
			}
			return false;
		};
		arrowIcon->setLabel(
			(expanded) ? CodePointToUTF8(0xf056) : CodePointToUTF8(0xf055));
		contentRegion->dimensions = CoordPerPX(1.0f, 0.0f,
			-Composite::scrollBarSize, (float)expandHeight);
		contentRegion->setVisible(expanded);
	}
	ExpandBar::ExpandBar(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims) :
		Composite(name, pos, dims) {
		setOrientation(Orientation::Vertical);
		setScrollEnabled(true);
		setRoundCorners(true);
		setAlwaysShowVerticalScrollBar(true);
		cellPadding.y = 2;

	}

	CompositePtr ExpandBar::addRegion(const std::shared_ptr<Region>& region,
		pixel expandHeight,
		bool expanded) {
		CompositePtr container = MakeComposite(
			MakeString() << region->name << " content", CoordPX(0.0f, 0.0f),
			CoordPerPX(1.0f, 0.0f, -Composite::scrollBarSize, expandHeight));
		if (dynamic_cast<ExpandBar*>(region.get()) != nullptr) {
			region->borderColor = MakeColor(
				AlloyApplicationContext()->theme.NEUTRAL);
			region->borderWidth = UnitPX(1.0f);
			region->position = CoordPX(2.0f, 0.0f);
			region->dimensions = CoordPerPX(1.0f, 0.0f, -2.0f, expandHeight);
			region->setRoundCorners(true);
			container->setScrollEnabled(false);
		}
		else {
			region->borderColor = MakeColor(
				AlloyApplicationContext()->theme.NEUTRAL);
			region->borderWidth = UnitPX(1.0f);
			region->backgroundColor = MakeColor(
				AlloyApplicationContext()->theme.DARK);
			region->position = CoordPX(2.0f, 0.0f);
			region->dimensions = CoordPerPX(1.0f, 0.0f, -2.0f, expandHeight);
			region->setRoundCorners(true);
			container->setScrollEnabled(!region->isScrollEnabled());
		}
		container->add(region);
		std::shared_ptr<ExpandRegion> eregion = std::shared_ptr<ExpandRegion>(
			new ExpandRegion(region->name, container, CoordPX(2.0f, 0.0f),
				CoordPerPX(1.0f, 0.0f, -Composite::scrollBarSize - 2.0f,
					30.0f), expandHeight, expanded));
		expandRegions.push_back(eregion);
		contentRegions.push_back(container);
		Composite::add(eregion);
		Composite::add(container);
		return container;
	}
}