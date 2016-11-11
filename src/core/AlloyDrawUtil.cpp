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
#include "AlloyDrawUtil.h"
#include "nanovg.h"
namespace aly {
std::list<NVGscissorBounds> NVG_SCISSOR_STACK;

int pushScissor(NVGcontext* ctx, float x, float y, float w, float h) {
	NVGscissorBounds sb = nvgCurrentScissor(ctx);
	NVG_SCISSOR_STACK.push_back(sb);
	nvgScissor(ctx, x, y, w, h);
	return (int) NVG_SCISSOR_STACK.size();
}
int pushScissor(NVGcontext* ctx, const box2px& bounds) {
	return pushScissor(ctx, bounds.position.x, bounds.position.y,
			bounds.dimensions.x, bounds.dimensions.y);
}
int pushScissor(NVGcontext* ctx, const pixel2& position,
		const pixel2& dimensions) {
	return pushScissor(ctx, position.x, position.y, dimensions.x, dimensions.y);
}

int popScissor(NVGcontext* ctx) {
	if (NVG_SCISSOR_STACK.size() == 0)
		throw std::runtime_error("Cannot pop an empty scissor stack.");
	NVGscissorBounds back = NVG_SCISSOR_STACK.back();
	NVG_SCISSOR_STACK.pop_back();
	if (back.w >= 0 && back.h >= 0) {
		nvgScissor(ctx, back.x, back.y, back.w, back.h);
	} else {
		nvgResetScissor(ctx);
	}
	return (int) NVG_SCISSOR_STACK.size();
}
float drawParagraph(NVGcontext* nvg, float x, float y, float width, float lineh,
		const char* txt, const FontStyle& style, const Color& foreground,
		const Color& background) {
	std::vector<NVGtextRow> rows(3);
	nvgFontSize(nvg, lineh);
	const char* start = txt;
	const char* end = txt + strlen(txt);
	float h = lineh;
	while (int nrows = nvgTextBreakLines(nvg, start, end, width, rows.data(), 3)) {
		for (int i = 0; i < nrows; i++) {
			NVGtextRow* row = &rows[i];
			drawText(nvg, x, y, row->start, style, foreground, background,
					row->end);
			y += lineh;
			h = std::max(y, h);
		}
		start = rows[nrows - 1].next;
	}
	return h;
}
float paragraphHeight(NVGcontext* nvg,float width, float lineh,const char* txt){
	std::vector<NVGtextRow> rows(3);
	nvgFontSize(nvg, lineh);
	const char* start = txt;
	const char* end = txt + strlen(txt);
	float h = lineh;
	float y=0;
	while (int nrows = nvgTextBreakLines(nvg, start, end, width, rows.data(), 3)) {
		for (int i = 0; i < nrows; i++) {
			y += lineh;
			h = std::max(y, h);
		}
		start = rows[nrows - 1].next;
	}
	return h;
}
float drawText(NVGcontext* nvg, float x, float y, const char* txt,
		const FontStyle& style, const Color& foreground,
		const Color& background, const char* end) {
	float ret;
	const float shift = 1.0f;
	nvgFillColor(nvg, background);
	if (style == FontStyle::Outline) {
		nvgText(nvg, x + shift, y, txt, end);
		nvgText(nvg, x - shift, y, txt, end);
		nvgText(nvg, x, y + shift, txt, end);
		nvgText(nvg, x, y - shift, txt, end);
		nvgText(nvg, x + shift, y - shift, txt, end);
		nvgText(nvg, x + shift, y + shift, txt, end);
		nvgText(nvg, x + shift, y - shift, txt, end);
		nvgText(nvg, x + shift, y + shift, txt, end);
		nvgText(nvg, x - shift, y - shift, txt, end);
		nvgText(nvg, x - shift, y + shift, txt, end);
		nvgText(nvg, x - shift, y - shift, txt, end);
		nvgText(nvg, x - shift, y + shift, txt, end);
	} else if (style == FontStyle::Shadow) {
		nvgText(nvg, x, y + shift, txt, end);
	} else if (style == FontStyle::Glow) {
		nvgFontBlur(nvg, 4.0f);
		nvgText(nvg, x, y, txt, end);
		nvgFontBlur(nvg, 0.0f);
	}
	nvgFillColor(nvg, foreground);
	ret = nvgText(nvg, x, y, txt, end);
	return ret;
}
}

