/* the goal was to make reusable functions that behave like python decorator functions 
 * e.g. decorators wrap existing functions and return aggregate function
 * auto foo = exception_fail_safe(fileread("missing_file.txt"));
 * foo(); // works! 
 *
 * auto bar = output_result(foo); // further decoration
 * bar(); // output: "Exception caught: missing_file.txt not found!"
 *
 * Play with source at https://godbolt.org/z/jflOuu
 * View the tutorial at https://github.com/TheMaverickProgrammer/C-Python-like-Decorators
 */

#include <iostream>
#include <memory>
#include <cassert>
using namespace std;

/////////////////////////
// decorators          //
/////////////////////////

template<typename F>
constexpr auto stars(F func) {
    return [func](auto... args) {
        std::cout << "*******" << std::endl;
        func(args...);
        std::cout << std::flush << "\n*******" << std::endl;
    };
}

template<typename F>
constexpr auto smart_divide(F func) {
    return [func](float a, float b) {
        std::cout << "I am going to divide a=" << a << " and b=" << b << std::endl;

        if(b == 0) {
            std::cout << "Whoops! cannot divide" << std::endl;
            return 0.0f;
        }

        return func(a, b);
    };
}

template<typename F>
constexpr auto output(F func) {
    return [func](auto... args) {
        std::cout << func(args...);
    };
}

////////////////////////////////////////
//    function implementations        //
////////////////////////////////////////

void hello_impl() {
	cout << "hello, world!";
}


float divide_impl(float a, float b) {
    return a/b;
}

/////////////////////////////////////////
// final decorated functions           //
/////////////////////////////////////////

constexpr auto hello = stars(hello_impl);
constexpr auto divide = stars(output(smart_divide(divide_impl)));
constexpr auto print = stars(printf);

// example for declaring a decorated function in one step.
// foo() cannot be templated for now, but C++20 should make this possible
constexpr auto foo = stars(
[](unsigned n=10) {
    for (unsigned i = 0; i < n; ++i)
        cout << "FOO!\n";
}
);

int main() {
    
    hello();

    divide(12.0f, 3.0f);

    print("C++ is %s!", "epic!");

    foo(3);

    return 0;
}
