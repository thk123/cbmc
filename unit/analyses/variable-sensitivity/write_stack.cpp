/*******************************************************************\

 Module: Write Stack Unit Tests

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/

/// \file
/// Unit tests for construction of write stack

#include "catch.hpp"

#include <memory>

#include <util/namespace.h>
#include <util/options.h>
#include <util/std_types.h>
#include <util/symbol.h>
#include <util/symbol_table.h>
#include <util/type.h>
#include <util/ui_message.h>

#include <ansi-c/ansi_c_language.h>
#include <ansi-c/expr2c.h>

#include <analyses/variable-sensitivity/abstract_enviroment.h>
#include <analyses/variable-sensitivity/continuation_stack.h>
#include <analyses/variable-sensitivity/variable_sensitivity_object_factory.h>

class write_stack_utilt
{
public:
  write_stack_utilt():
      message_handler(
        std::unique_ptr<message_handlert>(new ui_message_handlert()))
    {
      language.set_message_handler(*message_handler);
    }

  symbolt create_lvalue_symbol(const irep_idt &name, const typet &type)
  {
    symbolt basic_symbol;
    basic_symbol.name=name;
    basic_symbol.base_name=name;
    basic_symbol.type=type;
    basic_symbol.is_lvalue=true;
    return basic_symbol;
  }

  exprt to_expr(const std::string &input_string, const namespacet &ns)
  {
    exprt expr;
    bool result=language.to_expr(input_string, "",  expr, ns);
    assert(!result);
    return expr;
  }

private:
  std::unique_ptr<message_handlert> message_handler;
  ansi_c_languaget language;
};

SCENARIO("Constructing a write stack in top environment",
  "[core][analyses][variable-sensitivity][write_stack]")
{
  symbol_tablet symbol_table;
  namespacet ns(symbol_table);
  write_stack_utilt write_stack_util;

  // Configure the variable sensitivity domain
  optionst options;
  options.set_option("pointers", true);
  options.set_option("arrays", true);
  options.set_option("structs", true);
  variable_sensitivity_object_factoryt::instance().set_options(options);

  abstract_environmentt environment;
  environment.make_top();

  GIVEN("An integer symbol x")
  {
    // int x;
    symbolt basic_symbol=
      write_stack_util.create_lvalue_symbol("x", signedbv_typet(32));
    symbol_table.add(basic_symbol);

    WHEN("Constructing a write stack from &x")
    {
      exprt in_expr=write_stack_util.to_expr("&x", ns);
      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();

      THEN("Get a write stack for the address of x")
      {
        CAPTURE(expr2c(out_expr, ns));
        REQUIRE(out_expr.id()==ID_address_of);
        const exprt &object=out_expr.op0();
        REQUIRE(object.id()==ID_symbol);
        REQUIRE(object.get_string(ID_identifier)=="x");
      }
    }
    WHEN("Constructing a write stack from &x + 1")
    {
      exprt in_expr=write_stack_util.to_expr("&x + 1", ns);
      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);

      THEN("Get a top write stack")
      {
        REQUIRE(stack.is_top_value());
      }
    }
    WHEN("Constructing a write stack from &x - 1")
    {
      exprt in_expr=write_stack_util.to_expr("&x - 1", ns);
      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);

      THEN("Get a top write stack")
      {
        REQUIRE(stack.is_top_value());
      }
    }
  }
  GIVEN("An fixed length array of integers a[5]")
  {
    typet array_type=
      array_typet(signedbv_typet(32), constant_exprt::integer_constant(5));
    symbolt array_symbol=
      write_stack_util.create_lvalue_symbol("a", array_type);
    symbol_table.add(array_symbol);

    WHEN("Constructing a write stack from a")
    {
      exprt in_expr=write_stack_util.to_expr("a", ns);
      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();

      THEN("Get a write stack for the address of a")
      {
        CAPTURE(expr2c(out_expr, ns));
        REQUIRE(out_expr.id()==ID_address_of);
        const exprt &object=out_expr.op0();
        REQUIRE(object.id()==ID_symbol);
        REQUIRE(object.get_string(ID_identifier)=="a");
      }
    }
    WHEN("Constructing a write stack from &a")
    {
      exprt in_expr=write_stack_util.to_expr("&a", ns);
      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();

      THEN("Get a write stack for the address of a")
      {
        CAPTURE(expr2c(out_expr, ns));
        REQUIRE(out_expr.id()==ID_address_of);
        const exprt &object=out_expr.op0();
        REQUIRE(object.id()==ID_symbol);
        REQUIRE(object.get_string(ID_identifier)=="a");
      }
    }
    WHEN("Constructing a write stack from &a[0]")
    {
      exprt in_expr=write_stack_util.to_expr("&a", ns);
      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();

      THEN("Get a write stack for the address of a")
      {
        CAPTURE(expr2c(out_expr, ns));
        REQUIRE(out_expr.id()==ID_address_of);
        const exprt &object=out_expr.op0();
        REQUIRE(object.id()==ID_symbol);
        REQUIRE(object.get_string(ID_identifier)=="a");
      }
    }
    WHEN("Constructing a write stack from &a[1]")
    {
      exprt in_expr=write_stack_util.to_expr("&a[1]", ns);

      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();
      THEN("Get a write stack from")
      {
        CAPTURE(expr2c(out_expr, ns));

        REQUIRE(out_expr.id()==ID_address_of);
        const exprt &object=out_expr.op0();
        REQUIRE(object.id()==ID_index);
        REQUIRE(object.op0().id()==ID_symbol);
        REQUIRE(object.op0().get_string(ID_identifier)=="a");
        REQUIRE(object.op1().id()==ID_constant);
        REQUIRE(to_constant_expr(object.op1()).get_value()==integer2string(1));
      }
    }
    WHEN("Constructing a write stack from &a[0]+1")
    {
      exprt in_expr=write_stack_util.to_expr("&a[0]+1", ns);

      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();
      THEN("Get a write stack from")
      {
        CAPTURE(expr2c(out_expr, ns));

        REQUIRE(out_expr.id()==ID_address_of);
        const exprt &object=out_expr.op0();
        REQUIRE(object.id()==ID_index);
        REQUIRE(object.op0().id()==ID_symbol);
        REQUIRE(object.op0().get_string(ID_identifier)=="a");
        REQUIRE(object.op1().id()==ID_constant);
        REQUIRE(to_constant_expr(object.op1()).get_value()==integer2string(1));
      }
    }
    WHEN("Constructing a write stack from &a[1]+1")
    {
      exprt in_expr=write_stack_util.to_expr("&a[1]+1", ns);

      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();
      THEN("Get a write stack from")
      {
        CAPTURE(expr2c(out_expr, ns));

        REQUIRE(out_expr.id()==ID_address_of);
        const exprt &object=out_expr.op0();
        REQUIRE(object.id()==ID_index);
        REQUIRE(object.op0().id()==ID_symbol);
        REQUIRE(object.op0().get_string(ID_identifier)=="a");
        REQUIRE(object.op1().id()==ID_constant);
        REQUIRE(to_constant_expr(object.op1()).get_value()==integer2string(2));
      }
    }
    WHEN("Constructing a write stack from &a[1]-1")
    {
      exprt in_expr=write_stack_util.to_expr("&a[1]-1", ns);

      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();
      THEN("Get a write stack from")
      {
        CAPTURE(expr2c(out_expr, ns));

        REQUIRE(out_expr.id()==ID_address_of);
        const exprt &object=out_expr.op0();
        REQUIRE(object.id()==ID_index);
        REQUIRE(object.op0().id()==ID_symbol);
        REQUIRE(object.op0().get_string(ID_identifier)=="a");
        REQUIRE(object.op1().id()==ID_constant);
        REQUIRE(to_constant_expr(object.op1()).get_value()==integer2string(0));
      }
    }
    WHEN("Constructing a write stack from 1+&a[1]")
    {
      exprt in_expr=write_stack_util.to_expr("1+&a[1]", ns);

      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();
      THEN("Get a write stack from")
      {
        CAPTURE(expr2c(out_expr, ns));

        REQUIRE(out_expr.id()==ID_address_of);
        const exprt &object=out_expr.op0();
        REQUIRE(object.id()==ID_index);
        REQUIRE(object.op0().id()==ID_symbol);
        REQUIRE(object.op0().get_string(ID_identifier)=="a");
        REQUIRE(object.op1().id()==ID_constant);
        REQUIRE(to_constant_expr(object.op1()).get_value()==integer2string(2));
      }
    }
    // TODO: some WHEN("given valid but weird C involving arrays")
  }
  GIVEN("A struct with an integer component")
  {
    struct_union_typet::componentt component("comp", signedbv_typet(32));
    struct_typet struct_type;
    struct_type.set_tag("str");
    struct_type.components().push_back(component);

    symbolt struct_symbol=
      write_stack_util.create_lvalue_symbol("s", struct_type);
    symbol_table.add(struct_symbol);

    WHEN("Constructing a write stack from &s.comp")
    {
      exprt in_expr=write_stack_util.to_expr("&s.comp", ns);
      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);
      const exprt &out_expr=stack.to_expression();
      THEN("Get a write stack for the address of member(comp, s)")
      {
        CAPTURE(expr2c(out_expr, ns));
        REQUIRE(out_expr.id()==ID_address_of);
        const exprt &object=out_expr.op0();
        REQUIRE(object.id()==ID_symbol);
        REQUIRE(object.get_string(ID_identifier)=="x");
      }
    }
  }

  GIVEN("A pointer to an integer")
  {
    // in a top environment - to construct from say (p + 1) we should always
    // return top. If the enviroment was not top, we could do better if p has an
    // offset at the top of its write_stack. Of course if it doesn't

    // int * p
    typet pointer_type=
      pointer_typet(signedbv_typet(32));
    symbolt array_symbol=
      write_stack_util.create_lvalue_symbol("p", pointer_type);
    symbol_table.add(array_symbol);

    WHEN("Constructing the write stack from p")
    {
      exprt in_expr=write_stack_util.to_expr("p", ns);
      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);

      THEN("Get a top write stack")
      {
        REQUIRE(stack.is_top_value());
      }
    }
    WHEN("Constructing the write stack from p + 1")
    {
      exprt in_expr=write_stack_util.to_expr("p + 1", ns);
      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);

      THEN("Get a top write stack")
      {
        REQUIRE(stack.is_top_value());
      }
    }
    WHEN("Constructing the write stack from 1 + p")
    {
      exprt in_expr=write_stack_util.to_expr("1 + p", ns);
      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);

      THEN("Get a top write stack")
      {
        REQUIRE(stack.is_top_value());
      }
    }
    WHEN("Constructing the write stack from p - 1")
    {
      exprt in_expr=write_stack_util.to_expr("p - 1", ns);
      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);

      THEN("Get a top write stack")
      {
        REQUIRE(stack.is_top_value());
      }
    }
    WHEN("Constructing the write stack from p[1]")
    {
      exprt in_expr=write_stack_util.to_expr("p[1]", ns);
      CAPTURE(expr2c(in_expr, ns));

      auto stack=continuation_stackt(in_expr, environment, ns);

      THEN("Get a top write stack")
      {
        REQUIRE(stack.is_top_value());
      }
    }
  }
}
