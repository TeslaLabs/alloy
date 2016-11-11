#include "AlloyAnyFunction.h"
#define CATCH_CONFIG_RUNNER
#include "catch.h"

using namespace aly;
template<class T> void test_type_capture(const std::type_info & info, bool is_lvalue_reference, bool is_rvalue_reference, bool is_const, bool is_volatile)
{
    const auto t = AnyFunction::Type::capture<T>();
    REQUIRE((t.info                == &info              ));
    REQUIRE((t.is_lvalue_reference == is_lvalue_reference));
    REQUIRE((t.is_rvalue_reference == is_rvalue_reference));
    REQUIRE((t.is_const            == is_const           ));
    REQUIRE((t.is_volatile         == is_volatile        ));
}

// Test capturing the traits of a number of different types:                                fully qualified type          base type     lvref  rvref  const  vol
TEST_CASE( "test AnyFunction::Type::capture<int>()"                  ) { test_type_capture<               int   >(typeid(      int  ), false, false, false, false); }
TEST_CASE( "test AnyFunction::Type::capture<const int>()"            ) { test_type_capture<const          int   >(typeid(      int  ), false, false,  true, false); }
TEST_CASE( "test AnyFunction::Type::capture<const int &>()"          ) { test_type_capture<const          int & >(typeid(      int  ),  true, false,  true, false); }
TEST_CASE( "test AnyFunction::Type::capture<int &&>()"               ) { test_type_capture<               int &&>(typeid(      int  ), false,  true, false, false); }
TEST_CASE( "test AnyFunction::Type::capture<const volatile int>()"   ) { test_type_capture<const volatile int   >(typeid(      int  ), false, false,  true,  true); }
TEST_CASE( "test AnyFunction::Type::capture<const volatile int &>()" ) { test_type_capture<const volatile int & >(typeid(      int  ),  true, false,  true,  true); }
TEST_CASE( "test AnyFunction::Type::capture<float>()"                ) { test_type_capture<               float >(typeid(      float), false, false, false, false); }
TEST_CASE( "test AnyFunction::Type::capture<void>()"                 ) { test_type_capture<               void  >(typeid(      void ), false, false, false, false); }
TEST_CASE( "test AnyFunction::Type::capture<int *>()"                ) { test_type_capture<               int * >(typeid(      int *), false, false, false, false); }
TEST_CASE( "test AnyFunction::Type::capture<const int *>()"          ) { test_type_capture<const          int * >(typeid(const int *), false, false, false, false); }

/////////////////////////////////////////////////
// Test capturing different types of functions //
/////////////////////////////////////////////////

TEST_CASE( "AnyFunction can default construct" )
{
    const AnyFunction f;
    REQUIRE( !f );
    REQUIRE( f.getParameterTypes().empty() );
    REQUIRE( f.getResultType() == AnyFunction::Type{} );
}

TEST_CASE( "AnyFunction can constructed with nullptr" )
{
    const AnyFunction f {nullptr};
    REQUIRE( !f );
    REQUIRE( f.getParameterTypes().empty() );
    REQUIRE( f.getResultType() == AnyFunction::Type{} );
}

double global_function(int a, double b, float c) { return a*b+c; }
TEST_CASE( "AnyFunction can be constructed with a global function" )
{
    const AnyFunction f {&global_function};
    REQUIRE( f );
    REQUIRE( f.getParameterTypes().size() == 3 );
    REQUIRE( f.getParameterTypes()[0] == AnyFunction::Type::capture<int>() );
    REQUIRE( f.getParameterTypes()[1] == AnyFunction::Type::capture<double>() );
    REQUIRE( f.getParameterTypes()[2] == AnyFunction::Type::capture<float>() );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<double>() );
}

TEST_CASE( "AnyFunction can be constructed with a std::function" )
{
    std::function<double(int,double,float)> sf {[](int a, double b, float c) { return a*b+c; }};
    const AnyFunction f {sf};
    REQUIRE( f );
    REQUIRE( f.getParameterTypes().size() == 3 );
    REQUIRE( f.getParameterTypes()[0] == AnyFunction::Type::capture<int>() );
    REQUIRE( f.getParameterTypes()[1] == AnyFunction::Type::capture<double>() );
    REQUIRE( f.getParameterTypes()[2] == AnyFunction::Type::capture<float>() );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<double>() );
}

TEST_CASE( "AnyFunction can be constructed with an arity-0 stateless lambda" )
{
    const AnyFunction f {[]() { return 5.0; }};
    REQUIRE( f );
    REQUIRE( f.getParameterTypes().size() == 0 );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<double>() );
}

