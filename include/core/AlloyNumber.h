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

#ifndef ALLOYNUMBER_H_
#define ALLOYNUMBER_H_
#include "AlloyMath.h"
#include <algorithm>
#include <string>
#include <iostream>
#include <memory>
#include <list>
#include "cereal/types/polymorphic.hpp"
#include "cereal/types/memory.hpp"
#include "cereal/types/tuple.hpp"

namespace aly {
	enum class NumberType {
		Integer, Float, Double, Boolean
	};
	template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const NumberType& type) {
		switch (type) {
		case NumberType::Integer:
			return ss << "Integer";
		case NumberType::Float:
			return ss << "Float";
		case NumberType::Double:
			return ss << "Double";
		case NumberType::Boolean:
			return ss << "Boolean";
		}
		return ss;
	}
class Number {
public:
	struct Interface {
		virtual int toInteger() const = 0;
		virtual float toFloat() const = 0;
		virtual double toDouble() const = 0;
		virtual bool toBoolean() const = 0;
		virtual void setValue(int value) = 0;
		virtual void setValue(float value) = 0;
		virtual void setValue(double value) = 0;
		virtual void setValue(bool value) = 0;
		virtual std::string toString() const = 0;
		NumberType virtual type() const = 0;
	};

private:
	template<class T> struct Impl: public Interface {
		T value;
		Impl(const T& value) :
				value(value) {
		}
		NumberType virtual type() const {
			return value.type();
		}
		virtual int toInteger() const {
			return value.toInteger();
		}
		virtual float toFloat() const {
			return value.toFloat();
		}
		virtual double toDouble() const {
			return value.toDouble();
		}
		virtual bool toBoolean() const {
			return value.toBoolean();
		}
		virtual std::string toString() const {
			return value.toString();
		}
		virtual void setValue(int val) {
			value.setValue(val);
		}
		virtual void setValue(float val) {
			value.setValue(val);
		}
		virtual void setValue(double val) {
			value.setValue(val);
		}
		virtual void setValue(bool val) {
			value.setValue(val);
		}
	};
	std::shared_ptr<Interface> impl;
public:
	template<class Archive> void save(Archive& archive) const {
		if (impl.get()) {
			std::string typeName = MakeString() << impl->type();
			archive(cereal::make_nvp(typeName, impl->toString()));
		} else {
			archive("");
		}
	}
	template<class Archive> void load(Archive& archive) {

	}
	Number() {
	}
	Number(const Number & r) :
			impl(r.impl) {
	}

	template<class T> Number(const T & value) :
			impl(new Impl<T> { value }) {
	}
	template<class T> Number(T* value) :
			impl(value) {
	}
	Number & operator =(const Number & r) {
		impl = r.impl;
		return *this;
	}
	Number clone() const ;
	template<class T> Number & operator =(const T & value) {
		return *this = Number(value);
	}
	virtual int toInteger() const {
		if (impl.get() == nullptr)
			throw std::runtime_error("Number storage type has not been defined.");
		return impl->toInteger();
	}
	virtual float toFloat() const {
		if (impl.get() == nullptr)
			throw std::runtime_error("Number storage type has not been defined.");
		return impl->toFloat();
	}
	virtual double toDouble() const {
		if (impl.get() == nullptr)
			throw std::runtime_error("Number storage type has not been defined.");
		return impl->toDouble();
	}
	virtual bool toBoolean() const {
		if (impl.get() == nullptr)
			throw std::runtime_error("Number storage type has not been defined.");
		return impl->toBoolean();
	}
	std::string toString() const {
		if (impl.get() == nullptr)
			throw std::runtime_error("Number storage type has not been defined.");
		return impl->toString();
	}
	virtual void setValue(int val) {
		if (impl.get() == nullptr)
			throw std::runtime_error("Number storage type has not been defined.");
		impl->setValue(val);
	}
	virtual void setValue(float val) {
		if (impl.get() == nullptr)
			throw std::runtime_error("Number storage type has not been defined.");
		impl->setValue(val);
	}
	virtual void setValue(double val) {
		if (impl.get() == nullptr)
			throw std::runtime_error("Number storage type has not been defined.");
		impl->setValue(val);
	}
	virtual void setValue(bool val) {
		if (impl.get() == nullptr)
			throw std::runtime_error("Number storage type has not been defined.");
		impl->setValue(val);
	}
	virtual NumberType type() const {
                if (impl.get() == nullptr)
                        throw std::runtime_error("Number storage type has not been defined.");
                return impl->type();
        }
	bool hasValue() const {
	    return (impl.get()!=nullptr);
	}
	virtual inline ~Number() {

	}
};

struct Integer {
private:
	int value = 0;
public:
	Integer() {}
	NumberType virtual type() const {
		return NumberType::Integer;
	}
	template<class Archive> void serialize(Archive& archive) {
		std::string typeName = MakeString() << type();
		archive(cereal::make_nvp(typeName, value));
	}

