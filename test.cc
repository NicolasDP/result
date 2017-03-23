/**
 * @file test.cc
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
 * @brief unit tests for Result.hpp
 */
#include "gtest/gtest.h"

#include "Result.hpp"

Result<int> get_fib_recursive(int n) {
  if (n < 0) { return Result<int>::Err(std::exception()); }
  if (n == 0) { return Result<int>::Ok(1); }
  if (n == 1) { return Result<int>::Ok(1); }

  int n_1 = TRY(get_fib_recursive(n - 1));
  int n_2 = TRY(get_fib_recursive(n - 2));
  return Result<int>::Ok(n_1 + n_2);
}
Result<int> get_fib_iterative(int n) {
  if (n < 0) { return Result<int>::Err(std::exception()); }

  int n_1 = 1;
  int n_2 = 0;
  for (int i = n; i > 0; --i) {
    int next = n_1 + n_2;
    n_2 = n_1;
    n_1 = next;
  }
  return Result<int>::Ok(std::move(n_1));
}

TEST(ResultTest, CreateOk) {
  ASSERT_NO_THROW(
    {
      auto res = Result<int>::Ok(42);
      ASSERT_TRUE(res.is_ok());
    }
  );
}
TEST(ResultTest, CreateErr) {
  ASSERT_NO_THROW(
    {
      auto res = Result<int>::Err(std::exception());
      ASSERT_TRUE(res.is_error());
    }
  );
}
TEST(ResultTest, UnwrapThrow) {
  ASSERT_THROW(
    { Result<int>::Err(std::exception()).unwrap();
    }
  , std::exception
  );
}
TEST(ResultTest, UnwrapNoThrow) {
  ASSERT_NO_THROW(Result<int>::Ok(42).unwrap());
}
TEST(ResultTest, ExpectThrow) {
  ASSERT_THROW(
    { Result<int>::Err(std::exception())
        .expect(std::logic_error("logical error"));
    }
  , std::logic_error
  );
}
TEST(ResultTest, ExpectNoThrow) {
  ASSERT_NO_THROW(
    { Result<int>::Ok(42)
        .expect(std::logic_error("logical error"));
    }
  );
}

TEST(FibIterative, TestError) {
  ASSERT_THROW(
    { get_fib_iterative(-10).unwrap();
    }
  , std::exception
  );
}
TEST(FibIterative, TestOk) {
  int r;
  ASSERT_NO_THROW(
    { r = get_fib_iterative(10).unwrap();
    }
  );

  ASSERT_EQ(r, 89);
}

TEST(FibRecursive, TestError) {
  ASSERT_THROW(
    { get_fib_recursive(-10).unwrap();
    }
  , std::exception
  );
}
TEST(FibRecursive, TestOk) {
  int r;
  ASSERT_NO_THROW(
    { r = get_fib_recursive(10).unwrap();
    }
  );

  ASSERT_EQ(r, 89);
}

int  by_two(int&& i) { return i * 2; }
Result<int>  by_two_(int&& i) { return Result<int>::Ok(i * 2); }
TEST(ResultTest, TestMapResOk) {
  char c;

  ASSERT_NO_THROW(
    { c = get_fib_recursive(10)
                .map_res<char>([](int&&i) {return static_cast<char>(i); } )
                .unwrap();
    }
  );

  ASSERT_EQ(c, '\0' + 89);
}
TEST(ResultTest, TestMapResLambdaOk) {
  char c;

  ASSERT_NO_THROW(
    { c = get_fib_recursive(10)
                .map_res<char>([](int&& i) { return static_cast<char>(i); } )
                .unwrap();
    }
  );

  ASSERT_EQ(c, '\0' + 89);
}
TEST(ResultTest, TestMapResFail) {
  ASSERT_THROW(
    { get_fib_recursive(-1)
            .map_res<char>([](auto i) { return i + 1; })
            .unwrap();
    }
  , std::exception
  );
}
TEST(ResultTest, TestMapResChained) {
  char c;

  ASSERT_NO_THROW(
    { c = get_fib_recursive(0)
                .map_res<char>(by_two)
                .map_res<char>(by_two)
                .map_res<char>(by_two)
                .map_res<char>(by_two)
                .map_res<char>(by_two)
                .map_res<char>(by_two)
                .unwrap();
    }
  );

  ASSERT_EQ(c, 64);
}
TEST(ResultTest, TestMapErr) {
  struct error : virtual std::exception { };
  ASSERT_THROW(
    { get_fib_recursive(-1)
            .map_err<error>([](std::exception&&) { return error{}; })
            .unwrap();
    }
  , error
  );
}
TEST(ResultTest, TestAndThenChained) {
  int c;

  ASSERT_NO_THROW(
    { c = get_fib_recursive(0)
                .and_then<int>(by_two_)
                .and_then<int>(by_two_)
                .and_then<int>(by_two_)
                .and_then<int>(by_two_)
                .and_then<int>(by_two_)
                .and_then<int>(by_two_)
                .unwrap();
    }
  );

  ASSERT_EQ(c, 64);
}
TEST(ResultTest, TestAndThenFail) {
  struct error : virtual std::exception { };
  ASSERT_THROW(
    { get_fib_recursive(-1)
            .and_then<int>(by_two_)
            .map_err<error>([](std::exception&&) { return error{}; })
            .unwrap();
    }
  , error
  );
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

/* vim: set ts=2 sw=2 et:  */
