#include <unordered_set>


#include <util/std_expr.h>

#include "write_stack_entry.h"

bool continuation_stack_entryt::try_squash_in(
  std::shared_ptr<const continuation_stack_entryt> new_entry,
  const abstract_environmentt &enviroment,
  const namespacet &ns)
{
  return false;
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

bool offset_entryt::try_squash_in(
  std::shared_ptr<const continuation_stack_entryt> new_entry,
  const abstract_environmentt &enviroment,
  const namespacet &ns)
{
  std::shared_ptr<const offset_entryt> cast_entry=
    std::dynamic_pointer_cast<const offset_entryt>(new_entry);
  if(cast_entry)
  {
    exprt result_offset=plus_exprt(cast_entry->offset->to_constant(), offset->to_constant());
    offset=enviroment.eval(result_offset, ns);
    return true;
  }
  return false;
}

