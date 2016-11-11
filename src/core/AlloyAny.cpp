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
#include <cereal/archives/xml.hpp>
#include "AlloyAny.h"
#include  "AlloyNumber.h"
#include <iostream>
#include <fstream>
namespace aly {
	bool SANITY_CHECK_ANY() {

		AnyValue<Number> dvalue(Double(3.14156535));
		AnyValue<Number> ivalue(Integer(20));
		AnyValue<Number> bvalue(Boolean(true));

		std::vector<AnyInterface*> valueList = { &dvalue, &ivalue, &bvalue };
		for (AnyInterface* value : valueList) {
			std::cout << "Value " << value->getValue<Number>().toString() << " Type " << value->getValue<Number>().type() << std::endl;
		}

		std::shared_ptr<AnyInterface> v1 = std::shared_ptr<AnyInterface>(std::dynamic_pointer_cast<AnyInterface>(std::shared_ptr<AnyValue<Double>>(new AnyValue<Double>(Double(3.14156535)))));
		std::shared_ptr<AnyInterface> v2 = std::shared_ptr<AnyInterface>(std::dynamic_pointer_cast<AnyInterface>(std::shared_ptr<AnyValue<Integer>>(new AnyValue<Integer>(Integer(20)))));
		std::shared_ptr<AnyInterface> v3 = std::shared_ptr<AnyInterface>(std::dynamic_pointer_cast<AnyInterface>(std::shared_ptr<AnyValue<Boolean>>(new AnyValue<Boolean>(Boolean(true)))));
		std::cout << "Value " << v1->getValue<Double>() << std::endl;
		std::cout << "Value " << v2->getValue<Integer>() << std::endl;
		std::cout << "Value " << v3->getValue<Boolean>() << std::endl;
		v2->setValue(Integer(17));
		std::cout << "Value " << v2->getValue<Integer>() << std::endl;
		{
			std::ofstream os("any.xml");
			cereal::XMLOutputArchive archiver(os);
			archiver(*v1,*v2,*v3,dvalue,ivalue,bvalue);
		}
		return true;
	}
}
