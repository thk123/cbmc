// Copyright 2016-2020 Diffblue Limited. All Rights Reserved.

/// \file
/// C String Refinement

#ifndef CPROVER_GOTO_PROGRAMS_C_STRING_REFINEMENT_H
#define CPROVER_GOTO_PROGRAMS_C_STRING_REFINEMENT_H

#include "goto_functions.h"

class message_handlert;
class goto_modelt;

void c_string_refinement(
  goto_modelt &goto_model,
  message_handlert &message_handler,
  const std::string &max_nondet_string_length);

#endif // CPROVER_GOTO_PROGRAMS_C_STRING_REFINEMENT_H