TEST_CASE( "AnyFunction can be constructed with an arity-1 stateless lambda" )
{
    const AnyFunction f {[](int a) { return a; }};
    REQUIRE( f );
    REQUIRE( f.getParameterTypes().size() == 1 );
    REQUIRE( f.getParameterTypes()[0] == AnyFunction::Type::capture<int>() );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<int>() );
}

TEST_CASE( "AnyFunction can be constructed with an arity-2 stateless lambda" )
{
    const AnyFunction f {[](int a, double b) { return a*b; }};
    REQUIRE( f );
    REQUIRE( f.getParameterTypes().size() == 2 );
    REQUIRE( f.getParameterTypes()[0] == AnyFunction::Type::capture<int>() );
    REQUIRE( f.getParameterTypes()[1] == AnyFunction::Type::capture<double>() );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<double>() );
}

TEST_CASE( "AnyFunction can be constructed with an arity-3 stateless lambda" )
{
    const AnyFunction f {[](int a, double b, float c) { return a*b+c; }};
    REQUIRE( f );
    REQUIRE( f.getParameterTypes().size() == 3 );
    REQUIRE( f.getParameterTypes()[0] == AnyFunction::Type::capture<int>() );
    REQUIRE( f.getParameterTypes()[1] == AnyFunction::Type::capture<double>() );
    REQUIRE( f.getParameterTypes()[2] == AnyFunction::Type::capture<float>() );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<double>() );
}

TEST_CASE( "AnyFunction can be constructed with an arity-4 stateless lambda" )
{
    const AnyFunction f {[](int a, double b, float c, short d) { return a*b+c*d; }};
    REQUIRE( f );
    REQUIRE( f.getParameterTypes().size() == 4 );
    REQUIRE( f.getParameterTypes()[0] == AnyFunction::Type::capture<int>() );
    REQUIRE( f.getParameterTypes()[1] == AnyFunction::Type::capture<double>() );
    REQUIRE( f.getParameterTypes()[2] == AnyFunction::Type::capture<float>() );
    REQUIRE( f.getParameterTypes()[3] == AnyFunction::Type::capture<short>() );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<double>() );
}

double g;
TEST_CASE( "AnyFunction can be constructed with an arity-3 stateless lambda returning void" )
{
    const AnyFunction f {[](int a, double b, float c) { g=a*b+c; }};
    REQUIRE( f );
    REQUIRE( f.getParameterTypes().size() == 3 );
    REQUIRE( f.getParameterTypes()[0] == AnyFunction::Type::capture<int>() );
    REQUIRE( f.getParameterTypes()[1] == AnyFunction::Type::capture<double>() );
    REQUIRE( f.getParameterTypes()[2] == AnyFunction::Type::capture<float>() );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<void>() );
}

///////////////////////////////////////////////
// Test calling different types of functions //
///////////////////////////////////////////////

TEST_CASE( "AnyFunction is callable with a global function" )
{
    const AnyFunction f {&global_function};
    int a = 5; double b = 12.2; float c = 3.14f;
    auto r = f.invoke({&a,&b,&c});
    REQUIRE( r.getType() == AnyFunction::Type::capture<double>() );
    REQUIRE( r.getValue<double>() == a*b+c );
}

TEST_CASE( "AnyFunction is callable with a std::function" )
{
    std::function<double(int,double,float)> sf {[](int a, double b, float c) { return a*b+c; }};
    const AnyFunction f {sf};
    int a = 5; double b = 12.2; float c = 3.14f;
    auto r = f.invoke({&a,&b,&c});
    REQUIRE( r.getType() == AnyFunction::Type::capture<double>() );
    REQUIRE( r.getValue<double>() == a*b+c );
}

TEST_CASE( "AnyFunction is callable with an arity-0 stateless lambda" )
{
    const AnyFunction f {[]() { return 5.0; }};
    auto r = f.invoke({});
    REQUIRE( r.getType() == AnyFunction::Type::capture<double>() );
    REQUIRE( r.getValue<double>() == 5.0 );
}

TEST_CASE( "AnyFunction is callable with an arity-3 stateless lambda" )
{
    const AnyFunction f {[](int a, double b, float c) { return a*b+c; }};
    int a = 5; double b = 12.2; float c = 3.14f;
    auto r = f.invoke({&a,&b,&c});
    REQUIRE( r.getType() == AnyFunction::Type::capture<double>() );
    REQUIRE( r.getValue<double>() == a*b+c );
}

TEST_CASE( "AnyFunction is callable with an arity-0 stateless lambda returning void" )
{
    const AnyFunction f {[]() { g=5.0; }};
    auto r = f.invoke({});
    REQUIRE( r.getType() == AnyFunction::Type::capture<void>() );
}

