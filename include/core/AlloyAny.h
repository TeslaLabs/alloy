/*
* Copyright(C) 2015, Blake C. Lucas, Ph.D. (img.science@gmail.com)
*
* Permission is hereby granted, free of charge, to Any person obtaining a copy
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
//Kudos to Thomas Pigarelli (https://gist.github.com/Jefffrey/9202235) for reimplementing Boost's mysterious and wonderful "any" class
#ifndef ALLOYANY_H_
#define ALLOYANY_H_
#include "AlloyNumber.h"
#include <algorithm>
#include <typeinfo>
#include <exception>
#include <memory>
#include <type_traits>
#include <cereal/cereal.hpp>
#include "cereal/types/memory.hpp"
#include "cereal/types/polymorphic.hpp"
namespace aly
{

	bool SANITY_CHECK_ANY();

	class Any;

	template<class Type> Type AnyCast(Any&);
	template<class Type> Type AnyCast(const Any&);
	template<class Type> Type* AnyCast(Any*);
	template<class Type> const Type* AnyCast(const Any*);
	class Any {
	public:

		template<class Type> friend
			Type AnyCast(Any&);

		template<class Type>
		friend Type AnyCast(const Any&);

		template<class Type>
		friend Type* AnyCast(Any*);

		template<class Type>
		friend const Type* AnyCast(const Any*);

		Any()
			: ptr(nullptr)
		{}

		Any(Any&& x)
			: ptr(std::move(x.ptr))
		{}

		Any(const Any& x) {
			if (x.ptr)
				ptr = x.ptr->clone();
		}

		template<class Type> Any(const Type& x)
			: ptr(new Concrete<typename std::decay<const Type>::type>(x))
		{}

		Any& operator=(Any&& rhs) {
			ptr = std::move(rhs.ptr);
			return (*this);
		}

		Any& operator=(const Any& rhs) {
			ptr = std::move(Any(rhs).ptr);
			return (*this);
		}

		template<class T> Any& operator=(T&& x) {
			ptr.reset(new Concrete<typename std::decay<T>::type>(typename std::decay<T>::type(x)));
			return (*this);
		}

		template<class T> Any& operator=(const T& x) {
			ptr.reset(new Concrete<typename std::decay<T>::type>(typename std::decay<T>::type(x)));
			return (*this);
		}

		void clear() {
			ptr.reset(nullptr);
		}

		bool isEmpty() const {
			return ptr.get() == nullptr;
		}

		const std::type_info& type() const {
			return (!isEmpty())
				? ptr->type()
				: typeid(void);
		}

	public:
		struct Placeholder {

			virtual std::unique_ptr<Placeholder> clone() const = 0;
			virtual const std::type_info& type() const = 0;
			virtual ~Placeholder() {}

		};
		template<class T>
		struct Concrete : public Placeholder {

			Concrete(T&& x)
				: value(std::move(x))
			{}

			Concrete(const T& x)
				: value(x)
			{}

			virtual std::unique_ptr<Placeholder> clone() const override {
				return std::unique_ptr<Placeholder>(new Concrete<T>(value));
			}

			virtual const std::type_info& type() const override {
				return typeid(T);
			}
			Concrete() {}
			T value;
			template<class Archive> void save(Archive& ar) const {
				ar(CEREAL_NVP(value));
			}
			template<class Archive> void load(Archive& ar) {
				ar(CEREAL_NVP(value));
			}
		};
		std::unique_ptr<Placeholder> ptr;
	public:

		template<class Archive> void save(Archive& ar) const {
			ar(cereal::make_nvp("value", ptr));
		}
		template<class Archive> void load(Archive& ar) {
			ar(cereal::make_nvp("value", ptr));
		}
	};

	template<class Type>
	Type AnyCast(Any& val) {
		if (val.ptr->type() != typeid(Type))
			throw std::runtime_error(aly::MakeString()<<"Could not cast "<<val.ptr->type().name());
		return static_cast<Any::Concrete<Type>*>(val.ptr.get())->value;
	}

	template<class Type>
	Type AnyCast(const Any& val) {
		if (val.ptr.get()==nullptr||val.ptr->type() != typeid(Type))
		    throw std::runtime_error(aly::MakeString()<<"Could not cast "<<((val.ptr!=nullptr)?val.ptr->type().name():"nullptr"));
		return static_cast<Any::Concrete<Type>*>(val.ptr.get())->value;
	}

	template<class Type>
	Type* AnyCast(Any* ptr) {
		return dynamic_cast<Type*>(ptr->ptr.get());
	}

	template<class Type>
	const Type* AnyCast(const Any* ptr) {
		return dynamic_cast<const Type*>(ptr->ptr.get());
	}
	class AnyInterface {
	public:
		template <typename T>
		void setValue(const T& value) {
			Any any = value;
			setValueImpl(any);
		}
		template <typename T>
		T getValue() const {
			Any res = getValueImpl();
			return AnyCast<T>(res);
		}
		AnyInterface(){}
		template<class Archive> void save(Archive& ar) const {
			Any any = getValueImpl();
			ar(cereal::make_nvp("any", any));
		}
		template<class Archive> void load(Archive& ar) {
			Any any;
			ar(cereal::make_nvp("any",any));
			setValue(any);
		}
		virtual ~AnyInterface() {}
	protected:
		virtual void setValueImpl(Any const & value) = 0;
		virtual Any getValueImpl() const = 0;
	};
	template<class T> class AnyValue : public AnyInterface {
	public:
		AnyValue(const T& value) :value(value) {
		}
		AnyValue() {
		}
		template<class Archive> void save(Archive& ar) const {
			ar(cereal::base_class<AnyInterface>(this));
		}
		template<class Archive> void load(Archive& ar) {
			ar(cereal::base_class<AnyInterface>(this));
		}
	protected:
		T value;
		virtual void setValueImpl(Any const & val) override {
			value = AnyCast<T>(val);
		}
		virtual Any getValueImpl() const override {
			return value;
		}
	};
}

#define REGISTER_ANY_TYPE(T)                 \
	CEREAL_REGISTER_TYPE(aly::Any::Concrete<T>);	\
	CEREAL_REGISTER_TYPE(aly::AnyValue<T>);


REGISTER_ANY_TYPE(aly::Number);
REGISTER_ANY_TYPE(aly::Double);
REGISTER_ANY_TYPE(aly::Float);
REGISTER_ANY_TYPE(aly::Integer);
REGISTER_ANY_TYPE(aly::Boolean);
#endif
