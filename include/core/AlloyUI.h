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

#ifndef ALLOYUI_H_
#define ALLOYUI_H_
#include "AlloyMath.h"
#include "AlloyNumber.h"
#include "AlloyContext.h"
#include "AlloyUnits.h"
#include "AlloyWorker.h"
#include "AlloyEnum.h"
#include "nanovg.h"
#include  "GLTexture.h"
#include <iostream>
#include <memory>
#include <list>
#include <array>
#include <vector>
namespace aly {
bool SANITY_CHECK_UI();

class Composite;
class BorderComposite;
class Region: public EventHandler {
private:
	pixel2 dragOffset = pixel2(0, 0);
protected:
	box2px bounds;
	box2px extents;
	void drawBoundsLabel(AlloyContext* context, const std::string& name,
			int font);
	Region* mouseOverRegion = nullptr;
	Region* mouseDownRegion = nullptr;
	static uint64_t REGION_COUNTER;
	bool visible = true;
	bool ignoreCursorEvents = false;
	Origin origin = Origin::TopLeft;
	AspectRule aspectRule = AspectRule::Unspecified;
	double aspectRatio = -1.0; //Less than zero indicates undetermined. Will be computed at next pack() event.
	int dragButton = -1;
	bool roundCorners = false;
	bool detached = false;
	bool clampToParentBounds = false;
public:
	AUnit2D position = CoordPercent(0.0f, 0.0f);
	AUnit2D dimensions = CoordPercent(1.0f, 1.0f);
	friend class Composite;
	friend class BorderComposite;
	const std::string name;
	void setIgnoreCursorEvents(bool ignore) {
		ignoreCursorEvents = ignore;
	}
	void setClampDragToParentBounds(bool clamp) {
		clampToParentBounds = clamp;
	}
	void setDetached(bool enable) {
		detached = enable;
	}
	void setDragOffset(const pixel2& offset) {
		dragOffset = offset;
	}
	int getDragButton() const {
		return dragButton;
	}
	void clampDragOffset();
	inline void setRoundCorners(bool round) {
		this->roundCorners = round;
	}
	inline bool hasParent(Region* region) const {
		return (parent != nullptr
				&& (parent == region || parent->hasParent(region)));
	}
	inline bool isDetached() const {
		return (parent != nullptr && parent->isDetached()) || detached;
	}
	std::function<bool(AlloyContext*, const InputEvent& event)> onEvent;

	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	virtual inline bool isScrollEnabled() const {
		return false;
	}
	virtual std::string getName() const override {
		return name;
	}
	virtual inline pixel2 getDrawOffset() const {
		if (parent != nullptr) {
			return parent->getDrawOffset();
		} else {
			return pixel2(0, 0);
		}
	}

	virtual Region* locate(const pixel2& cursor);
	inline void setAspectRule(const AspectRule& aspect) {
		aspectRule = aspect;
	}
	inline void setAspectRatio(double val) {
		aspectRatio = val;
	}
	inline void setBounds(const AUnit2D& pt, const AUnit2D& dim) {
		position = pt;
		dimensions = dim;
	}
	inline void setBounds(const pixel2& pt, const pixel2& dim) {
		bounds.position = pt;
		bounds.dimensions = dim;
	}
	inline void setBounds(const box2px& bbox) {
		bounds = bbox;
	}
	AColor backgroundColor = MakeColor(COLOR_NONE);
	AColor borderColor = MakeColor(COLOR_NONE);
	AUnit1D borderWidth = UnitPX(2);
	std::function<void()> onPack;
	std::function<void()> onRemoveFromOnTop;
	std::function<bool(AlloyContext* context, const InputEvent& event)> onMouseDown;
	std::function<bool(AlloyContext* context, const InputEvent& event)> onMouseUp;
	std::function<bool(AlloyContext* context, const InputEvent& event)> onMouseOver;
	std::function<bool(AlloyContext* context, const InputEvent& event)> onScroll;
	std::function<bool(AlloyContext* context, const InputEvent& event)> onMouseDrag;
	void setDragOffset(const pixel2& cursor, const pixel2& delta);
	bool addDragOffset(const pixel2& delta);

