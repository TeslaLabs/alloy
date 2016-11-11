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
#ifndef ALLOYTablePane_H_
#define ALLOYTablePane_H_
#include "AlloyUI.h"
#include "AlloyWidget.h"
#include "AlloyAny.h"
#include "AlloyColorSelector.h"
#include <string>
#include <vector>
namespace aly {
	class TablePane;
	class TableEntry : public Composite {
	public:
		TableEntry(const std::string& name, const AUnit2D& pos, const AUnit2D& dims);
		virtual int compare(const std::shared_ptr<TableEntry>& entry) const = 0;
	};
	class TableStringEntry : public TableEntry {
		protected:
			ModifiableLabelPtr value;
		public:
			TableStringEntry(const std::string& name, const std::string& value, bool modifiable=false, const HorizontalAlignment& alignment= HorizontalAlignment::Left);
			std::string getValue() const{
				return value->getValue();
			}
			void setValue(const std::string& val) {
				value->setValue(val);
			}
			ModifiableLabelPtr getRegion() const {
				return value;
			}
			virtual int compare(const std::shared_ptr<TableEntry>& entry) const override;
	};
	class TableIconStringEntry : public TableEntry {
	protected:
		ModifiableLabelPtr value;
	public:
		TableIconStringEntry(const std::string& name, int iconCode, const std::string& value, bool modifiable = false, const HorizontalAlignment& alignment = HorizontalAlignment::Left);
		std::string getValue() const {
			return value->getValue();
		}
		void setValue(const std::string& val) {
			value->setValue(val);
		}
		ModifiableLabelPtr getRegion() const {
			return value;
		}
		virtual int compare(const std::shared_ptr<TableEntry>& entry) const override;
	};
	class TableNumberEntry : public TableEntry {
	protected:
		ModifiableNumberPtr value;
	public:
		TableNumberEntry(const std::string& name, const Number& value, bool modifiable=false);
		Number getValue() const {
			return value->getValue();
		}
		void setValue(const Number& num) {
			value->setNumberValue(num);
		}
		ModifiableNumberPtr getRegion() const {
			return value;
		}
		virtual int compare(const std::shared_ptr<TableEntry>& entry) const override;
	};
	class TableSelectionEntry : public TableEntry {
	protected:
		SelectionPtr value;
	public:
		TableSelectionEntry(const std::string& name, const std::vector<std::string>& options,int selectedIndex=-1);
		int getValue() const {
			return value->getSelectedIndex();
		}
		void setValue(const int& num) {
			value->setValue(num);
		}
		SelectionPtr getRegion() const {
			return value;
		}

		virtual int compare(const std::shared_ptr<TableEntry>& entry) const override;
	};
	class TableCheckBoxEntry : public TableEntry {
	protected:
		CheckBoxPtr value;
	public:
		TableCheckBoxEntry(const std::string& name, bool value);
		bool getValue() const {
			return value->getValue();
		}
		void setValue(const bool& val) {
			value->setValue(val);
		}
		CheckBoxPtr getRegion() const {
			return value;
		}

		virtual int compare(const std::shared_ptr<TableEntry>& entry) const override;
	};
	class TableToggleBoxEntry : public TableEntry {
	protected:
		ToggleBoxPtr value;
	public:
		TableToggleBoxEntry(const std::string& name, bool value);
		bool getValue() const {
			return value->getValue();
		}
		void setValue(const bool& val) {
			value->setValue(val);
		}
		ToggleBoxPtr getRegion() const {
			return value;
		}

		virtual int compare(const std::shared_ptr<TableEntry>& entry) const override;
	};
	class TableColorEntry : public TableEntry {
	protected:
		ColorSelectorPtr value;
	public:
		TableColorEntry(const std::string& name, const Color& value);
		Color getValue() const {
			return value->getValue();
		}
		void setValue(const Color& num) {
			value->setValue(num);
		}
		ColorSelectorPtr getRegion() const {
			return value;
		}

		virtual int compare(const std::shared_ptr<TableEntry>& entry) const override;
	};
	class TableProgressBarEntry : public TableEntry {
	protected:
		ProgressBarPtr value;
	public:
		TableProgressBarEntry(const std::string& name, float value=0.0f);
		float getValue() const {
			return value->getValue();
		}
		void setValue(const float& num) {
			value->setValue(num);
		}
		void setValue(const std::string& label,const float& num) {
			value->setValue(label,num);
		}
		ProgressBarPtr getRegion() const {
			return value;
		}

