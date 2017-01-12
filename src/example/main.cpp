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
#include "Alloy.h"
#include "../../include/example/UnitsEx.h"
#include "../../include/example/CompositeEx.h"
#include "../../include/example/EventsEx.h"
#include "../../include/example/DragEx.h"
#include "../../include/example/TweenEx.h"
#include "../../include/example/ImageEx.h"
#include "../../include/example/ControlsEx.h"
#include "../../include/example/DialogsEx.h"
#include "../../include/example/MeshMatcapEx.h"
#include "../../include/example/MeshWireframeEx.h"
#include "../../include/example/MeshSubdivideEx.h"
#include "../../include/example/MeshTextureEx.h"
#include "../../include/example/MeshVertexColorEx.h"
#include "../../include/example/MeshParticleEx.h"
#include "../../include/example/MeshDepthEx.h"
#include "../../include/example/MeshPhongEx.h"
#include "../../include/example/LaplaceFillEx.h"
#include "../../include/example/PoissonBlendEx.h"
#include "../../include/example/PoissonInpaintEx.h"
#include "../../include/example/ImageProcessingEx.h"
#include "../../include/example/MeshPickerEx.h"
#include "../../include/example/MeshSmoothEx.h"
#include "../../include/example/ColorSpaceEx.h"
#include "../../include/example/MeshPrimitivesEx.h"
#include "../../include/example/MenuEx.h"
#include "../../include/example/LocatorEx.h"
#include "../../include/example/GraphPaneEx.h"
#include "../../include/example/SplineEx.h"
#include "../../include/example/IntersectorEx.h"
#include "../../include/example/WindowPaneEx.h"
#include "../../include/example/DistanceFieldEx.h"
#include "../../include/example/DataFlowEx.h"
#include "../../include/example/ForceDirectedGraphEx.h"
#include "../../include/example/ExpandBarEx.h"
#include "../../include/example/ExpandTreeEx.h"
#include "../../include/example/OneEuroFilterEx.h"
#include "../../include/example/TabPaneEx.h"
#include "../../include/example/ParameterPaneEx.h"
#include "../../include/example/TablePaneEx.h"
#include "../../include/example/TimelineEx.h"
#include "../../include/example/MeshOptimizationEx.h"
#include "../../include/example/ImageFeaturesEx.h"
#include "../../include/example/MeshTextureMapEx.h"
#include "../../include/example/MeshReconstructionEx.h"
#include "../../include/example/SoftBodyEx.h"
#include <cstring>

/*
 For simple execution, main method should look like:

 int main(int argc, char *argv[]) {
	 try {
		 MyApplication app;
		 app.run();
		 return 0;
	 } catch (std::exception& e) {
		 std::cout << "Error: " << e.what() << std::endl;
		 return 1;
	 }
 }

 */
#ifndef EXAMPLE_NAME
#define EXAMPLE_NAME ""
#endif
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
using namespace aly;
struct Example {
	std::string name;
	Application* app;
	virtual Application* getApplication()=0;
	virtual std::string getName() const =0;
	Example(const std::string& name) :
			name(name), app(nullptr) {
	}
	virtual ~Example() {
	}
};
template<class T> struct ExampleT: public Example {
	std::string getName() const {
		return name;
	}
	ExampleT(const std::string& name) :
			Example(name) {
	}
	Application* getApplication() {
		if (app == nullptr) {
			app = new T();
		}
		return app;
	}
	virtual ~ExampleT() {
		if (app)
			 app;
	}
};
typedef std::unique_ptr<Example> ExamplePtr;
#define MAKE_EXAMPLE(NAME) ExamplePtr(new ExampleT<NAME>(#NAME))

