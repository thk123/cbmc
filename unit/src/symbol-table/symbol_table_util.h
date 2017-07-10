/*******************************************************************\

 Module: MODULE NAME

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/
#ifndef CPROVER_SRC_SYMBOL_TABLE_SYMBOL_TABLE_UTIL_H
#define CPROVER_SRC_SYMBOL_TABLE_SYMBOL_TABLE_UTIL_H

#include <util/symbol.h>

class symbol_table_utilt
{
public:
  static symbolt create_basic_symbol(const irep_idt &name, const typet &type);
};

#endif // CPROVER_SRC_SYMBOL_TABLE_SYMBOL_TABLE_UTIL_H
