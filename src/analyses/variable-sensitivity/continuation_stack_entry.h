/*******************************************************************\

 Module: Analyses Variable Sensitivity

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/
#ifndef CONTINUATION_STACK_ENTRY_H
#define CONTINUATION_STACK_ENTRY_H

#include <memory>
#include <stack>

#include <util/std_expr.h>
#include <analyses/variable-sensitivity/abstract_enviroment.h>
#include <util/namespace.h>


class continuation_stack_entryt
{
public:
  virtual ~continuation_stack_entryt() = default;
  virtual exprt get_access_expr() const=0;
  virtual bool try_squash_in(
    std::shared_ptr<const continuation_stack_entryt> new_entry,
    const abstract_environmentt &enviroment,
    const namespacet &ns);
};

class simple_entryt:public continuation_stack_entryt
{
public:
  simple_entryt(exprt expr);
  virtual exprt get_access_expr() const override;
private:
  exprt simple_entry;
};

class offset_entryt:public continuation_stack_entryt
{
public:
  offset_entryt(abstract_object_pointert offset_value);
  virtual exprt get_access_expr() const override;
  virtual bool try_squash_in(
    std::shared_ptr<const continuation_stack_entryt> new_entry,
    const abstract_environmentt &enviroment,
    const namespacet &ns) override;
private:
  abstract_object_pointert offset;
};

#endif // CONTINUATION_STACK_ENTRY_H

