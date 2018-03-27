/*******************************************************************\

 Module: Unit tests for converting invokedynamic instructions into codet

 Author: Diffblue Ltd.

\*******************************************************************/

#include <testing-utils/catch.hpp>
#include <testing-utils/load_java_class.h>
#include <testing-utils/require_goto_statements.h>
#include <testing-utils/require_expr.h>
#include <testing-utils/require_type.h>
#include <testing-utils/run_test_with_compilers.h>
#include <testing-utils/require_symbol.h>
#include <util/expr_iterator.h>
#include <goto-programs/class_hierarchy.h>

struct lambda_assignment_test_datat
{
  irep_idt lambda_interface;
  std::string lambda_interface_method_descriptor;
  irep_idt lambda_function_id;

  std::vector<exprt> expected_params;
  bool should_return_value = true;
};

/// Verifies for a given function that contains:
///  {lambda_variable_id} = invokedynamic method_handle_index
/// code looks
/// like:
///  tmp_object_symbol = java_new(lambda_tmp_type)
///  {lambda_variable_id} = (cast)tmp_object_symbol
/// and the type lambda_tmp_type:
///  implements {lambda_interface}
///  has method {lambda_interface_method_descriptor}
/// and the method body looks like
///  function_call_exprt(
///    should_return_value ? symbol_exprt : nil_exprt,
///    {lambda_function_id},
///    {expected_params})
/// \param symbol_table: The loaded symbol table
/// \param instructions: The instructions of the method that calls invokedynamic
/// \param test_data: The parameters for the test
void validate_lambda_assignment(
  const symbol_tablet &symbol_table,
  const std::vector<codet> &instructions,
  const lambda_assignment_test_datat &test_data,
  const require_goto_statements::pointer_assignment_locationt
  &lambda_assignment)
{
  const typecast_exprt &rhs_value = require_expr::require_typecast(
    lambda_assignment.non_null_assignments[0].rhs());

  const symbol_exprt &rhs_symbol =
    require_expr::require_symbol(rhs_value.op0());

  const irep_idt &tmp_object_symbol = rhs_symbol.get_identifier();

  const auto tmp_object_assignments =
    require_goto_statements::find_pointer_assignments(
      tmp_object_symbol, instructions);

  REQUIRE(tmp_object_assignments.non_null_assignments.size() == 1);
  REQUIRE_FALSE(tmp_object_assignments.null_assignment.has_value());

  const side_effect_exprt &side_effect_expr =
    require_expr::require_side_effect_expr(
      tmp_object_assignments.non_null_assignments[0].rhs(), ID_java_new);

  const pointer_typet &lambda_temp_type =
    require_type::require_pointer(side_effect_expr.type(), {});

  const symbol_typet &lambda_implementor_type =
    require_type::require_symbol(lambda_temp_type.subtype());

  const irep_idt &tmp_class_identifier =
    lambda_implementor_type.get_identifier();

  const symbolt &lambda_implementor_type_symbol =
    require_symbol::require_symbol_exists(symbol_table, tmp_class_identifier);

  REQUIRE(lambda_implementor_type_symbol.is_type);
  const class_typet &tmp_lambda_class_type =
    require_type::require_complete_class(lambda_implementor_type_symbol.type);

  REQUIRE(tmp_lambda_class_type.has_base(test_data.lambda_interface));
  REQUIRE(tmp_lambda_class_type.has_base("java::java.lang.Object"));

  class_hierarchyt class_hierarchy;
  class_hierarchy(symbol_table);

  const auto &parents = class_hierarchy.get_parents_trans(tmp_class_identifier);
  REQUIRE_THAT(
    parents,
    Catch::Matchers::Vector::ContainsElementMatcher<irep_idt>{
      test_data.lambda_interface});

  const auto &interface_children =
    class_hierarchy.get_children_trans(test_data.lambda_interface);

  REQUIRE_THAT(
    interface_children,
    Catch::Matchers::Vector::ContainsElementMatcher<irep_idt>{
      tmp_class_identifier});

  const java_class_typet::componentt super_class_component =
    require_type::require_component(tmp_lambda_class_type, "@java.lang.Object");

  const symbol_typet &super_class_type = require_type::require_symbol(
    super_class_component.type(), "java::java.lang.Object");

  const symbolt &base_class_symbol = require_symbol::require_symbol_exists(
    symbol_table, super_class_type.get_identifier());

  REQUIRE(base_class_symbol.is_type);
  const class_typet &super_class_type_struct =
    require_type::require_incomplete_class(base_class_symbol.type);

  require_type::require_component(super_class_type_struct, "@class_identifier");
  // TODO verify the components of the class have been set correctly

  THEN("The function in the class should call the lambda method")
  {
    const irep_idt method_identifier =
      id2string(tmp_class_identifier) +
      test_data.lambda_interface_method_descriptor;
    const symbolt &method_symbol =
      require_symbol::require_symbol_exists(symbol_table, method_identifier);

    REQUIRE(method_symbol.is_function());

    const std::vector<codet> &assignments =
      require_goto_statements::get_all_statements(method_symbol.value);

    std::vector<code_function_callt> function_calls =
      require_goto_statements::find_function_calls(
        assignments, test_data.lambda_function_id);

    INFO("Looking for function call of " << test_data.lambda_function_id);
    REQUIRE(function_calls.size() == 1);
    const code_function_callt &function_call = function_calls[0];

    std::string variable_prefix = id2string(method_identifier) + "::";
    // replace all symbol exprs with a prefixed symbol expr
    std::vector<exprt> expected_args = test_data.expected_params;
    for(exprt &arg : expected_args)
    {
      for(auto it = arg.depth_begin(); it != arg.depth_end(); ++it)
      {
        if(it->id() == ID_symbol)
        {
          symbol_exprt &symbol_expr = to_symbol_expr(it.mutate());
          const irep_idt simple_id = symbol_expr.get_identifier();
          symbol_expr.set_identifier(variable_prefix + id2string(simple_id));
        }
      }
    }

    REQUIRE_THAT(
      function_call.arguments(),
      Catch::Matchers::Vector::EqualsMatcher<exprt>{expected_args});

    if(test_data.should_return_value)
    {
      require_expr::require_symbol(function_call.lhs());
    }
    else
    {
      REQUIRE(function_call.lhs().is_nil());
    }
  }
}

