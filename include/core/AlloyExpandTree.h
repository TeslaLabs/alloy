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
#ifndef ALLOYEXPANDTREE_H_
#define ALLOYEXPANDTREE_H_
#include "AlloyUI.h"
#include "AlloyWidget.h"
namespace aly {

	class ExpandTree;
	class TreeItem {
	protected:
		static const int PADDING;
		std::string name;
		std::string iconCodeString;
		float fontSize = 24;
		float spaceWidth;
		box2px bounds;
		box2px selectionBounds;
		bool expanded;
		std::vector<std::shared_ptr<TreeItem>> children;
	public:
		std::function<void(TreeItem* item)> onExpand;
		std::function<void(TreeItem* item)> onCollapse;
		std::function<void(TreeItem* item, const InputEvent& e)> onSelect;
		bool isExpanded() const {
			return expanded;
		}
		std::string getName() const {
			return name;
		}
		void clear() {
			children.clear();
		}
		bool hasChildren() const {
			return (children.size() != 0);
		}
		std::vector<std::shared_ptr<TreeItem>>& getChildren() {
			return children;
		}
		const std::vector<std::shared_ptr<TreeItem>>& getChildren() const {
			return children;
		}
		void setExpanded(bool expand);
		virtual void addItem(const std::shared_ptr<TreeItem>& item);
		virtual TreeItem* locate(AlloyContext* context, const pixel2& pt,bool& overArrow);
		TreeItem(const std::string& name = "", int iconCode = 0,
			float fontSize = 24);
		virtual box2px getBounds() const;
		virtual box2px update(AlloyContext* context,
			const pixel2& offset = pixel2(0.0f));
		virtual void draw(ExpandTree* tree, AlloyContext* context,
			const pixel2& offset);
		virtual ~TreeItem() {

		}
	};
	class LeafItem : public TreeItem {
	protected:
		std::function<void(AlloyContext* context, const box2px& bounds)> onDraw;
	public:
		LeafItem(
			const std::function<
			void(AlloyContext* context, const box2px& bounds)>& onDraw,
			const pixel2& dimensions);
		virtual box2px getBounds() const;
		virtual void addItem(const std::shared_ptr<TreeItem>& item) override {
			throw std::runtime_error("Cannot add child to leaf node.");
		}
		virtual box2px update(AlloyContext* context,
			const pixel2& offset = pixel2(0.0f)) override;
		virtual void draw(ExpandTree* tree, AlloyContext* context,
			const pixel2& offset) override;
		virtual TreeItem* locate(AlloyContext* context, const pixel2& pt, bool& overArrow) override;
		virtual ~LeafItem() {
		}
	};
	class ExpandTree : public Composite {
	protected:
		TreeItem root;
		DrawPtr drawRegion;
		TreeItem* selectedItem;
		bool overArrow;
		bool dirty;
	public:
		TreeItem* getSelectedItem() const {
			return selectedItem;
		}
		void setDirty(bool d) {
			dirty = d;
		}
		bool isOverArrow() const {
			return overArrow;
		}
		TreeItem* getRoot() {
			return &root;
		}

		void update(AlloyContext* context);
		ExpandTree(const std::string& name, const AUnit2D& pos,
			const AUnit2D& dims);
		void addItem(const std::shared_ptr<TreeItem>& item);
		void pack(const pixel2& pos, const pixel2& dims, const double2& dpmm,
			double pixelRatio, bool clamp);
		virtual void draw(AlloyContext* context) override;
		virtual void drawDebug(AlloyContext* context) override;
	};
	typedef std::shared_ptr<TreeItem> TreeItemPtr;
	typedef std::shared_ptr<LeafItem> LeafItemPtr;
	typedef std::shared_ptr<ExpandTree> ExpandTreePtr;
}
#endif
