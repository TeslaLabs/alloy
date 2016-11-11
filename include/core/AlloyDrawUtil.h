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

#ifndef ALLOYDRAWUTIL_H_
#define ALLOYDRAWUTIL_H_

#include "AlloyEnum.h"
#include "AlloyMath.h"
#include "AlloyUI.h"
#include "AlloyUnits.h"
class NVGContext;

namespace aly {
int pushScissor(NVGcontext* ctx, float x, float y, float w, float h);
int pushScissor(NVGcontext* ctx, const box2px& bounds);
int pushScissor(NVGcontext* ctx, const pixel2& pt, const pixel2& dims);
int popScissor(NVGcontext* ctx);
float drawText(NVGcontext* ctx, float x, float y, const char* string,
		const FontStyle& style = FontStyle::Normal, const Color& foreground =
				COLOR_WHITE, const Color& background = COLOR_BLACK,
		const char* end = nullptr);
float drawParagraph(NVGcontext* ctx, float x, float y, float width,
		float lineHeight, const char* string, const FontStyle& style =
				FontStyle::Normal, const Color& foreground = COLOR_WHITE,
		const Color& background = COLOR_BLACK);
float paragraphHeight(NVGcontext* nvg,float width, float lineh,const char* txt);
inline float drawText(NVGcontext* ctx, const pixel2& pix,
		const std::string& txt, const FontStyle& style = FontStyle::Normal,
		const Color& foreground = COLOR_WHITE, const Color& background =
				COLOR_BLACK, const char* end = nullptr) {
	return drawText(ctx, pix.x, pix.y, txt.c_str(), style, foreground,
			background, end);
}
inline float drawParagraph(NVGcontext* ctx, const pixel2& pix, float width,
		float lineHeight, const std::string& txt, const FontStyle& style =
				FontStyle::Normal, const Color& foreground = COLOR_WHITE,
		const Color& background = COLOR_BLACK) {
	return drawParagraph(ctx, pix.x, pix.y, width, lineHeight, txt.c_str(), style,
			foreground, background);
}
}

#endif /* ALLOYDRAWUTIL_H_ */
