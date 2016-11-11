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
#ifndef ALLOYEXPANDBAR_H_
#define ALLOYEXPANDBAR_H_
#include "AlloyUI.h"
#include "AlloyWidget.h"
namespace aly {
	class ExpandRegion : public Composite {
	private:
		std::shared_ptr<TextLabel> selectionLabel;
		std::shared_ptr<TextLabel> arrowIcon;
		std::shared_ptr<Composite> contentRegion;
		std::shared_ptr<Composite> titleContainer;
		bool expanded;
	public:
		pixel expandHeight;
		void setExpanded(bool expanded);
		ExpandRegion(const std::string& name,
			const std::shared_ptr<Composite>& region, const AUnit2D& pos,
			const AUnit2D& dims, pixel expandHeight, bool expanded);
		virtual void draw(AlloyContext* context) override;
	};
	class ExpandBar : public Composite {
	private:
		std::list<std::shared_ptr<ExpandRegion>> expandRegions;
		std::list<std::shared_ptr<Region>> contentRegions;
	public:
		CompositePtr addRegion(const std::shared_ptr<Region>&, pixel expandHeight,
			bool expanded);

		ExpandBar(const std::string& name, const AUnit2D& pos, const AUnit2D& dims);
	};
	typedef std::shared_ptr<ExpandBar> ExpandBarPtr;
	typedef std::shared_ptr<ExpandRegion> ExpandRegionPtr;
}
#endif