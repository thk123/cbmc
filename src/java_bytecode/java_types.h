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

class java_generics_class_typet:public java_class_typet
{
 public:
  // uninstantiated type variables

  java_generics_class_typet()
  {
    set(ID_java_type_with_generic_type, true);
  }
};

inline bool is_java_generics_class_type(const typet &type)
{
  return type.get_bool(ID_java_type_with_generic_type);
}

inline const java_generics_class_typet &to_java_generics_class_type(
  const java_class_typet &type)
{
  PRECONDITION(is_java_generics_class_type(type));
  return static_cast<const java_generics_class_typet &>(type);
}

inline java_generics_class_typet &to_java_generics_class_type(
  java_class_typet &type)
{
  PRECONDITION(is_java_generics_class_type(type));
  return static_cast<java_generics_class_typet &>(type);
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
  const irep_idt &type_var_name;
  void set_bound(const typet &_bound)
  {
    subtype()=_bound;
  }

  java_generic_typet(const typet &_bound, const irep_idt &_type_var_name) :
    reference_typet(),
    type_var_name(_type_var_name)
  {
    set(ID_java_generic_type, true);
    // set bound
    set_bound(_bound);
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

/// class to hold instantiated type variable bound is exact, for example the
/// `java.lang.Integer` type in a `List<Integer>`
class java_inst_generic_typet:public java_generic_typet
{
public:
  java_inst_generic_typet(const reference_typet &type) :
    java_generic_typet(type, irep_idt())
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

/// class to hold type with generic type variable, for example `java.util.List`
/// in either List<Integer> or List<T>. The vector holds the types of the type
/// Variables, that is the vector has the length of the number of type variables
/// of the generic class. For example in `HashMap<K, V>` it would contain two
/// elements, each of type `java_generic_typet`, in `HashMap<Integer, V>` it
/// would contains two elements, the first of type `java_inst_generic_typet` and
/// the second of type `java_generic_typet`.
class java_type_with_generic_typet:public reference_typet
{
public:
  typedef std::vector<typet> type_parameterst;

  type_parameterst type_parameters;

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
