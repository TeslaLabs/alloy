// any_function - v0.9 - Single-header public domain library
//
// The intent of this library is to provide a functional counterpart to
// std::any, called any_function. any_function can receive anything that is
// callable with a unique signature, such as a function pointer, a
// std::function, or any callable object with a single, non-generic
// operator() overload. any_function can be called with an array of pointers
// to void, and returns the return value in a std::shared_ptr<void>. It also
// remembers the std::type_info of the parameters and return value of the
// function it was constructed with.
//
// The original author of this software is Sterling Orsten, and its permanent
// home is <http://github.com/sgorsten/any_function/>. If you find this software
// useful, an acknowledgement in your source text and/or product documentation
// is appreciated, but not required.

// This is free and unencumbered software released into the public domain.
// 
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
// 
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// For more information, please refer to <http://unlicense.org/>

#ifndef ALY_ANY_FUNCTION_H
#define ALY_ANY_FUNCTION_H

#include <cassert>      // For assert(...)
#include <functional>   // For std::function<F>
#include <vector>       // For std::vector<T>
#include <memory>       // For std::unique_ptr<T>
namespace aly {
	struct AnyFunction {
	public:
		struct Type {
			const std::type_info * info;
			bool is_lvalue_reference, is_rvalue_reference;
			bool is_const, is_volatile;
			bool operator ==(const Type & r) const {
				return info == r.info && is_lvalue_reference == r.is_lvalue_reference && is_rvalue_reference == r.is_rvalue_reference && is_const == r.is_const
						&& is_volatile == r.is_volatile;
			}
			bool operator !=(const Type & r) const {
				return !(*this == r);
			}
			template<class T> static Type capture() {
				return {&typeid(T), std::is_lvalue_reference<T>::value, std::is_rvalue_reference<T>::value, std::is_const<typename std::remove_reference<T>::type>::value, std::is_volatile<typename std::remove_reference<T>::type>::value};
			}
		};

		class Result {
			struct ResultBase {
				virtual ~ResultBase() {
				}
				virtual std::unique_ptr<ResultBase> clone() const = 0;
				virtual Type getType() const = 0;
				virtual void * getAddress() = 0;
			};
			template<class T> struct TypedResult: ResultBase {
				T x;
				TypedResult(T x) :
						x(get((void*) &x, Tag<T> { })) {
				}
				std::unique_ptr<ResultBase> clone() const {
					return std::unique_ptr<TypedResult>(new TypedResult(get((void*) &x, Tag<T> { })));
				}
				Type getType() const {
					return Type::capture<T>();
				}
				void * getAddress() {
					return (void *) &x;
				}
			};
			std::unique_ptr<ResultBase> p;
		public:
			Result() {
			}
			Result(Result && r) :
					p(move(r.p)) {
			}
			Result(const Result & r) {
				*this = r;
			}
			Result & operator =(Result && r) {
				p.swap(r.p);
				return *this;
			}
			Result & operator =(const Result & r) {
				p = r.p ? r.p->clone() : nullptr;
				return *this;
			}

			Type getType() const {
				return p ? p->getType() : Type::capture<void>();
			}
			void * getAddress() {
				return p ? p->getAddress() : nullptr;
			}
			template<class T> T getValue() {
				assert(getType() == Type::capture<T>());
				return get(p->getAddress(), Tag<T> { });
			}

			template<class T> static Result capture(T x) {
				Result r;
				r.p.reset(new TypedResult<T>(static_cast<T>(x)));
				return r;
			}
		};
		AnyFunction() :
				result_type { } {
		}
		AnyFunction(std::nullptr_t) :
				result_type { } {
		}
		template<class R, class ... A> AnyFunction(R (*p)(A...)) :
				AnyFunction(p, Tag<R> { }, Tag<A...> { }, BuildIndexes<sizeof...(A)> {}) {}
			template<class R, class... A> AnyFunction(std::function<R(A...)> f) : AnyFunction(f, Tag<R> {}, Tag<A...> {}, BuildIndexes<sizeof...(A)> {}) {}
			template<class F> AnyFunction(F f) : AnyFunction(f, &F::operator()) {}

			explicit operator bool() const {return static_cast<bool>(func);}
			const std::vector<Type> & getParameterTypes() const {return parameter_types;}
			const Type & getResultType() const {return result_type;}
			Result invoke(void * const args[]) const {return func(args);}
			Result invoke(std::initializer_list<void *> args) const {return invoke(args.begin());}

		private:
			template<class... T> struct Tag {};
			template<std::size_t... IS> struct Indexes {};
			template<std::size_t N, std::size_t... IS> struct BuildIndexes : BuildIndexes<N-1, N-1, IS...> {};
			template<std::size_t... IS> struct BuildIndexes<0, IS...> : Indexes<IS...> {};
			template<class T> static T get(void * arg, Tag<T> ) {return *reinterpret_cast<T *>(arg);}
			template<class T> static T & get(void * arg, Tag<T &> ) {return *reinterpret_cast<T *>(arg);}
			template<class T> static T && get(void * arg, Tag<T &&>) {return std::move(*reinterpret_cast<T *>(arg));}
			template<class F, class R, class... A, size_t... I> AnyFunction(F f, Tag<R >, Tag<A...>, Indexes<I...>) : parameter_types( {Type::capture<A>()...}),result_type(Type::capture<R   >()) { func = [f](void * const args[]) mutable { return Result::capture<R>(f(get(args[I], Tag<A>{})...));          }; }
    template<class F,          class... A, size_t... I> AnyFunction(F f, Tag<void>, Tag<A...>, Indexes<I...>)  : parameter_types({Type::capture<A>()...}), result_type(Type::capture<void>()) { func = [f](void * const args[]) mutable { return                    f(get(args[I], Tag<A>{})...), Result{}; }; }
    template<class F, class R                         > AnyFunction(F f, Tag<R   >, Tag<    >, Indexes<    >)  : parameter_types({                     }), result_type(Type::capture<R   >()) { func = [f](void * const args[]) mutable { return Result::capture<R>(f(                         ));          }; }
    template<class F                                  > AnyFunction(F f, Tag<void>, Tag<    >, Indexes<    >)  : parameter_types({                     }), result_type(Type::capture<void>()) { func = [f](void * const args[]) mutable { return                    f(                         ), Result{}; }; }
    template<class F, class R, class... A             > AnyFunction(F f, R (F::*p)(A...)      )                : AnyFunction(f, Tag<R>{}, Tag<A...>{}, BuildIndexes<sizeof...(A)>{}) {}
    template<class F, class R, class... A             > AnyFunction(F f, R (F::*p)(A...) const)                : AnyFunction(f, Tag<R>{}, Tag<A...>{}, BuildIndexes<sizeof...(A)>{}) {}

    std::function<Result(void * const *)>               func;
    std::vector<Type>                                   parameter_types;
    Type                                                result_type;
};
}
#endif
