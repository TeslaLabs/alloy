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

#ifndef ALLOYAPPLICATION_H_
#define ALLOYAPPLICATION_H_

#include "AlloyContext.h"
#include "AlloyCommon.h"
#include "GLShader.h"
#include "GLFrameBuffer.h"
#include "AlloyUI.h"
#include "CommonShaders.h"
#include <memory>
#include <list>
#include <chrono>
namespace aly {
class Application {
private:
	float frameRate;
	InputEvent inputEvent;
	std::chrono::steady_clock::time_point lastClickTime;
	static std::shared_ptr<AlloyContext>& context;
	void drawUI();
	void drawDebugUI();
	void draw();
	Composite rootRegion;
	bool showDebugIcon;
	bool forceClose = false;
	
	std::shared_ptr<ImageShader> imageShader;
	std::list<std::exception_ptr> caughtExceptions;
	//std::shared_ptr<GLFrameBuffer> uiFrameBuffer;
	std::function<void(const int2& dimensions)> onResize;
	void initInternal();
public:

	void close() {
		forceClose = true;
	}
	bool isForcedClose() const {
		return forceClose;
	}
	void setOnResize(
			const std::function<void(const int2& dimensions)>& onResizeEvent) {
		onResize = onResizeEvent;
	}
	static inline std::shared_ptr<AlloyContext>& getContext() {
		return context;
	}
	static inline void setContext(const std::shared_ptr<AlloyContext>& c) {
		context=c;
	}
	static inline void removeListener(const EventHandler* region) {
		if (context.get() != nullptr) {
			context->removeListener(region);
		}
	}
	static inline void clearEvents(Region* region) {
		if (context.get() != nullptr) {
			context->clearEvents(region);
		}
	}
	static inline void addListener(EventHandler* region) {
		if (context.get() != nullptr) {
			context->addListener(region);
		}
	}
	static inline bool hasListener(EventHandler* region) {
		if (context.get() != nullptr) {
			return context->hasListener(region);
		} else {
			return false;
		}
	}
	static inline std::string getFullPath(const std::string& partialFile) {
		return AlloyDefaultContext()->getFullPath(partialFile);
	}
	template<class A> std::shared_ptr<Tween>& addTween(AColor& out,
			const Color& start, const Color& end, double duration, const A& a =
					Linear()) {
		return context->addTween(out, start, end, duration, a);
	}
	template<class A> std::shared_ptr<Tween>& addTween(AUnit2D& out,
			const AUnit2D& start, const AUnit2D& end, double duration,
			const A& a = Linear()) {
		return context->addTween(out, start, end, duration, a);
	}
	template<class A> std::shared_ptr<Tween>& addTween(AUnit1D& out,
			const AUnit1D& start, const AUnit1D& end, double duration,
			const A& a = Linear()) {
		return context->addTween(out, start, end, duration, a);
	}

	inline std::shared_ptr<ImageGlyph> createImageGlyph(
			const std::string& fileName, bool mipmap = false) {
		return context->createImageGlyph(fileName);
	}

	inline std::shared_ptr<ImageGlyph> createImageGlyph(const ImageRGBA& img,
			bool mipmap = false) {
		return context->createImageGlyph(img);
	}

	inline std::shared_ptr<ImageGlyph> createImageGlyph(const ImageRGBAf& img,
			bool mipmap = false) {
		ImageRGBA tmp;
		ConvertImage(img, tmp);
		return context->createImageGlyph(tmp);
	}
	inline std::shared_ptr<ImageGlyph> createImageGlyph(const Image1f& img,
			bool mipmap = false) {
		ImageRGBA tmp;
		ConvertImage(img, tmp);
		return context->createImageGlyph(tmp);
	}
	inline std::shared_ptr<ImageGlyph> createImageGlyph(const Image1b& img,
			bool mipmap = false) {
		ImageRGBA tmp;
		ConvertImage(img, tmp);
		return context->createImageGlyph(tmp);
	}
	inline std::shared_ptr<AwesomeGlyph> createAwesomeGlyph(int codePoint,
			const FontStyle& style = FontStyle::Normal, pixel height = 32) {
		return context->createAwesomeGlyph(codePoint, style, height);
	}
	inline std::shared_ptr<Composite> getGlassPane(){
		return getContext()->getGlassPane();
	}
	virtual void onWindowRefresh() {
	}
	virtual void onWindowFocus(int focused);
	void onWindowSize(int width, int height);
	void onFrameBufferSize(int width, int height);
	void onChar(unsigned int codepoint);
	void onKey(int key, int scancode, int action, int mods);
	void onMouseButton(int button, int action, int mods);
	void onCursorPos(double xpos, double ypos);
	void onScroll(double xoffset, double yoffset);
	void onCursorEnter(int enter);
	void fireEvent(const InputEvent& event);
	void getScreenShot(ImageRGBA& img);
	void getScreenShot(ImageRGB& img);
	ImageRGBA getScreenShot();
	InputEvent getLastInputEvent() {
		return inputEvent;
	}
	inline void throwException(std::exception_ptr e) {
		caughtExceptions.push_back(e);
	}
	Application(int w, int h, const std::string& title = "",
			bool showDebugIcon = true);
	float getFrameRate() const {
		return frameRate;
	}
	std::shared_ptr<GLTextureRGB> loadTextureRGB(
			const std::string& partialFile);
	std::shared_ptr<GLTextureRGBA> loadTextureRGBA(
			const std::string& partialFile);
	std::shared_ptr<Font> loadFont(const std::string& name,
			const std::string& partialFile);

	virtual void draw(AlloyContext* context) {
	}
	Composite& getRoot() {
		return rootRegion;
	}
	virtual bool init(Composite& rootNode)=0;
	void run(int swapInterval = 0); //no vsync by default
	void runOnce(const std::string& fileName);
	virtual inline ~Application() {
		context.reset();
	}
};
inline std::shared_ptr<AlloyContext>& AlloyApplicationContext() {
	std::shared_ptr<AlloyContext>& context=Application::getContext();
	if (context.get() == nullptr){
	    throw std::runtime_error("Cannot get GLFW / NanoVG context.");
	}
	return context;
}
inline const Theme& AlloyTheme() {
	return AlloyApplicationContext()->theme;
}
}

#endif /* ALLOYAPPLICATION_H_ */
