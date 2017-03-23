/**
 * @file Result.hpp
 * @author Nicolas Di Prima <nicolas@di-prima.fr>
 * @date 2016/05/22
 * @copyright 2016, Nicolas Di Prima. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of his contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @brief define a templated type as place holder for a result of a given
 *        type or an error.
 *
 * The main idea is to provide code and APIs as simple to read and maintain
 * as possible. It is too often difficult to determine what a given function
 * may do, the kind of error it may throw etc...
 *
 * Uses this type to wrap the return values of your function.
 *
 * For example:
 *
 * @code{cpp}
 * // from the following line:
 * int devide_1(int input_1, int input_2, int* output);
 *
 * // to the function
 * Result<int> devide_2(int input_1, int input_2) {
 *   if (!input_2) {
 *     return Result<int>::Err(std::domain_error("division by zero"));
 *   }
 *   return Result<int>::Ok(input_1 / input_2);
 * }
 * @encode
 *
 * And it is also very simple to use:
 *
 * @code{cpp}
 * // you can simply unwrap it (will throw if error)
 * int res = divide_2(2, 2).unwrap();
 * @encode
 * @code{cpp}
 * // override with you own exception if needed
 * int res = divide_2(2, 0).except(std::logic_error("silly me!"));
 * @encode
 *
 */
#ifndef RESULT_HPP_
# define RESULT_HPP_

# include <exception>
# include <utility>
# include <type_traits>

# include <boost/variant/variant.hpp>
# include <boost/variant/get.hpp>

/* Helper Macros *********************************************************** */

/// @macro TRY
/// @brief helper to unwrap a Result without throwing an exception (but
///        propagating the error.
///
/// @code{cpp}
/// Result<int> my_func1();
/// Result<int> my_func2() {
///   int i = TRY(my_func1());
///   return Result<int>::Ok(i % 2);
/// }
/// @encode
///
/// include this header file with defined **DONT_DEFINE_TRY_MACRO** if you
/// want to define you own one (i.e. add more information along your
/// exception (such as a stack trace?)).
///
# if !defined(DONT_DEFINE_TRY_MACRO)
#  define TRY(expr)           \
    ({                       \
       auto res = expr;      \
       if (res.is_error()) { \
          return res;        \
       }                     \
       res.unwrap();         \
    })
# endif  // ! DONT_DEFINE_TRY_MACRO

/* Result Class ************************************************************ */

/**
 * @class Result
 * @brief placeholder for variant type (result or error)
 */
template<typename R, typename E = std::exception>
class Result {
  static_assert( std::is_base_of<std::exception, E>::value
                 && (    std::is_move_constructible<R>::value
                      || std::is_copy_constructible<R>::value
                    )
               , "expecting error type to be base of std::exception"
               );

  static_assert(    std::is_move_constructible<R>::value
                 || std::is_copy_constructible<R>::value
               , "The result type must be move constructible or copy constructible"
               );

 public:
  using return_type = R;
  using error_type  = E;

 public:
  /// @brief Constructors
  /// @{
  Result()                          = delete;
  Result(Result const&)             = delete;
  Result(Result&&)                  = default;

  /// @brief create a result containing a result object
  ///
  /// This is the only way to create a valid Result.
  ///
  /// @code{cpp}
  /// Result<int> my_func() {
  ///   return Result<int>::Ok(42);
  /// }
  /// @endcode
  static Result<return_type, error_type> Ok(return_type&&) noexcept(false);

  /// @brief create a result containing an error object
  ///
  /// This is the only way to create an error Result
  ///
  /// @code{cpp}
  /// Result<int> my_func() {
  ///   return Result<int>::Err(std::logic_error("no answer"));
  /// }
  /// @encode
  static Result<return_type, error_type> Err(error_type&&) noexcept(false);
  /// @}

  /// @brief Destructor
  /// @{
  ~Result()                        = default;
  /// @}

  /// @brief assignments
  /// @{
  Result& operator=(Result const&) = delete;
  Result& operator=(Result&&)      = default;
  /// @}

  /// @brief const accessors
  ///
  /// these member function does not modify the underlying state
  ///
  /// @{

  /// simple function to check if the given Result contains
  /// a result.
  bool is_ok() const noexcept;

  /// simple function to check if the given Result contains
  /// an error.
  bool is_error() const noexcept;
  /// @}

  /// @brief non-const accessors (will move the content)
  ///
  /// It means: do not reuse the object...
  ///
  /// @{

  /// @brief map the result with the given function
  ///
  /// If the given Result contains a return value (not an error)
  /// then the given function will be called.
  ///
  /// NB: the function validity is not checked.
  ///
  /// @code{cpp}
  /// Result<int> my_func();
  /// struct my_data { int i_; my_data(int i) : i_(i) {} };
  /// my_data c = my_func()
  ///                 .map_res([](int&& i) { return my_data(i); })
  ///                 .unwrap();
  /// @endcode
  template<typename RR, typename F>
  Result<RR, error_type> map_res(F);

