/*******************************************************************\

 Module: MODULE NAME

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/
#include "catch.hpp"

#include <stack>
#include <iostream>

#include <util/arith_tools.h>
#include <util/config.h>
#include <util/symbol.h>
#include <util/symbol_table.h>
#include <util/namespace.h>
#include <util/std_expr.h>
#include <util/std_code.h>
#include <util/ui_message.h>
#include <ansi-c/ansi_c_language.h>
#include <ansi-c/expr2c.h>
#include <analyses/variable-sensitivity/continuation_stack_entry.h>
#include <analyses/variable-sensitivity/abstract_enviroment.h>
#include <analyses/variable-sensitivity/variable_sensitivity_object_factory.h>


class fudged_to_exprt
{
public:
  fudged_to_exprt():
    message_handler(
      std::unique_ptr<message_handlert>(new ui_message_handlert()))
  {
    language.set_message_handler(*message_handler);
  }

  exprt operator()(const std::string &input_string, const namespacet &ns)
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


/// Verify a given exprt is an index_exprt with a a constant value equal to the
/// expected value
/// \param expr: The expression.
/// \param expected_index: The constant value that should be the index.
/// \return The expr cast to an index_exprt
index_exprt require_index(const exprt &expr, int expected_index)
{
  REQUIRE(expr.id()==ID_index);
  const index_exprt &index_expr=to_index_expr(expr);
  REQUIRE(index_expr.index().id()==ID_constant);
  const constant_exprt &index_value=to_constant_expr(index_expr.index());
  mp_integer index_integer_value;
  to_integer(index_value, index_integer_value);
  REQUIRE(index_integer_value==expected_index);

  return index_expr;
}

/// Verify a given exprt is an index_exprt with a nil value as its index
/// \param expr: The expression.
/// \return The expr cast to an index_exprt
index_exprt require_top_index(const exprt &expr)
{
  REQUIRE(expr.id()==ID_index);
  const index_exprt &index_expr=to_index_expr(expr);
  REQUIRE(index_expr.index().id()==ID_nil);
  return index_expr;
}

/// Verify a given exprt is an member_exprt with a component name equal to the
/// component_identifier
/// \param expr: The expression.
/// \param component_identifier: The name of the component that should be being
///   accessed.
/// \return The expr cast to a member_exprt.
member_exprt require_member(
  const exprt &expr, const irep_idt &component_identifier)
{
  REQUIRE(expr.id()==ID_member);
  const member_exprt &member_expr=to_member_expr(expr);
  REQUIRE(member_expr.get_component_name()==component_identifier);
  return member_expr;
}

/// Verify a given exprt is an symbol_exprt with a identifier name equal to the
/// symbol_name.
/// \param expr: The expression.
/// \param symbol_name: The intended identifier of the symbol
/// \return The expr cast to a symbol_exprt
symbol_exprt require_symbol(const exprt &expr, const irep_idt &symbol_name)
{
  REQUIRE(expr.id()==ID_symbol);
  const symbol_exprt &symbol_expr=to_symbol_expr(expr);
  REQUIRE(symbol_expr.get_identifier()==symbol_name);
  return symbol_expr;
}

/// Create a lvalue symbol of the specified type.
/// \param name: The name of the symbol.
/// \param type: The type of the symbol.
/// \return The symbol that has been created.
symbolt create_basic_symbol(const irep_idt &name, const typet &type)
{
  symbolt basic_symbol;
  basic_symbol.name=name;
  basic_symbol.base_name=name;
  basic_symbol.type=type;
  basic_symbol.is_lvalue=true;
  return basic_symbol;
}

SCENARIO("Constructing write stacks",
  "[core][analyses][variable-sensitivity][continuation-stack]")
{
  symbol_tablet symbol_table;
  namespacet ns(symbol_table);

  optionst options;
  options.set_option("pointers", true);
  options.set_option("arrays", true);
  options.set_option("structs", true);
  variable_sensitivity_object_factoryt::instance().set_options(options);

  config.set_arch("none");

  abstract_environmentt environment;
  environment.make_top();

  fudged_to_exprt to_expr;

  GIVEN("A int x")
  {
    typet basic_symbol_type=signedbv_typet(32);
    symbol_table.add(create_basic_symbol("x", basic_symbol_type));

    WHEN("Constructing from &x")
    {
      exprt in_expr=to_expr("&x", ns);
      CAPTURE(expr2c(in_expr, ns));

      THEN("The constructed stack should be &x")
      {
        auto stack=write_stackt(in_expr, environment, ns);
        const exprt &out_expr=stack.to_expression();

        CAPTURE(expr2c(out_expr, ns));

        REQUIRE(out_expr.id()==ID_address_of);
        const exprt &object=out_expr.op0();
        require_symbol(object, "x");
      }
    }
  }
  GIVEN("A int a[5]")
  {
    typet array_type=
      array_typet(signedbv_typet(32), constant_exprt::integer_constant(5));
    symbol_table.add(create_basic_symbol("a", array_type));

    WHEN("Constructing from a")
    {
      exprt in_expr=to_expr("a", ns);
      THEN("The constructed stack should be &a[0]")
      {
        auto stack=write_stackt(in_expr, environment, ns);
        const exprt &out_expr=stack.to_expression();

        CAPTURE(expr2c(out_expr, ns));

        REQUIRE(out_expr.id()==ID_address_of);
        const index_exprt &index_expr=require_index(out_expr.op0(), 0);
        require_symbol(index_expr.array(), "a");
      }
    }
    WHEN("Constructing from &a")
    {
      exprt in_expr=to_expr("&a", ns);

      THEN("The constructed stack should be &a")
      {
        auto stack=write_stackt(in_expr, environment, ns);
        const exprt &out_expr=stack.to_expression();

        CAPTURE(expr2c(out_expr, ns));

        // TODO: make consistent with above
        REQUIRE(out_expr.id()==ID_address_of);
        require_symbol(out_expr.op0(), "a");
      }
    }
    WHEN("Constructing from &a[0]")
    {
      exprt in_expr=to_expr("&a[0]", ns);

      CAPTURE(expr2c(in_expr, ns));
      THEN("The constructed stack should be &a[0]")
      {
        auto stack=write_stackt(in_expr, environment, ns);
        const exprt &out_expr=stack.to_expression();

        CAPTURE(expr2c(out_expr, ns));

        REQUIRE(out_expr.id()==ID_address_of);
        const exprt &object=out_expr.op0();
        const index_exprt &index_expr=require_index(object, 0);
        require_symbol(index_expr.array(), "a");
      }
    }
    WHEN("Constructing from &a[1]")
    {
      exprt in_expr=to_expr("&a[1]", ns);

      CAPTURE(expr2c(in_expr, ns));
      THEN("The constructed stack should be &a[1]")
      {
        auto stack=write_stackt(in_expr, environment, ns);
        const exprt &out_expr=stack.to_expression();

        CAPTURE(expr2c(out_expr, ns));

        REQUIRE(out_expr.id()==ID_address_of);
        const index_exprt &index_expr=require_index(out_expr.op0(), 1);
        require_symbol(index_expr.array(), "a");
      }
    }
    WHEN("Constructing from &a[0]+1")
    {
      exprt in_expr=to_expr("&a[0]+1", ns);

      CAPTURE(expr2c(in_expr, ns));
      THEN("The constructed stack should be &a[1]")
      {
        auto stack=write_stackt(in_expr, environment, ns);
        const exprt &out_expr=stack.to_expression();

        CAPTURE(expr2c(out_expr, ns));

        REQUIRE(out_expr.id()==ID_address_of);
        const index_exprt &index_expr=require_index(out_expr.op0(), 1);
        require_symbol(index_expr.array(), "a");
      }
    }
    WHEN("Constructing from &a[1]+1")
    {
      exprt in_expr=to_expr("&a[1]+1", ns);

      CAPTURE(expr2c(in_expr, ns));

      THEN("The constructed stack should be &a[2]")
      {
        auto stack=write_stackt(in_expr, environment, ns);
        const exprt &out_expr=stack.to_expression();

        CAPTURE(expr2c(out_expr, ns));

        REQUIRE(out_expr.id()==ID_address_of);
        const index_exprt &index_expr=require_index(out_expr.op0(), 2);
        require_symbol(index_expr.array(), "a");
      }
    }
    WHEN("Constructing from &a[1]-1")
    {
      exprt in_expr=to_expr("&a[1]-1", ns);

      CAPTURE(expr2c(in_expr, ns));

      THEN("The constructed stack should be &a[0]")
      {
        auto stack=write_stackt(in_expr, environment, ns);
        const exprt &out_expr=stack.to_expression();

        CAPTURE(expr2c(out_expr, ns));

        REQUIRE(out_expr.id()==ID_address_of);
        const index_exprt &index_expr=require_index(out_expr.op0(), 0);
        require_symbol(index_expr.array(), "a");
      }
    }
    WHEN("Constructing from 1+&a[1]")
    {
      exprt in_expr=to_expr("1+&a[1]", ns);

      CAPTURE(expr2c(in_expr, ns));

      THEN("The constructed stack should be &a[2]")
      {
        auto stack=write_stackt(in_expr, environment, ns);
        const exprt &out_expr=stack.to_expression();

        CAPTURE(expr2c(out_expr, ns));

        REQUIRE(out_expr.id()==ID_address_of);
        const index_exprt &index_expr=require_index(out_expr.op0(), 2);
        require_symbol(index_expr.array(), "a");
      }
    }
    GIVEN("A symbol int x")
    {
      typet basic_symbol_type=signedbv_typet(32);
      symbolt basic_symbol=create_basic_symbol("x", basic_symbol_type);
      symbol_table.add(basic_symbol);

      WHEN("Constructing from &a[x] (x top)")
      {
        exprt in_expr=to_expr("&a[x]", ns);

        CAPTURE(expr2c(in_expr, ns));
        THEN("The constructed stack should be &a[TOP]")
        {
          auto stack=write_stackt(in_expr, environment, ns);
          const exprt &out_expr=stack.to_expression();

          CAPTURE(expr2c(out_expr, ns));

          REQUIRE(out_expr.id()==ID_address_of);
          const index_exprt &index_expr=require_top_index(out_expr.op0());
          require_symbol(index_expr.array(), "a");
        }
      }
      WHEN("Constructing from &a[x] (x known to be 2")
      {
        // Create an abstract_object_pointer representing 2
        abstract_object_pointert x_value=
          environment.abstract_object_factory(
            basic_symbol_type,
            to_expr("2", ns),
            ns);
        environment.assign(basic_symbol.symbol_expr(), x_value, ns);

        exprt in_expr=to_expr("&a[x]", ns);

        CAPTURE(expr2c(in_expr, ns));
        THEN("The constructed stack should be &a[2]")
        {
          auto stack=write_stackt(in_expr, environment, ns);
          const exprt &out_expr=stack.to_expression();

          CAPTURE(expr2c(out_expr, ns));

          REQUIRE(out_expr.id()==ID_address_of);
          const index_exprt &index_expr=require_index(out_expr.op0(), 2);
          require_symbol(index_expr.array(), "a");
        }
      }
    }
  }

  GIVEN("A struct str{ int comp, int comp2 }")
  {
    struct_union_typet::componentt component("comp", signedbv_typet(32));
    struct_union_typet::componentt component2("comp2", signedbv_typet(32));
    struct_typet struct_type;
    struct_type.set_tag("str");
    struct_type.components().push_back(component);
    struct_type.components().push_back(component2);

    symbolt struct_symbol;
    struct_symbol.base_name="str";
    struct_symbol.name="tag-str";
    struct_symbol.type=struct_type;
    struct_symbol.is_type=true;

    symbol_table.add(struct_symbol);

    GIVEN("A struct str s")
    {
      symbol_table.add(create_basic_symbol("s", struct_type));

      WHEN("Constructing from &s.comp")
      {
        exprt in_expr=to_expr("&s.comp", ns);
        CAPTURE(expr2c(in_expr, ns));

        THEN("The constructed stack should be &s.comp")
        {
          auto stack=write_stackt(in_expr, environment, ns);
          const exprt &out_expr=stack.to_expression();

          CAPTURE(expr2c(out_expr, ns));

          REQUIRE(out_expr.id()==ID_address_of);
          const member_exprt &member_exrp=
            require_member(out_expr.op0(), "comp");
          // TODO: verify member expr
          require_symbol(member_exrp.compound(), "s");
        }
      }
      WHEN("Constructing from &s.comp2")
      {
        exprt in_expr=to_expr("&s.comp2", ns);
        CAPTURE(expr2c(in_expr, ns));

        THEN("The constructed stack should be &s.comp2")
        {
          auto stack=write_stackt(in_expr, environment, ns);
          const exprt &out_expr=stack.to_expression();

          CAPTURE(expr2c(out_expr, ns));

          REQUIRE(out_expr.id()==ID_address_of);
          const member_exprt &member_exrp=
            require_member(out_expr.op0(), "comp2");
          // TODO: verify member expr
          require_symbol(member_exrp.compound(), "s");
        }
      }
      WHEN("Constructing from &s")
      {
        // fiddly as it depends on the type to some degree
        // if we want to make a (struct str *) then we want to make a pointer to
        // &s
        // If it is a pointer to the type of the first component then we really
        // want &(s.comp)
        // for now we just reject and create a junk stack
        exprt in_expr=to_expr("&s", ns);
        CAPTURE(expr2c(in_expr, ns));

        THEN("Then should get a top stack")
        {
          auto stack=write_stackt(in_expr, environment, ns);
          REQUIRE(stack.is_top_value());
        }
      }
      WHEN("Constructing from &s.comp + 1")
      {
        // TODO: we could in theory analyse the struct and offset the pointer
        // but not yet
        exprt in_expr=to_expr("&s.comp + 1", ns);
        CAPTURE(expr2c(in_expr, ns));

        THEN("Then should get a top stack")
        {
          auto stack=write_stackt(in_expr, environment, ns);
          REQUIRE(stack.is_top_value());
        }
      }
    }
    GIVEN("struct str arr_s[5]")
    {
      typet array_type=
        array_typet(struct_type, constant_exprt::integer_constant(5));
      symbol_table.add(create_basic_symbol("arr_s", array_type));

      WHEN("&arr_s[1].comp")
      {
        exprt in_expr=to_expr("&arr_s[1].comp", ns);
        CAPTURE(expr2c(in_expr, ns));

        THEN("The constructed stack should be &arr_s[1].comp")
        {
          auto stack=write_stackt(in_expr, environment, ns);
          const exprt &out_expr=stack.to_expression();

          CAPTURE(expr2c(out_expr, ns));

          REQUIRE(out_expr.id()==ID_address_of);

          const member_exprt &member_expr=require_member(out_expr.op0(), "comp");
          const index_exprt &index_expr=
            require_index(member_expr.compound(), 1);

          require_symbol(index_expr.array(), "arr_s");
        }
      }
      WHEN("&arr_s[1]")
      {
        exprt in_expr=to_expr("&arr_s[1]", ns);
        CAPTURE(expr2c(in_expr, ns));

        THEN("The constructed stack should be TOP")
        {
          // Since we don't allow constructing a pointer to a struct yet
          auto stack=write_stackt(in_expr, environment, ns);
          REQUIRE(stack.is_top_value());
        }
      }
      GIVEN("A symbol int x")
      {
        typet basic_symbol_type=signedbv_typet(32);
        symbol_table.add(create_basic_symbol("x", basic_symbol_type));

        WHEN("Constructing from &arr_s[x].comp (x top)")
        {
          exprt in_expr=to_expr("&arr_s[x].comp", ns);

          CAPTURE(expr2c(in_expr, ns));
          THEN("The constructed stack should be &arr_s[TOP].comp")
          {
            auto stack=write_stackt(in_expr, environment, ns);
            const exprt &out_expr=stack.to_expression();

            CAPTURE(expr2c(out_expr, ns));

            REQUIRE(out_expr.id()==ID_address_of);
            const member_exprt &member_expr=
              require_member(out_expr.op0(), "comp");

            const index_exprt &index_expr=
              require_top_index(member_expr.compound());

            require_symbol(index_expr.array(), "arr_s");
          }
        }
      }
    }
  }
}
