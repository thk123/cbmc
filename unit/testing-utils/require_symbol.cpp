/*******************************************************************\

 Module: Unit test utilities

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/


#include "require_symbol.h"
#include "catch.hpp"

const symbolt &require_symbol::require_symbol(
  const symbol_tablet &symbol_table,
  const irep_idt &symbol_identifier)
{
  const symbolt *found_symbol=symbol_table.lookup(symbol_identifier);
  INFO("Looking for symbol: " + id2string(symbol_identifier));
  REQUIRE(found_symbol != nullptr);
  return *found_symbol;
}