/// Find the assignment to the lambda and then call validate_lamdba_assignement
/// for full validation.
void validate_local_variable_lambda_assignment(
  const symbol_tablet &symbol_table,
  const std::vector<codet> &instructions,
  const lambda_assignment_test_datat &test_data,
  const std::regex lambda_variable_id)
{
  const auto lambda_assignment =
    require_goto_statements::find_pointer_assignments(
      lambda_variable_id, instructions);

  REQUIRE(lambda_assignment.non_null_assignments.size() == 1);
  REQUIRE_FALSE(lambda_assignment.null_assignment.has_value());

  validate_lambda_assignment(
    symbol_table, instructions, test_data, lambda_assignment);
}

SCENARIO(
  "Converting invokedynamic with a local lambda",
  "[core]"
  "[lambdas][java_bytecode][java_bytecode_convert_method][!mayfail]")
{
  // NOLINTNEXTLINE(whitespace/braces)
  run_test_with_compilers([](const std::string &compiler) {
    GIVEN(
      "A class with a static lambda variables from " + compiler + " compiler.")
    {
      symbol_tablet symbol_table = load_java_class(
        "LocalLambdas",
        "./java_bytecode/java_bytecode_parse_lambdas/lambda_examples/" +
          compiler + "_classes/",
        "LocalLambdas.test");

      WHEN("Inspecting the assignments of the entry function")
      {
        const std::vector<codet> &instructions =
          require_goto_statements::get_all_statements(
            symbol_table.lookup_ref("java::LocalLambdas.test:()V").value);

        const std::string function_prefix_regex_str =
          R"(java::LocalLambdas\.test:\(\)V)";

        THEN(
          "The local variable should be assigned a temp object implementing "
          "SimpleLambda")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::SimpleLambda";
          test_data.lambda_interface_method_descriptor = ".Execute:()V";
          test_data.lambda_function_id = "java::LocalLambdas.pretendLambda:()V";
          test_data.expected_params = {};
          test_data.should_return_value = false;
          validate_local_variable_lambda_assignment(
            symbol_table,
            instructions,
            test_data,
            std::regex(function_prefix_regex_str + R"(::\d+::simpleLambda$)"));
        }
        THEN(
          "The local variable should be assigned a non-null pointer to a "
          "parameter interface implementor")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::ParameterLambda";
          test_data.lambda_interface_method_descriptor =
            ".Execute:(ILjava/lang/Object;LDummyGeneric;)V";
          test_data.lambda_function_id =
            "java::LocalLambdas.lambda$test$1:(ILjava/lang/"
            "Object;LDummyGeneric;)V";

          symbol_exprt integer_param{"primitive", java_int_type()};
          symbol_exprt ref_param{"reference",
                                 java_type_from_string("Ljava/lang/Object;")};
          symbol_exprt generic_param{
            "specalisedGeneric",
            java_type_from_string("LDummyGeneric<Ljava/lang/Interger;>;")};
          test_data.expected_params = {integer_param, ref_param, generic_param};
          test_data.should_return_value = false;

          validate_local_variable_lambda_assignment(
            symbol_table,
            instructions,
            test_data,
            std::regex(function_prefix_regex_str + R"(::\d+::paramLambda$)"));
        }
        THEN(
          "The local variable should be assigned a non-null pointer to an "
          "array parameter interface implementor")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::ArrayParameterLambda";
          test_data.lambda_interface_method_descriptor =
            ".Execute:([I[Ljava/lang/Object;[LDummyGeneric;)V";
          test_data.lambda_function_id =
            "java::LocalLambdas.lambda$test$2:"
            "[I[Ljava/lang/Object;[LDummyGeneric;)V";

          symbol_exprt integer_param{"primitive", java_type_from_string("[I")};
          symbol_exprt ref_param{"reference",
                                 java_type_from_string("[Ljava/lang/Object;")};
          symbol_exprt generic_param{
            "specalisedGeneric",
            java_type_from_string("[LDummyGeneric<Ljava/lang/Interger;>;")};
          test_data.expected_params = {integer_param, ref_param, generic_param};
          test_data.should_return_value = false;

          validate_local_variable_lambda_assignment(
            symbol_table,
            instructions,
            test_data,
            std::regex(
              function_prefix_regex_str + R"(::\d+::arrayParamLambda$)"));
        }
        THEN(
          "The local variable should be assigned a temp object implementing "
          "ReturningLambdaPrimitive")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::ReturningLambdaPrimitive";
          test_data.lambda_interface_method_descriptor = ".Execute:()I";
          test_data.lambda_function_id = "java::LocalLambdas.lambda$test$3:()I";
          test_data.expected_params = {};
          test_data.should_return_value = true;
          validate_local_variable_lambda_assignment(
            symbol_table,
            instructions,
            test_data,
            std::regex(
              function_prefix_regex_str + R"(::\d+::returnPrimitiveLambda$)"));
        }
        THEN(
          "The local variable should be assigned a temp object implementing "
          "ReturningLambdaReference")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::ReturningLambdaReference";

          test_data.lambda_interface_method_descriptor =
            ".Execute:()Ljava/lang/Object;";

          //"java::LocalLambdas.lambda$test$0:()V"
          test_data.lambda_function_id =
            "java::LocalLambdas.lambda$test$4:()Ljava/lang/Object;";
          test_data.expected_params = {};
          test_data.should_return_value = true;
          validate_local_variable_lambda_assignment(
            symbol_table,
            instructions,
            test_data,
            std::regex(
              function_prefix_regex_str + R"(::\d+::returnReferenceLambda$)"));
        }
        THEN(
          "The local variable should be assigned a temp object implementing "
          "ReturningLambdaSpecalisedGeneric")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::ReturningLambdaSpecalisedGeneric";

          test_data.lambda_interface_method_descriptor =
            ".Execute:()LDummyGeneric;";
          test_data.lambda_function_id =
            "java::LocalLambdas.lambda$test$5:()LDummyGeneric;";
          test_data.expected_params = {};
          test_data.should_return_value = true;
          validate_local_variable_lambda_assignment(
            symbol_table,
            instructions,
            test_data,
            std::regex(
              function_prefix_regex_str +
              R"(::\d+::returningSpecalisedGenericLambda$)"));
        }
        // TODO[TG-2482]: Tests for local lambdas that capture variables
      }
    }
  });
}

