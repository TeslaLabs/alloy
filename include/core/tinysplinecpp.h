/*
The MIT License (MIT)

Copyright (c) 2014 Marcel

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include "tinyspline.h"

class TsDeBoorNet {
public:
    TsDeBoorNet();
    TsDeBoorNet(const TsDeBoorNet& other);
    ~TsDeBoorNet();

    TsDeBoorNet& operator=(const TsDeBoorNet& other);

    float u() const;
    size_t k() const;
    size_t s() const;
    size_t h() const;
    size_t dim() const;
    size_t nPoints() const;
    float* points();
    float* result();
    tsDeBoorNet* data();
    
private:
    tsDeBoorNet deBoorNet;
};

class TsBSpline {
public:
    TsBSpline();
    TsBSpline(const TsBSpline& other);
    TsBSpline(const size_t deg, const size_t dim, const size_t nCtrlp,
              const tsBSplineType type);
    TsBSpline(const float* points, const size_t nPoints, const size_t dim);
    ~TsBSpline();

    TsBSpline& operator=(const TsBSpline& other);
    TsDeBoorNet operator()(const float u) const;
    
    size_t deg() const;
    size_t order() const;
    size_t dim() const;
    size_t nCtrlp() const;
    size_t nKnots() const;
    float* ctrlp();
    float* knots();
	const float* ctrlp() const;
	const float* knots() const;
    tsBSpline* data();
    
    void setDeg(const size_t deg);
    void setOrder(const size_t order);
    
    void setupKnots(const tsBSplineType type);
    void resize(const int n, const int back);
    void buckle(const float b);
    void toBeziers();
    size_t insertKnot(const float u, const size_t n);
    size_t split(const float u);
    TsDeBoorNet evaluate(const float u) const;
    
private:
    tsBSpline bspline;
};