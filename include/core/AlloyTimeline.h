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

#ifndef ALLOYTIMELINE_H_
#define ALLOYTIMELINE_H_
#include "AlloyUI.h"
#include "AlloyWidget.h"
namespace aly {
	class TimelineSlider : public Composite {
	protected:
		bool requestUpdate;
		AColor textColor;
		AUnit1D fontSize;
		Number minValue;
		Number maxValue;
		Number lowerValue;
		Number upperValue;
		Number timeValue;
		float entryWidth;
		float handleSize;
		float trackPadding;
		TextLabelPtr sliderLabel;
		std::shared_ptr<SliderHandle> lowerSliderHandle;
		std::shared_ptr<SliderHandle> upperSliderHandle;
		std::shared_ptr<SliderHandle> timeSliderHandle;
		std::shared_ptr<SliderTrack> rangeSliderTrack;
		ModifiableNumberPtr minValueLabel;
		ModifiableNumberPtr maxValueLabel;
		std::function<std::string(const Number& value)> labelFormatter;
		void update();
		double2 sliderPosition;
		double timePosition;
		double majorTick;
		double minorTick;
		bool modifiable;
		void drawLabel(const std::string& label, pixel2 pos,const box2px& bounds,bool clampLeft,bool clampRight,AlloyContext* context);
	public:
		std::function<void(const Number& timeValue,const Number& lowerValue, const Number& upperValue)> onChangeEvent;
		void setSliderColor(const Color& startColor, const Color& endColor) {
			rangeSliderTrack->startColor = startColor;
			rangeSliderTrack->endColor = endColor;
		}
		TimelineSlider(const std::string& name, const AUnit2D& pos, const AUnit2D& dims, const Number& min, const Number& max,
			const Number& timeValue);
		double getBlendValue() const;
		void setBlendValue(double value);
		void setMajorTick(double val) {
			majorTick = val;
		}
		void setMinorTick(double val) {
			minorTick = val;
		}
		void setModifiable(bool l);
		void setLowerValue(double value);
		void setUpperValue(double value);
		void setMinValue(double value);
		void setMaxValue(double value);
		void setTimeValue(double value);
		
		inline void setTimeValue(float value) {
			setTimeValue((double)value);
		}
		inline void setTimeValue(int value) {
			setTimeValue((double)value);
		}
		inline void setLowerValue(float value) {
			setLowerValue((double)value);
		}
		inline void setUpperValue(float value) {
			setUpperValue((double)value);
		}
		inline void setLowerValue(int value) {
			setLowerValue((double)value);
		}
		inline void setUpperValue(int value) {
			setUpperValue((double)value);
		}
		inline void setMinValue(float value) {
			setMinValue((double)value);
		}
		inline void setMaxValue(float value) {
			setMaxValue((double)value);
		}
		inline void setMinValue(int value) {
			setMinValue((double)value);
		}
		inline void setMaxValue(int value) {
			setMaxValue((double)value);
		}
		void setRange(double2 range);
		const Number& getTimeValue() {
			return timeValue;
		}
		const Number& getMinValue() {
			return minValue;
		}
		const Number& getMaxValue() {
			return maxValue;
		}
		const Number& getLowerValue() {
			return lowerValue;
		}
		const Number& getUpperValue() {
			return upperValue;
		}
		inline void setOnChangeEvent(
			const std::function<void(const Number& timeValue,const Number& lowerValue, const Number& upperValue)>& func) {
			onChangeEvent = func;
		}
		inline void setLabelFormatter(
			const std::function<std::string(const Number& value)>& func) {
			labelFormatter = func;
		}
		bool onMouseDown(AlloyContext* context, Region* region,
			const InputEvent& event);
		bool onMouseDrag(AlloyContext* context, Region* region,
			const InputEvent& event);
		virtual void draw(AlloyContext* context) override;
		virtual inline ~TimelineSlider() {
		}
	};

	typedef std::shared_ptr<TimelineSlider> TimelineSliderPtr;
}
#endif
