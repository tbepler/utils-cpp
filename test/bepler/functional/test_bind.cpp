#include "gtest/gtest.h"
#include "bepler/functional/bind.h"

#include <functional>
#include <iostream>
#include "bepler/benchmark.h"

struct MutableFunctor{
    std::function<int(int,int)> f;
    inline int operator()( int a , int b ) const{
        return f( a, b );
    }
};

template< typename T >
inline T copy( T x ){
    return x;
}

TEST( FunctionalBindTest, ParamReferences ){
    using namespace functional;
    
    auto add5_rval = bind( std::plus<int>(), 5 ); //both args by r-value
    //should have been captured by value
    EXPECT_EQ( 8, add5_rval( 3 ) );

    int n = 5;
    auto addn_lval = bind( std::plus<int>(), n );
    //n should have been capture by reference
    EXPECT_EQ( 8, addn_lval( 3 ) );
    n = 2;
    EXPECT_EQ( 5, addn_lval( 3 ) );

    MutableFunctor mut_f;
    mut_f.f = std::plus<int>();
    auto mut_f_lval = bind( mut_f, 5 );
    //function should have been captured by reference
    EXPECT_EQ( 8, mut_f_lval( 3 ) );
    mut_f.f = std::multiplies<int>();
    EXPECT_EQ( 15, mut_f_lval( 3 ) );

    auto mut_f_n_val = bind( copy( mut_f ), copy( n ) );
    //function and arg should have been captured by value
    EXPECT_EQ( 6, mut_f_n_val( 3 ) );
    n = 5;
    mut_f.f = std::plus<int>();
    EXPECT_EQ( 6, mut_f_n_val( 3 ) );

    auto mut_f_n_rval = bind( std::move( mut_f ), std::move( n ) );
    //when function or arg are rvalue references, they
    //are capture by value
    EXPECT_EQ( 8, mut_f_n_rval( 3 ) );
    n = 2;
    mut_f.f = std::multiplies<int>();
    EXPECT_EQ( 8, mut_f_n_rval( 3 ) );
}

struct sum{
    template< typename T >
    inline T operator()( T x ) const{ return x; }
    template< typename T, typename... Ts >
    inline T operator()( T x, Ts... xs ) const{
        return x + sum()( xs... );
    }
};

TEST( FunctionalBindTest, BindMultipleArgs ){
    using namespace functional;

    auto f15 = bind( std::plus<int>(), 6, 9 );
    EXPECT_EQ( 15, f15() );

    auto add10 = bind( sum(), 2, 2, 4, 1, 1 );
    EXPECT_EQ( 10, add10() );
    EXPECT_EQ( 15, add10( 5 ) );
    EXPECT_EQ( 15, add10( 4, 1 ) );

}

const static double array[] = { 1.2, 3, 0.5, 4, 3, 2, 1, 46, 21, 51, -3.33,
-5.44, -1313, 8352.1414, -241.14, 14140194.141, -13513, 9 };

struct AddFiveUnbound{
    double total = 0;
    double operator()(){
        for( double d : array ){
            __asm__ __volatile__("");
            total += d + 5;
        }
        return total;
    }
};

struct AddFiveBound{
    double total = 0;
    double operator()(){
        auto add5 = functional::bind( sum(), 1, 1, 1, 1, 1 );
        for( double d : array ){
            __asm__ __volatile__("");
            total += add5( d );
        }
        return total;
    }
};

TEST( FunctionalBindTest, Benchmark ){

    double time = benchmark::timeIters( 10000, AddFiveUnbound() );
    std::cout << "add five unbound:" << std::endl;
    std::cout << "cycles per iter: " << time << std::endl;

    time = benchmark::timeIters( 10000, AddFiveBound() );
    std::cout << "add five bound:" << std::endl;
    std::cout << "cycles per iter: " << time << std::endl;

}


