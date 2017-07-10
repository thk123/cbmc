/*******************************************************************\

 Module: MODULE NAME

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/
#include <unordered_set>

#include <util/std_expr.h>
#include <util/simplify_expr.h>

#include "continuation_stack.h"

continuation_stackt::continuation_stackt():
  stack(),
  junk_stack(true)
{}

continuation_stackt::continuation_stackt(
  const exprt &expr,
  const abstract_environmentt &enviroment,
  const namespacet &ns)
{
  junk_stack=false;
  if(expr.type().id()==ID_array)
  {
    // We are assigning an array to a pointer, which is equivalent to assigning
    // the first element of that arary
    // &(expr)[0]
    construct_stack_to_pointer(
      address_of_exprt(
        index_exprt(
          expr, constant_exprt::integer_constant(0))), enviroment, ns);
  }
  else
  {
    construct_stack_to_pointer(expr, enviroment, ns);
  }
}

void continuation_stackt::construct_stack_to_pointer(
  const exprt &expr,
  const abstract_environmentt &enviroment,
  const namespacet &ns)
{
  PRECONDITION(expr.type().id()==ID_pointer);

  // If we are a pointer to a struct, we do not currently support reading
  // writing directly to it so just create a top stack
  if(ns.follow(expr.type().subtype()).id()==ID_struct)
  {
    junk_stack=true;
    return;
  }

  if(expr.id()==ID_address_of)
  {
    // resovle reminder, can either be a symbol, member or index of
    // otherwise unsupported
    construct_stack_to_lvalue(expr.op0(), enviroment, ns);
  }
  else if(expr.id()==ID_plus || expr.id()==ID_minus)
  {
    exprt base;
    exprt offset;
    const integral_resultt &which_side=
      get_which_side_integral(expr, base, offset);
    INVARIANT(
      which_side!=integral_resultt::NEITHER_INTEGRAL,
      "An offset must be an integral amount");

    if(expr.id()==ID_minus)
    {
      // can't get a valid pointer by subtracting from a constant number
      if(which_side==integral_resultt::LHS_INTEGRAL)
      {
        junk_stack=true;
        return;
      }
      offset.negate();
    }

    abstract_object_pointert offset_value=enviroment.eval(offset, ns);

    add_to_stack(std::make_shared<offset_entryt>(offset_value), enviroment, ns);

    // Build the pointer part
    construct_stack_to_pointer(base, enviroment, ns);

    if(!junk_stack)
    {
      // check the top of the stack is pointing at an array - we don't support
      // offset apart from within arrays
      std::shared_ptr<const continuation_stack_entryt> entry=
        *std::prev(stack.cend());
      if(entry->get_access_expr().type().id()!=ID_array)
      {
        junk_stack=true;
      }
    }
  }
  else
  {
    // unknown expression type - play it safe and set to top
    junk_stack=true;
  }
}

void continuation_stackt::construct_stack_to_lvalue(
  const exprt &expr,
  const abstract_environmentt &enviroment,
  const namespacet &ns)
{
  if(!junk_stack)
  {
    if(expr.id()==ID_member)
    {
      add_to_stack(std::make_shared<simple_entryt>(expr), enviroment, ns);
      construct_stack_to_lvalue(expr.op0(), enviroment, ns);
    }
    else if(expr.id()==ID_symbol)
    {
      add_to_stack(std::make_shared<simple_entryt>(expr), enviroment, ns);
    }
    else if(expr.id()==ID_index)
    {
      construct_stack_to_array_index(to_index_expr(expr), enviroment, ns);
    }
    else
    {
      junk_stack=true;
    }
  }
}

void continuation_stackt::construct_stack_to_array_index(
  const index_exprt &index_expr,
  const abstract_environmentt &enviroment,
  const namespacet &ns)
{
  abstract_object_pointert offset_value=
    enviroment.eval(index_expr.index(), ns);

  add_to_stack(std::make_shared<offset_entryt>(offset_value), enviroment, ns);
  construct_stack_to_lvalue(index_expr.array(), enviroment, ns);
}

exprt continuation_stackt::to_expression() const
{
  exprt access_expr=nil_exprt();
  for(const stack_entry_pointert &entry : stack)
  {
    exprt new_expr=entry->get_access_expr();
    if(access_expr.id()==ID_nil)
    {
      access_expr=new_expr;
    }
    else
    {
      if(new_expr.operands().size()==0)
      {
        new_expr.operands().resize(1);
      }
      new_expr.op0()=access_expr;

      access_expr=new_expr;
    }
  }
  address_of_exprt top_expr(access_expr);
  return top_expr;
}

bool continuation_stackt::is_top_value() const
{
  return junk_stack;
}



void continuation_stackt::add_to_stack(
  continuation_stackt::stack_entry_pointert entry_pointer,
  const abstract_environmentt enviroment,
  const namespacet &ns)
{
  if(stack.empty() || !stack.front()->try_squash_in(entry_pointer, enviroment, ns))
  {
    stack.insert(stack.begin(), entry_pointer);
  }
}

continuation_stackt::integral_resultt
  continuation_stackt::get_which_side_integral(
  const exprt &expr,
  exprt &out_base_expr,
  exprt &out_integral_expr)
{
  PRECONDITION(expr.operands().size()==2);
  const static std::unordered_set<irep_idt, irep_id_hash> integral_types=
    { ID_signedbv, ID_unsignedbv, ID_integer };
  if(integral_types.find(expr.op0().type().id())!=integral_types.cend())
  {
    out_integral_expr=expr.op0();
    out_base_expr=expr.op1();
    return integral_resultt::LHS_INTEGRAL;
  }
  else if(integral_types.find(expr.op1().type().id())!=integral_types.cend())
  {
    out_integral_expr=expr.op1();
    out_base_expr=expr.op0();
    return integral_resultt::RHS_INTEGRAL;
  }
  else
  {
    out_integral_expr.make_nil();
    out_base_expr.make_nil();
    return integral_resultt::NEITHER_INTEGRAL;
  }
}
