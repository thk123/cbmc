/*******************************************************************\

 Module: Unit tests for parsing generic classes

 Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/

#include <catch.hpp>

#include <istream>
#include <memory>

#include <util/config.h>
#include <util/language.h>
#include <util/message.h>
#include <java_bytecode/java_bytecode_language.h>

SCENARIO("java_bytecode_parse_generics",
  "[core][java_bytecode][java_bytecode_parse_generics]")
{
  std::unique_ptr<languaget>java_lang(new_java_bytecode_language());

  // Configure the path loading
  cmdlinet command_line;
  command_line.set(
    "java-cp-include-files",
    "./java_bytecode/java_bytecode_parse_generics");
  config.java.classpath.push_back(
    "./java_bytecode/java_bytecode_parse_generics");

  // Configure the language
  null_message_handlert message_handler;
  java_lang->get_language_options(command_line);
  java_lang->set_message_handler(message_handler);

  std::istringstream java_code_stream("ignored");


  GIVEN("Some class files with Generics")
  {
    WHEN("Parsing a class with type variable")
    {
      java_lang->parse(java_code_stream, "generics$element.class");

      symbol_tablet new_symbol_table;
      java_lang->typecheck(new_symbol_table, "");

      java_lang->final(new_symbol_table);

      REQUIRE(new_symbol_table.has_symbol("java::generics$element"));
      THEN("The symbol type should be generic")
      {
        const symbolt &class_symbol=new_symbol_table.lookup("java::generics$element");
        const typet &symbol_type=class_symbol.type;

        REQUIRE(symbol_type.id()==ID_struct);
        class_typet class_type=to_class_type(symbol_type);
        REQUIRE(class_type.is_class());
        java_class_typet java_class_type=to_java_class_type(class_type);
        REQUIRE(is_java_generics_class_type(java_class_type));
        java_generics_class_typet java_generics_class_type=
          to_java_generics_class_type(java_class_type);
      }
    }
  }
}
