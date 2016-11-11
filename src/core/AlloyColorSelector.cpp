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
#include "AlloyColorSelector.h"
#include "AlloyApplication.h"
#include "AlloyDrawUtil.h"
using namespace std;
namespace aly {
	ColorSelector::ColorSelector(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims, bool showText) :
		Composite(name, pos, dims) {

		backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARK);
		borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
		borderWidth = UnitPX(1.0f);
		setRoundCorners(true);

		if (showText) {
			textLabel = MakeTextLabel(name, CoordPercent(0.0f, 0.0f),
				CoordPercent(1.0f, 1.0f), FontType::Bold, UnitPercent(1.0f),
				AlloyApplicationContext()->theme.LIGHTER,
				HorizontalAlignment::Left, VerticalAlignment::Middle);
		}

		if (checkerboard.get() == nullptr) {
			if (showText) {
				checkerboard = std::shared_ptr<CheckerboardGlyph>(
					new CheckerboardGlyph(64, 64, 8, 8,
						AlloyApplicationContext().get()));
			}
			else {
				checkerboard = std::shared_ptr<CheckerboardGlyph>(
					new CheckerboardGlyph(64 * 3, 64, 8 * 3, 8,
						AlloyApplicationContext().get()));
			}
		}
		colorLabel = std::shared_ptr<GlyphRegion>(
			new GlyphRegion("Color", checkerboard));

		colorLabel->glyph = checkerboard;
		colorLabel->backgroundColor = MakeColor(COLOR_BLACK);
		colorLabel->foregroundColor = MakeColor(255, 128, 32, 255);
		colorLabel->borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
		colorLabel->borderWidth = UnitPX(1.0f);

