/*******************************************************************\

 Module: MODULE NAME

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/
#include "catch.hpp"

#include <stack>
#include <iostream>

#include <util/symbol.h>
#include <util/symbol_table.h>
#include <util/namespace.h>
#include <util/std_expr.h>
#include <util/std_code.h>
#include <ansi-c/ansi_c_language.h>
#include <ansi-c/expr2c.h>
#include <analyses/variable-sensitivity/continuation_stack_entry.h>
#include <analyses/variable-sensitivity/abstract_enviroment.h>
#include <analyses/variable-sensitivity/variable_sensitivity_object_factory.h>

TEST_CASE( "Continuations stack built", "[continuation-stack]" )
{
  symbol_tablet symbol_table;
  namespacet ns(symbol_table);

  optionst options;
  options.set_option("pointers", true);
  options.set_option("arrays", true);
  options.set_option("structs", true);
  variable_sensitivity_object_factoryt::instance().set_options(options);


  abstract_environmentt environment;
  environment.make_top();

  SECTION("int x")
  {
    typet basic_symbol_type=signedbv_typet(32);

    symbolt basic_symbol;
    basic_symbol.name="x";
    basic_symbol.base_name="x";
    basic_symbol.type=basic_symbol_type;
    symbol_table.add(basic_symbol);

    SECTION("&x")
    {
      exprt in_expr=address_of_exprt(symbol_exprt("x", basic_symbol_type));

      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();

      CAPTURE(expr2c(out_expr, ns));

      REQUIRE(out_expr.id()==ID_address_of);
      const exprt &object=out_expr.op0();
      REQUIRE(object.id()==ID_symbol);
    }
  }
  SECTION("int a[5]")
  {
    typet array_type=
      array_typet(signedbv_typet(32), constant_exprt::integer_constant(5));

    symbolt array_symbol;
    array_symbol.name="a";
    array_symbol.base_name="a";
    array_symbol.type=array_type;
    symbol_table.add(array_symbol);

    SECTION("a")
    {
      // TODO
    }

    SECTION("&a")
    {
      // TODO?
    }
    SECTION("&a[0]")
    {
      exprt in_expr=address_of_exprt(
        index_exprt(
          symbol_exprt("a", array_type),
          constant_exprt::integer_constant(0)));

      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();

      CAPTURE(expr2c(out_expr, ns));

      REQUIRE(out_expr.id()==ID_address_of);
      const exprt &object=out_expr.op0();
      REQUIRE(object.id()==ID_index);
      REQUIRE(object.op0().id()==ID_symbol);
      REQUIRE(object.op1().id()==ID_constant);
    }
    SECTION("&a[1]")
    {
      exprt in_expr=address_of_exprt(
        index_exprt(
          symbol_exprt("a", array_type),
          constant_exprt::integer_constant(1)));

      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();

      CAPTURE(expr2c(out_expr, ns));

      REQUIRE(out_expr.id()==ID_address_of);
      const exprt &object=out_expr.op0();
      REQUIRE(object.id()==ID_index);
      REQUIRE(object.op0().id()==ID_symbol);
      REQUIRE(object.op1().id()==ID_constant);
      REQUIRE(to_constant_expr(object.op1()).get_value()==integer2string(1));
    }
    SECTION("&a[0]+1")
    {
      exprt in_expr=plus_exprt(
        address_of_exprt(
          index_exprt(
            symbol_exprt("a", array_type),
            constant_exprt::integer_constant(0))),
        constant_exprt::integer_constant(1));

      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();

      CAPTURE(expr2c(out_expr, ns));

      REQUIRE(out_expr.id()==ID_address_of);
      const exprt &object=out_expr.op0();
      REQUIRE(object.id()==ID_index);
      REQUIRE(object.op0().id()==ID_symbol);
      REQUIRE(object.op1().id()==ID_constant);
      REQUIRE(to_constant_expr(object.op1()).get_value()==integer2string(1));
    }
    SECTION("&a[1]+1")
    {
      exprt in_expr=plus_exprt(
        address_of_exprt(
          index_exprt(
            symbol_exprt("a", array_type),
            constant_exprt::integer_constant(1))),
        constant_exprt::integer_constant(1));

      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();

      CAPTURE(expr2c(out_expr, ns));

      REQUIRE(out_expr.id()==ID_address_of);
      const exprt &object=out_expr.op0();
      REQUIRE(object.id()==ID_index);
      REQUIRE(object.op0().id()==ID_symbol);
      REQUIRE(object.op1().id()==ID_constant);
      REQUIRE(to_constant_expr(object.op1()).get_value()==integer2string(2));
    }
    SECTION("&a[1]-1")
    {
      exprt in_expr=minus_exprt(
        address_of_exprt(
          index_exprt(
            symbol_exprt("a", array_type),
            constant_exprt::integer_constant(1))),
        constant_exprt::integer_constant(1));

      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();

      CAPTURE(expr2c(out_expr, ns));

      REQUIRE(out_expr.id()==ID_address_of);
      const exprt &object=out_expr.op0();
      REQUIRE(object.id()==ID_index);
      REQUIRE(object.op0().id()==ID_symbol);
      REQUIRE(object.op1().id()==ID_constant);
      REQUIRE(to_constant_expr(object.op1()).get_value()==integer2string(0));
    }
    SECTION("1+&a[1]")
    {
      exprt in_expr=plus_exprt(
        constant_exprt::integer_constant(1),
        address_of_exprt(
          index_exprt(
            symbol_exprt("a", array_type),
            constant_exprt::integer_constant(1))));

      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();

      CAPTURE(expr2c(out_expr, ns));

      REQUIRE(out_expr.id()==ID_address_of);
      const exprt &object=out_expr.op0();
      REQUIRE(object.id()==ID_index);
      REQUIRE(object.op0().id()==ID_symbol);
      REQUIRE(object.op1().id()==ID_constant);
      REQUIRE(to_constant_expr(object.op1()).get_value()==integer2string(2));
    }
  }
  SECTION("pointer + 1")
  {
    // TODO
  }
  SECTION("pointer - 1")
  {
    // TODO
  }
  SECTION("address of member")
  {
      struct_union_typet::componentt component("comp", signedbv_typet(32));
      struct_typet struct_type;
      struct_type.set_tag("str");
      struct_type.components().push_back(component);

      symbolt struct_symbol;
      struct_symbol.base_name="str";
      struct_symbol.name="tag-str";
      struct_symbol.type=struct_type;
      struct_symbol.is_type=true;

      symbol_table.add(struct_symbol);

      symbolt struct_variable;
      struct_variable.base_name="s";
      struct_variable.name="s";
      struct_symbol.type=struct_type;
      symbol_table.add(struct_variable);

      exprt in_expr=
        address_of_exprt(
          member_exprt(
            symbol_exprt("s", struct_type),
            "comp",
            component.type()));

      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();

      CAPTURE(expr2c(out_expr, ns));

      REQUIRE(out_expr.id()==ID_address_of);
      const exprt &object=out_expr.op0();

      REQUIRE(object.id()==ID_member);
      REQUIRE(object.op0().id()==ID_symbol);
  }
  SECTION("address of array member")
  {
      // TODO
  }
  SECTION("int x = 4; &a[x]")
  {
    // environment.assign(symbol_exprt("x"), environment.abstract_object_factory())
  }
}
