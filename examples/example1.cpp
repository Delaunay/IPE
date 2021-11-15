#include <version.h>
#include <symbolic.h>

#include <iostream>


using namespace ipe;


int main()
{
    // Print Build info
    std::cout << "Commit Hash: " << _HASH  << std::endl <<
                 "Commit Date: " << _DATE  << std::endl <<
                 "     Branch: " << _BRANCH << std::endl;

    //  build function
    // ======================

    auto x = make_var("x");
    auto y = make_var("y");

            // x * y * 2
    auto f = mult(mult(x, y), make_val(2));

    // print ((x * y) * 2)
    std::cout << "Function: ";  print(f);


    //  full eval
    // ======================

    // Create a context
    Context ctx = {
        {"x", make_val(2)},
        {"y", make_val(2)}
    };

    auto ret = f->full_eval(ctx);
    std::cout << "full_eval Result: " << ret << " Expected: " << 2 * 2 * 2 << std::endl;


    //  partial eval
    // ======================

    // Create a context
    Context ctx2 = {
        {"x", make_val(2)}
    };

    auto partial_f = f->partial_eval(ctx2);
    std::cout << "Partial Function: ";  print(partial_f);

    return 0;
}
