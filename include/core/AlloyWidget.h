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

#ifndef ALLOYWIDGET_H_
#define ALLOYWIDGET_H_

#include "AlloyUI.h"

namespace aly {
enum class SliderHandleShape {Whole,Hat, HalfLeft, HalfRight};
class TextButton: public Region {
protected:
	bool truncate;
public:
	AColor textColor;
	AUnit1D fontSize;
	void setTruncate(bool t) {
		truncate = t;
	}
	TextButton(const std::string& label, const AUnit2D& position,
			const AUnit2D& dimensions,bool truncate=true);
	virtual void draw(AlloyContext* context) override;
	virtual inline ~TextButton() {
	}
};

class TextIconButton: public Composite {
private:
	std::string iconCodeString;
	IconAlignment iconAlignment;
	HorizontalAlignment alignment;
	bool truncate;
	std::string label;
public:
	AColor textColor;
	AUnit1D fontSize;
	void setTruncate(bool t) {
		truncate = t;
	}
	void setLabel(const std::string& label);
	void setIcon(int code);
	TextIconButton(const std::string& label, int iconCode,
			const AUnit2D& position, const AUnit2D& dimensions,
			const HorizontalAlignment& alignment = HorizontalAlignment::Center,
			const IconAlignment& iconAlignment = IconAlignment::Left,bool truncate=true);
	virtual void draw(AlloyContext* context) override;
	virtual inline ~TextIconButton() {
	}
};
enum class IconType {
	CIRCLE, SQUARE
};
class IconButton: public Composite {
private:
	std::string iconCodeString;
	IconType iconType;
	bool truncate;
	bool rescale;
public:
	AColor foregroundColor;
	AColor iconColor;
	void setTruncate(bool t) {
		truncate = t;
	}
	IconButton(int iconCode, const AUnit2D& position, const AUnit2D& dimensions,
			IconType iconType = IconType::SQUARE,bool truncate=true);
	void setIcon(int iconCode);
	void setRescaleOnHover(bool r){
		rescale=r;
	}
	virtual void draw(AlloyContext* context) override;
	virtual inline ~IconButton() {
	}
};
class CheckBox: public Composite {
private:
	TextLabelPtr checkLabel;
	TextLabelPtr valueLabel;
	bool checked;
	bool handleMouseDown(AlloyContext* context, const InputEvent& event);
public:
	std::function<void(bool)> onChange;
	inline bool getValue() {
		return checked;
	}
	void setValue(bool value);
	CheckBox(const std::string& label, const AUnit2D& position,
			const AUnit2D& dimensions, bool checked ,bool showText=true);
	virtual void draw(AlloyContext* context) override;
};
class ToggleBox: public Composite {
private:
	TextLabelPtr toggleLabel;
	TextLabelPtr onLabel;
	TextLabelPtr offLabel;
	CompositePtr clickRegion;
	bool toggledOn;
	bool handleMouseDown(AlloyContext* context, const InputEvent& e);
public:
	inline bool getValue() {
		return toggledOn;
	}
	std::function<void(bool)> onChange;