		if (showText) {
			colorLabel->position = CoordPerPX(1.0f, 0.0f, -4.0f, 4.0f);
			colorLabel->dimensions = CoordPerPX(0.0f, 1.0f, 0.0f, -8.0f);
			colorLabel->setOrigin(Origin::TopRight);
			colorLabel->setAspectRatio(1.0f);
			colorLabel->setAspectRule(AspectRule::FixedHeight);
		}
		else {
			colorLabel->position = CoordPX(2.0f, 2.0f);
			colorLabel->dimensions = CoordPerPX(1.0f, 1.0f, -4.0f, -4.0f);
		}
		colorWheel = ColorWheelPtr(
			new ColorWheel("Color Wheel", CoordPX(0.0f, 0.0f),
				CoordPerPX(1.0f, 0.0f, 0.0f, 300.0f)));
		colorWheel->setAspectRatio(1.0f);
		colorWheel->setAspectRule(AspectRule::FixedHeight);
		colorLabel->onMouseDown =
			[this](AlloyContext* context, const InputEvent& e) {
			if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
				if (!colorSelectionPanel->isVisible()) {
					colorWheel->reset();
				}
				colorSelectionPanel->setVisible(true);
				context->getGlassPane()->setVisible(true);
				return true;
			}
			return false;
		};
		if (showText) {
			textLabel->onMouseDown =
				[this](AlloyContext* context, const InputEvent& e) {
				if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
					if (!colorSelectionPanel->isVisible()) {
						colorWheel->reset();
					}
					colorSelectionPanel->setVisible(true);
					context->getGlassPane()->setVisible(true);
					return true;
				}
				return false;
			};
		}
		colorSelectionPanel = MakeComposite("Color Selection Panel",
			CoordPerPX(0.5f, 0.5, 0.0f, 0.0f),
			CoordPX(
				18 + 300.0f
				+ (60 + AlloyApplicationContext()->theme.SPACING.x)
				* 5, 22 + 300.0f), COLOR_NONE);
		colorSelectionPanel->setVisible(false);
		colorSelectionPanel->setOrigin(Origin::MiddleCenter);

		redSlider = std::shared_ptr<VerticalSlider>(
			new VerticalSlider("R", CoordPX(0.0f, 0.0f),
				CoordPerPX(0.0f, 1.0f, 60.0f, 0.0f), Float(0.0), Float(1.0),
				Float(0.5)));
		redSlider->setLabelFormatter(
			[](const Number& value) {
			std::string str = MakeString() << (int)std::floor(100.0f*value.toFloat()) << "%";
			return str;
		});
		redSlider->setOnChangeEvent([this](const Number& value) {
			Color c = colorWheel->getSelectedColor();
			c.r = value.toFloat();
			colorWheel->setColor(c);
			HSV hsv = c.toHSV();
			lumSlider->setValue(hsv.z);
			updateColorSliders(c);
		});

		greenSlider = std::shared_ptr<VerticalSlider>(
			new VerticalSlider("G", CoordPX(0.0f, 0.0f),
				CoordPerPX(0.0f, 1.0f, 60.0f, 0.0f), Float(0.0), Float(1.0),
				Float(0.5)));
		greenSlider->setOnChangeEvent([this](const Number& value) {
			Color c = colorWheel->getSelectedColor();
			c.g = value.toFloat();
			colorWheel->setColor(c);
			HSV hsv = c.toHSV();
			lumSlider->setValue(hsv.z);
			updateColorSliders(c);
		});
		greenSlider->setLabelFormatter(
			[](const Number& value) {
			string str = MakeString() << (int)std::floor(100.0f*value.toFloat()) << "%";
			return str;
		});
		blueSlider = std::shared_ptr<VerticalSlider>(
			new VerticalSlider("B", CoordPX(0.0f, 0.0f),
				CoordPerPX(0.0f, 1.0f, 60.0f, 0.0f), Float(0.0), Float(1.0),
				Float(0.5)));
		blueSlider->setOnChangeEvent([this](const Number& value) {
			Color c = colorWheel->getSelectedColor();
			c.b = value.toFloat();
			colorWheel->setColor(c);
			HSV hsv = c.toHSV();
			lumSlider->setValue(hsv.z);
			updateColorSliders(c);
		});
		blueSlider->setLabelFormatter(
			[](const Number& value) {
			string str = MakeString() << (int)std::floor(100.0f*value.toFloat()) << "%";
			return str;
		});
		lumSlider = std::shared_ptr<VerticalSlider>(
			new VerticalSlider("L", CoordPX(0.0f, 0.0f),
				CoordPerPX(0.0f, 1.0f, 60.0f, 0.0f), Float(0.0), Float(1.0),
				Float(0.5)));
		lumSlider->setLabelFormatter(
			[](const Number& value) {
			string str = MakeString() << (int)std::floor(100.0f*value.toFloat()) << "%";
			return str;
		});
		lumSlider->setOnChangeEvent([this](const Number& value) {
			Color c = colorWheel->getSelectedColor();
			HSVA hsv = c.toHSVA();
			hsv.z = value.toFloat();
			c = HSVAtoRGBAf(hsv);
			colorWheel->setColor(c);
			redSlider->setValue(c.r);
			greenSlider->setValue(c.g);
			blueSlider->setValue(c.b);
			updateColorSliders(c);
		});

		alphaSlider = std::shared_ptr<VerticalSlider>(
			new VerticalSlider("A", CoordPX(0.0f, 0.0f),
				CoordPerPX(0.0f, 1.0f, 60.0f, 0.0f), Float(0.0), Float(1.0),
				Float(0.5)));
		alphaSlider->setLabelFormatter(
			[](const Number& value) {
			string str = MakeString() << (int)std::floor(100.0f*value.toFloat()) << "%";
			return str;
		});
		alphaSlider->setOnChangeEvent([this](const Number& value) {
			Color c = colorWheel->getSelectedColor();
			c.a = value.toFloat();
			colorWheel->setColor(c);
			updateColorSliders(c);
		});
		colorWheel->setOnChangeEvent([this](const Color& c) {

			redSlider->setValue(c.r);
			greenSlider->setValue(c.g);
			blueSlider->setValue(c.b);
			HSV hsv = c.toHSV();
			lumSlider->setValue(hsv.z);
			updateColorSliders(c);
		});
		IconButtonPtr cancelButton = std::shared_ptr<IconButton>(
			new IconButton(0xf00d, CoordPerPX(1.0, 0.0, -30, 30),
				CoordPX(30, 30), IconType::CIRCLE));
		cancelButton->setOrigin(Origin::BottomLeft);
		cancelButton->backgroundColor = MakeColor(COLOR_NONE);
		cancelButton->borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHTEST);
		cancelButton->onMouseDown =
			[this](AlloyContext* context, const InputEvent& event) {
			colorSelectionPanel->setVisible(false);
			context->getGlassPane()->setVisible(false);
			return true;
		};
		CompositePtr hContainer = MakeComposite("Horizontal Layout",
			CoordPX(0.0f, 18.0f), CoordPerPX(1.0f, 1.0f, -30.0f, -22.0f),
			AlloyApplicationContext()->theme.LIGHT);

		hContainer->setRoundCorners(true);
		hContainer->setOrientation(Orientation::Horizontal);
		hContainer->add(colorWheel);
		hContainer->add(redSlider);
		hContainer->add(greenSlider);
		hContainer->add(blueSlider);
		hContainer->add(lumSlider);
		hContainer->add(alphaSlider);
		colorSelectionPanel->add(hContainer);
		colorSelectionPanel->onEvent =
			[=](AlloyContext* context, const InputEvent& e) {
			if (colorSelectionPanel->isVisible()) {
				if (e.type == InputType::MouseButton&&e.isDown() && !context->isMouseContainedIn(hContainer->getBounds())) {
					colorSelectionPanel->setVisible(false);
					context->getGlassPane()->setVisible(false);
					if (onSelect) {
						onSelect(colorWheel->getSelectedColor());
					}
					return true;
				}
			}
			return false;
		};
		colorSelectionPanel->add(cancelButton);
		Application::addListener(colorSelectionPanel.get());
		if (showText) {
			add(textLabel);
		}
		add(colorLabel);
		AlloyApplicationContext()->getGlassPane()->add(colorSelectionPanel);
		setColor(*colorLabel->foregroundColor);
	}
	void ColorSelector::updateColorSliders(const Color& c) {
		redSlider->setSliderColor(Color(0.0f, c.g, c.b), Color(1.0f, c.g, c.b));
		greenSlider->setSliderColor(Color(c.r, 0.0f, c.b), Color(c.r, 1.0f, c.b));
		blueSlider->setSliderColor(Color(c.r, c.g, 0.0f), Color(c.r, c.g, 1.0f));
		HSV hsv = c.toHSV();
		lumSlider->setSliderColor(HSVtoColor(HSV(hsv.x, hsv.y, 0.0f)),
			HSVtoColor(HSV(hsv.x, hsv.y, 1.0f)));
		alphaSlider->setSliderColor(Color(c.r, c.g, c.b, 0.0f),
			Color(c.r, c.g, c.b, 1.0f));

	}
	void ColorSelector::setColor(const Color& c) {
		*colorLabel->foregroundColor = c;
		HSVA hsv = c.toHSVA();
		colorWheel->setColor(c);
		redSlider->setValue(c.r);
		greenSlider->setValue(c.g);
		blueSlider->setValue(c.b);
		lumSlider->setValue(hsv.z);
		alphaSlider->setValue(c.a);
		updateColorSliders(c);
		if (onSelect) {
			onSelect(c);
		}
	}
	Color ColorSelector::getColor() {
		return colorWheel->getSelectedColor();
	}
	void ColorWheel::reset() {
		circleSelected = false;
		triangleSelected = false;
		updateWheel();
	}
	ColorWheel::ColorWheel(const std::string& name, const AUnit2D& pos,
		const AUnit2D& dims) :
		Composite(name, pos, dims) {

		setColor(Color(32, 64, 255));
		this->onPack = [this]() {
			this->updateWheel();
		};
		this->onMouseDown = [this](AlloyContext* context, const InputEvent& e) {
			if (e.button == GLFW_MOUSE_BUTTON_LEFT&&e.isDown()) {
				float r2 = distanceSqr(e.cursor, center);
				if (r2 < rInner*rInner) {
					triangleSelected = true;
					circleSelected = false;
				}
				else if (r2 < rOuter*rOuter) {
					triangleSelected = false;
					circleSelected = true;
				}
				else {
					circleSelected = false;
					triangleSelected = false;
				}
				this->setColor(e.cursor);
				return true;
			}
			return false;
		};
		this->onScroll = [this](AlloyContext* context, const InputEvent& e) {
			hsvColor.x += e.scroll.y*0.01f;
			if (hsvColor.x < 0.0f)hsvColor.x += 1.0f;
			if (hsvColor.x > 1.0f)hsvColor.x -= 1.0f;
			setColor(HSVAtoColor(hsvColor));
			updateWheel();
			if (onChangeEvent)onChangeEvent(selectedColor);
			return true;
		};
		this->onMouseOver = [this](AlloyContext* context, const InputEvent& e) {
			if (context->isLeftMouseButtonDown()) {
				this->setColor(e.cursor);
				return true;
			}
			return false;
		};
		Application::addListener(this);
	}

	void ColorWheel::updateWheel() {
		box2px bounds = getBounds();
		float x = bounds.position.x;
		float y = bounds.position.y;
		float w = bounds.dimensions.x;
		float h = bounds.dimensions.y;

		float cx = x + w * 0.5f;
		float cy = y + h * 0.5f;
		rOuter = (w < h ? w : h) * 0.5f - 5.0f;
		rInner = rOuter - 20.0f;
		center = float2(cx, cy);
		float r = rInner - 6;
		tPoints[0] = float2(r, 0);
		tPoints[1] = float2(cos(120.0f / 180.0f * NVG_PI) * r,
			sin(120.0f / 180.0f * NVG_PI) * r);
		tPoints[2] = float2(cosf(-120.0f / 180.0f * NVG_PI) * r,
			sin(-120.0f / 180.0f * NVG_PI) * r);
		float angle = -hsvColor.x * NVG_PI * 2;
		for (int i = 0; i < 3; i++) {
			tBounds[i] = Rotate(tPoints[i], angle) + center;
		}
	}
	void ColorWheel::setColor(const Color& c) {
		selectedColor = c;
		hsvColor = c.toHSVA();
		updateWheel();
	}
	void ColorWheel::setColor(const pixel2& cursor) {
		if (triangleSelected) {

			float2 mid = 0.5f * (tBounds[0] + tBounds[1]);
			float u = clamp(
				dot(cursor - tBounds[0], tBounds[1] - tBounds[0])
				/ lengthSqr(tBounds[1] - tBounds[0]), 0.0f, 1.0f);
			float v = clamp(
				dot(cursor - tBounds[2], mid - tBounds[2])
				/ lengthSqr(mid - tBounds[2]), 0.0f, 1.0f);
			RGBAf hc = HSVAtoRGBAf(HSVA(hsvColor.x, 1.0f, 1.0f, 1.0f));
			RGBAf c = v * (hc + u * (1.0f - hc));
			HSVA hsv = Color(c).toHSVA();
			hsvColor.y = hsv.y;
			hsvColor.z = hsv.z;
			selectedColor = HSVAtoColor(hsvColor);
			updateWheel();
			if (onChangeEvent)
				onChangeEvent(selectedColor);
		}
		else if (circleSelected) {
			float2 vec = cursor - center;
			hsvColor.x = (atan2(vec.y, vec.x)) / (2.0f * NVG_PI);
			if (hsvColor.x < 0.0f) {
				hsvColor.x += 1.0f;
			}
			selectedColor = HSVAtoColor(hsvColor);
			updateWheel();
			if (onChangeEvent)
				onChangeEvent(selectedColor);
		}
	}
	void ColorWheel::draw(AlloyContext* context) {
		NVGcontext* nvg = context->nvgContext;
		box2px bounds = getBounds();

		nvgBeginPath(nvg);
		nvgRoundedRect(nvg, bounds.position.x, bounds.position.y,
			bounds.dimensions.x, bounds.dimensions.y,
			context->theme.CORNER_RADIUS);
		nvgFillColor(nvg, context->theme.DARK);
		nvgFill(nvg);

		nvgBeginPath(nvg);

		NVGpaint hightlightPaint = nvgBoxGradient(nvg, bounds.position.x,
			bounds.position.y, bounds.dimensions.x, bounds.dimensions.y,
			context->theme.CORNER_RADIUS, 2,
			context->theme.DARK.toSemiTransparent(0.0f),
			context->theme.LIGHTEST);
		nvgFillPaint(nvg, hightlightPaint);
		nvgRoundedRect(nvg, bounds.position.x, bounds.position.y,
			bounds.dimensions.x, bounds.dimensions.y,
			context->theme.CORNER_RADIUS);
		nvgFill(nvg);

		int i;
		float ax, ay, bx, by, aeps;

		float hue = hsvColor.x;

		NVGpaint paint;
		nvgSave(nvg);
		aeps = 0.5f / rOuter; // half a pixel arc length in radians (2pi cancels out).
		for (i = 0; i < 6; i++) {
			float a0 = (float)i / 6.0f * NVG_PI * 2.0f - aeps;
			float a1 = (float)(i + 1.0f) / 6.0f * NVG_PI * 2.0f + aeps;
			nvgBeginPath(nvg);
			nvgArc(nvg, center.x, center.y, rInner, a0, a1, NVG_CW);
			nvgArc(nvg, center.x, center.y, rOuter, a1, a0, NVG_CCW);
			nvgClosePath(nvg);
			ax = center.x + cosf(a0) * (rInner + rOuter) * 0.5f;
			ay = center.y + sinf(a0) * (rInner + rOuter) * 0.5f;
			bx = center.x + cosf(a1) * (rInner + rOuter) * 0.5f;
			by = center.y + sinf(a1) * (rInner + rOuter) * 0.5f;
			paint = nvgLinearGradient(nvg, ax, ay, bx, by,
				nvgHSLA(a0 / (NVG_PI * 2), 1.0f, 0.55f, 255),
				nvgHSLA(a1 / (NVG_PI * 2), 1.0f, 0.55f, 255));
			nvgFillPaint(nvg, paint);
			nvgFill(nvg);
		}

		nvgBeginPath(nvg);
		nvgCircle(nvg, center.x, center.y, rInner - 0.5f);
		nvgCircle(nvg, center.x, center.y, rOuter + 0.5f);
		nvgStrokeColor(nvg, context->theme.NEUTRAL.toSemiTransparent(0.5f));
		nvgStrokeWidth(nvg, 1.0f);
		nvgStroke(nvg);

		// Selector
		nvgSave(nvg);
		nvgTranslate(nvg, center.x, center.y);
		nvgRotate(nvg, hue * NVG_PI * 2);
		// Marker on
		nvgStrokeWidth(nvg, 2.0f);
		nvgBeginPath(nvg);
		nvgRect(nvg, rInner - 1, -3, rOuter - rInner + 2, 6);
		nvgStrokeColor(nvg, context->theme.LIGHTEST.toSemiTransparent(0.9f));
		nvgStroke(nvg);

		paint = nvgBoxGradient(nvg, rInner - 3, -5, rOuter - rInner + 6, 10, 2, 4,
			nvgRGBA(0, 0, 0, 128), nvgRGBA(0, 0, 0, 0));
		nvgBeginPath(nvg);
		nvgRect(nvg, rInner - 2 - 10, -4 - 10, rOuter - rInner + 4 + 20, 8 + 20);
		nvgRect(nvg, rInner - 2, -4, rOuter - rInner + 4, 8);
		nvgPathWinding(nvg, NVG_HOLE);
		nvgFillPaint(nvg, paint);
		nvgFill(nvg);

		nvgBeginPath(nvg);
		nvgMoveTo(nvg, tPoints[0].x, tPoints[0].y);
		nvgLineTo(nvg, tPoints[1].x, tPoints[1].y);
		nvgLineTo(nvg, tPoints[2].x, tPoints[2].y);
		nvgClosePath(nvg);
		NVGcolor chue = nvgHSLA(hue, 1.0f, 0.5f, 255);
		paint = nvgLinearGradient(nvg, tPoints[0].x, tPoints[0].y, tPoints[1].x,
			tPoints[1].y, chue, nvgRGBA(255, 255, 255, 255));
		nvgFillPaint(nvg, paint);
		nvgFill(nvg);
		paint = nvgLinearGradient(nvg, (tPoints[0].x + tPoints[1].x) * 0.5f,
			(tPoints[0].y + tPoints[1].y) * 0.5f, tPoints[2].x, tPoints[2].y,
			nvgRGBA(0, 0, 0, 0), nvgRGBA(0, 0, 0, 255));
		nvgFillPaint(nvg, paint);
		nvgFill(nvg);
		nvgStrokeColor(nvg, context->theme.NEUTRAL.toSemiTransparent(0.5f));
		nvgStroke(nvg);

		RGBf c3(0.0f, 0.0f, 0.0f);
		RGBf c2(1.0f, 1.0f, 1.0f);
		RGBf hc(chue.r, chue.g, chue.b);
		RGBf c = selectedColor.toRGBf();

		float2 bvec, pt;
		float u, v;

		v = std::max(std::max(c.x, c.y), c.z);
		if (v > 0.0f) {
			u = dot(c / v - hc, 1.0f - hc) / lengthSqr(1.0f - hc);

			float2 mid = 0.5f * (tPoints[0] + tPoints[1]);
			bvec.x = dot(tPoints[0], tPoints[1] - tPoints[0])
				+ lengthSqr(tPoints[1] - tPoints[0]) * u;
			bvec.y = dot(tPoints[2], mid - tPoints[2])
				+ lengthSqr(mid - tPoints[2]) * v;
			float2x2 M;
			float2 row1 = tPoints[1] - tPoints[0];
			float2 row2 = mid - tPoints[2];
			M(0, 0) = row1.x;
			M(0, 1) = row1.y;
			M(1, 0) = row2.x;
			M(1, 1) = row2.y;
			pt = inverse(M) * bvec;

			if (crossMag(pt - tPoints[2], tPoints[1] - tPoints[2]) < 0) {
				pt = dot(pt - tPoints[2], tPoints[1] - tPoints[2])
					* (tPoints[1] - tPoints[2])
					/ lengthSqr(tPoints[1] - tPoints[2]) + tPoints[2];
			}
			else if (crossMag(tPoints[0] - tPoints[2], pt - tPoints[2]) < 0) {
				pt = dot(pt - tPoints[2], tPoints[0] - tPoints[2])
					* (tPoints[0] - tPoints[2])
					/ lengthSqr(tPoints[0] - tPoints[2]) + tPoints[2];
			}
		}
		else {
			pt = tPoints[2];
		}
		ax = pt.x;
		ay = pt.y;

		nvgStrokeWidth(nvg, 2.0f);
		nvgBeginPath(nvg);
		nvgCircle(nvg, ax, ay, 5);
		nvgStrokeColor(nvg, context->theme.LIGHTEST.toSemiTransparent(0.9f));
		nvgStroke(nvg);
		paint = nvgRadialGradient(nvg, ax, ay, 7, 9, nvgRGBA(0, 0, 0, 64),
			nvgRGBA(0, 0, 0, 0));
		nvgBeginPath(nvg);
		nvgRect(nvg, ax - 20, ay - 20, 40, 40);
		nvgCircle(nvg, ax, ay, 7);
		nvgPathWinding(nvg, NVG_HOLE);
		nvgFillPaint(nvg, paint);
		nvgFill(nvg);

		nvgRestore(nvg);
		nvgRestore(nvg);
	}
	void ColorSelector::draw(AlloyContext* context) {
		bool hover = context->isMouseContainedIn(this);
		if (colorWheel->isVisible()) {
			*colorLabel->foregroundColor = colorWheel->getSelectedColor();
		}
		if (hover) {
			if (textLabel.get() != nullptr)textLabel->textColor = MakeColor(context->theme.LIGHTEST);
			colorLabel->borderWidth = UnitPX(2.0f);
			colorLabel->borderColor = MakeColor(context->theme.LIGHTEST);
		}
		else {
			if (textLabel.get() != nullptr)textLabel->textColor = MakeColor(context->theme.LIGHTER);
			colorLabel->borderWidth = UnitPX(1.0f);
			colorLabel->borderColor = MakeColor(context->theme.LIGHTER);
		}

		Composite::draw(context);
	}
}
