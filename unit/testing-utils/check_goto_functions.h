/*******************************************************************\

 Module: Unit test utilities

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/

/// \file
/// Utilties for inspecting goto functions

#include <util/irep.h>
#include <util/expr.h>
#include <util/optional.h>
#include <util/std_code.h>

#ifndef CPROVER_TESTING_UTILS_CHECK_GOTO_FUNCTIONS_H
#define CPROVER_TESTING_UTILS_CHECK_GOTO_FUNCTIONS_H

struct pointer_assignment_locationt
{
  optionalt<code_assignt> null_assignment;
  std::vector<code_assignt> non_null_assignments;
};

pointer_assignment_locationt find_pointer_assignments(
  const irep_idt &pointer_name,
  const exprt::operandst &instructions);

#endif // CPROVER_TESTING_UTILS_CHECK_GOTO_FUNCTIONS_H
