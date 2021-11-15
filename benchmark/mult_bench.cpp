#ifndef VANAGANDR_BENCH_EIGENDATE_HEADER
#define VANAGANDR_BENCH_EIGENDATE_HEADER

#include <hayai.hpp>

#include <Symbolic.h>


class SymbolicBench: public ::hayai::Fixture
{
public:
    virtual void SetUp() {

        using namespace ipe;

        mult = mult(
            mult(
                mult(make_val(3475), make_val(5743)),
                mult(make_val(3475), make_val(5743))),
            mult(
                mult(make_val(3475), make_val(5743)),
                mult(make_val(3475), make_val(5743))));
    }

    virtual void TearDown(){
        delete mult;
    }

    Context c;
    ABSExpr* mult;
};

BENCHMARK_F(SymbolicBench, Mult, 10, 100)
{
    mult->full_eval(c);
}

volatile int v1 = 5743;
volatile int v2 = 3475;
volatile int v3 = 5743;
volatile int v4 = 3475;
volatile int v5 = 5743;
volatile int v6 = 3475;
volatile int v7 = 5743;
volatile int v8 = 3475;

BENCHMARK(Math, MultNative, 10, 100)
{
    ((v1 * v2) * (v3 * v4)) * ((v5 * v6) * (v7 * v8));
}


#endif

