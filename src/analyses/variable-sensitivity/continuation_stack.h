/*******************************************************************\

 Module: MODULE NAME

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/
#ifndef CONTINUATION_STACK_H
#define CONTINUATION_STACK_H

#include <analyses/variable-sensitivity/continuation_stack_entry.h>

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
class write_stackt
{
public:
  typedef std::shared_ptr<continuation_stack_entryt> stack_entry_pointert;
  typedef std::vector<stack_entry_pointert> continuation_stack_storet;

  write_stackt();

  write_stackt(const exprt & expr,
    const abstract_environmentt &environment,
    const namespacet &ns);

  exprt to_expression() const;
  bool is_top_value() const;

  void append_stack(const continuation_stack_storet &stack_to_append);

private:
  void construct_stack_to_pointer(
    const exprt &expr,
    const abstract_environmentt &environment,
    const namespacet &ns);

  void construct_stack_to_lvalue(
    const exprt &expr,
    const abstract_environmentt &environment,
    const namespacet &ns);

  void construct_stack_to_array_index(
    const index_exprt &expr,
    const abstract_environmentt &environment,
    const namespacet &ns);

  void add_to_stack(
    stack_entry_pointert entry_pointer,
    const abstract_environmentt environment,
    const namespacet &ns);

  enum class integral_resultt { LHS_INTEGRAL, RHS_INTEGRAL, NEITHER_INTEGRAL };

  static integral_resultt get_which_side_integral(
    const exprt &expr,
    exprt &out_base_expr,
    exprt &out_integral_expr);


  continuation_stack_storet stack;
  bool junk_stack;
};

#endif // CONTINUATION_STACK_H
