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

#ifndef ALLOYTABPANE_H_
#define ALLOYTABPANE_H_
#include "AlloyUI.h"
#include "AlloyWidget.h"
namespace aly {
	class TabBar;
	struct TabPane;
	class TabHeader : public Composite {
	protected:
		TextLabelPtr textLabel;
		TabPane* parentPane;
		bool focused;
		bool closeable;
	public:
		friend struct TabPane;
		TabHeader(const std::string& name,bool closeable,TabPane* parent = nullptr);
		virtual void draw(AlloyContext* context) override;
		TabBar* getBar() const;
		void setSelected() const;
		bool isSelected() const;
		pixel2 getPreferredDimensions(AlloyContext* context) const;
	};
	struct TabPane {
		std::shared_ptr<TabHeader> header;
		std::shared_ptr<Composite> region;
		TabBar* parent;
		box2px bounds;
		void setLabel(const std::string& name);
		bool isFocused() const {
			return header->focused;
		}
		virtual ~TabPane() {}
		TabPane(const std::shared_ptr<Composite>& region,bool closeable=true);
	};
	class TabBar : public Composite {
	protected:
		std::vector<std::shared_ptr<TabPane>> panes;
		std::shared_ptr<Composite> barRegion;
		std::shared_ptr<Composite> contentRegion;
		std::shared_ptr<SelectionBox> selectionBox;
		std::shared_ptr<IconButton> tabDropButton;
		pixel2 cursorDownPosition;
		TabPane* selectedPane;
		TabPane* dragPane;
		void sortPanes();
	public:
		static const float TAB_SPACING;
		static const float TAB_HEIGHT;
		std::function<bool(TabPane*)> onClose;
		std::function<void(TabPane*)> onSelect;
		void close(TabPane* pane);
		void setSelected(TabPane* s);
		bool isSelected(TabPane* s) const {
			return (s != nullptr&&s == selectedPane);
		}
		TabPane* getSelected() const {
			return selectedPane;
		}
		std::shared_ptr<TabPane> getPane(const std::shared_ptr<Region>& region) const;
		void addPane(const std::shared_ptr<TabPane>& tabPane);
		TabBar(const std::string& name, const AUnit2D& position,
			const AUnit2D& dimensions);
		virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)override;
		virtual void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm, double pixelRatio, bool clamp = false) override;
		virtual void draw(AlloyContext* context) override;
	};
	typedef std::shared_ptr<TabHeader> TabHeaderPtr;
	typedef std::shared_ptr<TabPane> TabPanePtr;
	typedef std::shared_ptr<TabBar> TabBarPtr;
}
#endif
