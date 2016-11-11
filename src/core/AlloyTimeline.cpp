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

#include "AlloyTimeline.h"
#include "AlloyApplication.h"
#include "AlloyDrawUtil.h"
namespace aly {
	TimelineSlider::TimelineSlider(const std::string& name, const AUnit2D& pos, const AUnit2D& dims, const Number& min, const Number& max,
		const Number& timeValue) :
		Composite(name, pos, dims),requestUpdate(false), minValue(min), maxValue(max),timeValue(timeValue), sliderPosition(0.0),majorTick(10.0),minorTick(1.0),modifiable(true) {
		lowerValue = min.clone();
		upperValue = max.clone();
		labelFormatter = [](const Number& value) {return value.toString();};
		this->position = position;
		this->dimensions = dimensions;
		handleSize = 30.0f;
		trackPadding = 10.0f;
		this->aspectRatio = 4.0f;
		entryWidth = 60.0f;
		textColor = MakeColor(AlloyApplicationContext()->theme.LIGHTER);
		backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARK);
		borderColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
		borderWidth = UnitPX(1.0f);
		setRoundCorners(false);
		minValueLabel = ModifiableNumberPtr(new ModifiableNumber("Low", CoordPerPX(0.0f, 0.5f,trackPadding*0.5f, 0.5f*handleSize), CoordPX(entryWidth, 0.75f*handleSize), lowerValue.type()));
		maxValueLabel = ModifiableNumberPtr(new ModifiableNumber("High", CoordPerPX(1.0f, 0.5f, -trackPadding*0.5f, 0.5f*handleSize), CoordPX(entryWidth, 0.75f*handleSize), upperValue.type()));
		minValueLabel->setNumberValue(minValue);
		minValueLabel->borderWidth = UnitPX(0.0f);
		minValueLabel->fontType = FontType::Bold;
		minValueLabel->fontSize = UnitPX(0.75f*handleSize);
		minValueLabel->textColor = MakeColor(AlloyDefaultContext()->theme.LIGHTEST);
		minValueLabel->setAlignment(HorizontalAlignment::Left, VerticalAlignment::Top);
		minValueLabel->backgroundColor = MakeColor(0,0,0,0);
		maxValueLabel->borderWidth = UnitPX(0.0f);
		maxValueLabel->fontSize = UnitPX(0.75f*handleSize);
		maxValueLabel->fontType = FontType::Bold;
		maxValueLabel->textColor = MakeColor(AlloyDefaultContext()->theme.LIGHTEST);
		maxValueLabel->backgroundColor = MakeColor(0, 0, 0, 0);
		maxValueLabel->setAlignment(HorizontalAlignment::Right, VerticalAlignment::Top);
		maxValueLabel->setNumberValue(maxValue);
		maxValueLabel->setOrigin(Origin::TopRight);

		minValueLabel->onTextEntered = [this](NumberField* field) {
			setMinValue(field->getValue().toDouble());
			if (onChangeEvent)onChangeEvent(this->timeValue,this->lowerValue, this->upperValue);
			AlloyApplicationContext()->requestPack();
		};
		maxValueLabel->onTextEntered = [this](NumberField* field) {
			setMaxValue(field->getValue().toDouble());
			if (onChangeEvent)onChangeEvent(this->timeValue,this->lowerValue, this->upperValue);
			AlloyApplicationContext()->requestPack();
		};

		lowerSliderHandle = std::shared_ptr<SliderHandle>(new SliderHandle("Lower Handle", SliderHandleShape::HalfLeft));

		lowerSliderHandle->position = CoordPercent(0.0, 0.0);
		lowerSliderHandle->dimensions = CoordPX(handleSize*0.5f, handleSize);
		lowerSliderHandle->backgroundColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
		lowerSliderHandle->setDragEnabled(true);

		upperSliderHandle = std::shared_ptr<SliderHandle>(new SliderHandle("Upper Handle", SliderHandleShape::HalfRight));
		upperSliderHandle->position = CoordPercent(0.0, 0.0);
		upperSliderHandle->dimensions = CoordPX(handleSize*0.5f, handleSize);
		upperSliderHandle->backgroundColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
		upperSliderHandle->setDragEnabled(true);

