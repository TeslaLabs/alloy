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
#ifndef ALLOYCOLORSELECTOR_H_
#define ALLOYCOLORSELECTOR_H_
#include "AlloyUI.h"
#include "AlloyWidget.h"
namespace aly {

	struct ColorWheel : public Composite {
	protected:
		Color selectedColor;
		float2 tBounds[3];
		float2 tPoints[3];
		float rInner, rOuter;
		float2 center;
		HSVA hsvColor;
		bool triangleSelected = false;
		bool circleSelected = false;
		std::function<void(const Color& value)> onChangeEvent;
		void updateWheel();
	public:
		inline void setOnChangeEvent(
			const std::function<void(const Color& value)>& func) {
			onChangeEvent = func;
		}
		void reset();
		Color getSelectedColor() const {
			return selectedColor;
		}
		void setColor(const Color& c);
		void setColor(const pixel2& cursor);
		ColorWheel(const std::string& name, const AUnit2D& pos,
			const AUnit2D& dims);
		void draw(AlloyContext* context) override;
	};
	class ColorSelector : public Composite {
	private:
		TextLabelPtr textLabel;
		GlyphRegionPtr colorLabel;
		CompositePtr colorSelectionPanel;
		std::shared_ptr<CheckerboardGlyph> checkerboard;
		std::shared_ptr<VerticalSlider> redSlider;
		std::shared_ptr<VerticalSlider> greenSlider;
		std::shared_ptr<VerticalSlider> blueSlider;
		std::shared_ptr<VerticalSlider> lumSlider;
		std::shared_ptr<VerticalSlider> alphaSlider;
		std::shared_ptr<ColorWheel> colorWheel;
		void updateColorSliders(const Color& c);
	public:
		std::function<void(const Color& c)> onSelect;
		void setColor(const Color& color);
		Color getColor();
		inline void setValue(const Color& color) {
			setColor(color);
		}
		inline Color getValue() {
			return getColor();
		}
		ColorSelector(const std::string& name, const AUnit2D& pos,
			const AUnit2D& dims, bool showText = true);
		virtual void draw(AlloyContext* context) override;
	};
	typedef std::shared_ptr<ColorSelector> ColorSelectorPtr;
	typedef std::shared_ptr<ColorWheel> ColorWheelPtr;
}
#endif
