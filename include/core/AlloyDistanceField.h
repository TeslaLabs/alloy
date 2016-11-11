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
#ifndef DISTANCEFIELD_H_
#define DISTANCEFIELD_H_
#include "BinaryMinHeap.h"
#include "AlloyMath.h"
#include "AlloyVolume.h"
namespace aly {
	bool SANITY_CHECK_DISTANCE_FIELD();
	class DistanceField3f {
		typedef Indexable<float, 3> VoxelIndex;
		typedef vec<int, 3> Coord;
	private:
		static const ubyte1 ALIVE;
		static const ubyte1 NARROW_BAND;
		static const ubyte1 FAR_AWAY;

		float march(float Nv, float Sv, float Ev, float Wv, float Fv, float Bv, int Nl, int Sl, int El, int Wl, int Fl, int Bl);
	public:
		static const float DISTANCE_UNDEFINED;
		DistanceField3f()  {}
		void solve(const Volume1f& vol, Volume1f& out,float maxDistance=2.5f);
	};
	class DistanceField2f {
		typedef Indexable<float, 2> PixelIndex;
		typedef vec<int, 2> Coord;
	private:
		static const ubyte1 ALIVE;
		static const ubyte1 NARROW_BAND;
		static const ubyte1 FAR_AWAY;

		float march(float Nv, float Sv, float Fv, float Bv, int Nl, int Sl, int Fl, int Bl);
	public:
		static const float DISTANCE_UNDEFINED;
		DistanceField2f() {}
		void solve(const Image1f& vol, Image1f& out, float maxDistance = 2.5f);
	};
} /* namespace imagesci */

#endif /* DISTANCEFIELD_H_ */