TEST_CASE( "AnyFunction is callable with an arity-3 stateless lambda returning void" )
{
    const AnyFunction f {[](int a, double b, float c) { g=a*b+c; }};
    int a = 5; double b = 12.2; float c = 3.14f;
    auto r = f.invoke({&a,&b,&c});
    REQUIRE( r.getType() == AnyFunction::Type::capture<void>() );
}

//////////////////////////////////////////////////////
// Test calling functions with qualified parameters //
//////////////////////////////////////////////////////

TEST_CASE( "AnyFunction is callable with l-value references" )
{
    const AnyFunction f {[](double & x) { x=5; }};
    REQUIRE( f.getParameterTypes().size() == 1 );
    REQUIRE( f.getParameterTypes()[0] == AnyFunction::Type::capture<double &>() );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<void>() );

    double x {0};
    REQUIRE( x == 0 );

    auto r = f.invoke({&x});
    REQUIRE( x == 5 ); // x should have been modified by reference by f
    REQUIRE( r.getType() == AnyFunction::Type::capture<void>() );
}

TEST_CASE( "AnyFunction is callable with const l-value references" )
{
    static double s {0};
    REQUIRE( s == 0 );

    const AnyFunction f {[](const double & x) { s=x; }};
    REQUIRE( f.getParameterTypes().size() == 1 );
    REQUIRE( f.getParameterTypes()[0] == AnyFunction::Type::capture<const double &>() );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<void>() );

    double x {5};
    auto r = f.invoke({&x});
    REQUIRE( s == 5 ); // s should have been modified by f
    REQUIRE( r.getType() == AnyFunction::Type::capture<void>() );
}

TEST_CASE( "AnyFunction is callable with volatile l-value references" )
{
    static double s {0};
    REQUIRE( s == 0 );

    const AnyFunction f {[](volatile double & x) { s=x; }};
    REQUIRE( f.getParameterTypes().size() == 1 );
    REQUIRE( f.getParameterTypes()[0] == AnyFunction::Type::capture<volatile double &>() );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<void>() );

    double x {5};
    auto r = f.invoke({&x});
    REQUIRE( s == 5 ); // s should have been modified by f
    REQUIRE( r.getType() == AnyFunction::Type::capture<void>() );
}

TEST_CASE( "AnyFunction is callable with const volatile l-value references" )
{
    static double s {0};
    REQUIRE( s == 0 );

    const AnyFunction f {[](const volatile double & x) { s=x; }};
    REQUIRE( f.getParameterTypes().size() == 1 );
    REQUIRE( f.getParameterTypes()[0] == AnyFunction::Type::capture<const volatile double &>() );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<void>() );

    double x {5};
    auto r = f.invoke({&x});
    REQUIRE( s == 5 ); // s should have been modified by f
    REQUIRE( r.getType() == AnyFunction::Type::capture<void>() );
}

TEST_CASE( "AnyFunction is callable with r-value references" )
{
    const AnyFunction f {[](std::vector<double> && x) { return std::move(x); }};
    REQUIRE( f.getParameterTypes().size() == 1 );
    REQUIRE( f.getParameterTypes()[0] == AnyFunction::Type::capture<std::vector<double> &&>() );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<std::vector<double>>() );

    std::vector<double> x = {1,2,3,4,5,6,7,8,9,10};
    REQUIRE( x.size() == 10 );

    auto r = f.invoke({&x});
    REQUIRE( x.size() == 0 ); // x should have been moved-from by f
    REQUIRE( r.getType() == AnyFunction::Type::capture<std::vector<double>>() );
    REQUIRE( r.getValue<std::vector<double>>().size() == 10 );
}

TEST_CASE( "AnyFunction is callable with const volatile r-value references" )
{
    static double s {0};
    REQUIRE( s == 0 );

    const AnyFunction f {[](const volatile double && x) { s=x; }};
    REQUIRE( f.getParameterTypes().size() == 1 );
    REQUIRE( f.getParameterTypes()[0] == AnyFunction::Type::capture<const volatile double &&>() );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<void>() );

    double x {5};
    auto r = f.invoke({&x});
    REQUIRE( s == 5 ); // s should have been modified by f
    REQUIRE( r.getType() == AnyFunction::Type::capture<void>() );
}

/////////////////////////////////////////////////////////
// Test calling functions with qualified return values //
/////////////////////////////////////////////////////////

