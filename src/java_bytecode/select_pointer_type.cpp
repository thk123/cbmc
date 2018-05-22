/*******************************************************************\

 Module: Java Bytecode Language Conversion

 Author: Diffblue Ltd.

\*******************************************************************/

/// \file
/// Handle selection of correct pointer type (for example changing abstract
/// classes to concrete versions).

#include "select_pointer_type.h"
#include "java_types.h"
#include <util/std_types.h>

/// Select what type should be used for a given pointer type. For the base class
/// we just use the supplied type. Derived classes can override this behaviour
/// to provide more sophisticated type selection. Generic parameters are
/// replaced with their concrete type.
/// \param pointer_type: The pointer type replace
/// \param generic_parameter_specialization_map map of types for all generic
/// parameters in the current scope
/// \return A pointer type where the subtype may have been modified
pointer_typet select_pointer_typet::convert_pointer_type(
  const pointer_typet &pointer_type,
  const generic_parameter_specialization_mapt
    &generic_parameter_specialization_map,
  const namespacet &ns) const
{
  return specialize_generics(
    pointer_type, generic_parameter_specialization_map);
}

/// Specialize generic parameters in a pointer type based on the current map
/// of parameters -> types. We specialize generics if the pointer is a java
/// generic parameter or an array with generic parameters (java generic types
/// are specialized recursively, their concrete types are already stored in
/// the map and will be retrieved when needed e.g., to initialize fields).
/// Example:
/// - generic type: T
/// - map: T -> U; U -> String
/// - result: String
///
/// - generic type: T[]
/// - map: T -> U; U -> String
/// - result: String
/// \param pointer_type pointer to be specialized
/// \param generic_parameter_specialization_map map of types for all generic
/// parameters in the current scope
/// \return pointer type where generic parameters are replaced with concrete
/// types, if set in the current scope
pointer_typet select_pointer_typet::specialize_generics(
  const pointer_typet &pointer_type,
  const generic_parameter_specialization_mapt
    &generic_parameter_specialization_map) const
{
  if(is_java_generic_parameter(pointer_type))
  {
    const java_generic_parametert &parameter =
      to_java_generic_parameter(pointer_type);
    const irep_idt &parameter_name = parameter.get_name();
    optionalt<reference_typet> parameter_value =
      generic_parameter_specialization_map.get_reference_type(parameter_name);
    if(parameter_value)
      return *parameter_value;
    else
      return pointer_type;
  }
  else if(pointer_type.subtype().id() == ID_symbol)
  {
    // if the pointer is an array, recursively specialize its element type
    const symbol_typet &array_subtype = to_symbol_type(pointer_type.subtype());
    if(is_java_array_tag(array_subtype.get_identifier()))
    {
      const typet &array_element_type = java_array_element_type(array_subtype);
      if(array_element_type.id() == ID_pointer)
      {
        const pointer_typet &new_array_type = specialize_generics(
          to_pointer_type(array_element_type),
          generic_parameter_specialization_map);

        pointer_typet replacement_array_type = java_array_type('a');
        replacement_array_type.subtype().set(ID_C_element_type, new_array_type);
        return replacement_array_type;
      }
    }
  }
  return pointer_type;
}

optionalt<reference_typet>
generic_parameter_specialization_mapt::get_reference_type(const irep_idt &parameter_name) const
{
  if(underlying_map.count(parameter_name) == 0)
  {
    // this means that the generic pointer_type has not been specialized
    // in the current context (e.g., the method under test is generic);
    // we return the pointer_type itself which is basically a pointer to
    // its upper bound
    return {};
  }
  const reference_typet &type =
    underlying_map.find(parameter_name)->second.top();

  // generic parameters can be adopted from outer classes or superclasses so
  // we may need to search for the concrete type recursively
  return is_java_generic_parameter(type)
         ? get_reference_type(to_java_generic_parameter(type).get_name())
         : type;
}

void generic_parameter_specialization_mapt::erase_keys(
  std::vector<irep_idt> erase_keys)
{
  for(const auto key : erase_keys)
  {
    PRECONDITION(underlying_map.count(key) != 0);
    (*underlying_map.find(key)).second.pop();
    if((*underlying_map.find(key)).second.empty())
    {
      underlying_map.erase(key);
    }
  }
}

bool generic_parameter_specialization_mapt::add_pair(
  std::pair<java_generic_parametert, reference_typet> pair)
{
  if(
    !(is_java_generic_parameter(pair.second) &&
      to_java_generic_parameter(pair.second).get_name() ==
        pair.first.get_name()))
  {
    const irep_idt &key = pair.first.get_name();
    if(underlying_map.count(key) == 0)
      underlying_map.emplace(key, std::stack<reference_typet>());
    (*underlying_map.find(key)).second.push(pair.second);
    return true;
  }
  return false;
}
