/*******************************************************************\

 Module: MODULE NAME

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/
#include "continuation_stack_entry.h"

#include <util/std_expr.h>


continuation_stackt::continuation_stackt():
  stack(),
  junk_stack(true)
{}

continuation_stackt::continuation_stackt(
  const exprt &expr, const abstract_environmentt &enviroment, const namespacet &ns)
{
  exprt s=expr;
  junk_stack=false;
  while (s.id() != ID_symbol)
  {
    if(s.id()==ID_address_of)
    {
      assert(stack.empty());
      stack.push_back(std::make_shared<simple_entryt>(s));
    }
    else if(s.id()==ID_member || s.id()==ID_dereference)
    {
      stack.push_back(std::make_shared<simple_entryt>(s));
    }
    else if(s.id()==ID_index || s.id()==ID_plus || s.id()==ID_minus)
    {
      exprt offset;
      if(s.op0().type().id()==ID_signedbv||s.op0().type().id()==ID_unsignedbv)
      {
        offset=s.op0();
      }
      else
      {
        assert(s.op1().type().id()==ID_integer|| s.op1().type().id()==ID_signedbv || s.op1().type().id()==ID_unsignedbv);
        offset=s.op1();
      }

      abstract_object_pointert offset_value=enviroment.eval(offset, ns);
      stack.push_back(std::make_shared<offset_entryt>(offset_value));
    }
    else
    {
      // Empty the stack as we weren't able to build a sensible stack
      junk_stack=true;
      break;
    }
    s=s.op0();
  }
  //assert(s.id()==ID_symbol);
  if(!junk_stack)
  {
    stack.push_back(std::make_shared<simple_entryt>(s));
  }
}

exprt continuation_stackt::to_expression() const
{
  exprt access_expr=nil_exprt();
  for(const std::shared_ptr<continuation_stack_entryt> &entry : stack)
  {
    if(access_expr.id()==ID_nil)
    {
      access_expr=entry->get_access_expr();
    }
    else
    {
      access_expr.op0()=entry->get_access_expr();
    }
  }

  return access_expr;
}

bool continuation_stackt::is_top_value() const
{
  return junk_stack;
}

simple_entryt::simple_entryt(exprt expr):
simple_entry(expr)
{}

exprt simple_entryt::get_access_expr() const
{
  return simple_entry;
}



offset_entryt::offset_entryt(abstract_object_pointert offset_value):
offset(offset_value)
{}

exprt offset_entryt::get_access_expr() const
{
  return index_exprt(exprt(), offset->to_constant());
}