	virtual void setDragEnabled(bool enabled) {
		dragButton = (enabled)?GLFW_MOUSE_BUTTON_LEFT:-1;
		if (enabled)
			clampToParentBounds = true;
	}
	virtual void setDragButton(int button) {
		dragButton = button;
		if (dragButton!=-1)
			clampToParentBounds = true;
	}
	inline void setOrigin(const Origin& org) {
		origin = org;
	}
	virtual bool isDragEnabled() const {
		return (dragButton!=-1);
	}
	virtual box2px getBounds(bool includeOffset = true) const;
	virtual box2px getExtents() const;
	virtual box2px getCursorBounds(bool includeOffset = true) const;
	pixel2 getBoundsPosition(bool includeOffset = true) const {
		return getBounds(includeOffset).position;
	}
	pixel2 getBoundsDimensions(bool includeOffset = true) const {
		return getBounds(includeOffset).dimensions;
	}
	pixel2 getDragOffset() const {
		return dragOffset;
	}
	pixel getBoundsPositionX(bool includeOffset = true) const {
		return getBounds(includeOffset).position.x;
	}
	pixel getBoundsDimensionsX(bool includeOffset = true) const {
		return getBounds(includeOffset).dimensions.x;
	}
	pixel getBoundsPositionY(bool includeOffset = true) const {
		return getBounds(includeOffset).position.y;
	}
	pixel getBoundsDimensionsY(bool includeOffset = true) const {
		return getBounds(includeOffset).dimensions.y;
	}
	Origin getOrigin() const {
		return origin;
	}
	AspectRule getAspectRule() const {
		return aspectRule;
	}
	double getAspectRatio() const {
		return aspectRatio;
	}
	virtual void setVisible(bool vis);
	Region* parent = nullptr;
	Region(
			const std::string& name = MakeString() << "r" << std::setw(8)
					<< std::setfill('0') << (REGION_COUNTER++));
	Region(const std::string& name, const AUnit2D& pos, const AUnit2D& dims);
	virtual void pack(const pixel2& pos, const pixel2& dims,
			const double2& dpmm, double pixelRatio, bool clamp = false);
	virtual void pack(AlloyContext* context);
	virtual void pack();
	virtual void draw(AlloyContext* context);
	virtual void updateCursor(CursorLocator* cursorLocator);
	virtual void drawDebug(AlloyContext* context);
	virtual void removeListener() const;
	bool isVisible() const;
	virtual ~Region();
};
class Draw: public Region {
public:
	std::function<void(AlloyContext* context, const box2px& bounds)> onDraw;
	Draw(const std::string& name, const AUnit2D& pos, const AUnit2D& dims,
			const std::function<
					void(AlloyContext* context, const box2px& bounds)>& func =
					nullptr) :
			Region(name, pos, dims), onDraw(func) {
	}
	virtual void draw(AlloyContext* context) override;
};
class ScrollHandle: public Region {
public:
	const Orientation orientation;
	ScrollHandle(const std::string& name, Orientation orient) :
			Region(name), orientation(orient) {
	}
	virtual void draw(AlloyContext* context) override;
};
class ScrollTrack: public Region {
public:
	const Orientation orientation;
	ScrollTrack(const std::string& name, Orientation orient) :
			Region(name), orientation(orient) {
	}
	virtual void draw(AlloyContext* context) override;
};
class Composite: public Region {
protected:
	Orientation orientation = Orientation::Unspecified;
	bool scrollEnabled = false;
	bool alwaysShowVerticalScrollBar = false;
	bool alwaysShowHorizontalScrollBar = false;

	float horizontalScrollExtent = 0;
	pixel2 scrollPosition = pixel2(0, 0);
	std::shared_ptr<ScrollTrack> verticalScrollTrack, horizontalScrollTrack;
	std::shared_ptr<ScrollHandle> verticalScrollHandle, horizontalScrollHandle;
	std::vector<std::shared_ptr<Region>> children;
	typedef std::shared_ptr<Region> ValueType;
	pixel2 cellPadding = pixel2(0, 0);
	pixel2 cellSpacing = pixel2(0, 0);
	void updateExtents();
public:
	virtual void removeListener() const override;
	void erase(const std::shared_ptr<Region>& node);
	void erase(Region* node);
	bool isVerticalScrollVisible() const {
		if (verticalScrollTrack.get() == nullptr) {
			return false;
		}
		return verticalScrollTrack->isVisible();
	}
	Orientation getOrientation() const {
		return orientation;
	}

	bool isHorizontalScrollVisible() const {
		if (horizontalScrollTrack.get() == nullptr) {
			return false;
		}
		return horizontalScrollTrack->isVisible();
	}
	void setAlwaysShowVerticalScrollBar(bool show) {
		alwaysShowVerticalScrollBar = show;
		scrollEnabled |= show;
	}
	void setAlwaysShowHorizontalScrollBar(bool show) {
		alwaysShowHorizontalScrollBar = show;
		scrollEnabled |= show;
	}
	void resetScrollPosition();
	static const float scrollBarSize;
	typedef std::vector<ValueType>::iterator iterator;
	typedef std::vector<ValueType>::const_iterator const_iterator;