		timeSliderHandle = std::shared_ptr<SliderHandle>(new SliderHandle("Time Handle", SliderHandleShape::Hat));
		timeSliderHandle->position = CoordPerPX(0.5f,0.0f,0.0f, 0.0f);
		timeSliderHandle->dimensions = CoordPerPX(0.0f,1.0f,handleSize, 0.0f);
		timeSliderHandle->backgroundColor = MakeColor(AlloyApplicationContext()->theme.LIGHT);
		timeSliderHandle->setDragEnabled(true);

		rangeSliderTrack = std::shared_ptr<SliderTrack>(new SliderTrack("Range Track", Orientation::Horizontal, AlloyApplicationContext()->theme.LIGHTEST, AlloyApplicationContext()->theme.LIGHTEST));
		rangeSliderTrack->backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARK);
		rangeSliderTrack->add(lowerSliderHandle);
		rangeSliderTrack->add(upperSliderHandle);
		rangeSliderTrack->onMouseDown =  [this](AlloyContext* context, const InputEvent& e) {return this->onMouseDown(context, rangeSliderTrack.get(), e);};
		timeSliderHandle->onMouseDown = [this](AlloyContext* context, const InputEvent& e) {return this->onMouseDown(context, timeSliderHandle.get(), e);};
		timeSliderHandle->onMouseDrag = [this](AlloyContext* context, const InputEvent& e) {return this->onMouseDrag(context, timeSliderHandle.get(), e);};
		lowerSliderHandle->onMouseDown = [this](AlloyContext* context, const InputEvent& e) {return this->onMouseDown(context, lowerSliderHandle.get(), e);};
		lowerSliderHandle->onMouseDrag = [this](AlloyContext* context, const InputEvent& e) {return this->onMouseDrag(context, lowerSliderHandle.get(), e);};
		upperSliderHandle->onMouseDown = [this](AlloyContext* context, const InputEvent& e) {return this->onMouseDown(context, upperSliderHandle.get(), e);};
		upperSliderHandle->onMouseDrag = [this](AlloyContext* context, const InputEvent& e) {return this->onMouseDrag(context, upperSliderHandle.get(), e);};
		rangeSliderTrack->position = CoordPerPX(0.0f, 0.5f, 0.0f, -0.5f*handleSize);
		rangeSliderTrack->dimensions = CoordPerPX(1.0f, 0.0f, 0.0f, handleSize);
	
		CompositePtr timeSliderTrack=CompositePtr(new Composite("Time Track", CoordPerPX(0.0f, 0.5f, 0.0f, -1.5f*handleSize), CoordPerPX(1.0f, 0.0f, 0.0f, handleSize)));
		timeSliderTrack->add(timeSliderHandle);
		rangeSliderTrack->position = CoordPerPX(0.0f, 0.5f, 0.0f, -0.5f*handleSize);
		rangeSliderTrack->dimensions = CoordPerPX(1.0f, 0.0f, 0.0f, handleSize);
		add(rangeSliderTrack);
		add(timeSliderTrack);
		add(minValueLabel);
		add(maxValueLabel);
		this->onPack = [this]() {
			this->setTimeValue(timePosition);
			this->setRange(sliderPosition);
			if(requestUpdate&&!AlloyApplicationContext()->isMouseDown()){
				setTimeValue(getTimeValue().toDouble());
				setLowerValue(getLowerValue().toDouble());
				setUpperValue(getUpperValue().toDouble());
				update();
				requestUpdate=false;
			}
		};
		this->onEvent =
			[this](AlloyContext* context, const InputEvent& event) {
			if (event.type == InputType::Scroll&&isVisible() && context->isMouseContainedIn(this)) {
				double oldV = getBlendValue();
				double newV = clamp(event.scroll.y*0.1 + oldV, (0.0), (1.0));
				if (newV != oldV) {
					this->setBlendValue(newV);
					if (onChangeEvent)onChangeEvent(this->timeValue,this->lowerValue, this->upperValue);
					requestUpdate=true;
					context->requestPack();
					return true;
				}
			}
			return false;
		};


		setLowerValue(lowerValue.toDouble());
		setUpperValue(upperValue.toDouble());
		setTimeValue(timeValue.toDouble());
		Application::addListener(this);
	}
	void TimelineSlider::drawLabel(const std::string& label, pixel2 pos, const box2px& bounds, bool clampLeft, bool clampRight, AlloyContext* context) {
		NVGcontext* nvg = context->nvgContext;
		float th = 0.75f*handleSize;
		nvgTextAlign(nvg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgFontFaceId(nvg, context->getFontHandle(FontType::Normal));
		nvgFontSize(nvg, th);
		float tw = 2 + nvgTextBounds(nvg, 0, 0, label.c_str(), nullptr, nullptr);
		float x = pos.x;
		float y = pos.y;
		if (clampLeft&&!clampRight) {
			x -= tw;
		}
		else 	if (clampLeft&&clampRight) {
			x -= 0.5f*tw;
		}
		if (clampLeft || bounds.dimensions.x >= tw) {
			x = std::max(x, bounds.position.x);
		}
		if (clampRight || bounds.dimensions.x >= tw) {
			x = std::min(x + tw, bounds.position.x + bounds.dimensions.x) - tw;
		}
		if (clampLeft&&clampRight) {
			nvgFillColor(nvg, *backgroundColor);
		}
		else {
			nvgFillColor(nvg, backgroundColor->toSemiTransparent(0.5f));
		}
		nvgBeginPath(nvg);
		nvgRoundedRect(nvg, x, y, tw, th, 2);
		nvgFill(nvg);
		drawText(nvg, pixel2(x + 1, y), label, FontStyle::Outline, context->theme.LIGHT, *backgroundColor);
	}
	void TimelineSlider::draw(AlloyContext* context) {
		NVGcontext* nvg = context->nvgContext;
		box2px bounds = getBounds();
		float w = bounds.dimensions.x;
		float h = bounds.dimensions.y;
		pixel lineWidth = borderWidth.toPixels(bounds.dimensions.y, context->dpmm.y,
			context->pixelRatio);
		if (backgroundColor->a > 0) {
			nvgBeginPath(nvg);
			if (roundCorners) {
				nvgRoundedRect(nvg, bounds.position.x, bounds.position.y,
					bounds.dimensions.x, bounds.dimensions.y,
					context->theme.CORNER_RADIUS);
			}
			else {
				nvgRect(nvg, bounds.position.x, bounds.position.y,
					bounds.dimensions.x, bounds.dimensions.y);
			}
			nvgFillColor(nvg, *backgroundColor);
			nvgFill(nvg);
		}
		if (minorTick > 0) {
			nvgStrokeColor(nvg, context->theme.LIGHT);
			nvgLineCap(nvg, NVG_SQUARE);
			nvgStrokeWidth(nvg, 1.0f);
			box2px tbounds = rangeSliderTrack->getBounds();
			box2px hbounds = timeSliderHandle->getBounds();
			double range = maxValue.toDouble() - minValue.toDouble();
			float w = tbounds.dimensions.x - hbounds.dimensions.x;
			double delta=minorTick*w/range;
			if(delta>2.0f){
				float xoff = tbounds.position.x + 0.5f*hbounds.dimensions.x;
				double xst =  std::floor(minValue.toDouble() / minorTick)*minorTick-minValue.toDouble();
				nvgBeginPath(nvg);
				for (double tick = 0.0;xst+tick <= range;tick += minorTick) {
					if(xst+tick<0)continue;
					float x = xoff + (float)(w*(xst+tick) / range);
					float y = bounds.position.y + 0.5f*bounds.dimensions.y;
					nvgMoveTo(nvg, x, y);
					nvgLineTo(nvg, x, y - 0.125f*bounds.dimensions.y);
				}
				nvgStroke(nvg);
			}
		}
		if (majorTick > 0) {
			nvgStrokeColor(nvg, context->theme.LIGHT);
			nvgLineCap(nvg, NVG_SQUARE);
			nvgStrokeWidth(nvg, 2.0f);
			box2px tbounds = rangeSliderTrack->getBounds();
			box2px hbounds = timeSliderHandle->getBounds();
			double range = maxValue.toDouble() - minValue.toDouble();
			float w = tbounds.dimensions.x - hbounds.dimensions.x;
			double delta=majorTick*w/range;
			if(delta>2.0f){
				float xoff = tbounds.position.x + 0.5f*hbounds.dimensions.x;
				double xst = std::floor(minValue.toDouble() / majorTick)*majorTick-minValue.toDouble();
				nvgBeginPath(nvg);
				for (double tick = 0.0;xst+tick <= range;tick += majorTick) {
					if(xst+tick<0)continue;
					float x = xoff + (float)(w*(xst+tick) / range);
					float y = bounds.position.y + 0.5f*bounds.dimensions.y;
					nvgMoveTo(nvg, x, y);
					nvgLineTo(nvg, x, y - 0.25f*bounds.dimensions.y);
				}
				nvgStroke(nvg);
			}
		}

		box2px tbbox = timeSliderHandle->getBounds();
		{
			
			box2px bbox = lowerSliderHandle->getBounds();
			drawLabel(labelFormatter(lowerValue), pixel2(bbox.position.x + bbox.dimensions.x, bbox.position.y - 0.75f*handleSize), box2px(bounds.position,aly::min(bounds.dimensions,tbbox.position-bounds.position)),true,false, context);
		}
		{
			box2px bbox = upperSliderHandle->getBounds();
			drawLabel(labelFormatter(upperValue), pixel2(bbox.position.x, bbox.position.y - 0.75f*handleSize), box2px(tbbox.position+tbbox.dimensions, aly::min(bounds.dimensions, bounds.position+bounds.dimensions- (tbbox.position + tbbox.dimensions))),false,true, context);
		}
		for (std::shared_ptr<Region>& region : children) {
			if (region->isVisible()) {
				region->draw(context);
			}
		}
		if (context->isMouseOver(maxValueLabel.get(), true)|| context->isFocused(maxValueLabel.get())) {
			drawLabel(labelFormatter(timeValue), pixel2(tbbox.position.x + tbbox.dimensions.x*0.5f, tbbox.position.y + tbbox.dimensions.y + handleSize), box2px(bounds.position, bounds.dimensions - pixel2( (entryWidth + 0.5f*trackPadding))), true, true, context);
		}
		else if (context->isMouseOver(minValueLabel.get(), true) || context->isFocused(minValueLabel.get())) {
			drawLabel(labelFormatter(timeValue), pixel2(tbbox.position.x + tbbox.dimensions.x*0.5f, tbbox.position.y + tbbox.dimensions.y + handleSize), box2px(bounds.position + pixel2(entryWidth + 0.5f*trackPadding, 0.0f), bounds.dimensions - pixel2( (entryWidth + 0.5f*trackPadding))), true, true, context);
		}
		else {
			drawLabel(labelFormatter(timeValue), pixel2(tbbox.position.x + tbbox.dimensions.x*0.5f, tbbox.position.y + tbbox.dimensions.y + handleSize), bounds, true, true, context);
		}
		if (verticalScrollTrack.get() != nullptr) {
			if (isScrollEnabled()) {
				if (extents.dimensions.y > h) {
					verticalScrollTrack->draw(context);
					verticalScrollHandle->draw(context);
				}
				else {
					verticalScrollTrack->draw(context);
				}
				if (extents.dimensions.x > w) {
					horizontalScrollTrack->draw(context);
					horizontalScrollHandle->draw(context);
				}
				else {
					horizontalScrollTrack->draw(context);
				}
			}
		}
		if (borderColor->a > 0) {

			nvgLineJoin(nvg, NVG_ROUND);
			nvgBeginPath(nvg);
			if (roundCorners) {
				nvgRoundedRect(nvg, bounds.position.x + lineWidth * 0.5f,
					bounds.position.y + lineWidth * 0.5f,
					bounds.dimensions.x - lineWidth,
					bounds.dimensions.y - lineWidth,
					context->theme.CORNER_RADIUS);
			}
			else {
				nvgRect(nvg, bounds.position.x + lineWidth * 0.5f,
					bounds.position.y + lineWidth * 0.5f,
					bounds.dimensions.x - lineWidth,
					bounds.dimensions.y - lineWidth);
			}
			nvgStrokeColor(nvg, *borderColor);
			nvgStrokeWidth(nvg, lineWidth);
			nvgStroke(nvg);
			nvgLineJoin(nvg, NVG_MITER);
		}

	}
	void TimelineSlider::setBlendValue(double value) {
		value = clamp(value, 0.0, 1.0);
		setTimeValue(
			value * (maxValue.toDouble() - minValue.toDouble())
			+ minValue.toDouble());
	}

	double TimelineSlider::getBlendValue() const {
		return (timePosition - minValue.toDouble())
			/ (maxValue.toDouble() - minValue.toDouble());
	}
	bool TimelineSlider::onMouseDown(AlloyContext* context, Region* region,
		const InputEvent& event) {
		if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
			if (region == rangeSliderTrack.get()) {
				if(modifiable){
					float dl = distanceSqr(event.cursor, lowerSliderHandle->getBounds().center());
					float du = distanceSqr(event.cursor, upperSliderHandle->getBounds().center());
					float dt= distanceSqr(event.cursor, timeSliderHandle->getBounds().center());
					if (dt < dl&&dt < du) {
						timeSliderHandle->setDragOffset(event.cursor, timeSliderHandle->getBoundsDimensions() * 0.5f);
						context->setDragObject(timeSliderHandle.get());
					}
					else {
						if (dl<du) {
							lowerSliderHandle->setDragOffset(event.cursor, lowerSliderHandle->getBoundsDimensions() * 0.5f);
							context->setDragObject(lowerSliderHandle.get());
						}
						else {
							upperSliderHandle->setDragOffset(event.cursor, upperSliderHandle->getBoundsDimensions() * 0.5f);
							context->setDragObject(upperSliderHandle.get());
						}
					}
				} else {
					float dl =( event.cursor.x-lowerSliderHandle->getBounds().center().x);
					float du = (upperSliderHandle->getBounds().center().x-event.cursor.x);
					if(dl>=0&&du>=0){
						timeSliderHandle->setDragOffset(event.cursor, timeSliderHandle->getBoundsDimensions() * 0.5f);
						context->setDragObject(timeSliderHandle.get());
					}
				}
				update();
				if (onChangeEvent)
					onChangeEvent(timeValue,lowerValue, upperValue);
				return true;

			}
			else if (region == lowerSliderHandle.get()&&modifiable) {
				update();
				if (onChangeEvent)
					onChangeEvent(timeValue, lowerValue, upperValue);
				return true;
			}
			else if (region == upperSliderHandle.get()&&modifiable) {
				update();
				if (onChangeEvent)
					onChangeEvent(timeValue, lowerValue, upperValue);
				return true;
			}
		}
		return false;
	}
	bool TimelineSlider::onMouseDrag(AlloyContext* context, Region* region,
		const InputEvent& event) {
		if (region == timeSliderHandle.get()) {
			region->setDragOffset(event.cursor,
				context->getRelativeCursorDownPosition());
			update();
			if(modifiable){
				if (timePosition < sliderPosition.x) {
					setLowerValue(timePosition);
				}
				if (timePosition > sliderPosition.y) {
					setUpperValue(timePosition);
				}
			} else {
				if (timePosition < sliderPosition.x) {
					timePosition=sliderPosition.x;
				}
				if (timePosition > sliderPosition.y) {
					timePosition=sliderPosition.y;
				}
			}
			if (onChangeEvent)
				onChangeEvent(timeValue,lowerValue, upperValue);
			requestUpdate=true;
			return true;
		} else if (region == lowerSliderHandle.get()&&modifiable) {
			region->setDragOffset(event.cursor,
				context->getRelativeCursorDownPosition());
			update();
			if (sliderPosition.x > sliderPosition.y) {
				setUpperValue(sliderPosition.x);
			}
			if (timePosition < sliderPosition.x) {
				setTimeValue(sliderPosition.x);
			}
			if (onChangeEvent)
				onChangeEvent(timeValue, lowerValue, upperValue);
			requestUpdate=true;
			return true;
		}
		else if (region == upperSliderHandle.get()&&modifiable) {
			region->setDragOffset(event.cursor,
				context->getRelativeCursorDownPosition());
			update();
			if (sliderPosition.x > sliderPosition.y) {
				setLowerValue(sliderPosition.y);
			}
			if (timePosition > sliderPosition.y) {
				setTimeValue(sliderPosition.y);
			}
			if (onChangeEvent)
				onChangeEvent(timeValue, lowerValue, upperValue);
			requestUpdate=true;
			return true;
		}
		return false;
	}
	void TimelineSlider::update() {

		double interpLo = (lowerSliderHandle->getBoundsPositionX() - rangeSliderTrack->getBoundsPositionX()) / (double)(rangeSliderTrack->getBoundsDimensionsX() - 2 * lowerSliderHandle->getBoundsDimensionsX());
		double val = (double)((1.0 - interpLo) * minValue.toDouble() + interpLo * maxValue.toDouble());
		sliderPosition.x = val;
		lowerValue.setValue(clamp(val, minValue.toDouble(), maxValue.toDouble()));

		double interpHi = (upperSliderHandle->getBoundsPositionX() - rangeSliderTrack->getBoundsPositionX() - upperSliderHandle->getBoundsDimensionsX()) / (double)(rangeSliderTrack->getBoundsDimensionsX() - 2 * upperSliderHandle->getBoundsDimensionsX());
		val = (double)((1.0 - interpHi) * minValue.toDouble() + interpHi * maxValue.toDouble());
		sliderPosition.y = val;
		upperValue.setValue(clamp(val, minValue.toDouble(), maxValue.toDouble()));
		
		double interpT= (timeSliderHandle->getBoundsPositionX() - rangeSliderTrack->getBoundsPositionX()) / (double)(rangeSliderTrack->getBoundsDimensionsX() - timeSliderHandle->getBoundsDimensionsX());
		val = (double)((1.0 - interpT) * minValue.toDouble() + interpT * maxValue.toDouble());
		timePosition = val;
		rangeSliderTrack->setCurrent((timeSliderHandle->getBoundsPositionX()+0.5f*timeSliderHandle->getBoundsDimensionsX() - rangeSliderTrack->getBoundsPositionX()) / (rangeSliderTrack->getBoundsDimensionsX()));
		rangeSliderTrack->setLower((lowerSliderHandle->getBoundsPositionX() - rangeSliderTrack->getBoundsPositionX() + 0.5f*lowerSliderHandle->getBoundsDimensionsX()) / rangeSliderTrack->getBoundsDimensionsX());
		rangeSliderTrack->setUpper((upperSliderHandle->getBoundsPositionX() - rangeSliderTrack->getBoundsPositionX() + 0.5f*upperSliderHandle->getBoundsDimensionsX()) / rangeSliderTrack->getBoundsDimensionsX());
	}
	void TimelineSlider::setTimeValue(double value) {
		double interp = clamp(
			(value - minValue.toDouble())
			/ (maxValue.toDouble() - minValue.toDouble()), 0.0, 1.0);
		float xoff = (float)(rangeSliderTrack->getBoundsPositionX()	+ interp * (rangeSliderTrack->getBoundsDimensionsX()- timeSliderHandle->getBoundsDimensionsX()));
		timeSliderHandle->setDragOffset(
			pixel2(xoff, timeSliderHandle->getBoundsDimensionsY()),
			pixel2(0.0f, 0.0f));
		timePosition = value;
		timeValue.setValue(clamp(value, minValue.toDouble(), maxValue.toDouble()));
		rangeSliderTrack->setCurrent((timeSliderHandle->getBoundsPositionX() - rangeSliderTrack->getBoundsPositionX() + 0.5f*timeSliderHandle->getBoundsDimensionsX()) / rangeSliderTrack->getBoundsDimensionsX());
		if(modifiable){
			if (timeValue.toDouble() < lowerValue.toDouble()) {
				setLowerValue(timeValue.toDouble());
			}
			if (timeValue.toDouble() > upperValue.toDouble()) {
				setUpperValue(timeValue.toDouble());
			}
		} else {
			if (timeValue.toDouble() < lowerValue.toDouble()) {
				timeValue.setValue(lowerValue.toDouble());
			}
			if (timeValue.toDouble() > upperValue.toDouble()) {
				timeValue.setValue(upperValue.toDouble());
			}
		}
	}
	void TimelineSlider::setModifiable(bool l){
		modifiable=l;
		minValueLabel->setModifiable(modifiable);
		maxValueLabel->setModifiable(modifiable);
	}
	void TimelineSlider::setLowerValue(double value) {
		double interp = clamp(
			(value - minValue.toDouble())
			/ (maxValue.toDouble() - minValue.toDouble()), 0.0, 1.0);
		float xoff = (float)(rangeSliderTrack->getBoundsPositionX()
			+ interp
			* (rangeSliderTrack->getBoundsDimensionsX()
				- 2 * lowerSliderHandle->getBoundsDimensionsX()));
		lowerSliderHandle->setDragOffset(
			pixel2(xoff, lowerSliderHandle->getBoundsDimensionsY()),
			pixel2(0.0f, 0.0f));
		sliderPosition.x = value;
		lowerValue.setValue(clamp(value, minValue.toDouble(), maxValue.toDouble()));
		rangeSliderTrack->setLower((lowerSliderHandle->getBoundsPositionX() - rangeSliderTrack->getBoundsPositionX() + 0.5f*lowerSliderHandle->getBoundsDimensionsX()) / rangeSliderTrack->getBoundsDimensionsX());
		if (timeValue.toDouble() < lowerValue.toDouble()) {
			setTimeValue(lowerValue.toDouble());
		}
	}
	void TimelineSlider::setUpperValue(double value) {
		double interp = clamp(
			(value - minValue.toDouble())
			/ (maxValue.toDouble() - minValue.toDouble()), 0.0, 1.0);
		float xoff = (float)(rangeSliderTrack->getBoundsPositionX()
			+ upperSliderHandle->getBoundsDimensionsX() + interp
			* (rangeSliderTrack->getBoundsDimensionsX()
				- 2 * upperSliderHandle->getBoundsDimensionsX()));
		upperSliderHandle->setDragOffset(
			pixel2(xoff, upperSliderHandle->getBoundsDimensionsY()),
			pixel2(0.0f, 0.0f));
		sliderPosition.y = value;
		upperValue.setValue(clamp(value, minValue.toDouble(), maxValue.toDouble()));
		rangeSliderTrack->setUpper((upperSliderHandle->getBoundsPositionX() - rangeSliderTrack->getBoundsPositionX() + 0.5f*upperSliderHandle->getBoundsDimensionsX()) / rangeSliderTrack->getBoundsDimensionsX());
		if (timeValue.toDouble() > upperValue.toDouble()) {
			setTimeValue(upperValue.toDouble());
		}
	}
	void TimelineSlider::setMinValue(double value) {
		minValue.setValue(value);
		minValueLabel->setNumberValue(minValue);
		if (minValue > maxValue) {
			setMaxValue(value);
		}
		if (minValue > lowerValue) {
			setLowerValue(value);
		}
	}
	void TimelineSlider::setMaxValue(double value) {
		maxValue.setValue(value);
		maxValueLabel->setNumberValue(maxValue);
		if (maxValue < minValue) {
			setMinValue(value);
		}
		if (maxValue < upperValue) {
			setUpperValue(value);
		}
	}
	void TimelineSlider::setRange(double2 value) {

		double interp = clamp(
			(value.x - minValue.toDouble())
			/ (maxValue.toDouble() - minValue.toDouble()), 0.0, 1.0);
		float xoff = (float)(rangeSliderTrack->getBoundsPositionX()
			+ interp
			* (rangeSliderTrack->getBoundsDimensionsX()
				- 2 * lowerSliderHandle->getBoundsDimensionsX()));
		lowerSliderHandle->setDragOffset(
			pixel2(xoff, lowerSliderHandle->getBoundsDimensionsY()),
			pixel2(0.0f, 0.0f));
		interp = clamp(
			(value.y - minValue.toDouble())
			/ (maxValue.toDouble() - minValue.toDouble()), 0.0, 1.0);
		xoff = (float)(rangeSliderTrack->getBoundsPositionX()
			+ upperSliderHandle->getBoundsDimensionsX()
			+ interp
			* (rangeSliderTrack->getBoundsDimensionsX()
				- 2 * upperSliderHandle->getBoundsDimensionsX()));
		upperSliderHandle->setDragOffset(
			pixel2(xoff, upperSliderHandle->getBoundsDimensionsY()),
			pixel2(0.0f, 0.0f));
		sliderPosition = value;
		lowerValue.setValue(clamp(value.x, minValue.toDouble(), maxValue.toDouble()));
		upperValue.setValue(clamp(value.y, minValue.toDouble(), maxValue.toDouble()));
		rangeSliderTrack->setLower((lowerSliderHandle->getBoundsPositionX() - rangeSliderTrack->getBoundsPositionX() + 0.5f*lowerSliderHandle->getBoundsDimensionsX()) / rangeSliderTrack->getBoundsDimensionsX());
		rangeSliderTrack->setUpper((upperSliderHandle->getBoundsPositionX() - rangeSliderTrack->getBoundsPositionX() + 0.5f*upperSliderHandle->getBoundsDimensionsX()) / rangeSliderTrack->getBoundsDimensionsX());

	}
}