/// Find the assignment to the lambda in the constructor
/// and then call validate_lamdba_assignement for full validation.
void validate_member_variable_lambda_assignment(
  const symbol_tablet &symbol_table,
  const std::vector<codet> &instructions,
  const lambda_assignment_test_datat &test_data,
  const std::string lambda_variable_id)
{
  const auto lambda_assignment =
    require_goto_statements::find_this_component_assignment(
      instructions, lambda_variable_id);

  REQUIRE(lambda_assignment.non_null_assignments.size() == 1);
  REQUIRE_FALSE(lambda_assignment.null_assignment.has_value());

  validate_lambda_assignment(
    symbol_table, instructions, test_data, lambda_assignment);
}

SCENARIO(
  "Converting invokedynamic with a member lambda",
  "[core]"
  "[lamdba][java_bytecode][java_bytecode_convert_method][!mayfail]")
{
  // NOLINTNEXTLINE(whitespace/braces)
  run_test_with_compilers([](const std::string &compiler) {
    GIVEN(
      "A class with a static lambda variables from " + compiler + " compiler.")
    {
      symbol_tablet symbol_table = load_java_class(
        "MemberLambdas",
        "./java_bytecode/java_bytecode_parse_lambdas/lambda_examples/" +
          compiler + "_classes/",
        "MemberLambdas.<init>");

      WHEN("Inspecting the assignments of the entry function")
      {
        const std::vector<codet> &instructions =
          require_goto_statements::get_all_statements(
            symbol_table.lookup_ref("java::MemberLambdas.<init>:()V").value);

        const std::string function_prefix_regex_str = "java::MemberLambdas";

        THEN(
          "The local variable should be assigned a temp object implementing "
          "SimpleLambda")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::SimpleLambda";
          test_data.lambda_interface_method_descriptor = ".Execute:()V";
          test_data.lambda_function_id = "java::MemberLambdas.lambda$new$0:()V";
          test_data.expected_params = {};
          test_data.should_return_value = false;
          validate_member_variable_lambda_assignment(
            symbol_table, instructions, test_data, "simpleLambda");
        }
        THEN(
          "The local variable should be assigned a temp object implementing "
          "ParameterLambda")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::ParameterLambda";
          test_data.lambda_interface_method_descriptor =
            ".Execute:(ILjava/lang/Object;LDummyGeneric;)V";
          test_data.lambda_function_id =
            "java::MemberLambdas.lambda$new$1:"
            "(ILjava/lang/Object;LDummyGeneric;)V";

          symbol_exprt integer_param{"primitive", java_int_type()};
          symbol_exprt ref_param{"reference",
                                 java_type_from_string("Ljava/lang/Object;")};
          symbol_exprt generic_param{
            "specalisedGeneric",
            java_type_from_string("LDummyGeneric<Ljava/lang/Interger;>;")};
          test_data.expected_params = {integer_param, ref_param, generic_param};

          test_data.should_return_value = false;
          validate_member_variable_lambda_assignment(
            symbol_table, instructions, test_data, "paramLambda");
        }
        THEN(
          "The local variable should be assigned a non-null pointer to an "
          "array parameter interface implementor")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::ArrayParameterLambda";
          test_data.lambda_interface_method_descriptor =
            ".Execute:([I[Ljava/lang/Object;[LDummyGeneric;)V";
          test_data.lambda_function_id =
            "java::MemberLambdas.lambda$new$2:"
            "([I[Ljava/lang/Object;[LDummyGeneric;)V";

          symbol_exprt integer_param{"primitive", java_type_from_string("[I")};
          symbol_exprt ref_param{"reference",
                                 java_type_from_string("[Ljava/lang/Object;")};
          symbol_exprt generic_param{
            "specalisedGeneric",
            java_type_from_string("[LDummyGeneric<Ljava/lang/Interger;>;")};
          test_data.expected_params = {integer_param, ref_param, generic_param};
          test_data.should_return_value = false;

          validate_member_variable_lambda_assignment(
            symbol_table, instructions, test_data, "arrayParamLambda");
        }
        THEN(
          "The local variable should be assigned a temp object implementing "
          "ReturningLambdaPrimitive")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::ReturningLambdaPrimitive";
          test_data.lambda_interface_method_descriptor = ".Execute:()I";
          test_data.lambda_function_id = "java::MemberLambdas.lambda$new$3:()I";
          test_data.expected_params = {};
          test_data.should_return_value = true;
          validate_member_variable_lambda_assignment(
            symbol_table, instructions, test_data, "returnPrimitiveLambda");
        }
        THEN(
          "The local variable should be assigned a temp object implementing "
          "ReturningLambdaReference")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::ReturningLambdaReference";
          test_data.lambda_interface_method_descriptor =
            ".Execute:()Ljava/lang/Object;";
          test_data.lambda_function_id =
            "java::MemberLambdas.lambda$new$4:()Ljava/lang/Object;";
          test_data.expected_params = {};
          test_data.should_return_value = true;
          validate_member_variable_lambda_assignment(
            symbol_table, instructions, test_data, "returnReferenceLambda");
        }
        THEN(
          "The local variable should be assigned a temp object implementing "
          "ReturningLambdaSpecalisedGeneric")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::ReturningLambdaSpecalisedGeneric";
          test_data.lambda_interface_method_descriptor =
            ".Execute:()LDummyGeneric;";
          test_data.lambda_function_id =
            "java::MemberLambdas.lambda$new$5:()LDummyGeneric;";
          test_data.expected_params = {};
          test_data.should_return_value = true;
          validate_member_variable_lambda_assignment(
            symbol_table,
            instructions,
            test_data,
            "returningSpecalisedGenericLambda");
        }

        // TODO[TG-2486]: Tests for member lambdas that capture member variables
      }
    }
  });
}