	virtual void clear();
	std::vector<std::shared_ptr<Region>>& getChildren() {
		return children;
	}
	iterator begin() {
		return children.begin();
	}
	iterator end() {
		return children.end();
	}
	const_iterator cbegin() const {
		return children.cbegin();
	}
	const_iterator cend() const {
		return children.cend();
	}
	bool addVerticalScrollPosition(float pix);
	bool addHorizontalScrollPosition(float pix);
	void putLast(const std::shared_ptr<Region>& region);
	void putFirst(const std::shared_ptr<Region>& region);
	void putLast(Region* region);
	void putFirst(Region* region);

	Composite(
			const std::string& name = MakeString() << "c" << std::setw(8)
					<< std::setfill('0') << (REGION_COUNTER++));
	Composite(const std::string& name, const AUnit2D& pos, const AUnit2D& dims);
	virtual Region* locate(const pixel2& cursor) override;

	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	inline void setOrientation(const Orientation& orient, pixel2 cellSpacing =
			pixel2(5, 5), pixel2 cellPadding = pixel2(0, 0)) {
		orientation = orient;
		this->cellSpacing = cellSpacing;
		this->cellPadding = cellPadding;
	}
	virtual inline bool isScrollEnabled() const override {
		return scrollEnabled;
	}
	void setScrollEnabled(bool enabled) {
		scrollEnabled = enabled;
	}


	virtual pixel2 getDrawOffset() const override;
	virtual void draw(AlloyContext* context) override;
	virtual void drawDebug(AlloyContext* context) override;
	virtual void updateCursor(CursorLocator* cursorLocator) override;
	virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm,
			double pixelRatio, bool clamp = false) override;

	virtual void add(const std::shared_ptr<Region>& region);
	virtual void insertAtFront(const std::shared_ptr<Region>& region);
	virtual void pack() override {
		Region::pack();
	}
	virtual void pack(AlloyContext* context) override {
		Region::pack(context);
	}
	void draw();
	virtual ~Composite();
};

class BorderComposite: public Region {
protected:
	std::array<std::shared_ptr<Region>, 5> children;
	std::shared_ptr<Region>& northRegion;
	std::shared_ptr<Region>& southRegion;
	std::shared_ptr<Region>& eastRegion;
	std::shared_ptr<Region>& westRegion;
	AUnit1D northFraction, southFraction, eastFraction, westFraction;
	std::shared_ptr<Region>& centerRegion;
	bool resizing;
	WindowPosition winPos;
	bool resizeable;
	pixel2 cellPadding;
	box2px windowInitialBounds;
	box2px currentBounds;
	pixel2 cursorDownPosition;
public:
	bool isResizeable() const {
		return resizeable;
	}
	bool isResizing() const {
		return resizing;
	}
	void setCellPadding(const pixel2& padding){
		cellPadding=padding;
	}
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	BorderComposite(const std::string& name, const AUnit2D& pos,
			const AUnit2D& dims, bool resizeable = false);
	virtual Region* locate(const pixel2& cursor) override;
	virtual void draw(AlloyContext* context) override;
	virtual void drawDebug(AlloyContext* context) override;
	virtual void updateCursor(CursorLocator* cursorLocator) override;
	void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm,
			double pixelRatio, bool clamp = false) override;
	void setNorth(const std::shared_ptr<Region>& region,
			const AUnit1D& fraction);
	void setSouth(const std::shared_ptr<Region>& region,
			const AUnit1D& fraction);
	void setEast(const std::shared_ptr<Region>& region,
			const AUnit1D& fraction);
	void setWest(const std::shared_ptr<Region>& region,
			const AUnit1D& fraction);
	inline void setNorth(const std::shared_ptr<Region>& region,
			float fraction) {
		setNorth(region, UnitPercent(fraction));
	}
	inline void setSouth(const std::shared_ptr<Region>& region,
			float fraction) {
		setSouth(region, UnitPercent(fraction));
	}
	inline void setEast(const std::shared_ptr<Region>& region, float fraction) {
		setEast(region, UnitPercent(fraction));
	}
	inline void setWest(const std::shared_ptr<Region>& region, float fraction) {
		setWest(region, UnitPercent(fraction));
	}

	void setCenter(const std::shared_ptr<Region>& region);
	virtual void pack() override {
		Region::pack();
	}
	virtual void pack(AlloyContext* context) override {
		Region::pack(context);
	}
	void draw();
};

