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
#ifndef ALLOYPARAMETERPANE_H_
#define ALLOYPARAMETERPANE_H_
#include "AlloyUI.h"
#include "AlloyWidget.h"
#include "AlloyExpandBar.h"
#include "AlloyColorSelector.h"
#include "AlloyAny.h"
#include <string>
#include <vector>
namespace aly {
template<int M, int N> class MatrixField: public aly::Composite {
protected:
	aly::matrix<float, M, N> Mat;
	aly::ModifiableNumberPtr matField[M][N];
public:
	std::function<void(MatrixField<M, N>*)> onTextEntered;
	MatrixField(const std::string& name, const aly::AUnit2D& pos, const aly::AUnit2D& dims, const aly::matrix<float, M, N>& Mat = aly::matrix<float, M, N>()) :
			Composite(name, pos, dims), Mat(Mat) {
		using namespace aly;
		for (int m = 0; m < M; m++) {
			for (int n = 0; n < N; n++) {
				ModifiableNumberPtr field = ModifiableNumberPtr(
						new ModifiableNumber(MakeString() << "M[" << m << "][" << n << "]", CoordPercent(n / (float) N, m / (float) M),
								CoordPercent(1.0f / M, 1.0f / N), NumberType::Float));
				field->setNumberValue(Float(Mat[m][n]));
				field->setAlignment(HorizontalAlignment::Center, VerticalAlignment::Middle);
				field->onTextEntered = [this,m,n](NumberField* field) {
					this->Mat[m][n]=field->getValue().toFloat();
					if(onTextEntered) {
						onTextEntered(this);
					}
				};
				Composite::add(field);
				matField[m][n] = field;
			}
		}
	}
	void setValue(const aly::matrix<float, M, N>& m) {
		Mat = m;
		for (int m = 0; m < M; m++) {
			for (int n = 0; n < N; n++) {
				matField[m][n]->setNumberValue(aly::Float(Mat[m][n]));
			}
		}
	}
	aly::matrix<float, M, N> getValue() const {
		return Mat;
	}
};

template<int N> class VectorFloatField: public aly::Composite {
protected:
	aly::vec<float, N> value;
	aly::ModifiableNumberPtr vecField[N];
public:
	std::function<void(VectorFloatField<N>*)> onTextEntered;
	VectorFloatField(const std::string& name, const aly::AUnit2D& pos, const aly::AUnit2D& dims, const aly::vec<float, N>& Mat = aly::vec<float, N>()) :
			Composite(name, pos, dims), value(Mat) {
		using namespace aly;
		for (int n = 0; n < N; n++) {
			ModifiableNumberPtr field = ModifiableNumberPtr(
					new ModifiableNumber(MakeString() << "V[" << n << "]", CoordPercent(n / (float) N, 0.0f), CoordPercent(1.0f / N, 1.0f), NumberType::Float));
			field->setNumberValue(Float(Mat[n]));
			field->setAlignment(HorizontalAlignment::Center, VerticalAlignment::Middle);
			field->onTextEntered = [this,n](NumberField* field) {
				this->value[n]=field->getValue().toFloat();
				if(onTextEntered) {
					onTextEntered(this);
				}
			};
			Composite::add(field);
			vecField[n] = field;
		}
	}
	void setValue(const aly::vec<float, N>& m) {
		value = m;
		for (int n = 0; n < N; n++) {
			vecField[n]->setNumberValue(aly::Float(value[n]));
		}
	}
	aly::vec<float, N> getValue() const {
		return value;
	}
};

template<int N> class VectorIntegerField: public aly::Composite {
protected:
	aly::vec<int, N> value;
	aly::ModifiableNumberPtr vecField[N];
public:
	std::function<void(VectorIntegerField<N>*)> onTextEntered;
	VectorIntegerField(const std::string& name, const aly::AUnit2D& pos, const aly::AUnit2D& dims, const aly::vec<int, N>& Mat = aly::vec<int, N>()) :
			Composite(name, pos, dims), value(Mat) {
		using namespace aly;
		for (int n = 0; n < N; n++) {
			ModifiableNumberPtr field = ModifiableNumberPtr(
					new ModifiableNumber(MakeString() << "V[" << n << "]", CoordPercent(n / (float) N, 0.0f), CoordPercent(1.0f / N, 1.0f),
							NumberType::Integer));
			field->setNumberValue(Integer(Mat[n]));
			field->setAlignment(HorizontalAlignment::Center, VerticalAlignment::Middle);
			field->onTextEntered = [this,n](NumberField* field) {
				this->value[n]=field->getValue().toInteger();
				if(onTextEntered) {
					onTextEntered(this);
				}
			};
			Composite::add(field);
			vecField[n] = field;
		}
	}
	void setValue(const aly::vec<int, N>& m) {
		value = m;
		for (int n = 0; n < N; n++) {
			vecField[n]->setNumberValue(aly::Integer(value[n]));
		}
	}
	aly::vec<int, N> getValue() const {
		return value;
	}
};
class ParameterPane: public Composite {
protected:
	static const float SPACING;
	float entryHeight;
	std::vector<std::shared_ptr<AnyInterface>> values;
	void setCommonParameters(const CompositePtr& compRegion, const TextLabelPtr& textLabel, const RegionPtr& regionLabel);
	std::list<RegionPtr> groupQueue;
	CompositePtr lastRegion;
	ExpandBarPtr expandBar;
	float estimatedHeight = 0;
	bool lastExpanded = false;
	NumberFieldPtr addNumberFieldItem(const std::string& label, Number& value, float aspect = 3.0f);
	void updateGroups();
public:
	std::function<void(const std::string& name, const AnyInterface& value)> onChange;
	AColor entryTextColor;
	AColor entryBackgroundColor;
	AColor entryBorderColor;
	AUnit1D entryBorderWidth;
	virtual void clear() override;
	virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm, double pixelRatio, bool clamp) override;
	ParameterPane(const std::string& name, const AUnit2D& pos, const AUnit2D& dim, float entryHeight = 30.0f);
	CompositePtr addGroup(const std::string& name, bool expanded);
	NumberFieldPtr addNumberField(const std::string& label, Number& value, float aspect = 3.0f);
	std::pair<NumberFieldPtr, NumberFieldPtr> addRangeField(const std::string& label, Number& lowerValue, Number& upperValue, float aspect = 6.0f);
	std::pair<ModifiableNumberPtr, ModifiableNumberPtr> addRangeField(const std::string& label, Number& lowerValue, Number& upperValue, const Number& minValue,
			const Number& maxValue, float aspect = 9.0f);
	TextFieldPtr addTextField(const std::string& label, std::string& value, float aspect = 3.0f);
	ModifiableNumberPtr addNumberField(const std::string& label, Number& value, const Number& minValue, const Number& maxValue, float aspect = 7.0f);
	SelectionPtr addSelectionField(const std::string& label, int& selectedIndex, const std::vector<std::string>& options, float aspect = 4.0f);
	ToggleBoxPtr addToggleBox(const std::string& label, bool& value, float aspect = 2.1f);
	CheckBoxPtr addCheckBox(const std::string& label, bool& value, float aspect = 1.0f);
	ColorSelectorPtr addColorField(const std::string& label, Color& color, float aspect = 3.0f);
	FileSelectorPtr addFileField(const std::string& label, std::string& file, float aspect = -1.0f);
	FileSelectorPtr addDirectoryField(const std::string& label, std::string& file, float aspect = -1.0f);
	MultiFileSelectorPtr addMultiFileSelector(const std::string& label, std::vector<std::string>& files, float aspect = -1.0f);
	std::shared_ptr<Region> addNumberVectorField(const std::string& label, std::vector<Number>& value, const NumberType& type, float aspect = -1.0f);