	void setValue(bool value);
	ToggleBox(const std::string& label, const AUnit2D& position,
			const AUnit2D& dimensions, bool toggledOn ,bool showText=true);
	virtual void draw(AlloyContext* context) override;
};

class SliderHandle: public Region {
protected:
	SliderHandleShape handleShape;
public:
	SliderHandle(const std::string& name, const SliderHandleShape& handleShape=SliderHandleShape::Whole) :
			Region(name),handleShape(handleShape) {
	}
	virtual void draw(AlloyContext* context) override;
};
class SliderTrack: public Composite {
protected:
	const Orientation orientation;
	float2 activeRegion;
	float currentPosition;
public:
	Color startColor, endColor;
	SliderTrack(const std::string& name, Orientation orientColor,
			const Color& st, const Color& ed);
	void setLower(float x) {
		activeRegion.x = x;
	}
	void setCurrent(float c) {
		currentPosition = c;
	}
	void setUpper(float y) {
		activeRegion.y = y;
	}
	virtual void draw(AlloyContext* context) override;
};
class ProgressBar: public Composite {
private:
	TextLabelPtr textLabel;
	float value;
	std::string label;
public:
	virtual void draw(AlloyContext* context) override;
	inline void setValue(float p) {
		value = clamp(p, 0.0f, 1.0f);
	}
	inline float getValue() const {
		return value;
	}
	inline std::string getLabel() const {
		return label;
	}
	inline void setValue(const std::string& l) {
		label = l;
	}
	inline void setValue(const std::string& l, float p) {
		label = l;
		value = clamp(p, 0.0f, 1.0f);
	}
	ProgressBar(const std::string& name, const AUnit2D& pt,
			const AUnit2D& dims);
};
class Slider: public Composite {
protected:
	AColor textColor;
	AUnit1D fontSize;
	Number minValue;
	Number maxValue;
	Number value;
	TextLabelPtr sliderLabel;
	TextLabelPtr valueLabel;
	std::shared_ptr<SliderHandle> sliderHandle;
	std::shared_ptr<SliderTrack> sliderTrack;
	std::function<std::string(const Number& value)> labelFormatter;
	std::function<void(const Number& value)> onChangeEvent;
	virtual void update()=0;
	double sliderPosition;
public:
	void setSliderColor(const Color& startColor, const Color& endColor) {
		sliderTrack->startColor = startColor;
		sliderTrack->endColor = endColor;
	}
	void setMinValue(const Number& v) {
		minValue=v;
	}
	void setMaxValue(const Number& v) {
		maxValue = v;
	}
	Slider(const std::string& name, const Number& min, const Number& max,
			const Number& val) :
			Composite(name), minValue(min), maxValue(max), value(val), sliderPosition(
					0.0) {
		labelFormatter = [](const Number& value) {return value.toString();};
	}
	Slider(const std::string& name, const AUnit2D& pos, const AUnit2D& dims,
			const Number& min, const Number& max, const Number& val) :
			Composite(name, pos, dims), minValue(min), maxValue(max), value(
					val), sliderPosition(0.0) {
		labelFormatter = [](const Number& value) {return value.toString();};
	}
	double getBlendValue() const;
	void setBlendValue(double value);
	virtual void setValue(double value)=0;
	inline void setValue(int value) {
		setValue((double) value);
	}
	inline void setValue(float value) {
		setValue((double) value);
	}
	const Number& getValue() {
		return value;
	}
	inline void setOnChangeEvent(
			const std::function<void(const Number& value)>& func) {
		onChangeEvent = func;
	}
	inline void setLabelFormatter(
			const std::function<std::string(const Number& value)>& func) {
		labelFormatter = func;
	}
};
class HorizontalSlider: public Slider {
protected:
	virtual void update() override;
public:
	virtual void setValue(double value) override;
	bool onMouseDown(AlloyContext* context, Region* region,
			const InputEvent& event);
	bool onMouseDrag(AlloyContext* context, Region* region,
			const InputEvent& event);
	HorizontalSlider(const std::string& label, const AUnit2D& position,
			const AUnit2D& dimensions, bool showLabel,
		const Number& minValue = Float(0.0f),
			const Number& maxValue = Float(1.0f),
			const Number& value = Float(0.0f));
	HorizontalSlider(const std::string& label, const AUnit2D& position,
		const AUnit2D& dimensions,
		const Number& minValue = Float(0.0f),
		const Number& maxValue = Float(1.0f),
		const Number& value = Float(0.0f)):HorizontalSlider(label,position,dimensions,true,minValue,maxValue,value) {

	}
	virtual void draw(AlloyContext* context) override;
	virtual inline ~HorizontalSlider() {
	}
};
class RangeSlider : public Composite {
protected:
	AColor textColor;
	AUnit1D fontSize;
	Number minValue;
	Number maxValue;
	Number lowerValue;
	Number upperValue;
	TextLabelPtr sliderLabel;
	TextLabelPtr lowerValueLabel;
	TextLabelPtr upperValueLabel;
	std::shared_ptr<SliderHandle> lowerSliderHandle;
	std::shared_ptr<SliderHandle> upperSliderHandle;
	std::shared_ptr<SliderTrack> sliderTrack;
	std::function<std::string(const Number& value)> labelFormatter;
	void update();
	double2 sliderPosition;
public:
	std::function<void(const Number& lowerValue, const Number& upperValue)> onChangeEvent;
	void setSliderColor(const Color& startColor, const Color& endColor) {
		sliderTrack->startColor = startColor;
		sliderTrack->endColor = endColor;
	}
	RangeSlider(const std::string& name, const AUnit2D& pos,const AUnit2D& dims,const Number& min, const Number& max,
		const Number& lowerValue, const Number& upperValue,bool showLabel=true);
	double2 getBlendValue() const;
	void setBlendValue(double2 value);