class GlyphRegion: public Region {
public:
	AColor foregroundColor = MakeColor(Theme::Default.LIGHTER);
	std::shared_ptr<Glyph> glyph;
	GlyphRegion(const std::string& name, const std::shared_ptr<Glyph>& glyph) :
			Region(name), glyph(glyph) {
		aspectRule = AspectRule::FixedHeight;
	}
	GlyphRegion(const std::string& name, const std::shared_ptr<Glyph>& glyph,
			const AUnit2D& pos, const AUnit2D& dims) :
			Region(name, pos, dims), glyph(glyph) {
		aspectRule = AspectRule::FixedHeight;
	}
	;
	virtual void drawDebug(AlloyContext* context) override;
	virtual void draw(AlloyContext* context) override;
};

class TextLabel: public Region {
protected:
	std::string label;
	bool truncate;
public:
	virtual pixel2 getTextDimensions(AlloyContext* context);
	HorizontalAlignment horizontalAlignment = HorizontalAlignment::Left;
	VerticalAlignment verticalAlignment = VerticalAlignment::Top;
	FontStyle fontStyle = FontStyle::Normal;
	FontType fontType = FontType::Normal;
	AUnit1D fontSize = UnitPX(24);
	AColor textColor = MakeColor(Theme::Default.LIGHTER);
	AColor textAltColor = MakeColor(Theme::Default.DARKER);
	void setAlignment(const HorizontalAlignment& horizontalAlignment,
			const VerticalAlignment& verticalAlignment) {
		this->horizontalAlignment = horizontalAlignment;
		this->verticalAlignment = verticalAlignment;
	}
	void setTruncate(bool t) {
		truncate = t;
	}
	void setLabel(const std::string& label) {
		this->label = label;
	}
	std::string getLabel() const {
		return label;
	}
	TextLabel(
			const std::string& name = MakeString() << "t" << std::setw(8)
					<< std::setfill('0') << (REGION_COUNTER++)) :
			Region(name), label(name),truncate(true) {
	}
	TextLabel(const std::string& name, const AUnit2D& pos, const AUnit2D& dims) :
			Region(name, pos, dims), label(name) {
	}

	virtual void draw(AlloyContext* context) override;
};
class TextLink: public TextLabel {
protected:
	bool enabled;
public:
	std::function<bool()> onClick;
	void setEnabled(bool m){
		enabled=m;
	}
	TextLink(
			const std::string& name = MakeString() << "t" << std::setw(8)
					<< std::setfill('0') << (REGION_COUNTER++));
	TextLink(const std::string& name, const AUnit2D& pos, const AUnit2D& dims);
	virtual void draw(AlloyContext* context) override;
};
class TextRegion: public TextLabel {

public:
	virtual pixel2 getTextDimensions(AlloyContext* context) override;

	TextRegion(
			const std::string& name = MakeString() << "t" << std::setw(8)
					<< std::setfill('0') << (REGION_COUNTER++)) :
			TextLabel(name) {
	}
	TextRegion(const std::string& name, const AUnit2D& pos, const AUnit2D& dims) :
			TextLabel(name, pos, dims) {
	}

	virtual void draw(AlloyContext* context) override;
};
class TextField: public Composite {
protected:
	bool showDefaultLabel = false;
	bool focused = false;
	std::string label;
	std::string value;
	float th=0;
	float textOffsetX = 0;
	bool showCursor = false;
	std::chrono::high_resolution_clock::time_point lastTime;
	void clear();
	void erase();
	bool handleCursorInput(AlloyContext* context, const InputEvent& e);
	bool handleMouseInput(AlloyContext* context, const InputEvent& e);
	void handleKeyInput(AlloyContext* context, const InputEvent& e);
	void handleCharacterInput(AlloyContext* context, const InputEvent& e);
	void moveCursorTo(int index, bool isShiftHeld = false);
	void dragCursorTo(int index);
	int cursorStart = 0, cursorEnd = 0, textStart = 0;
	bool dragging = false;
	std::string lastValue;
	bool modifiable;
public:
	static const float PADDING;
	AUnit1D fontSize;
	AColor textColor = MakeColor(Theme::Default.LIGHTER);
	void setModifiable(bool m) {
		modifiable = m;
	}
	bool isFocused() const {
		return focused;
	}
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	virtual inline ~TextField() {
	}
	void setShowDefaultLabel(bool show) {
		showDefaultLabel = show;
	}
	void setLabel(const std::string& l) {
		label = l;
	}
	TextField(
			const std::string& name = MakeString() << "t" << std::setw(8)
					<< std::setfill('0') << (REGION_COUNTER++));
	TextField(const std::string& name, const AUnit2D& position,
			const AUnit2D& dimensions);
	virtual void draw(AlloyContext* context) override;
	virtual void setValue(const std::string& value);
	std::string getValue() const {
		return value;
	}
	std::function<void(TextField*)> onTextEntered;
	std::function<void(TextField*)> onKeyInput;
};