bool SANITY_CHECK() {
	bool ret = true;
	ret &= SANITY_CHECK_LOCATOR();
	//SANITY_CHECK_ANY();
	//SANITY_CHECK_SVD();
	//SANITY_CHECK_ALGO();
	//SANITY_CHECK_IMAGE();
	//SANITY_CHECK_UI();
	//SANITY_CHECK_CEREAL();
	//SANITY_CHECK_KDTREE();
	//SANITY_CHECK_PYRAMID();
	//SANITY_CHECK_SPARSE_SOLVE();
	//SANITY_CHECK_DENSE_SOLVE();
	//SANITY_CHECK_DENSE_MATRIX();
	//SANITY_CHECK_IMAGE_PROCESSING();
	//SANITY_CHECK_IMAGE_IO();
	//SANITY_CHECK_ROBUST_SOLVE();
	//SANITY_CHECK_SUBDIVIDE();
	return ret;
}
int main(int argc, char *argv[]) {
	const int N = 45;
	std::array<ExamplePtr,N> apps = { 
		MAKE_EXAMPLE(UnitsEx), MAKE_EXAMPLE(CompositeEx),MAKE_EXAMPLE(EventsEx), 
		MAKE_EXAMPLE(DragEx), MAKE_EXAMPLE(TweenEx),MAKE_EXAMPLE(ImageEx), 
		MAKE_EXAMPLE(ControlsEx), MAKE_EXAMPLE(DialogsEx), MAKE_EXAMPLE(ExpandBarEx), 
		MAKE_EXAMPLE(MeshMatcapEx), MAKE_EXAMPLE(MeshWireframeEx), MAKE_EXAMPLE(MeshSubdivideEx),
		MAKE_EXAMPLE(MeshTextureEx), MAKE_EXAMPLE(MeshVertexColorEx), MAKE_EXAMPLE(MeshParticleEx),
		MAKE_EXAMPLE(MeshDepthEx), MAKE_EXAMPLE(MeshPhongEx), MAKE_EXAMPLE(LaplaceFillEx), 
		MAKE_EXAMPLE(PoissonBlendEx), MAKE_EXAMPLE(PoissonInpaintEx), MAKE_EXAMPLE(ImageProcessingEx),
		MAKE_EXAMPLE(MeshPickerEx), MAKE_EXAMPLE(IntersectorEx),MAKE_EXAMPLE(MeshSmoothEx), 
		MAKE_EXAMPLE(ColorSpaceEx),MAKE_EXAMPLE(MeshPrimitivesEx), MAKE_EXAMPLE(MenuEx), 
		MAKE_EXAMPLE(LocatorEx), MAKE_EXAMPLE(GraphPaneEx), MAKE_EXAMPLE(WindowPaneEx), 
		MAKE_EXAMPLE(SplineEx), MAKE_EXAMPLE(DistanceFieldEx), MAKE_EXAMPLE(ExpandTreeEx),
		MAKE_EXAMPLE(DataFlowEx),MAKE_EXAMPLE(ForceDirectedGraphEx),MAKE_EXAMPLE(OneEuroFilterEx),
		MAKE_EXAMPLE(TabPaneEx),MAKE_EXAMPLE(ParameterPaneEx),MAKE_EXAMPLE(TablePaneEx),
		MAKE_EXAMPLE(TimelineEx),MAKE_EXAMPLE(MeshOptimizationEx),MAKE_EXAMPLE(ImageFeaturesEx),
		MAKE_EXAMPLE(MeshTextureMapEx),MAKE_EXAMPLE(MeshReconstructionEx),MAKE_EXAMPLE(SoftBodyEx)
	};
	std::sort(apps.begin(),apps.end(),[=](const ExamplePtr& a,const ExamplePtr& b){
		return std::lexicographical_compare(a->name.begin(), a->name.end(), b->name.begin(), b->name.end());
	});aly::WorkerTaskPtr workerTask;

	try {
		//Example name is specified in a makefile at compile time so 
		//all example applications are compiled to seperate exe targets.
		std::string exName = TOSTRING(EXAMPLE_NAME);
		int exampleIndex = -1;
		for (int n = 0; n < N; n++) {
			if (apps[n]->getName() == exName) {
				exampleIndex = n;
				break;
			}
		}
		if (exampleIndex >= 0) {
			apps[exampleIndex]->getApplication()->run(1);
		} else {
			bool error = false;
			if (argc >= 2) {
				int index = atoi(argv[1]);
				if (index < N) {
					std::string dir =
							(argc > 2) ?
									RemoveTrailingSlash(argv[2]) :
									GetDesktopDirectory();
					if (index == -2) {
						SANITY_CHECK();
						return 0;
					} else if (index == -1) {
						for (index = 0; index < N; index++) {
							ExamplePtr& ex = apps[index];
							std::string screenShot = MakeString() << dir
									<< ALY_PATH_SEPARATOR<<"screenshot"<<std::setw(2)<<std::setfill('0')<<index<<"_"<<ex->getName()<<".png";
							std::cout << "Saving " << screenShot << std::endl;
							ex->getApplication()->runOnce(screenShot);
							ex.reset();
						}
					} else if (index >= 0) {
						ExamplePtr& ex = apps[index];
						ex->getApplication()->run(1);
					} else {
						error = true;
					}
				} else {
					error = true;
				}
			} else {
				error = true;
			}
			if (error) {
				std::cout << "Usage: " << argv[0]
						<< " [example index] [output directory]\nAlloy Graphics Library Examples:"
						<< std::endl;
				std::cout << "[" << -2 << "] Sanity Check" << std::endl;
				std::cout << "[" << -1
						<< "] Generate screenshots for all examples"
						<< std::endl;
				for (int i = 0; i < N; i++) {
					std::cout << "[" << std::setw(2) << std::setfill(' ') << i
							<< "] " << apps[i]->getName() << std::endl;
				}
				std::cout << ">> Enter Example Number: ";
				int index = -1;
				std::cin >> index;
				if (index == -2) {
					SANITY_CHECK();
				} else if (index == -1) {
					std::string dir = GetDesktopDirectory();
					for (index = 0; index < N; index++) {
						ExamplePtr& ex = apps[index];
						std::string screenShot = MakeString() << dir
								<< ALY_PATH_SEPARATOR<<"screenshot"<<std::setw(2)<<std::setfill('0')<<index<<"_"<<ex->getName()<<".png";
						std::cout << "Saving " << screenShot << std::endl;
						ex->getApplication()->runOnce(screenShot);
						ex.reset();
					}
				} else if (index >= 0 && index < N) {
					ExamplePtr& ex = apps[index];
					ex->getApplication()->run(1);
				} else {
					throw std::runtime_error("Invalid example index.");
				}
			}
		}

		return 0;
	} catch (std::exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		std::flush(std::cout);
		std::cout << "Exiting ..." << std::endl;
		//std::cout<<"Hit any key ..."<<std::endl;
		//getchar();
		return 1;
	}
}

