/*******************************************************************\

 Module: MODULE NAME

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/
#include "symbol_table_util.h"

/// Create a lvalue symbol of the specified type.
/// \param name: The name of the symbol.
/// \param type: The type of the symbol.
/// \return The symbol that has been created.
symbolt symbol_table_utilt::create_basic_symbol(
  const irep_idt &name, const typet &type)
{
  symbolt basic_symbol;
  basic_symbol.name=name;
  basic_symbol.base_name=name;
  basic_symbol.type=type;
  basic_symbol.is_lvalue=true;
  return basic_symbol;
}