	void setLowerValue(double value);
	void setUpperValue(double value);
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
	void setValue(double2 range);
	const Number& getLowerValue() {
		return lowerValue;
	}
	const Number& getUpperValue() {
		return upperValue;
	}
	inline void setOnChangeEvent(
		const std::function<void(const Number& lowerValue,const Number& upperValue)>& func) {
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
	virtual inline ~RangeSlider() {
	}
};

class VerticalSlider: public Slider {
protected:
	virtual void update() override;
public:
	virtual void setValue(double value) override;
	bool onMouseDown(AlloyContext* context, Region* region,
			const InputEvent& event);
	bool onMouseDrag(AlloyContext* context, Region* region,
			const InputEvent& event);
	VerticalSlider(const std::string& label, const AUnit2D& position,
			const AUnit2D& dimensions, const Number& minValue = Float(0.0f),
			const Number& maxValue = Float(1.0f),
			const Number& value = Float(0.0f));
	virtual void draw(AlloyContext* context) override;
	virtual inline ~VerticalSlider() {
	}
};
class Selection: public Composite {
private:
	TextLabelPtr selectionLabel;
	TextLabelPtr arrowLabel;
	std::shared_ptr<SelectionBox> selectionBox;
	int selectedIndex;
	void hide(AlloyContext* context);
	void show(AlloyContext* context);
	bool handleMouseClick(AlloyContext* context, const InputEvent& event);
public:
	void setTextColor(const AColor& c);
	std::function<void(int)> onSelect;
	//bool onEventHandler(AlloyContext* context, const InputEvent& event)override;
	void clearSelections() {
		selectionBox->clearSelections();
	}
	inline int getSelectedIndex() const {
		return selectedIndex;
	}
	void setMaxDisplayEntries(int mx) {
		selectionBox->setMaxDisplayEntries(mx);
	}
	std::string getValue() {
		return selectionBox->getSelection(selectedIndex);
	}
	std::string getSelection() {
		return selectionBox->getSelection(selectedIndex);
	}

