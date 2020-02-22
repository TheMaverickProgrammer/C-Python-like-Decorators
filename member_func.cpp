// practical example of modern C++ decorators
// view the full tutorial at https://github.com/TheMaverickProgrammer/C-Python-like-Decorators

#include <iostream>
#include <memory>
#include <cassert>
#include <chrono>
#include <ctime> 
#include <stdexcept>
#include <functional>
#include <type_traits>
#include <string>
#include <variant>

using namespace std::placeholders;
using namespace std;

///////////////////////////////////
// weak optional value structure //
///////////////////////////////////
template<typename T>
struct optional_type {
    T value;
    bool OK;
    bool BAD;
    std::string msg;

    optional_type(T&& t) : value(std::move(t)) { OK = true; BAD = false; }
    optional_type(bool ok, std::string msg="") : msg(std::move(msg)) { OK = ok; BAD = !ok; }
};

////////////////////////////////////
//     decorators                 //
////////////////////////////////////

// exception decorator for optional return types
template<typename F>
auto exception_fail_safe(const F& func)  {
    return [func](auto&&... args) 
    -> optional_type<decltype(func(std::forward<decltype(args)>(args)...))> {
        using R = optional_type<decltype(func(std::forward<decltype(args)>(args)...))>;

        try {
            return R(func(std::forward<decltype(args)>(args)...));
        } catch(std::iostream::failure& e) {
            return R(false, e.what());
        } catch(std::exception& e) {
            return R(false, e.what());
        } catch(...) {
            // This ... catch clause will capture any exception thrown
            return R(false, std::string("Exception caught: default exception"));
        }
    };
}

template<typename F>
auto output(const F& func) {
    return [func](auto&&... args) {
        std::cout << func(std::forward<decltype(args)>(args)...) << std::endl;
    };
}

template<typename F>
auto log_time(const F& func) {
    return [func](auto&&... args) {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now); 
        func(std::forward<decltype(args)>(args)...);
        std::cout << "> Logged at " << std::ctime(&time) << std::endl;
    };
}

/////////////////////////////////////////////
// an example class with a member function //
/////////////////////////////////////////////

struct apples {
    apples(double cost_per_apple) : cost_per_apple(cost_per_apple) { }

    double calculate_cost(int count, double weight) {
        if(count <= 0)
            throw std::runtime_error("must have 1 or more apples");
        
        if(weight <= 0)
            throw std::runtime_error("apples must weigh more than 0 ounces");

        return count*weight*cost_per_apple;
    }

    double cost_per_apple;
};

int main() {
    // $1.09 per apple
    apples groceries(1.09);

    // we must bind the object and member function in scope
    auto get_cost = exception_fail_safe(std::bind(&apples::calculate_cost, &groceries, _1, _2));

    // create a vector of optional result values
    auto vec = { get_cost(4, 0), get_cost(2, 1.1), get_cost(5, 1.3), get_cost(0, 2.45) };

    // step through the vector and print values
    int idx = 0;
    for(auto& opt : vec) {
        std::cout << "[" << ++idx << "] ";

        if(opt.BAD) {
            std::cout << "There was an error: " << opt.msg << std::endl;
        } else {
            std::cout << "Bag cost $" << opt.value << std::endl;
        }
    }

    return 0;
}
