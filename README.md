Alloy Graphics Library
========
Alloy is yet another 2D/3D graphics library written in C++11. As mundane as that sounds, this API provides an extensive collection of GUI components and engineering reference implementations of common 2D/3D graphics algorithms for fast prototyping on windows, linux, and mac platforms.


## Contents
- Anti-aliased vector graphics drawing (NanoVG).
- Truetype fonts (stb_truetype).
- OpenGL core 3.3 and higher support. Legacy OpenGL will not run, but you won't need it!
- Generic handling of absolute/relative coordinates in pixels, dp, in, and mm.
- Tweenable positions, dimensions, font sizes, and colors.
- GUI components: Region, Composite, Border Layout, Vertical Layout, Horizontal Layout, Scroll Bars, Draw 2D Region, Text Button, Icon Button, Text Icon Button, Text Label, Text Region, Text Field, Number Field, Message Dialog, Selection Box, Drop Down Box, Menu Bar, Vertical Slider, Horizontal Slider, Color Selection, File Dialog, File Field, Multi-File Filed, File Button, List Box, Search Box, Window Pane, Graph Plot, Progress Bar, Expand Region, Expand Tree, Tab Pane, Table Pane, Timeline, Toggle Box, Check Box, Glass Pane, Parameter Pane.
- Worker, timer, and recurrent worker.
- Vector types from one to four dimensions.
- Quaternions.
- Color space support for RGBA, HSV, CieLAB, XYZ, Gray, and look-up tables.
- Sparse Matrix, Dense Matrix, Dense Vector, Array, Image, and Volume data structures.
- Data structure serialization to json, binary, and xml (cereal).
- Image IO (stb_image and tinyexr) for PNG, JPEG, TIFF, HDR, PSD, BMP, GIF, PNM, PIC, TGA, EXR, and XML (Mipav's encoding).
- Mesh IO for PLY and OBJ (tinyobj).
- Dense and sparse matrix solvers including SVD, QR, LU, CG, and BiCGstab.
- The "Any" class to enable methods with arbitrary left-hand return types.
- Perspective and orthographic cameras with methods to transform from world to screen or vice versa.
- "Awesome font" icons.
- Window screenshot (F11).
- Arbitrary mouse cursors.
- Common shaders for deferred rendering in OpenGL 3.3 and higher.
- Distance Field 2D/3D (fast-marching method).
- Delaunay Triangulation.
- Point Kd-Tree 2D/3D (libkdtree).
- Approximate Nearest Neighbors in N-dimensions (nanoflann).
- Mesh Kd-Tree (ray-intersection and closest point).
- SHA1, SHA-256, SHA-384, SHA-512.
- Cross-Platform file system IO.
- NURB curves, B-Splines, and Beziers (tinyspline).
- Catmull-Clark and Loop mesh sub-division.
- Mesh primitives for box, icosahedron, sphere, cylinder, torus, plane, cone, pyramid, frustum, capsule, tessellated sphere, grid, and asteroid.
- Iso-contour generation (with connectivity rules)
- Dataflow UI for graphics/imaging pipelines
- Force directed graph
- "One Euro" filter

## Dependencies
GLFW 3.1+ and GLEW. Source code for all other libraries is included in the repository and compiles on windows, linux, and mac.

## Building
#### Ubuntu Linux:
 
 ```bash
 sudo apt-get install g++-4.8
 sudo apt-get install gcc-4.8
 sudo apt-get install libglu1-mesa-dev
 sudo apt-get install libxxf86vm-dev 
 sudo apt-get install libxrandr-dev
 sudo apt-get install libxcursor-dev
 sudo apt-get install libxinerama-dev
 sudo apt-get install libxdamage-dev
 sudo apt-get install libxi-dev
 sudo apt-get install libglew-dev
 sudo apt-get install libxi6
 sudo apt-get install libglew1.10
 sudo apt-get install cmake
 cmake -G "Unix Makefiles" -DCMAKE_CXX_COMPILER=g++-4.8 -DCMAKE_C_COMPILER=gcc-4.8
 make -j4
 ```
 
#### OS X:
```bash
brew update
brew install glew
brew install cmake
brew tap homebrew/versions
brew install glfw3
cmake -G "Unix Makefiles" -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang
make -j4
```

#### Windows 10:
 - Download [Visual Studio 2015 Community Edition](https://www.visualstudio.com/downloads/download-visual-studio-vs)
 - Open solution file Alloy-Graphics-Library/vs2015/alloy
 - Change build configuration to Release|x64 for best performance 

## License
Alloy is [BSD licensed](https://github.com/rgb2hsv/Alloy-Graphics-Library/edit/master/LICENSE) and only uses libraries that also have a permissive license for commercial and academic use.

Absolute/Relative Positioning Example
-------------------------
[![UnitsEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0000.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/UnitsEx.cpp)

Composite Example
-------------------------
[![CompisteEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0001.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/CompositeEx.cpp)

Events Example
-------------------------
[![EventsEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0002.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/EventsEx.cpp)

Drag and Drop Example
-------------------------
[![DragEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0003.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/DragEx.cpp)

Tween Animation Example
-------------------------
[![TweenEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0004.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/TweenEx.cpp)

Image Example
-------------------------
[![ImageEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0005.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/ImageEx.cpp)

Controls Example
-------------------------
[![ControlsEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0006.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/ControlsEx.cpp)

Dialog Example
-------------------------
[![DialogEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0007.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/DialogEx.cpp)

Parameter Pane Example
-------------------------
[![ParameterPaneEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0037.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/ParameterPaneEx.cpp)

Timeline Example
-------------------------
[![TimelineEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0039.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/TimelineEx.cpp)

Expand Bar Example
-------------------------
[![ExpandBarEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0008.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/ExpandBarEx.cpp)

Expand Tree Example
-------------------------
[![ExpandTreeEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0032.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/ExpandTreeEx.cpp)

Tab Pane Example
-------------------------
[![TabPaneEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0036.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/TabPaneEx.cpp)

Menu System Example
-------------------------
[![MenuEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0026.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/MenuEx.cpp)

Graph Example
-------------------------
[![GraphPaneEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0028.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/GraphPaneEx.cpp)

Window Pane Example
-------------------------
[![WindowPaneEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0029.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/WindowPaneEx.cpp)

Table Pane Example
-------------------------
[![TablePaneEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0038.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/TablePaneEx.cpp)

DataFlow Example
-------------------------
[![DataFlowEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0033.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/DataFlowEx.cpp)

Force Directed Graph Example
-------------------------
[![ForceDirectedGraphEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0034.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/ForceDirectedGraphEx.cpp)

Mesh with Matcap Shading Example
-------------------------
[![MeshMatcapEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0009.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/MeshMatcapEx.cpp)

Mesh Wireframe Example
-------------------------
[![MeshWireframeEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0010.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/MeshWireframeEx.cpp)

Mesh Subdivision Example
-------------------------
[![MeshSubdivideEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0011.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/MeshSubdivideEx.cpp)

Mesh Texture Example
-------------------------
[![MeshTextureEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0012.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/MeshTextureEx.cpp)

Mesh with Per Vertex Color Example
-------------------------
[![MeshVertexColorEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0013.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/MeshVertexColorEx.cpp)

Particles Example
-------------------------
[![MeshParticleEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0014.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/MeshParticleEx.cpp)

Mesh Depth / Normals / Distances Example
-------------------------
[![MeshDepthEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0015.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/MeshDepthEx.cpp)

Mesh Phong Shading Example
-------------------------
[![MeshPhongEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0016.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/MeshPhongEx.cpp)

Laplace Fill Example
-------------------------
[![LaplaceFillEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0017.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/LaplaceFillEx.cpp)

Poisson Blend Example
-------------------------
[![PoissonBlendEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0018.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/PoissonBlendEx.cpp)

Poisson Fill Example
-------------------------
[![PoissonInpaintEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0019.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/PoissonInpaintEx.cpp)

Image Filter Example
-------------------------
[![ImageProcessingEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0020.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/ImageProcessingEx.cpp)

Object/Face Picker Example
-------------------------
[![MeshPickerEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0021.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/MeshPickerEx.cpp)

Mesh Ray Intersection Example
-------------------------
[![IntersectorEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0022.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/IntersectorEx.cpp)

Mesh Smoothing Example
-------------------------
[![MeshSmoothEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0023.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/MeshSmoothEx.cpp)

Color Space Example
-------------------------
[![ColorSpaceEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0024.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/ColorSpaceEx.cpp)

Mesh Primitives Example
-------------------------
[![MeshPrimitivesEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0025.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/MeshPrimitivesEx.cpp)

Point Locator Example
-------------------------
[![LocatorEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0027.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/LocatorEx.cpp)

B-Spline Example
-------------------------
[![SplineEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0030.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/SplineEx.cpp)

Distance Field Example
-------------------------
[![DistanceFieldEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0031.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/DistanceFieldEx.cpp)

One Euro Filter Example
-------------------------
[![OneEuroFilterEx](https://github.com/rgb2hsv/blob/blob/master/screenshots/screenshot0035.png)](https://github.com/rgb2hsv/alloy/blob/master/alloy/src/example/OneEuroFilterEx.cpp)