	std::string getSelection(int index) {
		return selectionBox->getSelection(index);
	}
	void setValue(int selection) {
		selectedIndex = selection;
		selectionBox->setSelectedIndex(selection);
		selectionLabel->setLabel(this->getValue());
		if (onSelect)
			onSelect(selectedIndex);
	}
	void addSelection(const std::string& selection) {
		selectionBox->addSelection(selection);
	}
	size_t getSelectionSize() const {
		return selectionBox->getSelectionSize();
	}
	void setSelectedIndex(int selection) {
		selectedIndex = selection;
		selectionBox->setSelectedIndex(selection);
		selectionLabel->setLabel(this->getValue());
		if (onSelect)
			onSelect(selectedIndex);
	}
	virtual void draw(AlloyContext* context) override;
	Selection(const std::string& label, const AUnit2D& position,
			const AUnit2D& dimensions, const std::vector<std::string>& options =
					std::vector<std::string>());
};


class MessageDialog: public Composite {
protected:
	bool returnValue = false;
	MessageOption option;
	MessageType type;
	std::shared_ptr<Composite> containerRegion;
	std::shared_ptr<TextLabel> textLabel;
public:
	bool getValue() const {
		return returnValue;
	}
	void setMessage(const std::string& message);
        std::string getMessage() const;
	void setVisible(bool visible);
	MessageDialog(const std::string& name, const AUnit2D& pos,
			const AUnit2D& dims, bool wrap, const MessageOption& option,
			const MessageType& type);
	MessageDialog(const std::string& name, bool wrap,
			const MessageOption& option, const MessageType& type);
	virtual void draw(AlloyContext* context) override;
	std::function<void(MessageDialog* dialog)> onSelect;
};



class FileDialog;
class ListBox;
class ListEntry: public Composite {
protected:
	std::string iconCodeString;
	std::string label;
	bool selected;
	ListBox* dialog;
	float entryHeight;
	AUnit1D fontSize;
public:
	friend class ListBox;
	void setSelected(bool selected);
	bool isSelected();
	virtual void setLabel(const std::string& label);
	void setIcon(int icon);
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	ListEntry(ListBox* listBox, const std::string& name, float entryHeight);
	virtual void draw(AlloyContext* context) override;
};
class FileEntry: public ListEntry {
private:
	std::string creationTime;
	std::string lastModifiedTime;
	std::string lastAccessTime;
	std::string fileSize;
public:
	FileDescription fileDescription;
	FileEntry(FileDialog* dialog, const std::string& name, float fontHeight);
	void setValue(const FileDescription& fileDescription);
};
struct FileFilterRule {
	std::string name;
	std::vector<std::string> extensions;
	FileFilterRule(const std::string& name, const std::string& extension) :
			name(name) {
		extensions.push_back(extension);
	}
	FileFilterRule(const FileFilterRule& rule) :name(rule.name),extensions(rule.extensions) {
	}
	FileFilterRule(const std::string& name="") :
			name(name) {
	}
	template<class Archive> void save(Archive& ar) const {
		ar(CEREAL_NVP(name),CEREAL_NVP(extensions));
	}
	template<class Archive> void load(Archive& ar) {
		ar(CEREAL_NVP(name),CEREAL_NVP(extensions));
	}
	FileFilterRule(const std::string& name,
			std::initializer_list<std::string> extension) :
			name(name) {
		for (std::string ext : extension) {
			extensions.push_back(ext);
		}
	}
	std::string toString();
	virtual bool accept(const std::string& file);
	virtual ~FileFilterRule() {
	}
};
class ListBox: public Composite {
protected:
	bool enableMultiSelection;
	box2px dragBox;
	std::shared_ptr<TimerTask> downTimer, upTimer;
	bool scrollingDown;
	bool scrollingUp;
	bool dirty;
	std::vector<std::shared_ptr<ListEntry>> listEntries;
	std::list<ListEntry*> lastSelected;
	void addToActiveList(ListEntry* entry) {
		lastSelected.push_back(entry);
	}
	void clearActiveList() {
		lastSelected.clear();
	}
public:
	void update();

	virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm,double pixelRatio, bool clamp) override;
	box2px getDragBox() const {
		return dragBox;
	}
	std::vector<std::shared_ptr<ListEntry>>& getEntries() {
		return listEntries;
	}
	const std::vector<std::shared_ptr<ListEntry>>& getEntries() const {
		return listEntries;
	}
	void addEntry(const std::shared_ptr<ListEntry>& entry) {
		listEntries.push_back(entry);
		dirty = true;
	}
	void clearEntries();
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& e) override;
	ListEntry* getLastSelected() {
		if (lastSelected.size() > 0)
			return lastSelected.back();
		else
			return nullptr;
	}
	bool isDraggingOver(ListEntry* entry);
	ListBox(const std::string& name, const AUnit2D& pos, const AUnit2D& dims);
	virtual void draw(AlloyContext* context) override;
	void setEnableMultiSelection(bool enable) {
		enableMultiSelection = enable;
	}
	bool onMouseDown(ListEntry* entry, AlloyContext* context,
			const InputEvent& e);
	std::function<void(ListEntry*, const InputEvent&)> onSelect;
};
class FileDialog: public Composite {
private:
	std::vector<std::shared_ptr<FileFilterRule>> filterRules;
	std::shared_ptr<FileField> fileLocation;
	std::shared_ptr<Composite> directoryTree;
	std::shared_ptr<ListBox> directoryList;
	std::shared_ptr<Selection> fileTypeSelect;
	std::shared_ptr<TextIconButton> actionButton;
	std::shared_ptr<IconButton> upDirButton;

	std::shared_ptr<IconButton> cancelButton;
	std::shared_ptr<BorderComposite> containerRegion;
	std::string lastDirectory;
	void setSelectedFile(const std::string& file);
	const FileDialogType type;
	pixel fileEntryHeight;
	bool valid = false;
	void updateDirectoryList();
	bool updateValidity();
public:

