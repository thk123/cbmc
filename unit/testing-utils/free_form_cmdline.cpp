/*******************************************************************\

 Module: Unit test utilities

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/

#include "free_form_cmdline.h"

void free_form_cmdlinet::create_flag(const std::string &flag_name)
{
  optiont option;
  option.optstring = flag_name;
  options.push_back(option);
}

void free_form_cmdlinet::add_flag(std::string flag)
{
  create_flag(flag);
  set(flag);
}

void free_form_cmdlinet::add_option(std::string flag, std::string value)
{
  create_flag(flag);
  set(flag, value);
}