class NumberField: public Composite {
protected:
	bool showDefaultLabel = false;
	std::string label;
	std::string value;
	float th;
	float textOffsetX;
	bool showCursor = false;
	std::chrono::high_resolution_clock::time_point lastTime;
	void clear();
	void erase();
	bool handleCursorInput(AlloyContext* context, const InputEvent& e);
	bool handleMouseInput(AlloyContext* context, const InputEvent& e);
	void handleKeyInput(AlloyContext* context, const InputEvent& e);
	void handleCharacterInput(AlloyContext* context, const InputEvent& e);
	void moveCursorTo(int index, bool isShiftHeld = false);
	void dragCursorTo(int index);
	int cursorStart = 0, cursorEnd = 0, textStart = 0;
	bool dragging = false;
	bool valid = true;
	bool isFocused=false;
	std::string lastValue;
	Number numberValue;
	NumberType numberType;
	bool modifiable;
public:
	AUnit1D fontSize;
	AColor invalidNumberColor;
	bool isValid() const {
		return valid;
	}
	void setModifiable(bool m) {
		modifiable = m;
	}
	static const float PADDING;
	AColor textColor = MakeColor(Theme::Default.LIGHTER);
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	virtual inline ~NumberField() {
	}
	void setShowDefaultLabel(bool show) {
		showDefaultLabel = show;
	}
	void setLabel(const std::string& l) {
		label = l;
	}
	NumberField(const std::string& name, const NumberType& numberType);
	NumberField(const std::string& name, const AUnit2D& position,
			const AUnit2D& dimensions, const NumberType& numberType);
	virtual void draw(AlloyContext* context) override;
	virtual bool setValue(const std::string& value);
	bool setNumberValue(const Number& val);
	bool validate();
	Number getValue() const {
		return numberValue;
	}
	Number& getValue() {
		return numberValue;
	}
	std::function<void(NumberField*)> onTextEntered;
	std::function<void(NumberField*)> onKeyInput;
};
class SelectionBox: public Region {
protected:
	int selectedIndex = -1;
	std::string label;
	int maxDisplayEntries = 8;
	int selectionOffset = 0;
	bool scrollingDown = false, scrollingUp = false;
	std::shared_ptr<TimerTask> downTimer, upTimer;
	std::shared_ptr<AwesomeGlyph> downArrow, upArrow;

public:
	FontStyle fontStyle = FontStyle::Normal;
	FontType fontType = FontType::Normal;
	AColor textColor = MakeColor(COLOR_WHITE);
	AColor textAltColor = MakeColor(COLOR_WHITE);
	std::function<bool(SelectionBox*)> onSelect;
	std::vector<std::string> options;
	void setMaxDisplayEntries(int mx) {
		maxDisplayEntries = mx;
	}
	virtual box2px getBounds(bool includeBounds = true) const override;
	std::string getSelection(int index);
	int getSelectedIndex() const {
		return selectedIndex;
	}

	inline void setSelectionOffset(bool offset) {
		selectionOffset = offset;
	}
	void setSelectedIndex(int index);
	void draw(AlloyContext* context) override;
	void addSelection(const std::string& selection) {
		options.push_back(selection);
	}
	size_t getSelectionSize() const {
		return options.size();
	}
	void clearSelections() {
		selectedIndex = -1;
		selectionOffset = 0;
		options.clear();
	}
	void eraseSelection(int index) {
		options.erase(options.begin() + index);
	}
	SelectionBox(const std::string& name,
			const std::vector<std::string>& options =
					std::vector<std::string>());
	SelectionBox(const std::string& name,const AUnit2D& pos,const AUnit2D& dims,
			const std::vector<std::string>& options =
					std::vector<std::string>());
};
class MenuBar;
class MenuItem: public Composite {
protected:

	std::mutex showLock;
	std::shared_ptr<MenuItem> currentSelected;
	std::shared_ptr<MenuItem> requestedSelected;
	std::shared_ptr<MenuItem> currentVisible;
	std::shared_ptr<TimerTask> showTimer;
	const int MENU_DISPLAY_DELAY = 250;
public:
	MenuItem* getSelectedItem();
	MenuBar* menuBar = nullptr;
	FontStyle fontStyle = FontStyle::Normal;
	FontType fontType = FontType::Normal;
	AUnit1D fontSize = UnitPX(24.0f);
	AColor textColor = MakeColor(COLOR_WHITE);
	AColor textAltColor = MakeColor(COLOR_BLACK);
	std::function<void()> onSelect;
	virtual bool isMenu() const {
		return false;
	}
	virtual void setVisible(bool visible) override;
	MenuItem(const std::string& name);
	MenuItem(const std::string& name, const AUnit2D& position,
			const AUnit2D& dimensions);

