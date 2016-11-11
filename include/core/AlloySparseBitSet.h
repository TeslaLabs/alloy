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
#ifndef SPARSEBITSET_H_
#define SPARSEBITSET_H_
#include <bitset>
#include <vector>
#include <map>
#include <cereal/cereal.hpp>
#include <cereal/types/bitset.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/polymorphic.hpp>
namespace aly
{

class SparseBitSet{
        static const size_t STRIDE=32;
        std::map<size_t,std::bitset<STRIDE>> bits;
        size_t length;
    public:
        template<class Archive> void save(Archive & ar) const
        {
            ar(CEREAL_NVP(length),CEREAL_NVP(bits));
        }
        template<class Archive> void load(Archive & ar)
        {
            ar(CEREAL_NVP(length),CEREAL_NVP(bits));
        }
        SparseBitSet(size_t sz=0);
        std::bitset<STRIDE>::reference operator[](size_t idx);
        void resize(size_t sz);
        size_t size() const ;
        size_t capacity() const;
        void set(size_t idx,bool val);
        void reset();
        void clear();
        bool test(size_t idx);
        std::vector<size_t> indexes();
        size_t count();
};
void WriteSparseBitSetToFile(const std::string& file,const SparseBitSet& bitset);
void ReadSparseBitSetFromFile(const std::string& file,SparseBitSet& bitset);
} /* namespace bfx */

#endif /* DYNAMICBITSET_H_ */