	void addFileExtensionRule(const std::string& name,const std::string& extension);
	void addFileExtensionRule(const FileFilterRule& rule);
	void addFileExtensionRule(const std::string& name,const std::initializer_list<std::string> & extension);
	friend class FileEntry;
	std::function<void(const std::vector<std::string>&)> onSelect;
	virtual void draw(AlloyContext* context) override;
	FileDialog(const std::string& name, const AUnit2D& pos, const AUnit2D& dims,
			const FileDialogType& type, pixel fileEntryHeight = 30);
	void update();
	void setValue(const std::string& file);
	std::string getValue() const;
	void setFileExtensionRule(int index) {
		fileTypeSelect->setValue(index);
		updateDirectoryList();
	}
};
class FileSelector: public BorderComposite {
private:
	std::shared_ptr<FileField> fileLocation;
	std::shared_ptr<IconButton> openIcon;
	std::shared_ptr<FileDialog> fileDialog;
	bool directoryInput;
public:
	void setTextColor(const AColor& c);
	std::function<void(const std::string& file)> onChange;
	void addFileExtensionRule(const std::string& name,
			const std::string& extension) {
		fileDialog->addFileExtensionRule(name, extension);
	}
	void addFileExtensionRule(const FileFilterRule& rule) {
		fileDialog->addFileExtensionRule(rule);
	}
	void addFileExtensionRule(const std::string& name,
			const std::initializer_list<std::string>& extension) {
		fileDialog->addFileExtensionRule(name, extension);
	}

	void setFileExtensionRule(int index) {
		fileDialog->setFileExtensionRule(index);
	}
	FileSelector(const std::string& name, const AUnit2D& pos,
			const AUnit2D& dims,bool directoryInput=false);
	void setValue(const std::string& file);
	std::string getValue() {
		return fileLocation->getValue();
	}
	void openFileDialog(AlloyContext* context,
			const std::string& workingDirectory = GetCurrentWorkingDirectory());
};
class SearchBox : public BorderComposite {
private:
	std::shared_ptr<TextField> searchField;
	std::shared_ptr<IconButton> searchIcon;
public:
	void setTextColor(const AColor& c);
	std::function<void(const std::string& query)> onChange;
	SearchBox(const std::string& name, const AUnit2D& pos,const AUnit2D& dims);
	void setValue(const std::string& file);
	std::string getValue() {
		return searchField->getValue();
	}
};
class FilterBox : public BorderComposite {
private:
	std::shared_ptr<TextField> filterField;
	std::shared_ptr<IconButton> filterIcon;
public:
	void setTextColor(const AColor& c);
	std::function<void(const std::string& query)> onChange;
	FilterBox(const std::string& name, const AUnit2D& pos,const AUnit2D& dims);
	void setValue(const std::string& file);
	std::string getValue() {
		return filterField->getValue();
	}
};
class FileButton: public IconButton {
private:
	std::shared_ptr<FileDialog> fileDialog;
public:

	std::function<void(const std::vector<std::string>& file)> onOpen;
	std::function<void(const std::string& file)> onSave;
	void addFileExtensionRule(const std::string& name,
			const std::string& extension) {
		fileDialog->addFileExtensionRule(name, extension);
	}
	void addFileExtensionRule(const std::string& name,
			const std::initializer_list<std::string>& extension) {
		fileDialog->addFileExtensionRule(name, extension);
	}
	void setFileExtensionRule(int index) {
		fileDialog->setFileExtensionRule(index);
	}
	FileButton(const std::string& name, const AUnit2D& pos, const AUnit2D& dims,
			const FileDialogType& type);
	void setValue(const std::string& file);
	std::string getValue() {
		return fileDialog->getValue();
	}
	void openFileDialog(AlloyContext* context,
			const std::string& workingDirectory = GetCurrentWorkingDirectory());
};
class WindowPane: public AdjustableComposite {
protected:
	CompositePtr titleRegion;
	CompositePtr contentRegion;
	bool maximized;
	bool dragging;
	std::shared_ptr<IconButton> maximizeIcon;
	TextLabelPtr label;
public:
	void setMaximize(bool max);
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	WindowPane(const RegionPtr& content);
	virtual void draw(AlloyContext* context) override;
};
class MultiFileEntry : public ListEntry {
private:
	std::string fileName;
public:
	MultiFileEntry(ListBox* listBox, const std::string& name, float fontHeight);
	void setValue(const std::string& file);
	std::string getValue() const {
		return fileName;
	}
};