	virtual void setVisibleItem(const std::shared_ptr<MenuItem>& item,
			bool forceShow = false);
};

class Menu: public MenuItem {
protected:
	int selectedIndex = -1;
	std::shared_ptr<MenuItem> label;
	int maxDisplayEntries = 8;
	int selectionOffset = 0;
	bool scrollingDown = false, scrollingUp = false;
	std::shared_ptr<TimerTask> downTimer, upTimer;
	std::shared_ptr<AwesomeGlyph> downArrow, upArrow;
	std::vector<std::shared_ptr<MenuItem>> options;
	bool fireEvent(int selectedIndex);
public:
	virtual bool isMenu() const override {
		return true;
	}
	virtual void setVisible(bool visible) override;
	void setMaxDisplayEntries(int mx) {
		maxDisplayEntries = mx;
	}
	virtual box2px getBounds(bool includeBounds = true) const override;
	std::string getItem(int index) {
		return (selectedIndex >= 0) ? options[selectedIndex]->name : name;
	}
	int getSelectedIndex() const {
		return selectedIndex;
	}
	inline void setSelectionOffset(bool offset) {
		selectionOffset = offset;
	}
	void setSelectedIndex(int index);
	void draw(AlloyContext* context) override;
	std::shared_ptr<MenuItem> addItem(const std::string& selection) {
		std::shared_ptr<MenuItem> item = std::shared_ptr<MenuItem>(
				new MenuItem(selection));
		options.push_back(item);
		return item;
	}
	void addItem(const std::shared_ptr<MenuItem>& selection);
	Menu(const std::string& name, float menuWidth = 150.0f,
			const std::vector<std::shared_ptr<MenuItem>>& options = std::vector<
					std::shared_ptr<MenuItem>>());
};
class MenuHeader: public MenuItem {
public:
	AColor backgroundAltColor;
	std::shared_ptr<Menu> menu;
	bool isMenuVisible() const {
		return menu->isVisible();
	}
	MenuHeader(const std::shared_ptr<Menu>& menu, const AUnit2D& position,
			const AUnit2D& dimensions);
	virtual void draw(AlloyContext* context) override;
	virtual inline ~MenuHeader() {
	}
};
class MenuBar: public MenuItem {
protected:
	std::list<std::shared_ptr<MenuHeader>> headers;
	std::shared_ptr<Composite> barRegion;
	virtual void setVisibleItem(const std::shared_ptr<MenuItem>& item,
			bool forceShow = false) override;
	bool active;
public:
	void addMenu(const std::shared_ptr<Menu>& menu);
	void hideMenus();
	MenuBar(const std::string& name, const AUnit2D& position,
			const AUnit2D& dimensions);
};
class FileField: public TextField {
protected:
	std::vector<std::string> segmentedPath;
	std::shared_ptr<SelectionBox> selectionBox;
	std::shared_ptr<TimerTask> showTimer;
	bool autoSuggest;
	bool directoryInput;
	int preferredFieldSize;
	void updateSuggestionBox(AlloyContext* context, bool forceValue);
public:
	void setEnableAutoSugest(bool b) {
		autoSuggest = b;
	}
	void setPreferredFieldSize(int w){
		preferredFieldSize=w;
	}
	AColor textColor = MakeColor(Theme::Default.LIGHTER);
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	virtual inline ~FileField() {
	}
	void hideDropDown(AlloyContext* context) {
		selectionBox->setVisible(false);
		context->removeOnTopRegion(selectionBox.get());
	}
	FileField(const std::string& name, const AUnit2D& position,
			const AUnit2D& dimensions,bool directoryInput=false);
	virtual void draw(AlloyContext* context) override;
	virtual void setValue(const std::string& value) override;
};
class ModifiableLabel : public TextField {
protected:
	bool truncate;
public:
	FontType fontType;
	FontStyle fontStyle;
	AColor textAltColor;
	HorizontalAlignment horizontalAlignment = HorizontalAlignment::Left;
	VerticalAlignment verticalAlignment = VerticalAlignment::Top;
	void setAlignment(const HorizontalAlignment& horizontalAlignment,
		const VerticalAlignment& verticalAlignment) {
		this->horizontalAlignment = horizontalAlignment;
		this->verticalAlignment = verticalAlignment;
	}
	void setTruncate(bool b){
		truncate=b;
	}
	virtual pixel2 getTextDimensions(AlloyContext* context);
	ModifiableLabel(const std::string& name, const AUnit2D& position,const AUnit2D& dimensions,bool modifiable=true);
	virtual void draw(AlloyContext* context) override;
};
class ModifiableNumber : public NumberField {
protected:
	bool truncate;
public:
	FontType fontType;
	FontStyle fontStyle;
	AColor textAltColor;

