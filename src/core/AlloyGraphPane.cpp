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
#include "AlloyGraphPane.h"
#include "AlloyApplication.h"
#include "AlloyDrawUtil.h"
namespace aly {
	GraphPane::GraphPane(const std::string& name, const AUnit2D& pos, const AUnit2D& dims) :
		Region(name, pos, dims) {
		graphBounds = box2f(float2(0.0f), float2(-1.0f));
		backgroundColor = MakeColor(AlloyApplicationContext()->theme.DARK);
		setRoundCorners(true);
		xAxisLabel = "x";
		yAxisLabel = "y=f(x)";
		cursorPosition.x = -1;
		cursorPosition.y = -1;
		onEvent = [this](const AlloyContext* context, const InputEvent& e) {
			GraphPane* region = this;
			if (context->isMouseOver(region)) {
				cursorPosition = e.cursor;
			}
			else {
				cursorPosition = float2(-1, -1);
			}
			return false;
		};
		Application::addListener(this);
	}
	void GraphPane::draw(AlloyContext* context) {
		Region::draw(context);
		box2px rbounds = getBounds();
		NVGcontext* nvg = context->nvgContext;
		box2px gbounds = rbounds;
		const float LARGE_TEXT = 18.0f;
		const float MEDIUM_TEXT = 16.0f;
		const float SMALL_TEXT = 12.0f;
		float2 gpos(-1, -1);
		gbounds.position = pixel2(rbounds.position.x + GRAPH_PADDING,
			rbounds.position.y + GRAPH_PADDING);
		gbounds.dimensions = pixel2(rbounds.dimensions.x - GRAPH_PADDING * 2,
			rbounds.dimensions.y - GRAPH_PADDING * 2);
		if (graphBounds.dimensions.x < 0 || graphBounds.dimensions.y < 0) {
			updateGraphBounds();
		}
		nvgBeginPath(nvg);
		nvgRoundedRect(nvg, gbounds.position.x - 2, gbounds.position.y - 2,
			gbounds.dimensions.x + 4, gbounds.dimensions.y + 4,
			context->theme.CORNER_RADIUS);
		nvgFillColor(nvg, context->theme.LIGHTEST);
		nvgFill(nvg);
		//Draw vertical line for x=0
		if (graphBounds.position.x < 0
			&& graphBounds.position.x + graphBounds.dimensions.x > 0) {
			float xpos = -graphBounds.position.x / graphBounds.dimensions.x;
			nvgBeginPath(nvg);
			nvgMoveTo(nvg, xpos * gbounds.dimensions.x + gbounds.position.x,
				gbounds.position.y);
			nvgLineTo(nvg, xpos * gbounds.dimensions.x + gbounds.position.x,
				gbounds.position.y + gbounds.dimensions.y);
			nvgStrokeWidth(nvg, 2.0f);
			nvgStrokeColor(nvg, context->theme.DARK.toSemiTransparent(0.75f));
			nvgStroke(nvg);
		}
		//Draw horizontal line for y=0
		if (graphBounds.position.y < 0
			&& graphBounds.position.y + graphBounds.dimensions.y > 0) {
			float ypos = -graphBounds.position.y / graphBounds.dimensions.y;
			nvgBeginPath(nvg);
			nvgMoveTo(nvg, gbounds.position.x,
				ypos * gbounds.dimensions.y + gbounds.position.y);
			nvgLineTo(nvg, gbounds.position.x + gbounds.dimensions.x,
				ypos * gbounds.dimensions.y + gbounds.position.y);
			nvgStrokeWidth(nvg, 2.0f);
			nvgStrokeColor(nvg, context->theme.DARK.toSemiTransparent(0.75f));
			nvgStroke(nvg);
		}
		if (gbounds.contains(cursorPosition)) {
			context->setCursor(&Cursor::CrossHairs);
			gpos = (cursorPosition - gbounds.position) / gbounds.dimensions;
			gpos.y = 1 - gpos.y;
			gpos = gpos * graphBounds.dimensions + graphBounds.position;

			nvgBeginPath(nvg);
			nvgMoveTo(nvg, cursorPosition.x, gbounds.position.y);
			nvgLineTo(nvg, cursorPosition.x,
				gbounds.position.y + gbounds.dimensions.y);
			nvgStrokeWidth(nvg, 1.0f);
			nvgStrokeColor(nvg, context->theme.DARK.toSemiTransparent(0.25f));
			nvgStroke(nvg);

			nvgBeginPath(nvg);
			nvgMoveTo(nvg, gbounds.position.x, cursorPosition.y);
			nvgLineTo(nvg, gbounds.position.x + gbounds.dimensions.x,
				cursorPosition.y);
			nvgStrokeWidth(nvg, 1.0f);
			nvgStrokeColor(nvg, context->theme.DARK.toSemiTransparent(0.25f));
			nvgStroke(nvg);

		}
		for (GraphDataPtr& curve : curves) {
			std::vector<float2> points = curve->points;
			if (points.size() > 1 && graphBounds.dimensions.x > 0.0f
				&& graphBounds.dimensions.y > 0.0f) {
				NVGcontext* nvg = context->nvgContext;
				float2 last = points[0];
				last = (last - graphBounds.position) / graphBounds.dimensions;
				last.y = 1.0f - last.y;
				last = last * gbounds.dimensions + gbounds.position;
				nvgBeginPath(nvg);
				nvgMoveTo(nvg, last.x, last.y);
				for (int i = 1; i < (int)points.size(); i++) {
					float2 pt = points[i];
					pt = (pt - graphBounds.position) / graphBounds.dimensions;
					pt.y = 1.0f - pt.y;
					pt = pt * gbounds.dimensions + gbounds.position;
					nvgLineTo(nvg, pt.x, pt.y);
					last = pt;
				}
				nvgStrokeWidth(nvg, 2.0f);
				nvgStrokeColor(nvg, curve->color);
				nvgStroke(nvg);
			}
		}

		nvgFontFaceId(nvg, context->getFontHandle(FontType::Bold));
		nvgFontSize(nvg, LARGE_TEXT);
		nvgTextAlign(nvg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
		drawText(nvg, rbounds.position + float2(rbounds.dimensions.x / 2, 2.0f),
			name, FontStyle::Outline, context->theme.LIGHTEST,
			context->theme.DARK);
		nvgFontSize(nvg, MEDIUM_TEXT);
		nvgFontFaceId(nvg, context->getFontHandle(FontType::Bold));
		nvgTextAlign(nvg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
		drawText(nvg,
			rbounds.position
			+ float2(rbounds.dimensions.x / 2,
				rbounds.dimensions.y - 4.0f), xAxisLabel,
			FontStyle::Outline, context->theme.LIGHTEST, context->theme.DARK);
		nvgTextAlign(nvg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
		nvgSave(nvg);
		pixel2 center = rbounds.position
			+ float2(2.0f, rbounds.dimensions.y * 0.5f);
		nvgTranslate(nvg, center.x, center.y);
		nvgRotate(nvg, -ALY_PI * 0.5f);
		drawText(nvg, pixel2(0, 2), yAxisLabel, FontStyle::Outline,
			context->theme.LIGHTEST, context->theme.DARK);
		nvgRestore(nvg);
		nvgFontSize(nvg, SMALL_TEXT);
		nvgTextAlign(nvg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
		drawText(nvg, rbounds.position + float2(GRAPH_PADDING, GRAPH_PADDING),
			MakeString() << std::setprecision(2)
			<< (graphBounds.position.y + graphBounds.dimensions.y),
			FontStyle::Outline, context->theme.LIGHTER, context->theme.DARK);
		nvgTextAlign(nvg, NVG_ALIGN_RIGHT | NVG_ALIGN_BOTTOM);
		drawText(nvg,
			rbounds.position
			+ float2(GRAPH_PADDING,
				rbounds.dimensions.y - GRAPH_PADDING),
			MakeString() << std::setprecision(2) << graphBounds.position.y,
			FontStyle::Outline, context->theme.LIGHTER, context->theme.DARK);
		nvgTextAlign(nvg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
		drawText(nvg,
			rbounds.position
			+ float2(rbounds.dimensions.x - GRAPH_PADDING,
				rbounds.dimensions.y - GRAPH_PADDING + 2),
			MakeString() << std::setprecision(2)
			<< (graphBounds.position.x + graphBounds.dimensions.x),
			FontStyle::Outline, context->theme.LIGHTER, context->theme.DARK);
		nvgTextAlign(nvg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		drawText(nvg,
			rbounds.position
			+ float2(GRAPH_PADDING,
				rbounds.dimensions.y - GRAPH_PADDING + 2),
			MakeString() << std::setprecision(2) << graphBounds.position.x,
			FontStyle::Outline, context->theme.LIGHTER, context->theme.DARK);

		if (cursorPosition.x >= 0) {
			float minDist = 1E30f;
			float bestY = 0;
			GraphDataPtr closestCurve;
			for (GraphDataPtr& curve : curves) {
				float y = curve->interpolate(gpos.x);
				if (y != GraphData::NO_INTERSECT) {
					if (std::abs(y - gpos.y) < minDist) {
						minDist = std::abs(y - gpos.y);
						bestY = y;
						closestCurve = curve;
					}
				}
			}
			if (closestCurve.get() != nullptr) {
				nvgBeginPath(nvg);
				nvgStrokeWidth(nvg, 2.0f);
				nvgFillColor(nvg, closestCurve->color);
				nvgStrokeColor(nvg, context->theme.LIGHTER);
				float2 pt(gpos.x, bestY);
				pt = (pt - graphBounds.position) / graphBounds.dimensions;
				pt.y = 1.0f - pt.y;
				pt = pt * gbounds.dimensions + gbounds.position;
				nvgCircle(nvg, pt.x, pt.y, 4);
				nvgFill(nvg);
				nvgStroke(nvg);

				nvgBeginPath(nvg);
				nvgFillColor(nvg, context->theme.DARK);
				nvgCircle(nvg, cursorPosition.x, cursorPosition.y, 2);
				nvgFill(nvg);

				nvgTextAlign(nvg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
				nvgFontSize(nvg, MEDIUM_TEXT);
				drawText(nvg, float2(pt.x - 8, pt.y), closestCurve->name,
					FontStyle::Outline, context->theme.LIGHTEST,
					context->theme.DARK);
				nvgTextAlign(nvg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
				drawText(nvg, float2(pt.x + 8, pt.y),
					MakeString() << "(" << std::setprecision(2) << gpos.x
					<< ", " << std::setprecision(2) << bestY << ")",
					FontStyle::Outline, context->theme.LIGHTEST,
					context->theme.DARK);

			}
			else {

				nvgBeginPath(nvg);
				nvgFillColor(nvg, context->theme.DARK);
				nvgCircle(nvg, cursorPosition.x, cursorPosition.y, 2);
				nvgFill(nvg);
			}
		}
	}
	void GraphPane::add(const GraphDataPtr& curve) {
		curves.push_back(curve);
	}
	std::shared_ptr<GraphData> GraphPane::add(const GraphData& curve) {
		std::shared_ptr<GraphData> curvePtr = std::shared_ptr<GraphData>(
			new GraphData(curve));
		curves.push_back(curvePtr);
		return curvePtr;
	}
	box2f GraphPane::updateGraphBounds() {
		float2 minPt(std::numeric_limits<float>::max());
		float2 maxPt(std::numeric_limits<float>::min());
		for (GraphDataPtr& curve : curves) {
			for (float2& pt : curve->points) {
				minPt = aly::min(pt, minPt);
				maxPt = aly::max(pt, maxPt);
			}
		}
		graphBounds = box2f(minPt, maxPt - minPt);
		return graphBounds;
	}
	const float GraphData::NO_INTERSECT = std::numeric_limits<float>::max();
	float GraphData::interpolate(float x) const {
		if (points.size() < 2) {
			return NO_INTERSECT;
		}
		if (x < points.front().x || x > points.back().x) {
			return NO_INTERSECT;
		}
		float y = 0;
		int startX = 0;
		int endX = (int)points.size() - 1;
		for (int i = 1; i < (int)points.size(); i++) {
			if (x < points[i].x) {
				startX = i - 1;
				break;
			}
		}
		for (int i = (int)points.size() - 2; i >= 0; i--) {
			if (x > points[i].x) {
				endX = i + 1;
				break;
			}
		}
		if (startX == endX) {
			return points[startX].y;
		}
		else if (endX - startX > 0) {
			float diff = (points[endX].x - points[startX].x);
			if (diff < 1E-10f) {
				return 0.5f * (points[endX].y + points[startX].y);
			}
			else {
				y = points[startX].y
					+ (x - points[startX].x)
					* (points[endX].y - points[startX].y) / diff;
				return y;
			}
		}
		return NO_INTERSECT;
	}

}