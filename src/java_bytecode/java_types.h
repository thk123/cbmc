/*******************************************************************\

Module:

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/


#ifndef CPROVER_JAVA_BYTECODE_JAVA_TYPES_H
#define CPROVER_JAVA_BYTECODE_JAVA_TYPES_H

#include <util/invariant.h>
#include <util/type.h>
#include <util/std_types.h>


class java_class_typet:public class_typet
{
 public:
  const irep_idt &get_access() const
  {
    return get(ID_access);
  }

  void set_access(const irep_idt &access)
  {
    return set(ID_access, access);
  }
};

inline const java_class_typet &to_java_class_type(const typet &type)
{
  assert(type.id()==ID_struct);
  return static_cast<const java_class_typet &>(type);
}

inline java_class_typet &to_java_class_type(typet &type)
{
  assert(type.id()==ID_struct);
  return static_cast<java_class_typet &>(type);
}

typet java_int_type();
typet java_long_type();
typet java_short_type();
typet java_byte_type();
typet java_char_type();
typet java_float_type();
typet java_double_type();
typet java_boolean_type();
reference_typet java_reference_type(const typet &subtype);
reference_typet java_lang_object_type();
symbol_typet java_classname(const std::string &);

reference_typet java_array_type(const char subtype);

bool is_reference_type(char t);

// i  integer
// l  long
// s  short
// b  byte
// c  character
// f  float
// d  double
// z  boolean
// a  reference

typet java_type_from_char(char t);
typet java_type_from_string(const std::string &);
char java_char_from_type(const typet &type);

typet java_bytecode_promotion(const typet &);
exprt java_bytecode_promotion(const exprt &);

bool is_java_array_tag(const irep_idt& tag);

bool is_valid_java_array(const struct_typet &type);


/// class to hold a Java generic type
/// upper bound can specify type requirements
class java_generic_typet:public reference_typet
{
public:
  typet bound;

  void set_bound(const typet &_bound)
  {
    bound=_bound;
  }
  java_generic_typet() : reference_typet()
  {
    set(ID_java_generic_type, true);
    // set standard bound
    set_bound(java_type_from_string("Ljava/lang/Object;"));
  }
};

inline bool is_java_generic_type(const typet &type)
{
  return type.get_bool(ID_java_generic_type);
}

inline const java_generic_typet &to_java_generic_type(const typet &type)
{
  PRECONDITION(
    type.id()==ID_pointer &&
    is_java_generic_type(type));
  return static_cast<const java_generic_typet &>(type);
}

inline java_generic_typet &to_java_generic_type(typet &type)
{
  PRECONDITION(
    type.id()==ID_pointer &&
    is_java_generic_type(type));
  return static_cast<java_generic_typet &>(type);
}

/// class to hold instantiated type variable
/// bound is exact
class java_inst_generic_typet:public java_generic_typet
{
public:
  java_inst_generic_typet(const reference_typet &type)
  {
    set(ID_java_inst_generic_type, true);
  }
};

inline bool is_java_inst_generic_type(const typet &type)
{
  return type.get_bool(ID_java_inst_generic_type);
}

inline const java_inst_generic_typet &to_java_inst_generic_type(const typet &type)
{
  PRECONDITION(
    type.id()==ID_pointer &&
    is_java_inst_generic_type(type));
  return static_cast<const java_inst_generic_typet &>(type);
}

inline java_inst_generic_typet &to_java_inst_generic_type(typet &type)
{
  PRECONDITION(
    type.id()==ID_pointer &&
    is_java_inst_generic_type(type));
  return static_cast<java_inst_generic_typet &>(type);
}

/// class to hold type with generic type variable
class java_type_with_generic_typet:public reference_typet
{
public:
  typedef std::vector<typet> type_parameterst;

  type_parameterst type_parameters;
  java_generic_typet generic_type;

  java_type_with_generic_typet() : reference_typet()
  {
    set(ID_java_type_with_generic_type, true);
  }
};

inline bool is_java_type_with_generic_type(const typet &type)
{
  return type.get_bool(ID_java_type_with_generic_type);
}

inline const java_type_with_generic_typet &to_java_type_with_generic_type(
  const typet &type)
{
  PRECONDITION(
    type.id()==ID_pointer &&
    is_java_type_with_generic_type(type));
  return static_cast<const java_type_with_generic_typet &>(type);
}

inline java_type_with_generic_typet &to_java_type_with_generic_type(typet &type)
{
  PRECONDITION(
    type.id()==ID_pointer &&
    is_java_type_with_generic_type(type));
  return static_cast<java_type_with_generic_typet &>(type);
}

#endif // CPROVER_JAVA_BYTECODE_JAVA_TYPES_H