	int toInteger() const {
		return value;
	}
	float toFloat() const {
		return (float) value;
	}
	double toDouble() const {
		return (double) value;
	}
	bool toBoolean() const {
		return (value != 0);
	}
	std::string toString() const {
		std::stringstream ss;
		ss << toInteger();
		return ss.str();
	}
	void setValue(float other) {
		value = (int)std::floor(0.5f + other);
	}
	void setValue(double other) {
		value = (int) std::floor(0.5+other);
	}
	void setValue(int other) {
		value = (int) other;
	}
	void setValue(bool other) {
		value = other ? 1 : 0;
	}
	Integer(int value) :
			value(value) {
	}
	Integer(const Number& num):value(num.toInteger()){

	}
	virtual ~Integer() {
	}
};

struct Float {
private:
	float value = 0;
public:
	Float() {}
	NumberType virtual type() const {
		return NumberType::Float;
	}
	template<class Archive> void serialize(Archive& archive) {
		std::string typeName = MakeString() << type();
		archive(cereal::make_nvp(typeName, value));
	}
	int toInteger() const {
		return (int) value;
	}
	float toFloat() const {
		return (float) value;
	}
	double toDouble() const {
		return (double) value;
	}
	bool toBoolean() const {
		return (value != 0);
	}
	std::string toString() const {
		std::stringstream ss;
		ss.setf(std::ios::fixed, std::ios::floatfield);
		ss << std::setprecision(4) << toFloat();
		return ss.str();
	}
	void setValue(float other) {
		value = (float) other;
	}
	void setValue(double other) {
		value = (float) other;
	}
	void setValue(int other) {
		value = (float) other;
	}
	void setValue(bool other) {
		value = other ? 1.0f : 0.0f;
	}
	Float(float value) :
			value(value) {
	}
	Float(const Number& num):value(num.toFloat()){

	}
	virtual ~Float() {
	}
};
struct Double {
private:
	double value = 0;
public:
	Double(){}
	NumberType virtual type() const {
		return NumberType::Double;
	}
	template<class Archive> void serialize(Archive& archive) {
		std::string typeName = MakeString() << type();
		archive(cereal::make_nvp(typeName, value));
	}
	int toInteger() const {
		return (int) value;
	}
	float toFloat() const {
		return (float) value;
	}
	double toDouble() const {
		return (double) value;
	}
	bool toBoolean() const {
		return (value != 0);
	}
	std::string toString() const {
		std::stringstream ss;
		ss.setf(std::ios::fixed, std::ios::floatfield);
		ss << std::setprecision(8) << toDouble();
		return ss.str();
	}
	void setValue(float other) {
		value = (double) other;
	}
	void setValue(double other) {
		value = (double) other;
	}
	void setValue(int other) {
		value = (double) other;
	}
	void setValue(bool other) {
		value = other ? 1.0 : 0.0;
	}
	Double(double value) :
			value(value) {
	}
	Double(const Number& num):value(num.toDouble()){

	}
	virtual ~Double() {
	}
};
struct Boolean {
private:
	bool value = 0;
public:
	Boolean(){}
	NumberType virtual type() const {
		return NumberType::Boolean;
	}
	template<class Archive> void serialize(Archive& archive) {
		std::string typeName = MakeString() << type();
		archive(cereal::make_nvp(typeName, value));
	}
	int toInteger() const {
		return value ? 1 : 0;
	}
	float toFloat() const {
		return value ? 1.0f : 0.0f;
	}
	double toDouble() const {
		return value ? 1.0 : 0.0;
	}
	bool toBoolean() const {
		return value;
	}
	std::string toString() const {
		return (value?"true":"false");
	}
	void setValue(float other) {
		value = (other != 0);
	}
	void setValue(double other) {
		value = (other != 0);
	}
	void setValue(int other) {
		value = (other != 0);
	}
	void setValue(bool other) {
		value = other;
	}
	Boolean(bool value) :
		value(value) {
	}
	Boolean(const Number& num):value(num.toBoolean()){

	}
	virtual ~Boolean() {
	}
};
template<class T> Number MakeNumber(const NumberType& numberType,T value) {
	switch (numberType) {
	case NumberType::Integer:
		return Integer((int)(value));
	case NumberType::Float:
		return Float((float)(value));
	case NumberType::Double:
		return Double((double)(value));
	case NumberType::Boolean:
		return Boolean((value!=0));
	default:
		throw std::runtime_error("Number type not defined.");
	}
	return Integer(0);
}
inline bool operator < (const Number& a, const Number& b) {
	return (a.toDouble() < b.toDouble());
}
inline bool operator > (const Number& a, const Number& b) {
	return (a.toDouble() > b.toDouble());
}
inline bool operator == (const Number& a, const Number& b) {
	return (a.toDouble() == b.toDouble());
}
inline bool operator != (const Number& a, const Number& b) {
	return (a.toDouble() != b.toDouble());
}

template<class C, class R> std::basic_ostream<C, R> & operator <<(
	std::basic_ostream<C, R> & ss, const Number& v) {
	return ss << v.toString();
}

}
#endif /* ALLOYUNITS_H_ */
