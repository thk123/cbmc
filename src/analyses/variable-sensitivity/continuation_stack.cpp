/*******************************************************************\

 Module: MODULE NAME

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/
#include <unordered_set>

#include <util/std_expr.h>

#include "continuation_stack.h"

continuation_stackt::continuation_stackt():
  stack(),
  junk_stack(true)
{}

continuation_stackt::continuation_stackt(
  const exprt &expr, const abstract_environmentt &enviroment, const namespacet &ns)
{
  PRECONDITION(expr.type().id()==ID_pointer);
  exprt s=expr;

  junk_stack=false;

  if(s.id()==ID_address_of)
  {
    // resovle reminder, can either be a symbol, member or index of
    // otherwise unsupported
  }
  else if(s.id()==ID_plus)
  {
    // exactly one of the lhs or rhs need to resolve to a write stack
    // and the other to an offset
    // otherwise unsupported
  }
  else if(s.id()==ID_minus)
  {
    // the lhs must resolve to a write stack
    // the rhs must resovle to an offset
    // otherwise not supported
  }
  // I think at top level member expression is invalid (here we should just
  // have copied the whole stack as the RHS is also a pointer value

  // dereference could be in an offset (i.e. unrelated pointer to int and we
  // use the value of the int to offset) so not valid at top level
  // or could be a dereference that later gets turned back into an address
  // In either case don't think it can be the head element


  while (s.id()!=ID_symbol)
  {
    if(s.id()==ID_address_of)
    {
      // We are the root address
      if(stack.empty())
      {
        s=s.op0();
      }
      else
      {
        // Here we are assuming we are taking an address of an index expression - why is this implied by a non-empty stack?
        // for stack to be not empty we need to have had another instruction
        // what about &a[&a - &a]? - we probably don't need to support this
        exprt offset=to_index_expr(s.op0()).index();
        abstract_object_pointert offset_value=enviroment.eval(offset, ns);
        add_to_stack(std::make_shared<offset_entryt>(offset_value), enviroment, ns);
        s=s.op0().op0();
      }

    }
    else if(s.id()==ID_member || s.id()==ID_dereference)
    {
      add_to_stack(std::make_shared<simple_entryt>(s), enviroment, ns);
      s=s.op0();
    }
    else if(s.id()==ID_index || s.id()==ID_plus || s.id()==ID_minus)
    {
      const static std::unordered_set<irep_idt, irep_id_hash> integral_types=
        { ID_signedbv, ID_unsignedbv, ID_integer };

      exprt offset;
      if(integral_types.find(s.op0().type().id())!=integral_types.cend())
      {
        // If the constant is first, we can't subtract something from that and
        // get a reasonable pointer
        assert(s.id()!=ID_minus);
        offset=s.op0();
        s=s.op1();
      }
      else
      {
        assert(integral_types.find(s.op1().type().id())!=integral_types.cend());
        offset=s.op1();
        if(s.id()==ID_minus)
        {
          offset.negate();
        }
        s=s.op0();
      }

      abstract_object_pointert offset_value=enviroment.eval(offset, ns);

      add_to_stack(std::make_shared<offset_entryt>(offset_value), enviroment, ns);
    }
    else
    {
      // Empty the stack as we weren't able to build a sensible stack
      junk_stack=true;
      break;
    }
  }

  if(!junk_stack)
  {
    add_to_stack(std::make_shared<simple_entryt>(s), enviroment, ns);
  }
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
