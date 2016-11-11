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
#include "GLFrameBuffer.h"
#include "GLShader.h"
#include "AlloyContext.h"
namespace aly {

GLFrameBuffer::GLFrameBuffer(bool onScreen, std::shared_ptr<AlloyContext> context) :
		GLComponent(onScreen, context),texture(onScreen,
				context),mFrameBufferId(0), mDepthBufferId(0){

}
void GLFrameBuffer::initialize(int w, int h) {
	if (context.get() == nullptr)throw std::runtime_error("Framebuffer has not been assigend a context.");
	context->begin(onScreen);
	if (mFrameBufferId != 0)
		glDeleteFramebuffers(1, &mFrameBufferId);
	if (mDepthBufferId != 0)
		glDeleteRenderbuffers(1, &mDepthBufferId);
	mFrameBufferId = 0;
	mDepthBufferId = 0;
	context->end();
	CHECK_GL_ERROR();
	texture.load(Image4f(w, h), false);
	CHECK_GL_ERROR();
	update();
}
GLFrameBuffer::~GLFrameBuffer() {
	context->begin(onScreen);
	if (mFrameBufferId != 0)
		glDeleteFramebuffers(1, &mFrameBufferId);
	if (mDepthBufferId != 0)
		glDeleteRenderbuffers(1, &mDepthBufferId);
	context->end();
}
void GLFrameBuffer::begin(const float4& clearColor, bool clearColorBit,
		bool clearDepthBit) const {
	if (texture.width() * texture.height() == 0)
		throw std::runtime_error("Framebuffer has not been initialized.");
	context->begin(onScreen);
	glViewport(0, 0, texture.width(), texture.height());
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, mDepthBufferId);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	GLuint flags = GL_STENCIL_BUFFER_BIT;
	if (clearColorBit)
		flags |= GL_COLOR_BUFFER_BIT;
	if (clearDepthBit)
		flags |= GL_DEPTH_BUFFER_BIT;
	glClear(flags);
	CHECK_GL_ERROR();
}
void GLFrameBuffer::end() const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glViewport(0, 0, context->getFrameBufferWidth(), context->getFrameBufferHeight());
	CHECK_GL_ERROR();
	context->end();
}
void GLFrameBuffer::draw() const {
	texture.draw();
}
void GLFrameBuffer::update() {
	texture.update();
	context->begin(onScreen);
	if (mDepthBufferId != 0) {
		glDeleteRenderbuffers(1, &mDepthBufferId);
	}
	glGenRenderbuffers(1, &mDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, mDepthBufferId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8,texture.width(), texture.height());
	if (mFrameBufferId != 0) {
		glDeleteRenderbuffers(1, &mFrameBufferId);
	}
	glGenFramebuffers(1, &mFrameBufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8,texture.width(), texture.height());
	glBindRenderbuffer( GL_RENDERBUFFER, 0 );
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,mDepthBufferId);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mFrameBufferId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,texture.getTextureId(), 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	CHECK_GL_ERROR();
	context->end();
}
}