/// Find the assignment to the lambda  in the <clinit> and then call
/// validate_lamdba_assignement for full validation.
void validate_static_member_variable_lambda_assignment(
  const symbol_tablet &symbol_table,
  const std::vector<codet> &instructions,
  const lambda_assignment_test_datat &test_data,
  const std::string static_field_name)
{
  const auto lambda_assignment =
    require_goto_statements::find_pointer_assignments(
      static_field_name, instructions);

  REQUIRE(lambda_assignment.non_null_assignments.size() == 1);
  REQUIRE_FALSE(lambda_assignment.null_assignment.has_value());

  validate_lambda_assignment(
    symbol_table, instructions, test_data, lambda_assignment);
}

SCENARIO(
  "Converting invokedynamic with a static member lambda",
  "[core]"
  "[lamdba][java_bytecode][java_bytecode_convert_method][!mayfail]")
{
  // NOLINTNEXTLINE(whitespace/braces)
  run_test_with_compilers([](const std::string &compiler) {
    GIVEN(
      "A class with a static lambda variables from " + compiler + " compiler.")
    {
      symbol_tablet symbol_table = load_java_class(
        "StaticLambdas",
        "./java_bytecode/java_bytecode_parse_lambdas/lambda_examples/" +
          compiler + "_classes/",
        "StaticLambdas.<clinit>");

      WHEN("Inspecting the assignments of the entry function")
      {
        const std::vector<codet> &instructions =
          require_goto_statements::get_all_statements(
            symbol_table.lookup_ref("java::StaticLambdas.<clinit>:()V").value);

        const std::string function_prefix_regex_str = "java::StaticLambdas";

        THEN(
          "The local variable should be assigned a temp object implementing "
          "SimpleLambda")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::SimpleLambda";
          test_data.lambda_interface_method_descriptor = ".Execute:()V";
          test_data.lambda_function_id =
            "java::StaticLambdas.lambda$static$0:()V";
          test_data.expected_params = {};
          test_data.should_return_value = false;
          validate_static_member_variable_lambda_assignment(
            symbol_table,
            instructions,
            test_data,
            function_prefix_regex_str + ".simpleLambda");
        }
        THEN(
          "The local variable should be assigned a temp object implementing "
          "ParameterLambda")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::ParameterLambda";
          test_data.lambda_interface_method_descriptor =
            ".Execute:(ILjava/lang/Object;LDummyGeneric;)V";
          test_data.lambda_function_id =
            "java::StaticLambdas.lambda$static$1:"
            "(ILjava/lang/Object;LDummyGeneric;)V";

          symbol_exprt integer_param{"primitive", java_int_type()};
          symbol_exprt ref_param{"reference",
                                 java_type_from_string("Ljava/lang/Object;")};
          symbol_exprt generic_param{
            "specalisedGeneric",
            java_type_from_string("LDummyGeneric<Ljava/lang/Interger;>;")};
          test_data.expected_params = {integer_param, ref_param, generic_param};

          test_data.should_return_value = false;
          validate_member_variable_lambda_assignment(
            symbol_table, instructions, test_data, "paramLambda");
        }
        THEN(
          "The local variable should be assigned a non-null pointer to an "
          "array parameter interface implementor")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::ArrayParameterLambda";
          test_data.lambda_interface_method_descriptor =
            ".Execute:([I[Ljava/lang/Object;[LDummyGeneric;)V";
          test_data.lambda_function_id =
            "java::StaticLambdas.lambda$static$2:"
            "([I[Ljava/lang/Object;[LDummyGeneric;)V";

          symbol_exprt integer_param{"primitive", java_type_from_string("[I")};
          symbol_exprt ref_param{"reference",
                                 java_type_from_string("[Ljava/lang/Object;")};
          symbol_exprt generic_param{
            "specalisedGeneric",
            java_type_from_string("[LDummyGeneric<Ljava/lang/Interger;>;")};
          test_data.expected_params = {integer_param, ref_param, generic_param};
          test_data.should_return_value = false;

          validate_member_variable_lambda_assignment(
            symbol_table, instructions, test_data, "arrayParamLambda");
        }
        THEN(
          "The local variable should be assigned a temp object implementing "
          "ReturningLambdaPrimitive")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::ReturningLambdaPrimitive";
          test_data.lambda_interface_method_descriptor = ".Execute:()I";
          test_data.lambda_function_id =
            "java::StaticLambdas.lambda$static$3:()I";
          test_data.expected_params = {};
          test_data.should_return_value = true;
          validate_member_variable_lambda_assignment(
            symbol_table, instructions, test_data, "returnPrimitiveLambda");
        }
        THEN(
          "The local variable should be assigned a temp object implementing "
          "ReturningLambdaReference")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::ReturningLambdaReference";
          test_data.lambda_interface_method_descriptor =
            ".Execute:()Ljava/lang/Object;";
          test_data.lambda_function_id =
            "java::StaticLambdas.lambda$static$4:()Ljava/lang/Object;";
          test_data.expected_params = {};
          test_data.should_return_value = true;
          validate_member_variable_lambda_assignment(
            symbol_table, instructions, test_data, "returnReferenceLambda");
        }
        THEN(
          "The local variable should be assigned a temp object implementing "
          "ReturningLambdaSpecalisedGeneric")
        {
          lambda_assignment_test_datat test_data;
          test_data.lambda_interface = "java::ReturningLambdaSpecalisedGeneric";
          test_data.lambda_interface_method_descriptor =
            ".Execute:()LDummyGeneric;";
          test_data.lambda_function_id =
            "java::StaticLambdas.lambda$static$5:()LDummyGeneric;";
          test_data.expected_params = {};
          test_data.should_return_value = true;
          validate_member_variable_lambda_assignment(
            symbol_table,
            instructions,
            test_data,
            "returningSpecalisedGenericLambda");
        }

        // TODO[TG-2486]: Tests for member lambdas that capture member variables
      }
    }
  });
}