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

#ifndef GLCOMPONENT_H_
#define GLCOMPONENT_H_
#include <vector>
#include <list>
#include <memory>
#include "AlloyContext.h"
#include "AlloyMath.h"
namespace aly {
class GLComponentGroup;
class GLComponent {
protected:
	std::shared_ptr<AlloyContext> context;
	bool onScreen=false;
public:
	bool isOnScreenRender() const {
		return onScreen;
	}
	friend class GLComponentGroup;
	box2i bounds;
	std::shared_ptr<AlloyContext>& getContext() {
		return context;
	}
	virtual void draw() const =0;
	virtual void update()=0;
	virtual inline ~GLComponent() {
	}

	GLComponent(bool onScreen = true, const std::shared_ptr<AlloyContext>& context =
			AlloyDefaultContext()) :
			context(context), onScreen(onScreen), bounds() {
	}
	GLComponent(const box2i& b, bool onScreen = true, std::shared_ptr<AlloyContext>& context =
			AlloyDefaultContext()) :
			context(context), onScreen(onScreen),bounds(b) {
	}

	inline void setBounds(int x, int y, int w, int h) {
		bounds.position.x = x;
		bounds.position.y = y;
		bounds.dimensions.x = w;
		bounds.dimensions.y = h;
	}
};

class GLComponentGroup: protected GLComponent {
protected:
	std::list<std::unique_ptr<GLComponent>> components;
public:
	virtual void update() override {
		for (std::unique_ptr<GLComponent>& comp : components) {
			comp->update();
		}
	}
	virtual void draw() const override {
		for (const std::unique_ptr<GLComponent>& comp : components) {
			comp->draw();
		}
	}
	GLComponentGroup(bool onScreen = true,const std::shared_ptr<AlloyContext>& context =
			AlloyDefaultContext()) :
			GLComponent(onScreen,context) {

	}
	virtual ~GLComponentGroup()=0;
};

} /* namespace imagesci */

#endif /* GLCOMPONENT_H_ */
