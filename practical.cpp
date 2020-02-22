// practical example of modern C++ decorators
// view the full tutorial at https://github.com/TheMaverickProgrammer/C-Python-like-Decorators

#include <iostream>
#include <memory>
#include <cassert>
#include <chrono>
#include <ctime> 

using namespace std;

/////////////////////////
//   decorators        //
/////////////////////////

// our basic fail-safe exception decorator will return strings for output in this demo
// see https://github.com/TheMaverickProgrammer/C-Python-like-Decorators/blob/master/README.md#further-applications-decorating-member-functions
// for a more robust example
template<typename F>
auto exception_fail_safe(const F& func) {
    return [func](auto&&... args) {
        try {
            func(std::forward<decltype(args)>(args)...);
        } catch(std::iostream::failure& e) {
            return std::string("Exception caught: ") + e.what();
        } catch(...) {
            // This ... catch clause will capture any exception thrown
            return std::string("Exception caught: default exception");
        }

        return std::string("OK"); // No exceptions!
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

//////////////////////////////
// function implementations //
//////////////////////////////

void file_read_impl(const char* path, char* data, int* sz) {
    // for demo purposes, always fail
    std::string msg = std::string(path) + std::string(" not found!");
    throw std::iostream::failure(msg.c_str());
}

///////////////////////////////
// final decorated functions //
///////////////////////////////

auto file_read = exception_fail_safe(file_read_impl);
auto print_file_read = log_time(output(file_read));

int main() {
    // some demo dummy data for our mock file read...
    char* buff = 0;
    int sz = 0;

    std::cout << "First read fails silently" << std::endl;
    file_read("missing_file.txt", buff, &sz);

    std::cout << "\nSecond read fails and prints to the console with time:" << std::endl;
    print_file_read("missing_file.txt", buff, &sz);

    return 0;
}