  /// @brief map the error of the given result
  ///
  /// you can update the error with more information or change its type
  template<typename EE, typename F>
  Result<return_type, EE> map_err(F);

  /// @brief if the Result is successful, call the next function
  template<typename RR, typename F>
  Result<RR, error_type> and_then(F f) {
    if (this->is_error()) {
      return Result<RR, error_type>::Err(this->get_error());
    }
    return f(this->get_return());
  }
  /// @brief if the Result has error, call the next function
  template<typename RR, typename F>
  Result<RR, error_type> or_else(F f) {
    if (this->is_ok()) {
      return Result<RR, error_type>::Ok(this->get_return());
    }
    return f(this->get_return());
  }

  /// @brief get the result (or throw the underlying error)
  return_type unwrap() noexcept(false);

  /// @brief expected an error (consume the error)
  template<class EE>
  return_type expect(EE const& err) noexcept(false);
  /// @}

 private:
  // private constructors
  explicit Result(return_type&&);
  explicit Result(error_type&&);

  // private getters
  return_type get_return() const noexcept(false);
  error_type get_error() const noexcept(false);

  // assertions
  void expect_valid_state() const noexcept(false);
  void expect_return() const noexcept(false);
  void expect_error() const noexcept(false);

 private:
  boost::variant<return_type, error_type> internal_;
};

/* ***************************************************************************
 *                                                                           *
 *                            Implementation                                 *
 *                                                                           *
 *************************************************************************** */

/* Public members ********************************************************** */

/* Constructors */

template<typename R, typename E>
Result<R, E> Result<R, E>::Ok(R&& ret) noexcept(false) {
  return Result<R, E>(std::move(ret));
}
template<typename R, typename E>
Result<R, E> Result<R, E>::Err(E&& err) noexcept(false) {
  return Result<R, E>(std::move(err));
}

/* Const accessors */

template<typename R, typename E>
bool Result<R, E>::is_ok() const noexcept { return this->internal_.which() == 0; }
template<typename R, typename E>
bool Result<R, E>::is_error() const noexcept { return this->internal_.which() == 1; }

/* Non-const accessors */

template<typename R, typename E>
template<typename RR, typename F>
Result<RR, E> Result<R, E>::map_res(F f) {
  if (this->is_error()) { return Result<RR, E>::Err(this->get_error()); }
  return Result<RR, E>::Ok(f(this->get_return()));
}

template<typename R, typename E>
template<typename EE, typename F>
Result<R, EE> Result<R, E>::map_err(F f) {
  if (this->is_ok()) { return Result<R, EE>::Ok(this->get_return()); }
  return Result<R, EE>::Err(f(this->get_error()));
}

template<typename R, typename E>
R Result<R, E>::unwrap() noexcept(false) {
  if (this->is_error()) { throw this->get_error(); }
  return this->get_return();
}

template<typename R, typename E>
template<class EE>
R Result<R, E>::expect(EE const& err) noexcept(false) {
  static_assert( std::is_base_of<std::exception, EE>::value
               , "expectation can only fail with std::exception or inherited types"
               );
  this->expect_valid_state();
  if (this->is_error()) { throw err; }
  return this->get_return();
}

/* Private members ********************************************************* */

/* constructors */

template<typename R, typename E>
Result<R, E>::Result(R&& ret) : internal_(std::move(ret)) { }
template<typename R, typename E>
Result<R, E>::Result(E&& err) : internal_(std::move(err)) { }

/* Getters */

template<typename R, typename E>
R Result<R, E>::get_return() const noexcept(false) {
  this->expect_valid_state();
  this->expect_return();
  return boost::get<return_type>(this->internal_);
}
template<typename R, typename E>
E Result<R, E>::get_error() const noexcept(false) {
  this->expect_valid_state();
  this->expect_error();
  return boost::get<error_type>(this->internal_);
}

/* Assertions */

template<typename R, typename E>
void Result<R, E>::expect_valid_state() const noexcept(false) {
  if (this->internal_.empty()) {
    // if this happens, it means the underlying boost variant
    // is empty. This should never happen!
    throw
      std::logic_error("this object is not in a valid state");
  }
}
template<typename R, typename E>
void Result<R, E>::expect_return() const noexcept(false) {
  if (this->is_error()) {
    throw std::logic_error("attempt to read the return of an error");
  }
}
template<typename R, typename E>
void Result<R, E>::expect_error() const noexcept(false) {
  if (this->is_ok()) {
    throw std::logic_error("attempt to read the error of a valid result");
  }
}

#endif /* ! RESULT_HPP_  */

/* vim: set ts=2 sw=2 et:  */
