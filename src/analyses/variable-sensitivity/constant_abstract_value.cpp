/*******************************************************************\

 Module: analyses variable-sensitivity

 Author: Thomas Kiley, thomas.kiley@diffblue.com

\*******************************************************************/
#include "constant_abstract_value.h"

#include <util/std_expr.h>

constant_abstract_valuet::constant_abstract_valuet(typet t):
  abstract_valuet(t), value()
{}

constant_abstract_valuet::constant_abstract_valuet(typet t, bool tp, bool bttm):
  abstract_valuet(t, tp, bttm), value()
{}

constant_abstract_valuet::constant_abstract_valuet(
  const constant_abstract_valuet &old):
    abstract_valuet(old), value(old.value)
{}

constant_abstract_valuet::constant_abstract_valuet(const exprt e):
  abstract_valuet(e), value(e)
{
  top=false;
}

exprt constant_abstract_valuet::to_constant() const
{
  if(!top && !bottom)
  {
    return this->value;
  }
  else
  {
    return abstract_objectt::to_constant();
  }
}

void constant_abstract_valuet::output(
  std::ostream &out, const ai_baset &ai, const namespacet &ns)
{
  if(!top && !bottom)
  {
    out << to_constant_expr(value).get_value();
  }
  else
  {
    abstract_objectt::output(out, ai, ns);
  }
}

bool constant_abstract_valuet::merge_state(
  constant_abstract_value_pointert op1,
  constant_abstract_value_pointert op2)
{
  bool parent_merge_change=abstract_objectt::merge_state(op1, op2);
  if (!top && !bottom)
  {
    if (op1->value==op2->value)
    {
      value=op1->value;
      return false;
    }
    else // values different
    {
      top=true;
      assert(bottom==false);
      // Clear out the expression
      value=exprt();
      return !op1->top;
    }
  }
  else // either top or bottom
  {
    return parent_merge_change;
  }
}