TEST_CASE( "AnyFunction can return const values" )
{
    const AnyFunction f {[]() -> const double { return 5.0; }};
    REQUIRE( f.getParameterTypes().size() == 0 );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<const double>() );

    auto r = f.invoke({});
    REQUIRE( r.getType() == AnyFunction::Type::capture<const double>() );
    REQUIRE( r.getValue<const double>() == 5.0 );
}

TEST_CASE( "AnyFunction can return l-value references" )
{
    double x {};
    const AnyFunction f {[&x]() -> double & { return x; }};
    REQUIRE( f.getParameterTypes().size() == 0 );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<double &>() );

    auto r = f.invoke({});
    REQUIRE( r.getType() == AnyFunction::Type::capture<double &>() );
    REQUIRE( &r.getValue<double &>() == &x );
}

TEST_CASE( "AnyFunction can return const l-value references" )
{
    double x {};
    const AnyFunction f {[&x]() -> const double & { return x; }};
    REQUIRE( f.getParameterTypes().size() == 0 );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<const double &>() );

    auto r = f.invoke({});
    REQUIRE( r.getType() == AnyFunction::Type::capture<const double &>() );
    REQUIRE( &r.getValue<const double &>() == &x );
}

TEST_CASE( "AnyFunction can return volatile l-value references" )
{
    double x {};
    const AnyFunction f {[&x]() -> volatile double & { return x; }};
    REQUIRE( f.getParameterTypes().size() == 0 );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<volatile double &>() );

    auto r = f.invoke({});
    REQUIRE( r.getType() == AnyFunction::Type::capture<volatile double &>() );
    REQUIRE( &r.getValue<volatile double &>() == &x );
}

TEST_CASE( "AnyFunction can return const volatile l-value references" )
{
    double x {};
    const AnyFunction f {[&x]() -> const volatile double & { return x; }};
    REQUIRE( f.getParameterTypes().size() == 0 );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<const volatile double &>() );

    auto r = f.invoke({});
    REQUIRE( r.getType() == AnyFunction::Type::capture<const volatile double &>() );
    REQUIRE( &r.getValue<const volatile double &>() == &x );
}

TEST_CASE( "AnyFunction can return r-value references" )
{
    double x {};
    const AnyFunction f {[&x]() -> double && { return std::move(x); }};
    REQUIRE( f.getParameterTypes().size() == 0 );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<double &&>() );

    auto r = f.invoke({});
    REQUIRE( r.getType() == AnyFunction::Type::capture<double &&>() );

    double && ref_x = r.getValue<double &&>();
    REQUIRE( &ref_x == &x );
}

TEST_CASE( "AnyFunction can return const volatile r-value references" )
{
    double x {};
    const AnyFunction f {[&x]() -> const volatile double && { return std::move(x); }};
    REQUIRE( f.getParameterTypes().size() == 0 );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<const volatile double &&>() );

    auto r = f.invoke({});
    REQUIRE( r.getType() == AnyFunction::Type::capture<const volatile double &&>() );

    const volatile double && ref_x = r.getValue<const volatile double &&>();
    REQUIRE( &ref_x == &x );
}

///////////////////////////////////////////
// Test calling mutable function objects //
///////////////////////////////////////////

struct counter { int i=0; int operator()() { return ++i; } };
TEST_CASE( "AnyFunction can capture mutable function objects" )
{
    AnyFunction f {counter()};
    REQUIRE( f.getParameterTypes().size() == 0 );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<int>() );

    auto r = f.invoke({});
    REQUIRE( r.getType() == AnyFunction::Type::capture<int>() );
    REQUIRE( r.getValue<int>() == 1 );
    REQUIRE( f.invoke({}).getValue<int>() == 2 );
    REQUIRE( f.invoke({}).getValue<int>() == 3 );
    REQUIRE( f.invoke({}).getValue<int>() == 4 );
    REQUIRE( f.invoke({}).getValue<int>() == 5 );
}

TEST_CASE( "AnyFunction can capture mutable lambdas" )
{
    int i=0;
    AnyFunction f {[i]() mutable { return ++i; }};
    REQUIRE( f.getParameterTypes().size() == 0 );
    REQUIRE( f.getResultType() == AnyFunction::Type::capture<int>() );

    auto r = f.invoke({});
    REQUIRE( r.getType() == AnyFunction::Type::capture<int>() );
    REQUIRE( r.getValue<int>() == 1 );
    REQUIRE( f.invoke({}).getValue<int>() == 2 );
    REQUIRE( f.invoke({}).getValue<int>() == 3 );
    REQUIRE( f.invoke({}).getValue<int>() == 4 );
    REQUIRE( f.invoke({}).getValue<int>() == 5 );
}
