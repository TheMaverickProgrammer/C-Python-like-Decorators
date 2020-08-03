# C++ Python-like Decorators
How to write decorator functions in modern C++14 or higher

Works across MSVC, GNU CC, and Clang compilers

## Skip the tutorial and view the final results 

[tutorial demo](https://godbolt.org/z/nV7gjP)

[practical demo](https://godbolt.org/z/o73nZh)

[compile-time decorator demo](https://godbolt.org/z/gCQk3S)

[run-time member function demo](https://godbolt.org/z/Fy-9XT)

[reusable member function demo](https://godbolt.org/z/w4P9V-)

# The goal
Python has a nice feature that allows function definitions to be wrapped by other existing functions. "Wrapping" consists of taking a function in as an argument and returning a new aggregated function composed of the input function and the wrapper function. The wrapper functions themselves are called 'decorator' functions because they decorate, or otherwise extend, the original input function's behavior. 

The syntax for this in python begin with `@` immediately followed by the decorator function name. On the next line, a brand new function definition begins. The result is that the new function will be automatically decorated by the function we declared with the `@` symbol. A python decorator would look like this:

```python
def stars(func):
  def inner(*args, **kwargs):
      print("**********")
      func(*args, **kwargs)
      print("**********")
      
  return inner
  
# decorator syntax
@stars
def hello_world():
   print("hello world!")
    
# The following prints:
# 
# **********
# hello world!
# **********
hello_world()
```

The decorating function `stars(...)` will take in any kind of input and pass it along to its inner `func` object, whatever that may be. In this case, `hello_world()` does not take in any arguments so `func()` will simply be called.

The `@stars` syntax is sugar for `hello_world = stars(hello_world)`. 

This is a really nice feature for python that, as a C++ enthusiast, I would like to use in my own projects. In this tutorial I'm going to make a close equivalent without using magic macros or meta-object compilation tools.

# Accepting any arbitrary functor in modern C++
Python decorator functions take in a _function_ as its argument. I toyed with a variety of concepts and discovered quickly that lambdas are not as versatile as I had hoped they would be. Consider the following code:

[goto godbolt](https://godbolt.org/z/4R7Elv)

```cpp
template<typename R, typename... Args>
auto stars(R(*in)(Args...)) {
    return [in](Args&&... args) {
        std::cout << "*******" << std::endl;
        in();
        std::cout << "*******" << std::endl;
    };
}

void hello() {
	cout << "hello, world!" << endl;
}

template<typename R, typename... Args>
auto smart_divide(R(*in)(Args...)) {
    return [in](float a, float b) {
        std::cout << "I am going to divide a and b" << std::endl;

        if(b == 0) {
            std::cout << "Whoops! cannot divide" << std::endl;
            return 0.0f;
        }

        return in(a, b);
    };
}

float divide(float a, float b) {
    return a/b;
}
```

This tries to achieve the following python code:

```python
def smart_divide(func):
   def inner(a,b):
      print("I am going to divide",a,"and",b)
      if b == 0:
         print("Whoops! cannot divide")
         return

      return func(a,b)
   return inner

@smart_divide
def divide(a,b):
    return a/b
```

It works! Great! But try uncommenting line 66. It does not compile. If you looks closely at the compiler output it has trouble deducing the function pointer types from the lambda object returned by the inner-most decorator. This is because **lambdas with capture cannot be converted to function pointers**

If we were to introduce a struct to hold our arbitrary functors, we'd fall into the same problem. By limiting ourselves to a specific expected function-pointer syntax, we lose the ability to accept just about any type we want. The solution is to use a single `template<typename F>` before our decorators to let the compiler know the decorator can take in just about anything we throw at it.

Now we can nest multiple decorator functions together! ... not so fast... 

Now we've lost the type information from `Args...` in our function signature. Luckily there is something we can do about this in C++14 and onward...

# Returning an closure that can accept any number of args
We need to build a function, in our function, that can also accept an arbitrary set of inputs and pass those along to our captured input function. To reiterate, our _returned function_ needs to be able to _forward all the arguments_ to the function we are trying to decorate.

Python gets around this problem by using special arguments `*args` and `**kwargs`. I won't go into detail what these two differerent notations mean, but for our problem task they are equivalent to C++ variadic arguments. They can be written like so:

```cpp
template<typename... Args>
void foo(Args&&... args) {
    bar(std::forward<decltype(args)>(args)...);
}
```

Anything passed into the function are forwarded as arguments for the inner function `bar`. If the types match, the compiler will accept the input. This is what we want, but remember we're returning a function inside another function. Prior to C++14 this might have been impossible to achieve nicely. Thankfully C++14 introduced **template lambdas**

```cpp
return [func]<typename... Args>(Args&&... args) {
        std::cout << "*******" << std::endl;
        func(std::forward<decltype(args)>(args)...); // forward all arguments
        std::cout << "\n*******" << std::endl;
    };
```

# Clang and MSVC - `auto` pack for the win
Try toggling the compiler options in godbolt from GNU C Compiler 9.1 to latest Clang or MSVC. No matter what standard you specify, it won't compile. We were so close! Let's inspect further. Some google searches and forum scrolling later, it seems the trusty GCC might be ahead of the curve with generic lambdas in C++14. To quote one forum user:

> C++20 will come with templated and conceptualized lambdas. The feature has already been integrated into the standard draft.

I was stuck on this for quite some time until I discovered this neat trick by trial and error:

```cpp
template<typename F>
auto output(const F& func) {
    return [func](auto&&... args) {
        std::cout << func(std::forward<decltype(args)>(args)...);
    };
}
```

By specifying the arguments for the closure as an `auto` pack, we can avoid template type parameters all together - much more readable!

# Nested decorators
Great, we can begin putting it all together! We have:

* Function that returns function (by lambda closures) (CHECK)
* "Decorator function" that can accept any input function using a template typename (CHECK)
* Inner function can also accept arbitrary args passed from outer function (using auto packs) (CHECK)

Now we can check to see if we can further nest the decorators...

[goto godbolt](https://godbolt.org/z/NW6jWE)

```cpp
// line 57 -- four decorator functions!
auto d = stars(output(smart_divide(divide)));
d(12.0f, 3.0f);
```

output is

```cpp
*******
I am going to divide a=12 and b=3
4
*******
```

First the `stars` decorator is called printing `**********` to our topmost row.
Then the `output` function prints the result of the next function to `cout` so we can see it. The result of the next function is covered by the next two nested functions.

The `smart_divide` function checks the input passed in from the top of the chain `12.0f, 3.0f` if we are dividing by zero or not before forwarding args to the next function `divide` which calculates the result. `divide` returns the result and `smart_divide` returns that result to `output`.

Finally `stars` scope is about to end and prints the last `*********` row

# Works out of the box as-is
Check out line 51 using `printf` 

```cpp
auto p = stars(printf);
p("C++ is %s!", "epic");
```

output is

```cpp
*******
C++ is epic!
*******
```

I think I found my new favorite C++ concept for outputting log files, don't you feel the same way? :)

# Practical examples
There's a lot of debate about C++'s exception handling, lack thereof, and controversial best practices. We can solve a lot of headache by providing decorator functions to let throwable functions fail without fear.

Consider this example: 
[goto godbolt](https://godbolt.org/z/VV2rRh)

We can let the function silently fail and we can choose to supply another decorator function to pipe the output to a log file. Alternatively we could also check the return value of the exception (using better value types of course this is just an example) to determine whether to shutdown the application or not.

```cpp
auto read_safe = exception_fail_safe(file_read);

// assume read_safe() returns some optional_type<> struct
if(!read_safe("missing_file.txt", buff, &sz).OK) {
    // Whoops! We needed this file. Quit immediately!
    app.abort();
    return;
}
```

# Decorating functions at compile-time!
After this tutorial was released a user by the online name [robin-m](http://robinmoussu.gitlab.io/blog) pointed out that the functions _could_ be decorated at compile-time as opposed to runtime (as I previously acknowledged this seemed to be the only way in C++ without macro magic). Robin-m suggests using `constexpr` in the function declaration. 

[goto godbolt](https://godbolt.org/z/gCQk3S)

```cpp
/////////////////////////////////////////
// final decorated functions           //
/////////////////////////////////////////

constexpr auto hello = stars(hello_impl);
constexpr auto divide = stars(output(smart_divide(divide_impl)));
constexpr auto print = stars(printf);

int main() {
 // ... 
 }
```

This allows us to separate the regular function implementation we may wish to decorate from the final decorated function we'll use in our programs. This means any modification to these 'final' functions will happen globally across our code base and not limited to a single routine's scope. This increases reusability, modularity, and readability - no sense repeating yourself twice!

# Further Applications: Decorating member functions
We can decorate member functions in C++. To be clear, we cannot change the existing member function itself, but we can bind a reference to the member function and call it.

Let's take an example that uses everything we learned so far. We want to produce a grocery checkout program to tell us the cost of each bag of apples we picked. We want to throw exceptions when invalid arguments are supplied but we want it to do so safely, log a nice timestamp somewhere, and display the price if valid.

[goto godbolt](https://godbolt.org/z/3fS4rG)

```cpp
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
```

This decorator returns an `optional_type` which for our purposes is very crude but allows us to check if the return value of the function was OK or if an exception was thrown. If it was, we want to see what it is. We declare the lambda to share the same return value as the closure with `-> optional_type<decltype(func(std::forward<decltype(args)>(args)...))>`. We use the same try-catch as before but supply different constructors for our `optional_type`.

We now want to use this decorator on our `double apple::calculate_cost(int, double)` member function. We cannot change what exists, but we can turn it into a functor using `std::bind`.

```cpp
apples groceries(1.09);
auto get_cost = exception_fail_safe(std::bind(&apples::calculate_cost, &groceries, _1, _2));
```

We create a vector of 4 results. 2 of them will throw errors while the other 2 will run just fine. Let's see our results.

```cpp
[1] There was an error: apples must weigh more than 0 ounces

[2] Bag cost $2.398

[3] Bag cost $7.085

[4] There was an error: must have 1 or more apples
```

# Reusable member-function decorators
The last example was not reusable. It was bound to exactly one object. Let's refactor that and decorate our output a little more.

```cpp
template<typename F>
auto visit_apples(const F& func) {
    return [func](apples& a, auto&&... args) {
        return (a.*func)(std::forward<decltype(args)>(args)...);
    };
}
```

All we need is a function at the very deepest part of our nest that takes in the object by reference and its member function.

We could wrap it like so

```cpp
apples groceries(2.45);
auto get_cost = exception_fail_safe(visit_apples(&apples::calculate_cost));
get_cost(groceries, 10, 3); // lots of big apples!
```

Crude but proves a point. We've basically reinvented a visitor pattern. Our decorator functions visit an object and invoke the member function on our behalf since we cannot modify the class definition. The rest is the same as it was before: functions nested in functions like little russian dolls.

We can completely take advantage of this functional-like syntax and have all our output decorators return the result value as well.

[goto godbolt](https://godbolt.org/z/w4P9V-)

```cpp
// Different prices for different apples
apples groceries1(1.09), groceries2(3.0), groceries3(4.0);
auto get_cost = log_time(output(exception_fail_safe(visit_apples(&apples::calculate_cost))));

auto vec = { 
    get_cost(groceries2, 2, 1.1), 
    get_cost(groceries3, 5, 1.3), 
    get_cost(groceries1, 4, 0) 
};
```

Which outputs

```cpp
Bag cost $6.6

> Logged at Mon Aug  5 02:17:10 2019


Bag cost $26

> Logged at Mon Aug  5 02:17:10 2019


There was an error: apples must weigh more than 0 ounces

> Logged at Mon Aug  5 02:17:10 2019
```

# Writing Python's @classmethod
In python, we have a similar decorator to properly decorate member functions: `@classmethod`. This decorator specifically tells the interpreter to pass `self` into the decorator chain, if used, so that the member function can be called correctly- specifically in the event of inherited member functions. [Further reading on stackoverflow](https://stackoverflow.com/questions/3782040/python-decorators-that-are-part-of-a-base-class-cannot-be-used-to-decorate-membe)

We needed to pass the instance of the object into the decorator chain and with a quick re-write we can make this class visitor decorate function universal.

Simply swap out `apples&` for `auto&`:

[goto godbolt](https://godbolt.org/z/nQpdfN)

```cpp
////////////////////////////////////
//    visitor function            //
////////////////////////////////////

template<typename F>
constexpr auto classmethod(F func) {
    return [func](auto& a, auto&&... args) {
        return (a.*func)(args...);
    };
}
```

Now we can visit any class type member function.

# After-thoughts
Unlike python, C++ doesn't let us redefine functions on the fly, but we could get closer to python syntax if we had some kind of intermediary functor type that we could reassign e.g.

```cpp
decorated_functor d = smart_divide(divide);

// reassignment
d = stars(output(d));
```

Unlike python, assignment of arbitrary types in C++ is almost next to impossible without type erasure.
We solved this by using templates but every new nested function returns a new type that the compiler sees.
And as we discovered at the beginning, lambdas with capture do not dissolve into C++ pointers as we might expect either. 

With all the complexities at hand, this task non-trivial. 

_update!_
I tackled this design [here](https://github.com/TheMaverickProgrammer/C-Python-Like-Class-Member-Decorators) making it possible for classes to have re-assignable member function types.

This challenge took about 2 days plugged in and was a lot of fun. I learned a lot on the way and discovered something pretty useful. Thanks for reading!
