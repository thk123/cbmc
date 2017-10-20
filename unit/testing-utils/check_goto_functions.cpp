/*******************************************************************\

 Module: Unit test utilities

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/

#include "check_goto_functions.h"

#include <util/std_expr.h>

/// Combine two pointer locations. Takes the null assignment if present in
/// either (fails if present in both) and appends the non-null assignments
/// \param a: The first set of assignments
/// \param b: The second set of assignments
/// \return The resulting assignment
pointer_assignment_locationt combine_locations(
  const pointer_assignment_locationt &a,
  const pointer_assignment_locationt &b)
{
  pointer_assignment_locationt new_locations;
  if(a.null_assignment.has_value())
  {
    INVARIANT(
      !b.null_assignment.has_value(), "Only expected one assignment to null");
    new_locations.null_assignment = a.null_assignment;
  }
  else if(b.null_assignment.has_value())
  {
    INVARIANT(
      !a.null_assignment.has_value(), "Only expected one assignment to null");
    new_locations.null_assignment = b.null_assignment;
  }

  new_locations.non_null_assignments.insert(
    new_locations.non_null_assignments.end(),
    a.non_null_assignments.begin(),
    a.non_null_assignments.end());

  new_locations.non_null_assignments.insert(
    new_locations.non_null_assignments.end(),
    b.non_null_assignments.begin(),
    b.non_null_assignments.end());

  return new_locations;
}

/// For a given variable name, gets the assignments to it in the functions
/// \param pointer_name: The name of the variable
/// \param instructions: The instructions to look through
/// \return: A structure that contains the null assignment if found, and a
/// vector of all other assignments
pointer_assignment_locationt find_pointer_assignments(
  const irep_idt &pointer_name,
  const exprt::operandst &instructions)
{
  pointer_assignment_locationt locations;
  for(const exprt &assignment : instructions)
  {
    const codet &statement = to_code(assignment);
    INVARIANT(assignment.id() == ID_code, "All instructions must be code");
    if(statement.get_statement() == ID_assign)
    {
      const code_assignt &code_assign = to_code_assign(to_code(assignment));
      if(
        code_assign.lhs().id() == ID_symbol &&
        to_symbol_expr(code_assign.lhs()).get_identifier() == pointer_name)
      {
        if(
          code_assign.rhs() ==
          null_pointer_exprt(to_pointer_type(code_assign.lhs().type())))
        {
          locations.null_assignment = code_assign;
        }
        else
        {
          locations.non_null_assignments.push_back(code_assign);
        }
      }
    }
    else if(statement.get_statement() == ID_block)
    {
      const auto &assignments_in_block =
        find_pointer_assignments(pointer_name, assignment.operands());
      locations = combine_locations(locations, assignments_in_block);
    }
    else if(statement.get_statement() == ID_ifthenelse)
    {
      const code_ifthenelset &if_else_block =
        to_code_ifthenelse(to_code(assignment));
      const auto &assignments_in_block = find_pointer_assignments(
        pointer_name, exprt::operandst{if_else_block.then_case()});
      locations = combine_locations(locations, assignments_in_block);

      if(if_else_block.has_else_case())
      {
        const auto &assignments_in_else = find_pointer_assignments(
          pointer_name, exprt::operandst{if_else_block.else_case()});
        locations = combine_locations(locations, assignments_in_else);
      }
    }
    else if(statement.get_statement() == ID_label)
    {
      const code_labelt &label_statement = to_code_label(statement);
      const auto &assignments_in_label = find_pointer_assignments(
        pointer_name, exprt::operandst{label_statement.code()});
      locations = combine_locations(locations, assignments_in_label);
    }
  }

  return locations;
}
