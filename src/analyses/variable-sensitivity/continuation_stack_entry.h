/*******************************************************************\

 Module: MODULE NAME

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
  virtual exprt get_access_expr() const=0;



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
private:
  abstract_object_pointert offset;
};

// So each pointer has effectively an offset stack stored as a continuation stack
//  Conceptually: writing to the pointer involves continuing writing to some other
//  location.
// Assign builds a continuation_stack off the LHS (currently builds a stack, but indexes
// would be stored as index_entries
// Read dereference of a pointer can now just turn this stack into an expression that can
// be evaluated like the normal .object() for address_of pointers, this will then just work
// for pointers to objects inside structs etc

// WHY USE A STACK AND NOT JUST THE EXPRESSION SYNTAX TREE?

// Write dereference works in the same way, with the pointer adding on to the continuation stack
// At the top?

class continuation_stackt
{
public:
  typedef std::vector<std::shared_ptr<continuation_stack_entryt>> continuation_stack_storet;

  continuation_stackt();

  continuation_stackt(
    const exprt & expr,
    const abstract_environmentt &enviroment,
    const namespacet &ns);

  exprt to_expression() const;
  bool is_top_value() const;

  void append_stack(const continuation_stack_storet &stack_to_append);

private:
  friend class test_continuation_stackt;

  continuation_stack_storet stack;
  bool junk_stack;
};

#endif // CONTINUATION_STACK_ENTRY_H

