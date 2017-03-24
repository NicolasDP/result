[![Build Status](https://travis-ci.org/NicolasDP/result.png?branch=master)](https://travis-ci.org/NicolasDP/result)
[![BSD](http://b.repl.ca/v1/license-BSD-blue.png)](http://en.wikipedia.org/wiki/BSD\_licenses)

# C++: Result

## Installation

* libboost-variant

compilers:

* Linux GCC: **4.8.4**, **5.4.2** and **6.2.0**
* Apple LLVM version 7.3.0

**Instructions:**

simply copy
[Result.hpp](https://raw.githubusercontent.com/NicolasDP/result/master/Result.hpp)
file into your project.

```
wget https://raw.githubusercontent.com/NicolasDP/result/master/Result.hpp
```

## How To

This C++ header file for boost's variants based Result object. This enable to
return more complex value (either an error or the value):

### Example

```C++
// define your own error type
struct parsing_error {};

// define meaning full APIs
template<typename T>
Result<T, parsing_error> parse(std::istream&);
```

And now compose meaning full objects:

```C++
// object
struct Log {
  int id;
  std::string message;
};

Result<Log, parsing_error> parse_log(std::istream& is)
{
  // use the TRY macro to simplify the use of the Result<>
  int id = TRY(parser<int>(is));
  std::string message = TRY(parse<std::string>(is));

  if (id % 2) {
    // return error when needed.
    return Result<Log, parsing_error>::Err(parsing_error{});
  }

  // return a valid value
  return Result<Log, parsing_error>::Ok(Log { id, message });
}
```

### APIs

The result templated class is only a wrapper using boost's variants. It enables
the semantic to provide meaningful APIs.

```C++
template< typename Value
            // the type of the object you want your function
            // to return.
        , typename Error = std::exception
            // the type of error you want to return
            // by default we use std::exception, but you can
            // redefine your own types
        >
class Result
{
 public:
  // only 2 constructors:
  // To return an error:
  Result<Value, Error> Err(Error&&);

  // To return successfully your value:
  Result<Value, Error> Ok(Value&&);
};
```

| Member Functions | Description |
|------------------|-------------|
| `Result<> Ok(value_type&&)` | create a `Result` object which contains the given value. |
| `Result<> Err(error_type&&)` | create a `Result` object which contains the given error. |
| `bool is_ok()` | True if the Result contains a value. |
| `bool is_err()` | True if the Result contains an error. |
| `value_type unwrap()` | returns the contained value if `is_ok()` or throw the error `error_type` |
| `value_type expect(other_error const&)` | returns the contained value if `is_ok()` or throw the error `other_error` |

### TRY macro

Helper macro to return early in your function if an error happen. This macro
helps propagating error code to the calling function, unwrapping the stack.

```C++
Result<std::ifstream> openFile(std::string);
Result<void> closeFile(std::ifstream&);
Result<std::string> readFile(std::string filename)
{
  auto fd = TRY(openFile(filename));
  // use fd here
  TRY(closeFile(fd));
  return Result::Ok<std::string>(str)
}
```

Without the macro you would:

```C++
Result<std::ifstream> openFile(std::string);
Result<void> closeFile(std::ifstream&);
Result<std::string> readFile(std::string filename)
{
  auto r = openFile(filename));
  if (r.is_error()) { return r; }
  auto fd = r.get_return();
  // use fd here
  auto rr = closeFile(fd));
  if (rr.is_error()) { return rr; }

  return Result::Ok<std::string>(str)
}
```

### Combinators

Result class also provide useful combinators:

#### operation on a result

```C++
Result<std::string> readFile(std::string);
std::string toUpperCase(std::string);

std::string res = readFile("README.md")
                    .map_res(toUpperCase)
                    .unwrap();
```

#### Operation on an error

```C++
struct MyError { std::exception nested; }

std::string res = readFile("README.md")
                    .map_error([](e) { return MyError{ e }; })
                    .unwrap();
```

#### Continue on success

```C++
Result<std::string> readFile(std::string);
template<typename T>
Result<T> parse(std::string);

int rest = readFile("pid.lock").and_then(parse<int>).unwrap();
```

#### Try something else on error

```C++
Result<std::string> readFile(std::string);
template<typename T>
Result<T> parse(std::string);

int rest = readFile("pid.lock")
            .or_else("lock.pid")
            .and_then(parse<int>).unwrap();
```

## Tests

**Prerequisites:**

* GCC-5.x or clang-3.5
* libboost-variant
* cmake 3.0 (for the tests only)
* libgtest (for the tests only)
* libpthread (for the tests only)


```Bash
mkdir Build
cd Build
cmake ..
make
```

## How to contribute

Any contributions is welcome, but a short list includes:

 * Improve the code base
 * Report an issue
 * Fix an issue
 * Improve the documentation
