/*
 * Copyright (c) 2015-2015, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include "MockDb.h"
#include "Sample.h"
#include <sqlpp11/sqlpp11.h>

namespace
{
  constexpr auto t = test::TabBar{};
  constexpr auto f = test::TabFoo{};

  template <typename T>
  void print_type_on_error(std::true_type)
  {
  }

  template <typename T>
  void print_type_on_error(std::false_type)
  {
    T::_print_me_;
  }

  template <typename Assert, typename... Expressions>
  void where_static_check(const Expressions&... expressions)
  {
    using CheckResult = sqlpp::check_where_static_t<Expressions...>;
    using ExpectedCheckResult = std::is_same<CheckResult, Assert>;
    print_type_on_error<CheckResult>(ExpectedCheckResult{});
    static_assert(ExpectedCheckResult::value, "Unexpected check result");

    using ReturnType = decltype(remove_from(t).where(expressions...));
    using ExpectedReturnType =
        sqlpp::logic::all_t<Assert::value xor std::is_same<ReturnType, sqlpp::bad_statement>::value>;
    print_type_on_error<ReturnType>(ExpectedReturnType{});
    static_assert(ExpectedReturnType::value, "Unexpected return type");
  }

  template <typename Assert, typename... Expressions>
  void set_dynamic_check(const Expressions&... expressions)
  {
    static auto db = MockDb{};
    using CheckResult = sqlpp::check_where_dynamic_t<decltype(db), Expressions...>;
    using ExpectedCheckResult = std::is_same<CheckResult, Assert>;
    print_type_on_error<CheckResult>(ExpectedCheckResult{});
    static_assert(ExpectedCheckResult::value, "Unexpected check result");

    using ReturnType = decltype(dynamic_remove_from(db, t).dynamic_where(expressions...));
    using ExpectedReturnType =
        sqlpp::logic::all_t<Assert::value xor std::is_same<ReturnType, sqlpp::bad_statement>::value>;
    print_type_on_error<ReturnType>(ExpectedReturnType{});
    static_assert(ExpectedReturnType::value, "Unexpected return type");
  }

  void static_where()
  {
    // OK
    where_static_check<sqlpp::consistent_t>(t.gamma);
    where_static_check<sqlpp::consistent_t>(t.gamma == true);

    // Try no expression
    where_static_check<sqlpp::assert_where_static_count_args_t>();

    // Try assignment as condition
    where_static_check<sqlpp::assert_where_expressions_t>(t.gamma = true);

    // Try non-boolean expression
    where_static_check<sqlpp::assert_where_boolean_t>(t.alpha);

    // Try some other types as expressions
    where_static_check<sqlpp::assert_where_expressions_t>("true");
    where_static_check<sqlpp::assert_where_expressions_t>(17);
    where_static_check<sqlpp::assert_where_expressions_t>('c');
    where_static_check<sqlpp::assert_where_expressions_t>(nullptr);

    // Try using aggregate functions in where
    where_static_check<sqlpp::assert_where_no_aggregate_functions_t>(count(t.alpha) > 0);
    where_static_check<sqlpp::assert_where_no_aggregate_functions_t>(t.gamma and count(t.alpha) > 0);
  }

  // column alpha is not allowed, column gamma is required
  void dynamic_where()
  {
    // OK
    set_dynamic_check<sqlpp::consistent_t>();
    where_static_check<sqlpp::consistent_t>(t.gamma);
    where_static_check<sqlpp::consistent_t>(t.gamma == true);

    // Try assignment as condition
    where_static_check<sqlpp::assert_where_expressions_t>(t.gamma = true);

    // Try non-boolean expression
    where_static_check<sqlpp::assert_where_boolean_t>(t.alpha);

    // Try some other types as expressions
    where_static_check<sqlpp::assert_where_expressions_t>("true");
    where_static_check<sqlpp::assert_where_expressions_t>(17);
    where_static_check<sqlpp::assert_where_expressions_t>('c');
    where_static_check<sqlpp::assert_where_expressions_t>(nullptr);

    // Try using aggregate functions in where
    where_static_check<sqlpp::assert_where_no_aggregate_functions_t>(count(t.alpha) > 0);
    where_static_check<sqlpp::assert_where_no_aggregate_functions_t>(t.gamma and count(t.alpha) > 0);

    // Try dynamic_where on a non-dynamic remove
    using CheckResult = sqlpp::check_where_dynamic_t<void>;
    using ExpectedCheckResult = std::is_same<CheckResult, sqlpp::assert_where_dynamic_statement_dynamic_t>;
    print_type_on_error<CheckResult>(ExpectedCheckResult{});
    static_assert(ExpectedCheckResult::value, "Unexpected check result");

    using ReturnType = decltype(remove_from(t).dynamic_where());
    using ExpectedReturnType = std::is_same<ReturnType, sqlpp::bad_statement>;
    print_type_on_error<ReturnType>(ExpectedReturnType{});
    static_assert(ExpectedReturnType::value, "Unexpected return type");
  }
}

int main(int, char**)
{
  static_where();
  dynamic_where();
}
