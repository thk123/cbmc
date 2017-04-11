/*******************************************************************\

 Module: MODULE NAME

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/
#include "continuation_stack_entry.h"

#include <util/std_expr.h>


continuation_stackt::continuation_stackt(
  const exprt &expr, const abstract_environmentt &enviroment, const namespacet &ns)
{
  exprt s=expr;
  while (s.id() != ID_symbol)
  {
    if(s.id()==ID_member || s.id()==ID_dereference)
    {
      stack.push(std::unique_ptr<continuation_stack_entryt>(new simple_entryt(s)));
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
      stack.push(std::unique_ptr<continuation_stack_entryt>(new offset_entryt(offset_value)));
    }
    else
    {
      break;
    }
    s=s.op0();
  }
  assert(s.id()==ID_symbol);
  stack.push(std::unique_ptr<continuation_stack_entryt>(new simple_entryt(s)));
}

simple_entryt::simple_entryt(exprt expr):
  simple_entry(expr)
{}



offset_entryt::offset_entryt(abstract_object_pointert offset_value):
  offset(offset_value)
{}
