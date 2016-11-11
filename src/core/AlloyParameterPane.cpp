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
#include "AlloyParameterPane.h"
#include "AlloyFileUtil.h"
#include "AlloyColorSelector.h"

namespace aly {
	const float ParameterPane::SPACING = 1.0f;
	void ParameterPane::pack(const pixel2& pos, const pixel2& dims, const double2& dpmm, double pixelRatio, bool clamp) {
		updateGroups();
		Composite::pack(pos, dims, dpmm, pixelRatio, clamp);
	}
	void ParameterPane::updateGroups() {
		if (groupQueue.size() > 0) {
			if (lastRegion.get() != nullptr) {
				for (RegionPtr region : groupQueue) {
					lastRegion->add(region);
					region->backgroundColor = MakeColor(AlloyDefaultContext()->theme.DARKER);
				}
				groupQueue.clear();
				CompositePtr eregion = expandBar->addRegion(lastRegion, estimatedHeight - SPACING, lastExpanded);
				lastRegion->setOrientation(Orientation::Vertical, pixel2(0.0f, SPACING), pixel2(0.0f));
				lastRegion->backgroundColor = MakeColor(AlloyDefaultContext()->theme.DARKEST);
				lastRegion->borderColor = MakeColor(0, 0, 0, 0);
				lastRegion->setRoundCorners(false);
				estimatedHeight = 0;
			} else {
				CompositePtr comp = CompositePtr(new Composite("Parameters", CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 1.0f,(isScrollEnabled())? -Composite::scrollBarSize:0.0f, 0.0f)));
				for (RegionPtr region : groupQueue) {
					comp->add(region);
				}
				Composite::add(comp);
				comp->setOrientation(Orientation::Vertical, pixel2(0.0f, SPACING), pixel2(0.0f));
				groupQueue.clear();
			}
		}
	}
	CompositePtr ParameterPane::addGroup(const std::string& name, bool expanded) {
		if (lastRegion.get() != nullptr && groupQueue.size() > 0) {
			for (RegionPtr region : groupQueue) {
				lastRegion->add(region);
				region->backgroundColor = MakeColor(AlloyDefaultContext()->theme.DARKER);
			}
			groupQueue.clear();
			CompositePtr eregion = expandBar->addRegion(lastRegion, estimatedHeight - SPACING, lastExpanded);

			lastRegion->setOrientation(Orientation::Vertical, pixel2(0.0f, SPACING), pixel2(0.0f));
			lastRegion->backgroundColor = MakeColor(AlloyDefaultContext()->theme.DARKEST);
			lastRegion->borderColor = MakeColor(0, 0, 0, 0);
			lastRegion->setRoundCorners(false);
			estimatedHeight = 0;
		}
		lastRegion = CompositePtr(new Composite(name, CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f)));
		lastExpanded = expanded;
		if (expandBar.get() == nullptr) {
			setAlwaysShowVerticalScrollBar(false);
			setScrollEnabled(false);
			expandBar = ExpandBarPtr(new ExpandBar("Parameters", CoordPX(0.0f, 0.0f), CoordPercent(1.0f, 1.0f)));
			expandBar->backgroundColor = MakeColor(0, 0, 0, 0);
			Composite::add(expandBar);
		}
		return lastRegion;
	}
	void ParameterPane::clear(){
		Composite::clear();
		expandBar.reset();
		lastRegion.reset();
		groupQueue.clear();
		values.clear();
		lastExpanded=false;
		estimatedHeight=0;
	}
	ParameterPane::ParameterPane(const std::string& name, const AUnit2D& pos, const AUnit2D& dim, float entryHeight) :
			Composite(name, pos, dim), entryHeight(entryHeight) {
		backgroundColor = MakeColor(AlloyDefaultContext()->theme.DARKEST);
		entryBackgroundColor = MakeColor(AlloyDefaultContext()->theme.DARK);
		entryBorderColor = MakeColor(AlloyDefaultContext()->theme.LIGHT);
		entryTextColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
		entryBorderWidth = UnitPX(0.0f);
		setAlwaysShowVerticalScrollBar(true);
		setScrollEnabled(true);
	}
	void ParameterPane::setCommonParameters(const CompositePtr& compRegion, const TextLabelPtr& textLabel, const RegionPtr& valueRegion) {
		valueRegion->setRoundCorners(true);
		compRegion->backgroundColor = entryBackgroundColor;
		compRegion->borderColor = entryBorderColor;
		compRegion->borderWidth = entryBorderWidth;
		valueRegion->backgroundColor = MakeColor(0, 0, 0, 0);
		valueRegion->borderWidth = UnitPX(0.0f);
		textLabel->textColor = entryTextColor;
		textLabel->fontSize = UnitPX(entryHeight - 2);
		compRegion->add(textLabel);
		compRegion->add(valueRegion);
		groupQueue.push_back(compRegion);
	}
	ModifiableNumberPtr ParameterPane::addNumberField(const std::string& label, Number& value, const Number& minValue, const Number& maxValue, float aspect) {
		CompositePtr comp = CompositePtr(new Composite(label + "_param", CoordPX(0, 0), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		TextLabelPtr labelRegion = TextLabelPtr(new TextLabel(label, CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		const float numAspect = 2.0f;
		if (aspect > 0) {
			aspect = std::max(numAspect, aspect);
		}
		ModifiableNumberPtr valueRegion = ModifiableNumberPtr(
				new ModifiableNumber(label+"_field", CoordPerPX(1.0f, 0.0f, -numAspect * entryHeight, 0.0f), CoordPX(numAspect * entryHeight, entryHeight),
						value.type()));
		HorizontalSliderPtr tweenRegion = HorizontalSliderPtr(
				new HorizontalSlider(label+"_tween", CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 0.0f), CoordPX((aspect - numAspect) * entryHeight, entryHeight),
						false, minValue, maxValue, value));
		valueRegion->setAlignment(HorizontalAlignment::Center, VerticalAlignment::Middle);
		valueRegion->fontSize = UnitPX(entryHeight - 6.0f);
		if (aspect <= 0) {
			pixel2 labelBounds = labelRegion->getTextDimensions(AlloyDefaultContext().get());
			labelBounds.x += 10;
			tweenRegion->position = CoordPX(labelBounds.x, 0.0f);
			tweenRegion->dimensions = CoordPerPX(1.0f, 0.0f, -labelBounds.x - numAspect * entryHeight, entryHeight);
		} else {
			labelRegion->position = CoordPX(0.0f, 0.0f);
			labelRegion->dimensions = CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, entryHeight);
		}
		valueRegion->setNumberValue(value);
		std::shared_ptr<AnyInterface> ref = std::shared_ptr<AnyInterface>(new AnyValue<Number*>(&value));
		values.push_back(ref);
		valueRegion->onTextEntered = [=](NumberField* field) {
			Number val = field->getValue();
			if (val < minValue) {
				val = minValue;
				field->setNumberValue(val);
			}
			if (val > maxValue) {
				val = maxValue;
				field->setNumberValue(val);
			}
			tweenRegion->setValue(val.toDouble());
			*(ref->getValue<Number*>()) = val;
			if(this->onChange)this->onChange(label,*ref);
		};
		tweenRegion->setOnChangeEvent([=](const aly::Number& value) {
			valueRegion->setNumberValue(value);
			*(ref->getValue<Number*>()) =value;
			if(this->onChange)this->onChange(label,*ref);
		});
		setCommonParameters(comp, labelRegion, valueRegion);

		valueRegion->textColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
		valueRegion->backgroundColor = MakeColor(0, 0, 0, 0);
		tweenRegion->backgroundColor = MakeColor(0, 0, 0, 0);
		tweenRegion->borderWidth = UnitPX(0.0f);
		comp->add(tweenRegion);
		estimatedHeight += entryHeight + SPACING;
		return valueRegion;

	}
	NumberFieldPtr ParameterPane::addNumberField(const std::string& label, Number& value, float aspect) {
		CompositePtr comp = CompositePtr(new Composite(label + "_param", CoordPX(0, 0), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		TextLabelPtr labelRegion = TextLabelPtr(new TextLabel(label, CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		NumberFieldPtr valueRegion = NumberFieldPtr(
				new NumberField(label, CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 0.0f), CoordPX(aspect * entryHeight, entryHeight), value.type()));
		if (aspect <= 0) {
			pixel2 labelBounds = labelRegion->getTextDimensions(AlloyDefaultContext().get());
			labelBounds.x += 10;
			valueRegion->position = CoordPX(labelBounds.x, 0.0f);
			valueRegion->dimensions = CoordPerPX(1.0f, 0.0f, -labelBounds.x, entryHeight);
		} else {
			labelRegion->position = CoordPX(0.0f, 0.0f);
			labelRegion->dimensions = CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, entryHeight);
		}
		valueRegion->textColor = MakeColor(AlloyDefaultContext()->theme.DARKER);
		valueRegion->setNumberValue(value);

		std::shared_ptr<AnyInterface> ref = std::shared_ptr<AnyInterface>(new AnyValue<Number*>(&value));
		values.push_back(ref);
		valueRegion->onTextEntered = [=](NumberField* field) {
			*(ref->getValue<Number*>()) = field->getValue();
			if(this->onChange)this->onChange(label,*ref);
		};
		setCommonParameters(comp, labelRegion, valueRegion);
		valueRegion->backgroundColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
		valueRegion->setRoundCorners(true);
		estimatedHeight += entryHeight + SPACING;
		return valueRegion;
	}

	std::shared_ptr<Region> ParameterPane::addNumberVectorField(const std::string& label,std::vector<Number>& value,const NumberType& type,float aspect) {
		CompositePtr comp = CompositePtr(new Composite(label + "_param", CoordPX(0, 0), CoordPerPX(1.0f, 0.0f, 0.0f, 4 * entryHeight)));
		TextLabelPtr labelRegion = TextLabelPtr(new TextLabel(label, CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		NumberListBoxPtr valueRegion = NumberListBoxPtr(new NumberListBox("Number Field", CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 0.0f), CoordPX(aspect * entryHeight, 4 * entryHeight),type,entryHeight));
		if (aspect <= 0) {
			pixel2 labelBounds = labelRegion->getTextDimensions(AlloyDefaultContext().get());
			labelBounds.x += 10;
			valueRegion->position = CoordPX(labelBounds.x, 0.0f);
			valueRegion->dimensions = CoordPerPX(1.0f, 0.0f, -labelBounds.x, 4 * entryHeight);
		} else {
			labelRegion->position = CoordPX(0.0f, 0.0f);
			labelRegion->dimensions = CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 4 * entryHeight);
		}
		std::shared_ptr<AnyInterface> ref = std::shared_ptr<AnyInterface>(new AnyValue<std::vector<Number>*>(&value));
		valueRegion->addNumbers(value);
		values.push_back(ref);
		valueRegion->onChange = [=](const std::vector<Number>& value) {
			*(ref->getValue<std::vector<Number>*>()) = value;
			if(this->onChange)this->onChange(label,*ref);
		};
		setCommonParameters(comp, labelRegion, valueRegion);
		valueRegion->backgroundColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
		valueRegion->setRoundCorners(true);
		estimatedHeight += 4 * entryHeight + SPACING;
		return valueRegion;
	}
	NumberFieldPtr ParameterPane::addNumberFieldItem(const std::string& label, Number& value, float aspect) {
		CompositePtr comp = CompositePtr(new Composite(label + "_param", CoordPX(0, 0), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		TextLabelPtr labelRegion = TextLabelPtr(new TextLabel(label, CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		NumberFieldPtr valueRegion = NumberFieldPtr(
				new NumberField(label, CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 0.0f), CoordPX(aspect * entryHeight, entryHeight), value.type()));
		if (aspect <= 0) {
			pixel2 labelBounds = labelRegion->getTextDimensions(AlloyDefaultContext().get());
			labelBounds.x += 10;
			valueRegion->position = CoordPX(labelBounds.x, 0.0f);
			valueRegion->dimensions = CoordPerPX(1.0f, 0.0f, -labelBounds.x, entryHeight);
		} else {
			labelRegion->position = CoordPX(0.0f, 0.0f);
			labelRegion->dimensions = CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, entryHeight);
		}
		valueRegion->textColor = MakeColor(AlloyDefaultContext()->theme.DARKER);
		valueRegion->setNumberValue(value);

		std::shared_ptr<AnyInterface> ref = std::shared_ptr<AnyInterface>(new AnyValue<Number*>(&value));
		values.push_back(ref);
		valueRegion->onTextEntered = [=](NumberField* field) {
			*(ref->getValue<Number*>()) = field->getValue();
			if(this->onChange)this->onChange(label,*ref);
		};
		setCommonParameters(comp, labelRegion, valueRegion);
		valueRegion->backgroundColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
		valueRegion->setRoundCorners(true);
		estimatedHeight += entryHeight + SPACING;
		return valueRegion;
	}
	std::pair<ModifiableNumberPtr, ModifiableNumberPtr> ParameterPane::addRangeField(const std::string& label, Number& lowerValue, Number& upperValue,
			const Number& minValue, const Number& maxValue, float aspect) {
		float numAspect = 2.0f;
		CompositePtr comp = CompositePtr(new Composite(label + "_param", CoordPX(0, 0), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		CompositePtr valueRegion = CompositePtr(
				new Composite("Value container", CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 0.0f), CoordPX(aspect * entryHeight, entryHeight)));
		TextLabelPtr labelRegion = TextLabelPtr(new TextLabel(label, CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		ModifiableNumberPtr lowValueRegion = ModifiableNumberPtr(
				new ModifiableNumber(label, CoordPercent(0.0f, 0.0f), CoordPX(numAspect * entryHeight, entryHeight), lowerValue.type()));
		ModifiableNumberPtr highValueRegion = ModifiableNumberPtr(
				new ModifiableNumber(label, CoordPerPX(1.0f, 0.0f, -numAspect * entryHeight, 0.0f), CoordPX(numAspect * entryHeight, entryHeight),
						upperValue.type()));
		RangeSliderPtr rangeSlider = RangeSliderPtr(
				new RangeSlider(label, CoordPX(numAspect * entryHeight, 0.0f), CoordPerPX(1.0f, 0.0f, -2.0f * numAspect * entryHeight, entryHeight), minValue,
						maxValue, lowerValue, upperValue, false));
		valueRegion->add(lowValueRegion);
		valueRegion->add(highValueRegion);
		valueRegion->add(rangeSlider);
		lowValueRegion->setNumberValue(lowerValue);
		lowValueRegion->borderWidth = UnitPX(0.0f);
		lowValueRegion->fontSize = UnitPerPX(1.0f, -6.0f);
		lowValueRegion->textColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
		lowValueRegion->setAlignment(HorizontalAlignment::Right, VerticalAlignment::Middle);
		lowValueRegion->backgroundColor = MakeColor(0, 0, 0, 0);
		highValueRegion->borderWidth = UnitPX(0.0f);
		highValueRegion->fontSize = UnitPerPX(1.0f, -6.0f);
		highValueRegion->textColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
		highValueRegion->backgroundColor = MakeColor(0, 0, 0, 0);
		highValueRegion->setAlignment(HorizontalAlignment::Center, VerticalAlignment::Middle);
		rangeSlider->borderWidth = UnitPX(0.0f);
		rangeSlider->backgroundColor = MakeColor(0, 0, 0, 0);
		if (aspect <= 0) {
			pixel2 labelBounds = labelRegion->getTextDimensions(AlloyDefaultContext().get());
			labelBounds.x += 10;
			valueRegion->position = CoordPX(labelBounds.x, 0.0f);
			valueRegion->dimensions = CoordPerPX(1.0f, 0.0f, -labelBounds.x, 2 * entryHeight);
		} else {
			labelRegion->position = CoordPX(0.0f, 0.0f);
			labelRegion->dimensions = CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, entryHeight);
		}
		highValueRegion->setNumberValue(upperValue);

		std::shared_ptr<AnyInterface> ref = std::shared_ptr<AnyInterface>(
				new AnyValue<std::pair<Number*, Number*>>(std::pair<Number*, Number*>(&lowerValue, &upperValue)));
		values.push_back(ref);
		lowValueRegion->onTextEntered = [=](NumberField* field) {
			if (field->getValue() < minValue) {
				field->setNumberValue(minValue);
			}
			if (field->getValue() > maxValue) {
				field->setNumberValue(maxValue);
			}
			*((ref->getValue<std::pair<Number*, Number*>>()).first) = field->getValue();
			rangeSlider->setLowerValue(field->getValue().toDouble());
			if (field->getValue() > highValueRegion->getValue()) {
				*((ref->getValue<std::pair<Number*, Number*>>()).second) = field->getValue();
				highValueRegion->setNumberValue(field->getValue());
				rangeSlider->setUpperValue(field->getValue().toDouble());
			}
			if(this->onChange)this->onChange(label,*ref);
		};
		highValueRegion->onTextEntered = [=](NumberField* field) {
			if (field->getValue() < minValue) {
				field->setNumberValue(minValue);
			}
			if (field->getValue() > maxValue) {
				field->setNumberValue(maxValue);
			}
			*((ref->getValue<std::pair<Number*, Number*>>()).second) = field->getValue();
			rangeSlider->setUpperValue(field->getValue().toDouble());
			if (field->getValue() < lowValueRegion->getValue()) {
				*((ref->getValue<std::pair<Number*, Number*>>()).first) = field->getValue();
				lowValueRegion->setNumberValue(field->getValue());
				rangeSlider->setLowerValue(field->getValue().toDouble());
			}
			if(this->onChange)this->onChange(label,*ref);
		};
		rangeSlider->onChangeEvent = [=](const Number& low, const Number& high) {
			*((ref->getValue<std::pair<Number*, Number*>>()).first) = low;
			lowValueRegion->setNumberValue(low);
			*((ref->getValue<std::pair<Number*, Number*>>()).second) = high;
			highValueRegion->setNumberValue(high);
			if(this->onChange)this->onChange(label,*ref);
		};
		setCommonParameters(comp, labelRegion, valueRegion);
		valueRegion->backgroundColor = MakeColor(0, 0, 0, 0);
		estimatedHeight += entryHeight + SPACING;
		return std::pair<ModifiableNumberPtr, ModifiableNumberPtr>(lowValueRegion, highValueRegion);
	}
	std::pair<NumberFieldPtr, NumberFieldPtr> ParameterPane::addRangeField(const std::string& label, Number& lowerValue, Number& upperValue, float aspect) {
		CompositePtr comp = CompositePtr(new Composite(label + "_param", CoordPX(0, 0), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		CompositePtr valueRegion = CompositePtr(
				new Composite("Value container", CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 0.0f), CoordPX(aspect * entryHeight, entryHeight)));
		TextLabelPtr labelRegion = TextLabelPtr(new TextLabel(label, CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		NumberFieldPtr lowValueRegion = NumberFieldPtr(
				new NumberField(label, CoordPercent(0.0f, 0.0f), CoordPerPX(0.5f, 1.0f, -2.0f, 0.0f), lowerValue.type()));
		NumberFieldPtr highValueRegion = NumberFieldPtr(
				new NumberField(label, CoordPerPX(0.5f, 0.0f, 2.0f, 0.0f), CoordPerPX(0.5f, 1.0f, -2.0f, 0.0f), upperValue.type()));
		TextLabelPtr spaceRegion = TextLabelPtr(new TextLabel(":", CoordPerPX(0.5f, 0.0f, -2.0f, 0.0f), CoordPerPX(0.0f, 1.0f, 4.0f, 0.0f)));
		spaceRegion->setAlignment(HorizontalAlignment::Center, VerticalAlignment::Middle);
		spaceRegion->textColor = MakeColor(AlloyDefaultContext()->theme.DARKER);
		spaceRegion->fontType = FontType::Bold;
		spaceRegion->fontSize = UnitPerPX(1.0f, -8.0f);
		valueRegion->add(lowValueRegion);
		valueRegion->add(spaceRegion);
		valueRegion->add(highValueRegion);
		lowValueRegion->textColor = MakeColor(AlloyDefaultContext()->theme.DARKER);
		lowValueRegion->setNumberValue(lowerValue);
		lowValueRegion->backgroundColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
		lowValueRegion->borderWidth = UnitPX(0.0f);
		lowValueRegion->setRoundCorners(true);
		if (aspect <= 0) {
			pixel2 labelBounds = labelRegion->getTextDimensions(AlloyDefaultContext().get());
			labelBounds.x += 10;
			valueRegion->position = CoordPX(labelBounds.x, 0.0f);
			valueRegion->dimensions = CoordPerPX(1.0f, 0.0f, -labelBounds.x, entryHeight);
		} else {
			labelRegion->position = CoordPX(0.0f, 0.0f);
			labelRegion->dimensions = CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, entryHeight);
		}
		highValueRegion->textColor = MakeColor(AlloyDefaultContext()->theme.DARKER);
		highValueRegion->setNumberValue(upperValue);

		std::shared_ptr<AnyInterface> ref = std::shared_ptr<AnyInterface>(
				new AnyValue<std::pair<Number*, Number*>>(std::pair<Number*, Number*>(&lowerValue, &upperValue)));
		values.push_back(ref);
		lowValueRegion->onTextEntered = [=](NumberField* field) {
			*((ref->getValue<std::pair<Number*, Number*>>()).first) = field->getValue();
			if (field->getValue() > highValueRegion->getValue()) {
				*((ref->getValue<std::pair<Number*, Number*>>()).second) = field->getValue();
				highValueRegion->setNumberValue(field->getValue());
			}
			if(this->onChange)this->onChange(label,*ref);
		};
		highValueRegion->onTextEntered = [=](NumberField* field) {
			*((ref->getValue<std::pair<Number*, Number*>>()).second) = field->getValue();
			if (field->getValue() < lowValueRegion->getValue()) {
				*((ref->getValue<std::pair<Number*, Number*>>()).first) = field->getValue();
				lowValueRegion->setNumberValue(field->getValue());
			}
			if(this->onChange)this->onChange(label,*ref);
		};
		highValueRegion->backgroundColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
		highValueRegion->borderWidth = UnitPX(0.0f);
		highValueRegion->setRoundCorners(true);
		setCommonParameters(comp, labelRegion, valueRegion);
		valueRegion->backgroundColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
		estimatedHeight += entryHeight + SPACING;
		return std::pair<NumberFieldPtr, NumberFieldPtr>(lowValueRegion, highValueRegion);
	}
	TextFieldPtr ParameterPane::addTextField(const std::string& label, std::string& value, float aspect) {
		CompositePtr comp = CompositePtr(new Composite(label + "_param", CoordPX(0, 0), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		TextLabelPtr labelRegion = TextLabelPtr(new TextLabel(label, CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		TextFieldPtr valueRegion = TextFieldPtr(
				new TextField("Text", CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 0.0f), CoordPX(aspect * entryHeight, entryHeight)));
		if (aspect <= 0) {
			pixel2 labelBounds = labelRegion->getTextDimensions(AlloyDefaultContext().get());
			labelBounds.x += 10;
			valueRegion->position = CoordPX(labelBounds.x, 0.0f);
			valueRegion->dimensions = CoordPerPX(1.0f, 0.0f, -labelBounds.x, entryHeight);
		} else {
			labelRegion->position = CoordPX(0.0f, 0.0f);
			labelRegion->dimensions = CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, entryHeight);
		}
		valueRegion->textColor = MakeColor(AlloyDefaultContext()->theme.DARKER);
		valueRegion->setValue(value);
		std::shared_ptr<AnyInterface> ref = std::shared_ptr<AnyInterface>(new AnyValue<std::string*>(&value));
		values.push_back(ref);
		valueRegion->onTextEntered = [=](TextField* field) {
			*(ref->getValue<std::string*>()) = field->getValue();
		};
		setCommonParameters(comp, labelRegion, valueRegion);
		valueRegion->backgroundColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
		valueRegion->setRoundCorners(true);
		estimatedHeight += entryHeight + SPACING;
		return valueRegion;
	}

	ColorSelectorPtr ParameterPane::addColorField(const std::string& label, Color& value, float aspect) {
		CompositePtr comp = CompositePtr(new Composite(label + "_param", CoordPX(0, 0), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		TextLabelPtr labelRegion = TextLabelPtr(new TextLabel(label, CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		ColorSelectorPtr valueRegion = ColorSelectorPtr(
				new ColorSelector(label, CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 0.0f), CoordPX(aspect * entryHeight, entryHeight), false));
		if (aspect <= 0) {
			pixel2 labelBounds = labelRegion->getTextDimensions(AlloyDefaultContext().get());
			labelBounds.x += 10;
			valueRegion->position = CoordPX(labelBounds.x, 0.0f);
			valueRegion->dimensions = CoordPerPX(1.0f, 0.0f, -labelBounds.x, entryHeight);
		} else {
			labelRegion->position = CoordPX(0.0f, 0.0f);
			labelRegion->dimensions = CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, entryHeight);
		}
		valueRegion->setColor(value);
		std::shared_ptr<AnyInterface> ref = std::shared_ptr<AnyInterface>(new AnyValue<Color*>(&value));
		values.push_back(ref);
		valueRegion->onSelect = [=](const Color& c) {
			*(ref->getValue<Color*>()) = c;
			if(this->onChange)this->onChange(label,*ref);
		};
		setCommonParameters(comp, labelRegion, valueRegion);
		estimatedHeight += entryHeight + SPACING;
		return valueRegion;
	}
	SelectionPtr ParameterPane::addSelectionField(const std::string& label, int& selectedIndex, const std::vector<std::string>& options, float aspect) {
		CompositePtr comp = CompositePtr(new Composite(label + "_param", CoordPX(0, 0), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		TextLabelPtr labelRegion = TextLabelPtr(new TextLabel(label, CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		SelectionPtr valueRegion = SelectionPtr(
				new Selection(label, CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 0.0f), CoordPX(aspect * entryHeight, entryHeight), options));
		if (aspect <= 0) {
			pixel2 labelBounds = labelRegion->getTextDimensions(AlloyDefaultContext().get());
			labelBounds.x += 10;
			valueRegion->position = CoordPX(labelBounds.x, 0.0f);
			valueRegion->dimensions = CoordPerPX(1.0f, 0.0f, -labelBounds.x, entryHeight);
		} else {
			labelRegion->position = CoordPX(0.0f, 0.0f);
			labelRegion->dimensions = CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, entryHeight);
		}
		valueRegion->setSelectedIndex(selectedIndex);
		std::shared_ptr<AnyInterface> ref = std::shared_ptr<AnyInterface>(new AnyValue<int*>(&selectedIndex));
		values.push_back(ref);
		valueRegion->onSelect = [=](int selection) {
			*(ref->getValue<int*>()) = selection;
			if(this->onChange)this->onChange(label,*ref);
		};
		setCommonParameters(comp, labelRegion, valueRegion);
		valueRegion->backgroundColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
		valueRegion->setRoundCorners(true);
		estimatedHeight += entryHeight + SPACING;
		return valueRegion;
	}
	ToggleBoxPtr ParameterPane::addToggleBox(const std::string& label, bool& value, float aspect) {
		CompositePtr comp = CompositePtr(new Composite(label + "_param", CoordPX(0, 0), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		TextLabelPtr labelRegion = TextLabelPtr(new TextLabel(label, CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		ToggleBoxPtr valueRegion = ToggleBoxPtr(
				new ToggleBox(label, CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 0.0f), CoordPX(aspect * entryHeight, entryHeight), value, false));
		if (aspect <= 0) {
			pixel2 labelBounds = labelRegion->getTextDimensions(AlloyDefaultContext().get());
			labelBounds.x += 10;
			valueRegion->position = CoordPX(labelBounds.x, 0.0f);
			valueRegion->dimensions = CoordPerPX(1.0f, 0.0f, -labelBounds.x, entryHeight);
		} else {
			labelRegion->position = CoordPX(0.0f, 0.0f);
			labelRegion->dimensions = CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, entryHeight);
		}
		std::shared_ptr<AnyInterface> ref = std::shared_ptr<AnyInterface>(new AnyValue<bool*>(&value));
		values.push_back(ref);
		valueRegion->onChange = [=](bool value) {
			*(ref->getValue<bool*>()) = value;
			if(this->onChange)this->onChange(label,*ref);
		};
		setCommonParameters(comp, labelRegion, valueRegion);
		estimatedHeight += entryHeight + SPACING;
		return valueRegion;
	}
	CheckBoxPtr ParameterPane::addCheckBox(const std::string& label, bool& value, float aspect) {
		CompositePtr comp = CompositePtr(new Composite(label + "_param", CoordPX(0, 0), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		TextLabelPtr labelRegion = TextLabelPtr(new TextLabel(label, CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		CheckBoxPtr valueRegion = CheckBoxPtr(
				new CheckBox(label, CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 0.0f), CoordPX(aspect * entryHeight, entryHeight), value, false));
		if (aspect <= 0) {
			pixel2 labelBounds = labelRegion->getTextDimensions(AlloyDefaultContext().get());
			labelBounds.x += 10;
			valueRegion->position = CoordPX(labelBounds.x, 0.0f);
			valueRegion->dimensions = CoordPerPX(1.0f, 0.0f, -labelBounds.x, entryHeight);
		} else {
			labelRegion->position = CoordPX(0.0f, 0.0f);
			labelRegion->dimensions = CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, entryHeight);
		}
		std::shared_ptr<AnyInterface> ref = std::shared_ptr<AnyInterface>(new AnyValue<bool*>(&value));
		values.push_back(ref);
		valueRegion->onChange = [=](bool value) {
			*(ref->getValue<bool*>()) = value;
			if(this->onChange)this->onChange(label,*ref);
		};
		setCommonParameters(comp, labelRegion, valueRegion);
		estimatedHeight += entryHeight + SPACING;
		return valueRegion;
	}
	FileSelectorPtr ParameterPane::addFileField(const std::string& label, std::string& file, float aspect) {
		CompositePtr comp = CompositePtr(new Composite(label + "_param", CoordPX(0, 0), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		TextLabelPtr labelRegion = TextLabelPtr(new TextLabel(label, CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		FileSelectorPtr valueRegion = FileSelectorPtr(
				new FileSelector("File", CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 0.0f), CoordPX(aspect * entryHeight, entryHeight),false));
		valueRegion->setValue(file);
		if (aspect <= 0) {
			pixel2 labelBounds = labelRegion->getTextDimensions(AlloyDefaultContext().get());
			labelBounds.x += entryHeight;
			valueRegion->position = CoordPX(labelBounds.x, 0.0f);
			valueRegion->dimensions = CoordPerPX(1.0f, 0.0f, -labelBounds.x, entryHeight);
		} else {
			labelRegion->position = CoordPX(0.0f, 0.0f);
			labelRegion->dimensions = CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, entryHeight);
		}
		std::shared_ptr<AnyInterface> ref = std::shared_ptr<AnyInterface>(new AnyValue<std::string*>(&file));
		values.push_back(ref);
		valueRegion->onChange = [=](const std::string& value) {
			*(ref->getValue<std::string*>()) = value;
			if(this->onChange)this->onChange(label,*ref);
		};
		setCommonParameters(comp, labelRegion, valueRegion);
		valueRegion->backgroundColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
		valueRegion->setRoundCorners(true);
		estimatedHeight += entryHeight + SPACING;
		return valueRegion;
	}
	FileSelectorPtr ParameterPane::addDirectoryField(const std::string& label, std::string& file, float aspect) {
			CompositePtr comp = CompositePtr(new Composite(label + "_param", CoordPX(0, 0), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
			TextLabelPtr labelRegion = TextLabelPtr(new TextLabel(label, CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
			FileSelectorPtr valueRegion = FileSelectorPtr(
					new FileSelector("Directory", CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 0.0f), CoordPX(aspect * entryHeight, entryHeight),true));
			valueRegion->setValue(file);
			if (aspect <= 0) {
				pixel2 labelBounds = labelRegion->getTextDimensions(AlloyDefaultContext().get());
				labelBounds.x += 10;
				valueRegion->position = CoordPX(labelBounds.x, 0.0f);
				valueRegion->dimensions = CoordPerPX(1.0f, 0.0f, -labelBounds.x, entryHeight);
			} else {
				labelRegion->position = CoordPX(0.0f, 0.0f);
				labelRegion->dimensions = CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, entryHeight);
			}
			std::shared_ptr<AnyInterface> ref = std::shared_ptr<AnyInterface>(new AnyValue<std::string*>(&file));
			values.push_back(ref);
			valueRegion->onChange = [=](const std::string& value) {
				*(ref->getValue<std::string*>()) = value;
				if(this->onChange)this->onChange(label,*ref);
			};
			setCommonParameters(comp, labelRegion, valueRegion);
			valueRegion->backgroundColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
			valueRegion->setRoundCorners(true);
			estimatedHeight += entryHeight + SPACING;
			return valueRegion;
		}
	MultiFileSelectorPtr ParameterPane::addMultiFileSelector(const std::string& label, std::vector<std::string>& files, float aspect) {
		CompositePtr comp = CompositePtr(new Composite(label + "_param", CoordPX(0, 0), CoordPerPX(1.0f, 0.0f, 0.0f, 4 * entryHeight)));
		TextLabelPtr labelRegion = TextLabelPtr(new TextLabel(label, CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		MultiFileSelectorPtr valueRegion = MultiFileSelectorPtr(
				new MultiFileSelector("Multi-File Field", CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 0.0f), CoordPX(aspect * entryHeight, 4 * entryHeight),
						entryHeight));
		if (aspect <= 0) {
			pixel2 labelBounds = labelRegion->getTextDimensions(AlloyDefaultContext().get());
			labelBounds.x += entryHeight;
			valueRegion->position = CoordPX(labelBounds.x, 0.0f);
			valueRegion->dimensions = CoordPerPX(1.0f, 0.0f, -labelBounds.x, 4 * entryHeight);
		} else {
			labelRegion->position = CoordPX(0.0f, 0.0f);
			labelRegion->dimensions = CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 4 * entryHeight);
		}
		std::shared_ptr<AnyInterface> ref = std::shared_ptr<AnyInterface>(new AnyValue<std::vector<std::string>*>(&files));
		valueRegion->addFiles(files);
		values.push_back(ref);
		valueRegion->onChange = [=](const std::vector<std::string>& value) {
			*(ref->getValue<std::vector<std::string>*>()) = value;
			if(this->onChange)this->onChange(label,*ref);
		};
		setCommonParameters(comp, labelRegion, valueRegion);
		valueRegion->backgroundColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
		valueRegion->setRoundCorners(true);
		estimatedHeight += 4 * entryHeight + SPACING;
		return valueRegion;
	}
}
