/*******************************************************************\

 Module: Unit tests for creating the generic specalization map

 Author: DiffBlue Limited.

\*******************************************************************/

#include <testing-utils/catch.hpp>
#include <java_bytecode/generic_parameter_specialization_map_keys.h>
#include <java_bytecode/java_types.h>

SCENARIO(
  "Constructing a specalization map",
  "[core][java_bytecode][generic_parameter_specialization_map]")
{
  GIVEN("A map contai")
  {
    // put some K::T -> java/lang/Integer, K::U -> java/lang/String
    // then put in a new K::T -> K::U
    // then we test K::T points to java/lang/String
    // and when popped K::T -> points to java/lang/Integer
    generic_parameter_specialization_mapt map;

    const typet integer_type = java_type_from_string("Ljava/lang/Integer;");
    const typet user_type = java_type_from_string("Lpack/Foo;");
    const java_generic_typet original_pointer = to_java_generic_type(
      java_type_from_string("Lpack/Bar<TLjava/lang/Integer;TLpack/foo;>;"));

    java_generic_parametert generic_param1{"K::T", symbol_typet{"Ljava/lang/Object;"}};
    java_generic_parametert generic_param2{"K::U", symbol_typet{"Ljava/lang/Object;"}};
    java_generic_class_typet base_generic_class;
    base_generic_class.generic_types().push_back(generic_param1);
    base_generic_class.generic_types().push_back(generic_param2);

    generic_parameter_specialization_map_keyst keys_l1{map};

    keys_l1.insert_pairs_for_pointer(original_pointer, base_generic_class);

    THEN("Generic parameters should map to their inital value")
    {
      REQUIRE(map[generic_param1.get_name()].top() == integer_type);
      REQUIRE(map[generic_param2.get_name()].top() == user_type);
    }



    //keys_l1.insert_pairs_for_pointer()

  }
}
