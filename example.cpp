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
constexpr auto stars(const F& func) {
    return [func](auto&&... args) {
        cout << "*******" << endl;
        func(forward<decltype(args)>(args)...);
        cout << "\n*******" << endl;
    };
}

template<typename F>
constexpr auto smart_divide(const F& func) {
    return [func](float a, float b) {
        cout << "I am going to divide a=" << a << " and b=" << b << endl;

        if(b == 0) {
            cout << "Whoops! cannot divide" << endl;
            return 0.0f;
        }

        return func(a, b);
    };
}

template<typename F>
constexpr auto output(const F& func) {
    return [func](auto&&... args) {
        cout << func(forward<decltype(args)>(args)...);
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