	HorizontalAlignment horizontalAlignment = HorizontalAlignment::Left;
	VerticalAlignment verticalAlignment = VerticalAlignment::Top;
	void setAlignment(const HorizontalAlignment& horizontalAlignment,
		const VerticalAlignment& verticalAlignment) {
		this->horizontalAlignment = horizontalAlignment;
		this->verticalAlignment = verticalAlignment;
	}
	void setTruncate(bool b){
		truncate=b;
	}
	virtual pixel2 getTextDimensions(AlloyContext* context);
	ModifiableNumber(const std::string& name, const AUnit2D& position, const AUnit2D& dimensions, const NumberType& type,bool modifiable=true);
	virtual void draw(AlloyContext* context) override;
};
std::shared_ptr<Composite> MakeComposite(const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const Color& bgColor = COLOR_NONE, const Color& lineColor = COLOR_NONE,
		const AUnit1D& lineWidth = UnitPX(2.0f),
		const Orientation& orientation = Orientation::Unspecified);
std::shared_ptr<GlyphRegion> MakeGlyphRegion(
		const std::shared_ptr<ImageGlyph>& glyph, const AUnit2D& position,
		const AUnit2D& dimensions, const AspectRule& aspectRatio =
				AspectRule::Unspecified, const Color& bgColor = COLOR_NONE,
		const Color& fgColor = COLOR_NONE,
		const Color& borderColor = COLOR_NONE, const AUnit1D& borderWidth =
				UnitPX(2));
std::shared_ptr<GlyphRegion> MakeGlyphRegion(
		const std::shared_ptr<ImageGlyph>& glyph, const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const AspectRule& aspectRatio = AspectRule::Unspecified,
		const Color& bgColor = COLOR_NONE, const Color& fgColor = COLOR_NONE,
		const Color& borderColor = COLOR_NONE, const AUnit1D& borderWidth =
				UnitPX(2));
std::shared_ptr<GlyphRegion> MakeGlyphRegion(
		const std::shared_ptr<AwesomeGlyph>& glyph, const AUnit2D& position,
		const AUnit2D& dimensions, const Color& bgColor = COLOR_NONE,
		const Color& fgColor = COLOR_NONE,
		const Color& borderColor = COLOR_NONE, const AUnit1D& borderWidth =
				UnitPX(2));
std::shared_ptr<GlyphRegion> MakeGlyphRegion(
		const std::shared_ptr<AwesomeGlyph>& glyph, const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const Color& bgColor = COLOR_NONE, const Color& fgColor = COLOR_NONE,
		const Color& borderColor = COLOR_NONE, const AUnit1D& borderWidth =
				UnitPX(2));
std::shared_ptr<TextLabel> MakeTextLabel(const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const FontType& fontType, const AUnit1D& fontSize = UnitPT(14.0f),
		const Color& fontColor = COLOR_WHITE,
		const HorizontalAlignment& halign = HorizontalAlignment::Left,
		const VerticalAlignment& valign = VerticalAlignment::Top);
std::shared_ptr<TextField> MakeTextField(const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const Color& bgColor = Theme::Default.DARK, const Color& textColor =
				Theme::Default.LIGHTER, const std::string& value = "");
std::shared_ptr<NumberField> MakeNumberField(const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const NumberType& type, const Color& bgColor = Theme::Default.DARK,
		const Color& textColor = Theme::Default.LIGHTER,
		const Color& invalidColor = Color(220, 64, 64));
