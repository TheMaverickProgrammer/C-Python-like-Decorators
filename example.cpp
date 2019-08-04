/* reusable functions that behave like python decorator functions 
 * e.g. decorators wrap existing functions and return aggregate function
 * auto foo = exception_fail_safe(fileread);
 * foo("missing_file.txt"); // works! 
 *
 * auto bar = output_result(foo); // further decoration
 * bar("missing_file.txt"); // output: "Exception caught: missing_file.txt not found!"
 *
 * Play source at https://godbolt.org/z/Q6C6qu
 *
 * View the tutorial at https://github.com/TheMaverickProgrammer/C-Python-like-Decorators
 */

#include <iostream>
#include <memory>
#include <cassert>
using namespace std;

template<typename F>
auto stars(F func) {
    return [func]<typename... Args>(Args... args) {
        std::cout << "*******" << std::endl;
        func(args...);
        std::cout << std::flush << "\n*******" << std::endl;
    };
}

template<typename F>
auto smart_divide(F func) {
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
auto output(F func) {
    return [func]<typename... Args>(Args... args) {
        std::cout << func(args...);
    };
}

// Regular functions

void hello() {
	cout << "hello, world!";
}


float divide(float a, float b) {
    return a/b;
}

int main() {
    auto p = stars(printf);
    p("C++ is %s!", "epic");

    auto h = stars(hello);
    h();

    auto d = stars(output(smart_divide(divide)));
    d(12.0f, 3.0f);

    return 0;
}