		virtual int compare(const std::shared_ptr<TableEntry>& entry) const override;
	};
	class TableRow: public Composite{
	protected:
		bool selected;
		TablePane* tablePane;
		std::map<int, std::shared_ptr<TableEntry>> columns;
	public:
		int compare(const std::shared_ptr<TableRow>& row,int column);
		friend class TablePane;
		void setSelected(bool selected);
		bool isSelected();
		void setColumn(int c,const std::shared_ptr<TableEntry>& region);
		std::shared_ptr<TableEntry> getColumn(int i) const;
		TableRow(TablePane* tablePane, const std::string& name);
		virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm,
			double pixelRatio, bool clamp) override;
	};
	class LazyTableComposite: public Composite{
	protected:
		float entryHeight;
	public:
		LazyTableComposite(const std::string& name, const AUnit2D& pos,const AUnit2D& dims,float entryHeight);
		virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm,
			double pixelRatio, bool clamp) override;
	};
	class TablePane : public Composite {
	protected:
		bool enableMultiSelection;
		box2px dragBox;
		bool scrollingDown;
		bool scrollingUp;
		bool dirty;
		const int columns;
		std::vector<std::shared_ptr<TableRow>> rows;
		std::list<TableRow*> lastSelected;
		void addToActiveList(TableRow* entry) {
			lastSelected.push_back(entry);
		}
		void update();

		void clearActiveList() {
			lastSelected.clear();
		}
		std::shared_ptr<LazyTableComposite> contentRegion;
		float entryHeight;
		std::vector<TextIconButtonPtr> columnHeaders;
		std::vector<AUnit1D> columnWidths;
		std::vector<pixel> columnWidthPixels;
		std::vector<int> sortDirections;
		std::vector<bool> sortMask;
		void sortColumn(int c);
	public:
		friend class TableRow;
		box2px getDragBox() const {
			return dragBox;
		}
		void setSortColumn(int c,bool toggle=true);
		void setSortEnabled(int c, bool s);
		void sortSelectedColumn();
		void setColumnLabel(int c, const std::string& label);
		void setColumnWidth(int c, const AUnit1D& unit);
		pixel getColumnWidthPixels(int c) const;
		AUnit1D getColumnWidth(int c) const;

		std::vector<std::shared_ptr<TableRow>>& getRows() {
			return rows;
		}
		void addRow(const std::shared_ptr<TableRow>& entry) {
			rows.push_back(entry);
			dirty = true;
		}
		virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm,
			double pixelRatio, bool clamp) override;
		virtual void draw(AlloyContext* context) override;
		void clearRows() {
			rows.clear();
			lastSelected.clear();
			dirty = true;
		}
		TableRow* getLastSelected() {
			if (lastSelected.size() > 0)
				return lastSelected.back();
			else
				return nullptr;
		}
		bool isDraggingOver(TableRow* entry);
		TablePane(const std::string& name, const AUnit2D& pos, const AUnit2D& dims,int columns, float entryHeight = 30.0f);
		std::shared_ptr<TableRow> addRow(const std::string& name="");
		int getColumns() const {
			return columns;
		}
		virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;

		void setEnableMultiSelection(bool enable) {
			enableMultiSelection = enable;
		}
		bool onMouseDown(TableRow* entry, AlloyContext* context,const InputEvent& e);
		std::function<void(TableRow*, const InputEvent&)> onSelect;
	};

	typedef std::shared_ptr<TablePane> TablePanePtr;
	typedef std::shared_ptr<TableRow> TableRowPtr;
	typedef std::shared_ptr<TableEntry> TableEntryPtr;

	typedef std::shared_ptr<TableNumberEntry> TableNumberEntryPtr;
	typedef std::shared_ptr<TableColorEntry> TableColorEntryPtr;
	typedef std::shared_ptr<TableSelectionEntry> TableSelectionEntryPtr;
	typedef std::shared_ptr<TableProgressBarEntry> TableProgressBarEntryPtr;
	typedef std::shared_ptr<TableCheckBoxEntry> TableCheckBoxEntryPtr;
	typedef std::shared_ptr<TableToggleBoxEntry> TableToggleBoxEntryPtr;
	typedef std::shared_ptr<TableStringEntry> TableStringEntryPtr;
}

#endif