	template<int M, int N> std::shared_ptr<MatrixField<M, N>> addMatrixField(const std::string& label, aly::matrix<float, M, N>& value, float aspect = -1.0f) {
		using namespace aly;
		CompositePtr comp = CompositePtr(new Composite(label + "_param", CoordPX(0, 0), CoordPerPX(1.0f, 0.0f, 0.0f, M * entryHeight)));
		TextLabelPtr labelRegion = TextLabelPtr(new TextLabel(label, CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		std::shared_ptr<MatrixField<M, N>> valueRegion = std::shared_ptr<MatrixField<M, N>>(
				new MatrixField<M, N>(label, CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 0.0f), CoordPX(aspect * entryHeight, M * entryHeight), value));
		if (aspect <= 0) {
			pixel2 labelBounds = labelRegion->getTextDimensions(AlloyDefaultContext().get());
			labelBounds.x += 10;
			valueRegion->position = CoordPX(labelBounds.x, 0.0f);
			valueRegion->dimensions = CoordPerPX(1.0f, 0.0f, -labelBounds.x, M * entryHeight);
		} else {
			labelRegion->position = CoordPX(0.0f, 0.0f);
			labelRegion->dimensions = CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, M * entryHeight);
		}
		std::shared_ptr<AnyInterface> ref = std::shared_ptr<AnyInterface>(new AnyValue<aly::matrix<float, M, N>*>(&value));
		values.push_back(ref);
		valueRegion->onTextEntered = [=](MatrixField<M,N>* field) {
			*(ref->getValue<aly::matrix<float,M,N>*>()) = field->getValue();
			if(this->onChange)this->onChange(label,*ref);
		};
		setCommonParameters(comp, labelRegion, valueRegion);
		valueRegion->backgroundColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
		valueRegion->setRoundCorners(true);
		estimatedHeight += M * entryHeight + SPACING;
		return valueRegion;
	}

