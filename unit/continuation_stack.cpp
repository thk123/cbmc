/*******************************************************************\

 Module: MODULE NAME

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/
#include "catch.hpp"

#include <stack>

#include <util/symbol.h>
#include <util/symbol_table.h>
#include <util/namespace.h>
#include <util/std_expr.h>
#include <util/std_code.h>
#include <ansi-c/ansi_c_language.h>
#include <analyses/variable-sensitivity/continuation_stack_entry.h>
#include <analyses/variable-sensitivity/abstract_enviroment.h>
#include <analyses/variable-sensitivity/variable_sensitivity_object_factory.h>

class test_continuation_stackt
{
public:
  static const continuation_stackt::continuation_stack_storet &get_stack(
    const continuation_stackt& continatuon_stack)
  {
    return continatuon_stack.stack;
  }
};

TEST_CASE( "Continuations stack built", "[continuation-stack]" )
{
//  ansi_c_languaget lang;
//  exprt new_expr;
  symbol_tablet symbol_table;
  namespacet ns(symbol_table);


//  exprt struct_declaration;

//  bool result=lang.to_expr("int s;", "", struct_declaration, ns);
//  CHECK_FALSE(result);

  //struct str s; int *a=&s.a[2];

  optionst options;
  options.set_option("pointers", true);
  options.set_option("arrays", true);
  options.set_option("structs", true);
  variable_sensitivity_object_factoryt::instance().set_options(options);
  abstract_environmentt environment;


  auto pointer_symbol=symbol_exprt("a", pointer_typet(signedbv_typet(32)));

  auto rhs=address_of_exprt(
    member_exprt(
      index_exprt(symbol_exprt("s"), constant_exprt::integer_constant(2)),
      "a"));

  //code_assignt(pointer_symbol, address_of_exprt);

  auto stack=continuation_stackt(rhs.object(), environment, ns);
  const auto &internal_stack=test_continuation_stackt::get_stack(stack);
  REQUIRE(internal_stack.size()==3);

  /*
   *
   * int a = 4;
   * int* pa = &a;
   * int a[] = {1, ..., 9}
   * int *p = &a[4]  --> p stack=[SYMBOL a][OFFSET 4]
   *
   *
   */
}
