/*
 * Copyright(C) 2014, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
#ifndef GLFRAMEBUFFER_H_
#define GLFRAMEBUFFER_H_
#include "GLTexture.h"
namespace aly {

class GLFrameBuffer: public GLComponent {
protected:

	GLTexture<float, 4, ImageType::FLOAT> texture;
	unsigned int mFrameBufferId;
	unsigned int mDepthBufferId;
public:
	GLFrameBuffer(bool onScreen=true,std::shared_ptr<AlloyContext> context =
			AlloyDefaultContext());
	virtual void update() override;
	virtual void draw() const override;
	void initialize(int w, int h);
	int width() const {
		return texture.width();
	}
	int height() const {
		return texture.height();
	}
	void begin(const float4& clearColor = float4(0, 0, 0, 0),
			bool clearColorBit = true, bool clearDepthBit = true) const;
	void end() const;
	int2 getDimensions() const {
		return int2(texture.width(), texture.height());
	}
	box2px getViewport() const {
		return box2px(pixel2(0.0f, 0.0f),
				pixel2((pixel) texture.width(), (pixel) texture.height()));
	}
	GLTexture<float, 4, ImageType::FLOAT>& getTexture() {
		return texture;
	}
	const GLTexture<float, 4, ImageType::FLOAT>& getTexture() const {
		return texture;
	}
	virtual ~GLFrameBuffer();

};

} /* namespace imagesci */

#endif /* GLFRAMEBUFFER_H_ */