	template<int N> std::shared_ptr<VectorFloatField<N>> addVectorFloatField(const std::string& label, aly::vec<float, N>& value, float aspect = -1.0f) {
		using namespace aly;
		CompositePtr comp = CompositePtr(new Composite(label + "_param", CoordPX(0, 0), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		TextLabelPtr labelRegion = TextLabelPtr(new TextLabel(label, CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		std::shared_ptr<VectorFloatField<N>> valueRegion = std::shared_ptr<VectorFloatField<N>>(
				new VectorFloatField<N>(label, CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 0.0f), CoordPX(aspect * entryHeight, entryHeight), value));
		if (aspect <= 0) {
			pixel2 labelBounds = labelRegion->getTextDimensions(AlloyDefaultContext().get());
			labelBounds.x += 10;
			valueRegion->position = CoordPX(labelBounds.x, 0.0f);
			valueRegion->dimensions = CoordPerPX(1.0f, 0.0f, -labelBounds.x, entryHeight);
		} else {
			labelRegion->position = CoordPX(0.0f, 0.0f);
			labelRegion->dimensions = CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, entryHeight);
		}
		std::shared_ptr<AnyInterface> ref = std::shared_ptr<AnyInterface>(new AnyValue<aly::vec<float, N>*>(&value));
		values.push_back(ref);
		valueRegion->onTextEntered = [=](VectorFloatField<N>* field) {
			*(ref->getValue<aly::vec<float,N>*>()) = field->getValue();
			if(this->onChange)this->onChange(label,*ref);
		};
		setCommonParameters(comp, labelRegion, valueRegion);
		valueRegion->backgroundColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
		valueRegion->setRoundCorners(true);
		estimatedHeight += entryHeight + SPACING;
		return valueRegion;
	}

	template<int N> std::shared_ptr<VectorIntegerField<N>> addVectorIntegerField(const std::string& label, aly::vec<int, N>& value, float aspect = -1.0f) {
		using namespace aly;
		CompositePtr comp = CompositePtr(new Composite(label + "_param", CoordPX(0, 0), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		TextLabelPtr labelRegion = TextLabelPtr(new TextLabel(label, CoordPX(0.0f, 0.0f), CoordPerPX(1.0f, 0.0f, 0.0f, entryHeight)));
		std::shared_ptr<VectorIntegerField<N>> valueRegion = std::shared_ptr<VectorIntegerField<N>>(
				new VectorIntegerField<N>(label, CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, 0.0f), CoordPX(aspect * entryHeight, entryHeight), value));
		if (aspect <= 0) {
			pixel2 labelBounds = labelRegion->getTextDimensions(AlloyDefaultContext().get());
			labelBounds.x += 10;
			valueRegion->position = CoordPX(labelBounds.x, 0.0f);
			valueRegion->dimensions = CoordPerPX(1.0f, 0.0f, -labelBounds.x, entryHeight);
		} else {
			labelRegion->position = CoordPX(0.0f, 0.0f);
			labelRegion->dimensions = CoordPerPX(1.0f, 0.0f, -aspect * entryHeight, entryHeight);
		}
		std::shared_ptr<AnyInterface> ref = std::shared_ptr<AnyInterface>(new AnyValue<aly::vec<int, N>*>(&value));
		values.push_back(ref);
		valueRegion->onTextEntered = [=](VectorIntegerField<N>* field) {
			*(ref->getValue<aly::vec<int, N>*>()) = field->getValue();
			if(this->onChange)this->onChange(label,*ref);
		};
		setCommonParameters(comp, labelRegion, valueRegion);
		valueRegion->backgroundColor = MakeColor(AlloyDefaultContext()->theme.LIGHTER);
		valueRegion->setRoundCorners(true);
		estimatedHeight += entryHeight + SPACING;
		return valueRegion;
	}
};

typedef std::shared_ptr<ParameterPane> ParameterPanePtr;
}

#endif