std::shared_ptr<Region> MakeRegion(const std::string& name,
		const AUnit2D& position, const AUnit2D& dimensions,
		const Color& bgColor = COLOR_NONE, const Color& lineColor = COLOR_WHITE,
		const AUnit1D& lineWidth = UnitPX(2.0f));
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const Region & region) {
	ss << "Region: " << region.name << std::endl;
	ss << "\tOrigin: " << region.getOrigin() << std::endl;
	ss << "\tRelative Position: " << region.position << std::endl;
	ss << "\tRelative Dimensions: " << region.dimensions << std::endl;
	ss << "\tBounds: " << region.getBounds() << std::endl;
	ss << "\tAspect Ratio: " << region.getAspectRule() << std::endl;
	ss << "\tBackground Color: " << region.backgroundColor << std::endl;
	if (region.parent != nullptr)
		ss << "\tParent: " << region.parent->name << std::endl;
	return ss;
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const GlyphRegion & region) {
	ss << "Glyph Region: " << region.name << std::endl;
	if (region.glyph.get() != nullptr)
		ss << "\t" << *region.glyph << std::endl;
	ss << "\tOrigin: " << region.getOrigin() << std::endl;
	ss << "\tRelative Position: " << region.position << std::endl;
	ss << "\tRelative Dimensions: " << region.dimensions << std::endl;
	ss << "\tBounds: " << region.getBounds() << std::endl;
	ss << "\tAspect Ratio: " << region.getAspectRule() << " (" << region.getAspectRatio()
			<< ")" << std::endl;
	if (region.parent != nullptr)
		ss << "\tParent: " << region.parent->name << std::endl;
	return ss;
}
template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const TextLabel & region) {
	ss << "Text Label: " << region.name << std::endl;
	ss << "\tOrigin: " << region.getOrigin() << std::endl;
	ss << "\tHorizontal Alignment: " << region.horizontalAlignment << std::endl;
	ss << "\tVertical Alignment: " << region.verticalAlignment << std::endl;
	ss << "\tRelative Position: " << region.position << std::endl;
	ss << "\tRelative Dimensions: " << region.dimensions << std::endl;
	ss << "\tBounds: " << region.getBounds() << std::endl;
	ss << "\tFont Type: " << region.fontType << std::endl;
	ss << "\tFont Size: " << region.fontSize << std::endl;
	ss << "\tFont Color: " << region.textColor << std::endl;
	if (region.parent != nullptr)
		ss << "\tParent: " << region.parent->name << std::endl;
	return ss;
}

template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const Composite& region) {
	ss << "Composite: " << region.name << std::endl;
	ss << "\tOrigin: " << region.getOrigin() << std::endl;
	ss << "\tOrientation: " << region.getOrientation() << std::endl;
	ss << "\tRelative Position: " << region.position << std::endl;
	ss << "\tRelative Dimensions: " << region.dimensions << std::endl;
	ss << "\tBackground Color: " << region.backgroundColor << std::endl;
	ss << "\tBounds: " << region.getBounds() << std::endl;
	return ss;
}
class AdjustableComposite: public Composite {
protected:
	pixel2 cursorDownPosition;
	box2px windowInitialBounds;
	bool resizing;
	WindowPosition winPos;
	bool resizeable;
public:
	bool isResizing() const {
		return resizing;
	}
	bool isResizeable() const {
		return resizeable;
	}
	virtual bool isDragEnabled() const override {
		if (resizeable) {
			return ((dragButton!=-1) && winPos == WindowPosition::Center);
		} else {
			return (dragButton!=-1);
		}
	}
	std::function<void(AdjustableComposite* composite, const box2px& bounds)> onResize;
	AdjustableComposite(const std::string& name, const AUnit2D& pos,
			const AUnit2D& dims, bool resizeable = true);
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	virtual void draw(AlloyContext* context) override;
};
typedef std::shared_ptr<TextLabel> TextLabelPtr;
typedef std::shared_ptr<TextLink> TextLinkPtr;
typedef std::shared_ptr<TextRegion> TextRegionPtr;
typedef std::shared_ptr<TextField> TextFieldPtr;
typedef std::shared_ptr<Composite> CompositePtr;
typedef std::shared_ptr<SelectionBox> SelectionBoxPtr;
typedef std::shared_ptr<BorderComposite> BorderCompositePtr;
typedef std::shared_ptr<GlyphRegion> GlyphRegionPtr;
typedef std::shared_ptr<Region> RegionPtr;
typedef std::shared_ptr<MenuItem> MenuItemPtr;
typedef std::shared_ptr<Menu> MenuPtr;
typedef std::shared_ptr<MenuHeader> MenuHeaderPtr;
typedef std::shared_ptr<MenuBar> MenuBarPtr;
typedef std::shared_ptr<Draw> DrawPtr;
typedef std::shared_ptr<NumberField> NumberFieldPtr;
typedef std::shared_ptr<FileField> FileFieldPtr;
typedef std::shared_ptr<AdjustableComposite> AdjustableCompositePtr;
typedef std::shared_ptr<ModifiableLabel> ModifiableLabelPtr;
typedef std::shared_ptr<ModifiableNumber> ModifiableNumberPtr;
}
#endif /* ALLOYUI_H_ */
