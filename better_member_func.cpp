// practical examples of modern C++ decorators
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

////////////////////////////////////
// weak optional value structure  //
////////////////////////////////////

template<typename T>
struct optional_type {
    T value;
    bool OK;
    bool BAD;
    std::string msg;

    optional_type(T t) : value(t) { OK = true; BAD = false; }
    optional_type(bool ok, std::string msg="") : msg(msg) { OK = ok; BAD = !ok; }
};

/////////////////////////////////////
//          decorators             //
/////////////////////////////////////

// exception decorator for optional return types
template<typename F>
auto exception_fail_safe(F func)  {
    return [func](auto... args) -> optional_type<decltype(func(args...))> {
        using R = optional_type<decltype(func(args...))>;

        try {
            return R(func(args...));
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

// this decorator can output our optional data
template<typename F>
auto output(F func) {
    return [func](auto... args) {
        auto opt = func(args...);
        
        if(opt.BAD) {
            std::cout << "There was an error: " << opt.msg << std::endl;
        } else {
            std::cout << "Bag cost $" << opt.value << std::endl;
        }

        return opt;
    };
}

// this decorator prints time and returns value of inner function
// returning is purely conditional based on our needs, in this case
// we want to take advantage of the functional-like syntax we've created
template<typename F>
auto log_time(F func) {
    return [func](auto... args) {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now); 
        auto opt = func(args...);
        std::cout << "> Logged at " << std::ctime(&time) << std::endl;

        return opt;
    };
}

///////////////////////////////////////////////
// an example class with a member function   //
///////////////////////////////////////////////
struct apples {
    // ctor
    apples(double cost_per_apple) : cost_per_apple(cost_per_apple) { }

    // member function that throws
    double calculate_cost(int count, double weight) {
        if(count <= 0)
            throw std::runtime_error("must have 1 or more apples");
        
        if(weight <= 0)
            throw std::runtime_error("apples must weigh more than 0 ounces");

        return count*weight*cost_per_apple;
    }

    double cost_per_apple;
};

////////////////////////////////////
//    visitor function            //
////////////////////////////////////

template<typename F>
auto visit_apples(F func) {
    return [func](apples& a, auto... args) {
        return (a.*func)(args...);
    };
}

////////////////////////////////////
// final decorated function       //
////////////////////////////////////

auto get_cost = log_time(output(exception_fail_safe(visit_apples(&apples::calculate_cost))));

int main() {
    // Different prices for different apples
    apples groceries1(1.09), groceries2(3.0), groceries3(4.0);

    // this vector will contain optional values
    // at construction, it will also print what we want to see
    auto vec = { 
        get_cost(groceries2, 2, 1.1), 
        get_cost(groceries3, 5, 1.3), 
        get_cost(groceries1, 4, 0) 
    };

    return 0;
}
