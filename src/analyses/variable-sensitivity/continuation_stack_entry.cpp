#include <unordered_set>

#include "continuation_stack_entry.h"

#include <util/std_expr.h>
#include <pointer-analysis/dereference.h>


continuation_stackt::continuation_stackt():
  stack(),
  junk_stack(true)
{}

continuation_stackt::continuation_stackt(
  const exprt &expr, const abstract_environmentt &enviroment, const namespacet &ns)
{
  exprt s=expr;

  junk_stack=false;
  while (s.id()!=ID_symbol)
  {
    if(s.id()==ID_address_of)
    {
      // We are the root address
      if(stack.empty())
      {
        //add_to_stack(std::make_shared<simple_entryt>(s));
        s=s.op0();
      }
      else
      {
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
  //assert(s.id()==ID_symbol);
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

//  exprt *current_postion=&access_expr;
//  for(const std::shared_ptr<continuation_stack_entryt> &entry : stack)
//  {
//    *current_postion=entry->get_access_expr();


//  }

//  exprt access_expr=nil_exprt();
//  exprt *current_position=&access_expr;
//  for(const stack_entry_pointert &entry : stack)
//  {
//    exprt new_expr=entry->get_access_expr();
//    *current_position=new_expr;
//    current_position=&(current_position->op0());
//  }

//  return access_expr;
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
  //stack.push_back(entry_pointer);
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

bool continuation_stack_entryt::try_squash_in(
  std::shared_ptr<const continuation_stack_entryt> new_entry,
  const abstract_environmentt &enviroment,
  const namespacet &ns)
{
  return false;
}