class MultiFileSelector: public Composite{
protected:
	std::shared_ptr<ListBox> valueRegion;
	std::shared_ptr<FileButton> openFileButton;
	std::shared_ptr<IconButton> upButton;
	std::shared_ptr<IconButton> downButton;
	std::shared_ptr<IconButton> eraseButton;
	float entryHeight;
	void update();
	void fireEvent();
public:
	std::function<void(const std::vector<std::string>& files)> onChange;
	void addFileExtensionRule(const std::string& name,
		const std::string& extension) {
		openFileButton->addFileExtensionRule(name, extension);
	}
	void addFileExtensionRule(const std::string& name,
		const std::initializer_list<std::string>& extension) {
		openFileButton->addFileExtensionRule(name, extension);
	}
	void setFileExtensionRule(int index) {
		openFileButton->setFileExtensionRule(index);
	}
	void addFiles(const std::vector<std::string>& files);
	void clearEntries();
	MultiFileSelector(const std::string& name, const AUnit2D& pos, const AUnit2D& dims, float entryHeight=30.0f);
};
class NumberEntry : public aly::ListEntry {
protected:
	ModifiableNumberPtr valueRegion;
public:
        NumberEntry(aly::ListBox* listBox,const NumberType&  number, float entryHeight=30.0f);
        void setValue(const Number& num);
        Number getValue() const {
            return valueRegion->getValue();
        }
        ModifiableNumberPtr getRegion() const {
            return valueRegion;
        }
        virtual void draw(aly::AlloyContext* context) override;
};
class NumberListBox : public aly::Composite {
protected:
        std::shared_ptr<aly::ListBox> valueRegion;
        std::shared_ptr<aly::IconButton> addButton;
        std::shared_ptr<aly::IconButton> upButton;
        std::shared_ptr<aly::IconButton> downButton;
        std::shared_ptr<aly::IconButton> eraseButton;
        float entryHeight;
        std::vector<Number> values;
        void fireEvent();
public:
        std::function<void(const std::vector<Number>& numbers)> onChange;
        void update();
        void clearEntries();
        void addNumbers(const std::vector<Number>& numbers);
        NumberListBox(const std::string& name, const aly::AUnit2D& pos, const aly::AUnit2D& dims, const NumberType& type, float entryHeight = 30.0f);
};
typedef std::shared_ptr<TextButton> TextButtonPtr;
typedef std::shared_ptr<HorizontalSlider> HSliderPtr;
typedef std::shared_ptr<VerticalSlider> VSliderPtr;
typedef std::shared_ptr<HorizontalSlider> HorizontalSliderPtr;
typedef std::shared_ptr<VerticalSlider> VerticalSliderPtr;
typedef std::shared_ptr<RangeSlider> RangeSliderPtr;
typedef std::shared_ptr<CheckBox> CheckBoxPtr;
typedef std::shared_ptr<ToggleBox> ToggleBoxPtr;
typedef std::shared_ptr<Selection> SelectionPtr;
typedef std::shared_ptr<ProgressBar> ProgressBarPtr;
typedef std::shared_ptr<FileSelector> FileSelectorPtr;
typedef std::shared_ptr<FileDialog> FileDialogPtr;
typedef std::shared_ptr<FileButton> FileButtonPtr;
typedef std::shared_ptr<TextIconButton> TextIconButtonPtr;
typedef std::shared_ptr<IconButton> IconButtonPtr;
typedef std::shared_ptr<ListBox> ListBoxPtr;
typedef std::shared_ptr<ListEntry> ListEntryPtr;
typedef std::shared_ptr<WindowPane> WindowPanePtr;
typedef std::shared_ptr<MessageDialog> MessageDialogPtr;
typedef std::shared_ptr<MultiFileEntry> MultiFileEntryPtr;
typedef std::shared_ptr<MultiFileSelector>  MultiFileSelectorPtr;
typedef std::shared_ptr<SearchBox> SearchBoxPtr;
typedef std::shared_ptr<FilterBox> FilterBoxPtr;
typedef std::shared_ptr<NumberListBox>  NumberListBoxPtr;
typedef std::shared_ptr<NumberEntry> NumberEntryPtr;
}

#endif /* ALLOYWIDGET_H_ */
