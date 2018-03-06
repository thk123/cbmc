/*******************************************************************\

 Module: Unit test utilities

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/

#ifndef CPROVER_TESTING_UTILS_FREE_FORM_CMDLINE_H
#define CPROVER_TESTING_UTILS_FREE_FORM_CMDLINE_H

#include <util/cmdline.h>

class free_form_cmdlinet : public cmdlinet
{
public:
  void add_flag(std::string flag);
  void add_option(std::string flag, std::string value);
private:
  void create_flag(const std::string &flag_name);
};



#endif //CPROVER_TESTING_UTILS_FREE_FORM_CMDLINE_H
